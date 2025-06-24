#ifndef DATA_H
#define DATA_H
/**
 * @file data.h
 * @brief Shared data structures for sensor readings and controller state.
 */

#include <zephyr/kernel.h>

struct shared_data {
    struct k_mutex mutex;
    int8_t current_temp;
};

struct controller_state {
    struct k_mutex mutex;
    int max_temp;
    int hys_half_band;
    bool system_on;
};

extern struct shared_data sensor_data;
extern struct controller_state ctrl_state;

#endif