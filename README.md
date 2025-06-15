# SETR_Assignment3

# Thermal Process Control System

**Prototype of a heater controller using nRF52840-DK and Zephyr** 

## Hardware

- **Board:** Nordic nRF52840-DK  
- **Sensor:** TC74A0-3.3VAT I²C temperature sensor :contentReference[oaicite:1]{index=1}  
- **Actuator:** 5 W resistor driven via TN0702 FET

## Software Modules

- **`main.c`**  
  Initializes UART, spawns two threads (A = UART RX→CMD, B = control task) :contentReference[oaicite:2]{index=2}
- **`i2c.c` / `i2c.h`**  
  I²C driver for waking and reading TC74 :contentReference[oaicite:3]{index=3}
- **`uart.c` / `uart.h`**  
  UART setup (115200 8N1), callback → message queue → semaphore :contentReference[oaicite:4]{index=4}
- **`cmdproc.c` / `cmdproc.h`**  
  ASCII-frame command parser/formatter (`#M`, `#C`, `#S`, checksum, ACK/ERR) :contentReference[oaicite:5]{index=5}
- **`data.h`**  
  Shared RTDB definitions (setpoint, curTemp, maxTemp, PID params…)