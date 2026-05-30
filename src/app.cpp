#include <App.h>
#include <Recognizer.h>
#include <Sendler.h>
#include <opencv2/opencv.hpp>
#include <iostream>

static double now() {
    return static_cast<double>(cv::getTickCount()) / cv::getTickFrequency();
}

int App::run() {
    sendler->send("stop");

    cv::VideoCapture cap(useCamera ? "0" : path);
    if (!cap.isOpened()) {
        std::cerr << "Не удалось открыть источник: " << (useCamera ? "камера" : path) << '\n';
        return -1;
    }

    double fps = cap.get(cv::CAP_PROP_FPS);
    if (fps <= 0.0 || fps > 120.0) fps = 25.0;
    const int delayMs = static_cast<int>(1000.0 / fps);

    std::cout << "FPS: " << fps << "  задержка: " << delayMs << " мс\n"
              << "ESC — выход\n";

    cv::Mat frame;
    bool engineOn       = false;
    double lastAnalysis = now();
    const double kAnalysisInterval = 0.133;

    for (;;) {
        if (!cap.read(frame) || frame.empty()) {
            if (useCamera) break;
            cap.set(cv::CAP_PROP_POS_FRAMES, 0);  // зациклить файл
            continue;
        }

        cv::Mat display = frame.clone();
        const double t  = now();

        if (t - lastAnalysis >= kAnalysisInterval) {
            lastAnalysis = t;
            recogniser->recognize(frame);

            const bool willCover = recogniser->isSunCoveragePredicted();
            const bool isCovered = recogniser->isSunCovered();
            const bool danger    = willCover || isCovered;

            if (danger && !engineOn) {
                std::cout << "START: " << sendler->send("start") << '\n';
                engineOn = true;
            } else if (!danger && engineOn) {
                std::cout << "STOP:  " << sendler->send("stop") << '\n';
                engineOn = false;
            }

            for (const auto& box : recogniser->getCloudBoxes())
                cv::rectangle(display, box, cv::Scalar(0, 220, 0), 2);

            const cv::Rect sun = recogniser->getSunBox();
            if (!sun.empty())
                cv::rectangle(display, sun, cv::Scalar(0, 0, 255), 3);
        }

        cv::imshow("Tracker", display);

        if (cv::waitKey(delayMs) == 27) {
            sendler->send("stop");
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
