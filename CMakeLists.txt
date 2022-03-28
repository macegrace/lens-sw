cmake_minimum_required(VERSION 3.10)

project(AUTOFOCUS_Test)

set(AUTOFOCUS AutofocusSweep)
set(OpenCV_DIR /installation/OpenCV-/lib/cmake/opencv4)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(OpenCV REQUIRED)

set(SOURCES_AUTOFOCUS
    src/Maxim14574.cpp
    src/SerialPort.cpp
    src/main.cpp
    src/camControl.cpp
)

add_executable(${AUTOFOCUS} ${SOURCES_AUTOFOCUS})

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(${AUTOFOCUS} PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(${AUTOFOCUS} "${CMAKE_THREAD_LIBS_INIT}")
endif()

include_directories(include)
target_link_libraries(${AUTOFOCUS} ${OpenCV_LIBS})

target_link_libraries(${AUTOFOCUS} /opt/ids/ueye/lib/libueye_api.so)
