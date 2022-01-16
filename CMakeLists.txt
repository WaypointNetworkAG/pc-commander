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
        arduino_serial
        src/arduino_serial/arduino_serial.cpp
        src/arduino_serial/serial_send.cpp
        src/arduino_serial/serial_base64.cpp
        src/arduino_serial/serial_crc.cpp
        src/windows_actions/windows_actions.cpp
        src/arduino_serial/serial_update.cpp)

target_include_directories(
        arduino_serial
        PUBLIC
        include/arduino_serial
        include/crc
        include/cppcodec
        include/base64
        include/windows_actions
)

target_link_libraries(
        arduino_serial
        serial
)

add_executable(PCCommander
        src/main.cpp)

target_link_libraries(
        PCCommander
        PUBLIC
        arduino_serial
)
