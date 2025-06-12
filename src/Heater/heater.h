#ifndef HEATER_H
#define HEATER_H

#include <stdint.h>

int heater_init(void);
void heater_on(void);
void heater_off(void);
void heater_toggle(void);

#endif /* HEATER_H */