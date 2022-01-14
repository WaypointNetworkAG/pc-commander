cmake_minimum_required(VERSION 3.21)
project(Test)

set(CMAKE_CXX_STANDARD 14)

add_library(
        serial
        src/serial/serialib.cpp
)

target_include_directories(
        serial
        PUBLIC
        include/serial
)

add_library(
        base64
        src/base64/base64.cpp
)

target_include_directories(
        base64
        PUBLIC
        include/base64
)

add_library(
        arduino_serial
        src/arduino_serial.cpp
)

target_include_directories(
        arduino_serial
        PUBLIC
        include/arduino_serial
        include/crc
        include/cppcodec
        include/base64
)

target_link_libraries(
        arduino_serial
        serial
        base64
)

add_executable(main
        src/main.cpp)

target_link_libraries(
        main
        PUBLIC
        arduino_serial
)
