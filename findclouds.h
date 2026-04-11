#pragma once
#include <opencv2/opencv.hpp>

class FindClouds
{
public:
    // Конструктор с настройками порогов (опционально)
    FindClouds(int minBrightness = 200, int maxSaturation = 50);

    // Основной метод: принимает BGR изображение, ищет облака
    void process(const cv::Mat &inputImage);

    // Получить бинарную маску облаков (белое - облака, чёрное - фон)
    cv::Mat getCloudMask() const;

    // Получить изображение с выделенными облаками (контуры или overlay)
    cv::Mat getVisualization() const;

    // Получить процент покрытия облаками (0..100)
    double getCloudCoveragePercent() const;

    // Установить пороги (яркость, насыщенность)
    void setThresholds(int minBrightness, int maxSaturation);

private:
    cv::Mat mask_;   // бинарная маска облаков
    cv::Mat visual_; // визуализация (исходное + контуры)
    double coveragePercent_;

    int brightnessThreshold_; // минимальная яркость (0-255)
    int saturationThreshold_; // максимальная насыщенность (0-255)

    void computeMask(const cv::Mat &hsvImage);
    void computeVisual(const cv::Mat &original);
};