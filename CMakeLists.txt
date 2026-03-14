cmake_minimum_required(VERSION 3.16)

project(Server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(Server main.cpp
    Engine.h
    Receiver.h
    FooEngine.cpp
    FooReceiver.cpp

)
