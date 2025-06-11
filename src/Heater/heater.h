#ifndef HEATER_H
#define HEATER_H

#include <stdint.h>

int heater_init(void);
void heater_set(uint8_t duty);

#endif /* HEATER_H */