cmake_minimum_required(VERSION 3.16)

project(Robot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(Robot main.cpp
    Engine.h
    Receiver.h
    FooEngine.cpp
    FooReceiver.cpp
    findclouds.h findclouds.cpp
)

target_link_libraries(Robot ${OpenCV_LIBS})
