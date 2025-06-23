#ifndef CONTROL_H
#define CONTROL_H

#include <zephyr/kernel.h>

int control_init(void);
void control_thread_func(void *argA, void *argB, void *argC);
void actuator_thread_func(void *argA, void *argB, void *argC);

#endif /* CONTROL_H */