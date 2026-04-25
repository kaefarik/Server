#include "sundetector.h"

SunDetector::SunDetector() {
    boxSize = 100; // по умолчанию 100x100 пикселей
}

void SunDetector::setBoxSize(int size) {
    boxSize = size;
}

cv::Rect SunDetector::detect(const cv::Mat& frame) {
    if (frame.empty()) return cv::Rect();

   
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    // Находим координаты самого яркого пикселя
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(gray, &minVal, &maxVal, &minLoc, &maxLoc);

    // Рисуем квадрат вокруг самого яркого пикселя
    int half = boxSize / 2;
    int x = maxLoc.x - half;
    int y = maxLoc.y - half;

    // Проверяем, чтобы квадрат не выходил за границы изображения
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + boxSize > frame.cols) x = frame.cols - boxSize;
    if (y + boxSize > frame.rows) y = frame.rows - boxSize;

    return cv::Rect(x, y, boxSize, boxSize);
}