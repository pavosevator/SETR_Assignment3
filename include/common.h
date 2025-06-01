/* include/common.h */

#ifndef COMMON_H
#define COMMON_H

#include <zephyr.h>

/* RTDB – globalna “baza” */
struct rtdb {
    bool system_on;
    int32_t current_temperature;
    int32_t setpoint_temperature;
    int32_t max_temperature;
    int32_t Kp;
    int32_t Ti;
    int32_t Td;
    int32_t hysteresis;
};

extern struct rtdb shared_rtdb;
extern struct k_mutex rtdb_mutex;

#endif /* COMMON_H */
