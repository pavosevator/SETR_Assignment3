cmake_minimum_required(VERSION 3.21.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Temperature_measurement)

target_sources(app PRIVATE src/main.c) 

target_include_directories(app PRIVATE src/I2C)
target_include_directories(app PRIVATE src/CmdProc)
target_include_directories(app PRIVATE src/data)
target_include_directories(app PRIVATE src/UART)
target_include_directories(app PRIVATE src/GPIO)

target_sources(app PRIVATE src/I2C/i2c.c)
target_sources(app PRIVATE src/CmdProc/cmdproc.c)
target_sources(app PRIVATE src/data/data.c)
target_sources(app PRIVATE src/UART/uart.c)
target_sources(app PRIVATE src/GPIO/gpio.c)