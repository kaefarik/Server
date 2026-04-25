#include "sundetector.h"
#include <iostream>

int main(int argc, char** argv) {
    std::string filename = "sun.jpg";
    if (argc >= 2) filename = argv[1];

    cv::Mat image = cv::imread(filename);
    if (image.empty()) {
        std::cerr << "Ошибка: не удалось загрузить " << filename << std::endl;
        return -1;
    }

    // Если изображение слишком большое, уменьшаем для отображения
    cv::Mat display;
    if (image.cols > 1000) {
        double scale = 1000.0 / image.cols;
        cv::resize(image, display, cv::Size(), scale, scale);
    } else {
        display = image.clone();
    }

    SunDetector detector;
    // Если солнце на фото маленькое, уменьшите размер квадрата (например, 50)
    detector.setBoxSize(120);

    cv::Rect sunRect = detector.detect(display);

    // Рисуем результат
    if (!sunRect.empty()) {
        cv::rectangle(display, sunRect, cv::Scalar(0, 0, 255), 3);
        cv::putText(display, "SUN", cv::Point(sunRect.x, sunRect.y - 5), 
                    cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);
        std::cout << "Солнце найдено в центре: (" << sunRect.x + sunRect.width/2 
                  << ", " << sunRect.y + sunRect.height/2 << ")" << std::endl;
    }

    cv::imshow("Sun Detector", display);
    cv::imwrite("output.jpg", display);
    std::cout << "Нажмите любую клавишу для выхода..." << std::endl;
    cv::waitKey(0);

    return 0;
}