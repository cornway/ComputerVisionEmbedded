
zephyr_get_compile_definitions_for_lang_as_string(       C definitions)
zephyr_get_compile_options_for_lang_as_string(           C options)

zephyr_get_compile_definitions_for_lang_as_string(       CXX definitions)
zephyr_get_compile_options_for_lang_as_string(           CXX options)

set(OPENCV_FORCE_3RDPARTY_BUILD OFF)
set(USE_ZLIB OFF)
if (CONFIG_OPENCV_FORCE_3RDPARTY_BUILD)
    set(OPENCV_FORCE_3RDPARTY_BUILD ON)
    set(USE_ZLIB ON)
endif()

set(external_project_cflags
  "${definitions} ${options}"
)

set(external_project_cxxflags
  "${definitions} ${options}"
)

file(GENERATE OUTPUT ${CMAKE_CURRENT_LIST_DIR}/toolchain.cmake CONTENT
"
set(CMAKE_C_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER})
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})

set(CMAKE_C_FLAGS \"${external_project_cflags} -DOPENCV_FLANN_USE_STD_RAND\" CACHE STRING \"C Compiler Base Flags\")
set(CMAKE_CXX_FLAGS \"${external_project_cxxflags} -DOPENCV_FLANN_USE_STD_RAND\" CACHE STRING \"CXX Compiler Base Flags\")
")

set(OPENCV_BINARY_DIR ${CMAKE_BINARY_DIR}/opencv)
set(OPENCV_INSTALL_DIR ${OPENCV_BINARY_DIR}/install CACHE INTERNAL "")

add_custom_target(opencv_cmake
    DEPENDS ${OPENCV_BINARY_DIR}/CMakeCache.txt
)

add_custom_command(
    OUTPUT ${OPENCV_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND}
        -G${CMAKE_GENERATOR}
        -DCMAKE_BUILD_TYPE=Release
        -DBUILD_SHARED_LIBS=OFF
        -DCV_DISABLE_OPTIMIZATION=ON
        -DWITH_IPP=OFF
        -DWITH_TBB=OFF
        -DWITH_OPENMP=OFF
        -DWITH_PTHREADS_PF=OFF
        -DWITH_QUIRC=OFF
        -DWITH_1394=OFF
        -DWITH_CUDA=OFF
        -DWITH_OPENCL=OFF
        -DWITH_OPENCLAMDFFT=OFF
        -DWITH_OPENCLAMDBLAS=OFF
        -DWITH_VA_INTEL=OFF
        -DWITH_EIGEN=OFF
        -DWITH_GSTREAMER=OFF
        -DWITH_GTK=OFF
        -DWITH_JASPER=OFF
        -DBUILD_JPEG=OFF
        -DWITH_WEBP=OFF
        -DBUILD_ZLIB=${USE_ZLIB}
        -DBUILD_PNG=OFF
        -DBUILD_OPENEXR=OFF
        -DWITH_TIFF=OFF
        -DWITH_V4L=OFF
        -DWITH_LAPACK=OFF
        -DWITH_ITT=OFF
        -DWITH_PROTOBUF=OFF
        -DWITH_IMGCODEC_HDR=OFF
        -DWITH_IMGCODEC_SUNRASTER=OFF
        -DWITH_IMGCODEC_PXM=OFF
        -DWITH_IMGCODEC_PFM=OFF
        -DBUILD_JAVA=OFF
        -DBUILD_opencv_python=OFF
        -DBUILD_opencv_java=OFF
        -DBUILD_opencv_ml=OFF
        -DBUILD_opencv_photo=OFF
        -DBUILD_opencv_stitching=OFF
        -DBUILD_opencv_video=OFF
        -DBUILD_opencv_videoio=OFF
        -DBUILD_opencv_highgui=OFF
        -DBUILD_opencv_imgcodecs=ON
        -DBUILD_opencv_imgproc=ON
        -DBUILD_opencv_objdetect=ON
        -DBUILD_opencv_gapi=OFF
        -DBUILD_opencv_apps=OFF
        -DBUILD_PACKAGE=OFF
        -DBUILD_PERF_TESTS=OFF
        -DBUILD_TESTS=OFF
        -DCV_ENABLE_INTRINSICS=OFF
        -DCV_TRACE=OFF
        -DOPENCV_ENABLE_MEMALIGN=OFF
        -DOPENCV_DISABLE_THREAD_SUPPORT=ON
        -DOPENCV_FORCE_3RDPARTY_BUILD=${OPENCV_FORCE_3RDPARTY_BUILD}
        -DENABLE_VFPV3=OFF
        -DCPU_BASELINE_DISABLE=ALL
        -DCPU_DISPATCH=""
        -DENABLE_PIC=OFF
        -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY
        -DCMAKE_INSTALL_PREFIX=${OPENCV_INSTALL_DIR}
        -DCMAKE_SYSTEM_NAME=Generic
        -DCMAKE_SYSTEM_PROCESSOR=arm
        -DCMAKE_CROSSCOMPILING=ON
        -DOPENCV_WORKAROUND_CMAKE_20989=ON
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/toolchain.cmake
        -DZEPHYR_CUSTOM_BUILD=ON
        ${CMAKE_CURRENT_LIST_DIR}/opencv
    WORKING_DIRECTORY ${OPENCV_BINARY_DIR}
    COMMAND_EXPAND_LISTS
)

set(BUILD_BYPRODUCTS
    ${OPENCV_INSTALL_DIR}/lib/libopencv_imgcodecs.a
    ${OPENCV_INSTALL_DIR}/lib/libopencv_imgproc.a
    ${OPENCV_INSTALL_DIR}/lib/libopencv_calib3d.a
    ${OPENCV_INSTALL_DIR}/lib/libopencv_features2d.a
    ${OPENCV_INSTALL_DIR}/lib/libopencv_flann.a
    ${OPENCV_INSTALL_DIR}/lib/libopencv_objdetect.a
    ${OPENCV_INSTALL_DIR}/lib/libopencv_core.a
)

if (OPENCV_FORCE_3RDPARTY_BUILD)
set(BUILD_BYPRODUCTS
    ${BUILD_BYPRODUCTS}
    ${OPENCV_INSTALL_DIR}/lib/opencv4/3rdparty/liblibpng.a
    ${OPENCV_INSTALL_DIR}/lib/opencv4/3rdparty/liblibopenjp2.a
    ${OPENCV_INSTALL_DIR}/lib/opencv4/3rdparty/libzlib.a
    ${OPENCV_INSTALL_DIR}/lib/opencv4/3rdparty/liblibjpeg-turbo.a
)
endif()

include(ExternalProject)

ExternalProject_Add(
    opencv
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/opencv
    BINARY_DIR ${OPENCV_BINARY_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${CMAKE_COMMAND} --build . ${PARALLEL_JOBS}
    INSTALL_COMMAND ${CMAKE_COMMAND} --install .
    BUILD_ALWAYS True
    USES_TERMINAL_BUILD True
    DEPENDS opencv_cmake
    BUILD_BYPRODUCTS ${BUILD_BYPRODUCTS}
)

function(opencv_add_lib LIBNAME)
    add_library(${LIBNAME} STATIC IMPORTED GLOBAL)
    set_target_properties(${LIBNAME} PROPERTIES IMPORTED_LOCATION ${OPENCV_INSTALL_DIR}/lib/${LIBNAME}.a)
    add_dependencies(${LIBNAME} opencv)
endfunction()

function(opencv_add_lib_3p LIBNAME)
    add_library(${LIBNAME} STATIC IMPORTED GLOBAL)
    set_target_properties(${LIBNAME} PROPERTIES IMPORTED_LOCATION ${OPENCV_INSTALL_DIR}/lib/opencv4/3rdparty/${LIBNAME}.a)
    add_dependencies(${LIBNAME} opencv)
endfunction()


opencv_add_lib(libopencv_imgcodecs)
opencv_add_lib(libopencv_imgproc)
opencv_add_lib(libopencv_calib3d)
opencv_add_lib(libopencv_features2d)
opencv_add_lib(libopencv_flann)
opencv_add_lib(libopencv_objdetect)
opencv_add_lib(libopencv_core)

if (OPENCV_FORCE_3RDPARTY_BUILD)
opencv_add_lib_3p(liblibpng)
opencv_add_lib_3p(liblibopenjp2)
opencv_add_lib_3p(libzlib)
opencv_add_lib_3p(liblibjpeg-turbo)
endif()

add_library(opencv_lib INTERFACE)
target_link_libraries(opencv_lib INTERFACE
libopencv_imgcodecs
libopencv_objdetect
libopencv_calib3d
libopencv_flann
libopencv_features2d
libopencv_imgproc
libopencv_core
)

if (OPENCV_FORCE_3RDPARTY_BUILD)
target_link_libraries(opencv_lib INTERFACE
liblibpng
liblibopenjp2
libzlib
liblibjpeg-turbo
)
endif()

set_target_properties(opencv_lib PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${OPENCV_INSTALL_DIR}/include/opencv4)