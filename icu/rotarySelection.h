#ifndef ROTARYSELECTION_H_
#define ROTARYSELECTION_H_

#include <Arduino.h>

#include "leds.h"
#include "can.h"
#include "config.h"

void buttons__init();
void buttons__poll();
void buttons__update_LCD();
void buttons__flag_reset();

#endif /* ROTARYSELECTION_H_ */
