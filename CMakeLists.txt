cmake_minimum_required(VERSION 3.21.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Temperature_measurement)

set(UNIT_TESTING ON)  # Flip this to OFF when building the full app

# Common source folders
target_include_directories(app PRIVATE
    src/I2C
    src/CmdProc
    src/data
    src/UART
    src/GPIO
    src/Heater
)

target_sources(app PRIVATE
    src/I2C/i2c.c
    src/CmdProc/cmdproc.c
    src/data/data.c
    src/UART/uart.c
    src/GPIO/gpio.c
    src/Heater/heater.c
)

if(UNIT_TESTING)
    message(STATUS "Building Unity unit tests")

    # Unity test support
    target_include_directories(app PRIVATE
        src/test
        src/Unity
    )

    target_sources(app PRIVATE
        src/test/test_1.c
        src/Unity/unity.c
    )

else()
    message(STATUS "Building full app")
    target_sources(app PRIVATE src/main.c)
endif()