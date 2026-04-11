#include "findclouds.h"
#include <iostream>
#include <opencv2/imgproc.hpp>

FindClouds::FindClouds(int minBrightness, int maxSaturation)
    : brightnessThreshold_(minBrightness)
    , saturationThreshold_(maxSaturation)
    , coveragePercent_(0.0)
{}

void FindClouds::process(const cv::Mat &inputImage)
{
    if (inputImage.empty()) {
        mask_.release();
        visual_.release();
        coveragePercent_ = 0.0;
        return;
    }

    // Преобразование BGR в HSV
    cv::Mat hsv;
    cv::cvtColor(inputImage, hsv, cv::COLOR_BGR2HSV);

    computeMask(hsv);
    computeVisual(inputImage);
}

cv::Mat FindClouds::getCloudMask() const
{
    return mask_.clone();
}

cv::Mat FindClouds::getVisualization() const
{
    return visual_.clone();
}

double FindClouds::getCloudCoveragePercent() const
{
    return coveragePercent_;
}

void FindClouds::setThresholds(int minBrightness, int maxSaturation)
{
    brightnessThreshold_ = minBrightness;
    saturationThreshold_ = maxSaturation;
}

void FindClouds::computeMask(const cv::Mat &hsvImage)
{
    // Разделяем каналы HSV
    std::vector<cv::Mat> channels;
    cv::split(hsvImage, channels);
    cv::Mat saturation = channels[1];
    cv::Mat value = channels[2];

    // Условие: яркость (Value) > brightnessThreshold_ И насыщенность (Saturation) < saturationThreshold_
    cv::Mat brightCondition, satCondition;
    cv::threshold(value, brightCondition, brightnessThreshold_, 255, cv::THRESH_BINARY);
    cv::threshold(saturation, satCondition, saturationThreshold_, 255, cv::THRESH_BINARY_INV);

    // Объединяем условия (логическое И)
    cv::Mat cloudMask;
    cv::bitwise_and(brightCondition, satCondition, cloudMask);

    // Морфологическое закрытие для заполнения дырок и удаления шума
    int morphSize = 5;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(morphSize, morphSize));
    cv::morphologyEx(cloudMask, cloudMask, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(cloudMask, cloudMask, cv::MORPH_OPEN, kernel);

    mask_ = cloudMask;

    // Вычисление процента покрытия
    int totalPixels = mask_.total();
    int cloudPixels = cv::countNonZero(mask_);
    coveragePercent_ = (totalPixels > 0) ? (100.0 * cloudPixels / totalPixels) : 0.0;
}

void FindClouds::computeVisual(const cv::Mat &original)
{
    visual_ = original.clone();
    // Находим контуры облаков
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask_, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Рисуем контуры зелёным цветом
    cv::drawContours(visual_, contours, -1, cv::Scalar(0, 255, 0), 2);

    // Добавляем текст с процентом покрытия
    std::string text = "Cloud coverage: " + std::to_string(static_cast<int>(coveragePercent_))
                       + "%";
    cv::putText(visual_,
                text,
                cv::Point(10, 30),
                cv::FONT_HERSHEY_SIMPLEX,
                0.7,
                cv::Scalar(0, 0, 255),
                2);
}