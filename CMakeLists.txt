cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Temperature_measurement)

zephyr_include_directories(include)

add_subdirectory(Button_LED)
add_subdirectory(I2C)
add_subdirectory(UART)
add_subdirectory(CmdProc)
add_subdirectory(Unity)

add_subdirectory(src)