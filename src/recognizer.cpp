#include <Recognizer.h>
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <cmath>

// ─── helpers ────────────────────────────────────────────────────────────────

static float iou(const cv::Rect& a, const cv::Rect& b) {
    const cv::Rect inter = a & b;
    if (inter.area() <= 0) return 0.f;
    return static_cast<float>(inter.area()) / (a.area() + b.area() - inter.area());
}

static cv::KalmanFilter makeKF(float cx, float cy, float w, float h, float dt) {
    cv::KalmanFilter kf(6, 4, 0);
    kf.transitionMatrix = (cv::Mat_<float>(6,6) <<
        1, 0, dt, 0,  0, 0,
        0, 1,  0, dt, 0, 0,
        0, 0,  1,  0, 0, 0,
        0, 0,  0,  1, 0, 0,
        0, 0,  0,  0, 1, 0,
        0, 0,  0,  0, 0, 1);
    kf.measurementMatrix = (cv::Mat_<float>(4,6) <<
        1,0,0,0,0,0,
        0,1,0,0,0,0,
        0,0,0,0,1,0,
        0,0,0,0,0,1);
    cv::setIdentity(kf.processNoiseCov,    cv::Scalar::all(1e-2));
    cv::setIdentity(kf.measurementNoiseCov, cv::Scalar::all(1e-1));
    cv::setIdentity(kf.errorCovPost,        cv::Scalar::all(1));
    kf.statePost = (cv::Mat_<float>(6,1) << cx, cy, 0, 0, w, h);
    return kf;
}

static cv::Rect stateToRect(const cv::Mat& s) {
    float x = s.at<float>(0), y = s.at<float>(1);
    float w = s.at<float>(4), h = s.at<float>(5);
    return cv::Rect(cvRound(x - w/2), cvRound(y - h/2), cvRound(w), cvRound(h));
}

// ─── recognize ──────────────────────────────────────────────────────────────

