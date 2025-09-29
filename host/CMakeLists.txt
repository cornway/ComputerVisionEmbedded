cmake_minimum_required(VERSION 3.10)
project(host_app)

# Find OpenCV4
find_package(OpenCV 4 REQUIRED)

set(OPENCV_HAAR_PATH "/usr/share/opencv4/")
#set(OPENCV_HAAR_PATH "${CMAKE_SOURCE_DIR}/Haar")

set(HAAR_HEADER "${CMAKE_BINARY_DIR}/Include/HaarPath.hpp")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Include")

configure_file(
    ${CMAKE_SOURCE_DIR}/HaarPath.hpp.in
    ${HAAR_HEADER}
    @ONLY
)

set(OPENCV_UTILS_DIR_PATH "../u5/app/src")

set(HOST_APP_SRCS
    Src/main.cpp
    ${OPENCV_UTILS_DIR_PATH}/opencv_utils.cpp
    ${OPENCV_UTILS_DIR_PATH}/opencv_utils.hpp
)

# Add executable
add_executable(host_app ${HOST_APP_SRCS})

# Link OpenCV libraries
target_include_directories(host_app
    PRIVATE "${CMAKE_BINARY_DIR}/Include"
    PRIVATE ${OpenCV_INCLUDE_DIRS}
    PRIVATE ${OPENCV_UTILS_DIR_PATH})

target_link_libraries(host_app PRIVATE ${OpenCV_LIBS})
