#ifndef _LEDS_H_
#define _LEDS_H_

void leds_setup();
void leds_set_completed_bars(uint8_t set);
void leds_set_blink(uint8_t row);
void leds_tick();

#endif