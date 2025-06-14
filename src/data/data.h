// data.h – shared header (very small)
#ifndef DATA_H
#define DATA_H

#include <zephyr/kernel.h>

struct shared_data {
    struct k_mutex mutex;
    int8_t current_temp;
};

struct controller_state {
    struct k_mutex mutex;
    int max_temp;
    int params[3];
    bool system_on;
};


/* One global instance, defined exactly once in main.c */
extern struct shared_data sensor_data;
extern struct controller_state ctrl_state;

#endif