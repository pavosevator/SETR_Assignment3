# SETR_Assignment3

# Thermal Process Control System

**Prototype of a heater controller using nRF52840-DK and Zephyr** 

## Hardware

- **Board:** Nordic nRF52840-DK  
- **Sensor:** TC74A0-3.3VAT I²C temperature sensor
- **Actuator:** 5 W resistor driven via TN0702 FET

## Software Modules

- **`main.c`**  
  Initializes UART, spawns two threads (A = UART RX→CMD, B = control task)
- **`i2c.c` / `i2c.h`**  
  I²C driver for waking and reading TC74 
- **`uart.c` / `uart.h`**  
  UART setup (115200 8N1), callback → message queue → semaphore 
- **`cmdproc.c` / `cmdproc.h`**  
  ASCII-frame command parser/formatter (`#M`, `#C`, `#S`, checksum, ACK/ERR) 
- **`data.c / data.h`**  
  Shared RTDB definitions
  - **`control.c / control.h`**  
  Control logic and actuatuion on the heater
- **`test_1.c`**  
  Unity testing file

  ## IMPORTANT NOTE
  To change between unity testing and running the main.c, access the root folder's CMakeLists.txt and switch UNIT_TESTING