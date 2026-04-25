cmake_minimum_required(VERSION 3.10)
project(MyOpenCVProject)

set(CMAKE_CXX_STANDARD 17)

set(OpenCV_DIR "C:/Users/user/opencv/opencv/build/install")

find_package(OpenCV REQUIRED)

add_executable(my_app 
    sundetector.cpp
    sundetector.h 
    main.cpp
)

target_link_libraries(my_app ${OpenCV_LIBS})