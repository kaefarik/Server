#ifndef SUNDETECTOR_H
#define SUNDETECTOR_H

#include <opencv2/opencv.hpp>

class SunDetector {
public:
    SunDetector();
    cv::Rect detect(const cv::Mat& frame);
    void setBoxSize(int size); // размер квадрата вокруг солнца

private:
    int boxSize; // сторона квадрата в пикселях
};

#endif