#include "Engine.h"
#include "Receiver.h"
#include "findclouds.h"
#include <iostream>

using namespace std;

int main()
{
    AReceiver *ptr = new FooReceiver;
    ptr->receive();

    // Загружаем изображение
    cv::Mat image = cv::imread("/home/kaefarik/cpp/server/docs/sky.jpg");
    if (image.empty()) {
        std::cerr << "Cannot load image" << std::endl;
        return -1;
    }

    // Используем существующий конструктор с параметрами
    FindClouds *cloud = new FindClouds(200, 50);
    cloud->process(image); // ← метод process(), не find()

    // Получаем результаты
    cv::Mat mask = cloud->getCloudMask();
    cv::Mat visual = cloud->getVisualization();
    std::cout << "Cloud coverage: " << cloud->getCloudCoveragePercent() << "%" << std::endl;

    // Показываем результаты
    cv::imshow("Original", image);
    cv::imshow("Cloud Mask", mask);
    cv::imshow("Detection", visual);
    cv::waitKey(0);

    delete ptr;
    delete cloud;

    return 0;
}