void FooRecognizer::recognize(cv::Mat image) {
    cloud_boxes.clear();
    sun_box = cv::Rect();

    if (image.empty()) {
        std::cerr << "recognize: пустой кадр\n";
        return;
    }

    // HSV + маска светлых областей (облака/солнце)
    cv::Mat hsv;
    cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask;
    cv::inRange(hsv, cv::Scalar(0, 0, 180), cv::Scalar(180, 60, 255), mask);
    cv::GaussianBlur(mask, mask, cv::Size(5,5), 0);
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, cv::Mat(), cv::Point(-1,-1), 2);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Канал яркости для выбора солнца
    cv::Mat chanV;
    cv::extractChannel(hsv, chanV, 2);

    const cv::Point imgCenter(image.cols / 2, image.rows / 2);
    const double maxSunDist = std::min(image.cols, image.rows) * 0.25;
    const float  maxSunSide = std::min(image.cols, image.rows) * 0.6f;

    double  bestBrightness = -1.0;
    cv::Rect bestSunBox;

    for (const auto& cnt : contours) {
        if (cv::contourArea(cnt) < 1000.0) continue;

        cv::Rect box = cv::boundingRect(cnt);
        cloud_boxes.push_back(box);

        // Кандидат в солнце: близко к центру и не слишком большой
        cv::Point center(box.x + box.width/2, box.y + box.height/2);
        if (cv::norm(center - imgCenter) > maxSunDist) continue;
        if (box.width > maxSunSide || box.height > maxSunSide) continue;

        cv::Rect safe = box & cv::Rect(0, 0, chanV.cols, chanV.rows);
        if (safe.area() <= 0) continue;

        double brightness = cv::mean(chanV(safe))[0];
        if (brightness > bestBrightness) {
            bestBrightness = brightness;
            bestSunBox     = box;
        }
    }

    sun_box  = bestSunBox;
    sunCover = sun_box.empty();

    // ── Kalman tracking ──────────────────────────────────────────────────────

    const double curTime = static_cast<double>(cv::getTickCount()) / cv::getTickFrequency();
    const float  dt      = (lastTime > 0.0) ? static_cast<float>(curTime - lastTime) : 1.f;
    lastTime = curTime;

    const size_t nOld = tracks.size();

    // Предсказание
    for (auto& tr : tracks) {
        if (!tr.isInitialized) continue;
        tr.kf.transitionMatrix.at<float>(0,2) = dt;
        tr.kf.transitionMatrix.at<float>(1,3) = dt;
        tr.predictedBox = stateToRect(tr.kf.predict());
    }

    // Жадная ассоциация по IoU
    const float kIoUThr = 0.3f;
    std::vector<bool> detUsed(cloud_boxes.size(), false);
    std::vector<bool> trkUsed(nOld, false);
    std::vector<int>  detToTrk(cloud_boxes.size(), -1);

    for (size_t d = 0; d < cloud_boxes.size(); ++d) {
        float best = kIoUThr;
        int   idx  = -1;
        for (size_t t = 0; t < nOld; ++t) {
            if (trkUsed[t] || !tracks[t].isInitialized) continue;
            float score = iou(tracks[t].predictedBox, cloud_boxes[d]);
            if (score > best) { best = score; idx = static_cast<int>(t); }
        }
        if (idx >= 0) { detToTrk[d] = idx; trkUsed[idx] = true; detUsed[d] = true; }
    }

    // Обновление совпавших треков
    for (size_t d = 0; d < cloud_boxes.size(); ++d) {
        if (!detUsed[d]) continue;
        int t = detToTrk[d];
        const cv::Rect& det = cloud_boxes[d];
        float mx = det.x + det.width  / 2.f;
        float my = det.y + det.height / 2.f;
        cv::Mat meas = (cv::Mat_<float>(4,1) << mx, my,
                        static_cast<float>(det.width), static_cast<float>(det.height));
        tracks[t].kf.correct(meas);
        tracks[t].age++;
        tracks[t].missedFrames = 0;
        tracks[t].updatedBox   = stateToRect(tracks[t].kf.statePost);
    }

    // Новые треки для несопоставленных детекций
    for (size_t d = 0; d < cloud_boxes.size(); ++d) {
        if (detUsed[d]) continue;
        const cv::Rect& det = cloud_boxes[d];
        TrackedCloud tr;
        tr.id           = nextTrackId++;
        tr.age          = 1;
        tr.missedFrames = 0;
        tr.isInitialized = true;
        tr.kf           = makeKF(det.x + det.width/2.f, det.y + det.height/2.f,
                                  static_cast<float>(det.width),
                                  static_cast<float>(det.height), dt);
        tr.predictedBox = det;
        tr.updatedBox   = det;
        tracks.push_back(std::move(tr));
    }

    // Счётчик пропусков + удаление старых треков
    for (size_t t = 0; t < nOld; ++t)
        if (!trkUsed[t] && tracks[t].isInitialized)
            tracks[t].missedFrames++;

    const int kMaxMiss = 5;
    tracks.erase(std::remove_if(tracks.begin(), tracks.end(),
        [kMaxMiss](const TrackedCloud& tr) {
            return !tr.isInitialized || tr.missedFrames >= kMaxMiss;
        }), tracks.end());

    // ── Прогноз перекрытия солнца ────────────────────────────────────────────

    sunCoveragePredicted = false;
    timeToCoverage       = -1.0;
    coveringCloudId      = -1;

    if (sun_box.empty()) return;

    const float sunCx = sun_box.x + sun_box.width  / 2.f;
    const float sunCy = sun_box.y + sun_box.height / 2.f;
    const float sunR  = std::max(sun_box.width, sun_box.height) / 2.f;
    const float sunArea = static_cast<float>(sun_box.width * sun_box.height);

    const int   kMinAge    = 4;
    const float kHorizon   = 3.0f;
    float       closestT   = kHorizon + 0.1f;

    for (const auto& tr : tracks) {
        if (!tr.isInitialized || tr.age < kMinAge) continue;

        const cv::Mat& s = tr.kf.statePost;
        float cx = s.at<float>(0), cy = s.at<float>(1);
        float vx = s.at<float>(2), vy = s.at<float>(3);
        float cr = std::max(tr.updatedBox.width, tr.updatedBox.height) / 2.f;

        float v2 = vx*vx + vy*vy;
        if (v2 < 1e-6f) continue;

        float dx = cx - sunCx, dy = cy - sunCy;
        float t  = -(dx*vx + dy*vy) / v2;
        if (t <= 0.f || t > kHorizon) continue;

        float dxt = dx + vx*t, dyt = dy + vy*t;
        float dist = std::sqrt(dxt*dxt + dyt*dyt);

        const float cloudArea = static_cast<float>(tr.updatedBox.width * tr.updatedBox.height);
        if (dist < cr + sunR && t < closestT && 1.4f * sunArea < cloudArea) {
            closestT        = t;
            coveringCloudId = tr.id;
        }
    }

    if (closestT <= kHorizon) {
        sunCoveragePredicted = true;
        timeToCoverage       = closestT;
    }
}

bool   FooRecognizer::isSunCoveragePredicted() const { return sunCoveragePredicted; }
bool   FooRecognizer::isSunCovered()           const { return sunCover; }
double FooRecognizer::getTimeToCoverage()      const { return timeToCoverage; }
int    FooRecognizer::getCoveringCloudId()     const { return coveringCloudId; }
