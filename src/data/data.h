// data.h – shared header (very small)
#ifndef DATA_H
#define DATA_H

#include <zephyr/kernel.h>

struct shared_data {
    struct k_mutex mutex;
    int8_t temperature;
};

struct controller_state {
    struct k_mutex mutex;
    int max_temp;
    int params[3];
};


/* One global instance, defined exactly once in main.c */
extern struct shared_data sensor_data;
extern struct controller_state ctrl_state;
extern bool system_on;

#endif