#ifndef CONTROL_H
#define CONTROL_H
/**
 * @file control.h
 * @brief Temperature control logic and actuator threads.
 */


#include <zephyr/kernel.h>

/**
 * @brief Initialise control timers and semaphores.
 */
int control_init(void);

/**
 * @brief Thread implementing the control state machine.
 */
void control_thread_func(void *argA, void *argB, void *argC);

/**
 * @brief Thread driving the heater actuator based on system state.
 */
void actuator_thread_func(void *argA, void *argB, void *argC);

#endif /* CONTROL_H */