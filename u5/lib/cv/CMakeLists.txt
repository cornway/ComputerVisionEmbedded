
cmake_minimum_required(VERSION 3.28)

# Set build configuration
set(CMAKE_BUILD_TYPE             Release)
set(BUILD_SHARED_LIBS            OFF)
set(CV_DISABLE_OPTIMIZATION      ON)
set(WITH_IPP                     OFF)
set(WITH_TBB                     OFF)
set(WITH_OPENMP                  OFF)
set(WITH_PTHREADS_PF             OFF)
set(WITH_QUIRC                   OFF)
set(WITH_1394                    OFF)
set(WITH_CUDA                    OFF)
set(WITH_OPENCL                  OFF)
set(WITH_OPENCLAMDFFT            OFF)
set(WITH_OPENCLAMDBLAS           OFF)
set(WITH_VA_INTEL                OFF)
set(WITH_EIGEN                   OFF)

set(WITH_GSTREAMER               OFF)
set(WITH_GTK                     OFF)
set(WITH_JASPER                  OFF)
set(BUILD_JPEG                   ON)
set(WITH_WEBP                    OFF)
set(BUILD_ZLIB                   ON)
set(BUILD_PNG                    ON)
set(BUILD_OPENEXR                OFF)
set(WITH_TIFF                    OFF)
set(WITH_V4L                     OFF)
set(WITH_LAPACK                  OFF)
set(WITH_ITT                     OFF)
set(WITH_PROTOBUF                OFF)
set(WITH_IMGCODEC_HDR            OFF)
set(WITH_IMGCODEC_SUNRASTER      OFF)
set(WITH_IMGCODEC_PXM            OFF)
set(WITH_IMGCODEC_PFM            OFF)

#set(BUILD_LIST                   core;imgproc;imgcodecs;objdetect)
#set(BUILD_LIST                   imgproc;imgcodecs)
set(BUILD_JAVA                   OFF)
set(BUILD_opencv_python          OFF)
set(BUILD_opencv_java            OFF)

set(BUILD_opencv_apps            OFF)
set(BUILD_opencv_calib3d         OFF)
set(BUILD_opencv_features2d      OFF)
set(BUILD_opencv_flann           OFF)
set(BUILD_opencv_ml              OFF) #
set(BUILD_opencv_photo           OFF) #
set(BUILD_opencv_stitching       OFF) #
set(BUILD_opencv_video           OFF) #
set(BUILD_opencv_videoio         OFF) #
set(BUILD_opencv_highgui         OFF) #
set(BUILD_opencv_imgcodecs       ON) #
set(BUILD_opencv_imgproc         ON)
set(BUILD_opencv_gapi            OFF)

set(BUILD_PACKAGE                OFF)
set(BUILD_PERF_TESTS             OFF)
set(BUILD_TESTS                  OFF)
set(CV_ENABLE_INTRINSICS         OFF)
set(CV_TRACE                     OFF)
set(OPENCV_ENABLE_MEMALIGN       OFF)
set(OPENCV_DISABLE_THREAD_SUPPORT ON)

set(OPENCV_FORCE_3RDPARTY_BUILD  ON)

set(CV_DISABLE_OPTIMIZATION       ON    CACHE BOOL   "" FORCE)
set(CPU_BASELINE_DISABLE          ALL   CACHE STRING "" FORCE)
set(CPU_DISPATCH                  ""    CACHE STRING "" FORCE)

set(ENABLE_PIC                   OFF)
set(CMAKE_SYSTEM_PROCESSOR "ARM" CACHE STRING "" FORCE)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/toolchain.cmake)


project(opencv_wrapper)

add_subdirectory(opencv)