cmake_minimum_required(VERSION 3.15)
project(server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Declare the external library
FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.41.0
  GIT_SHALLOW    TRUE
)
#   GIT_TAG        v0.40 # Use a specific tag, commit hash, or branch

FetchContent_Declare(
  json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz

)
FetchContent_MakeAvailable(httplib json)

add_executable(server server.cpp client.cpp)

target_link_libraries(server PRIVATE httplib nlohmann_json::nlohmann_json)