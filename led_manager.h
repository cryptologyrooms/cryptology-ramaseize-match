#ifndef _LEDS_H_
#define _LEDS_H_

void led_manager_setup();
void led_manager_set_completed_bars(uint8_t set);
void led_manager_set_blink(uint8_t row, uint8_t nblinks);
void led_manager_service();

#endif