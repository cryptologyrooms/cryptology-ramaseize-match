#ifndef _LEDS_H_
#define _LEDS_H_

enum led_state
{
	LED_OFF_1,
	LED_OFF_2,
	LED_ON_OK,
	LED_ON_NOT_OK,
	LED_BLINK
};
typedef enum led_state LED_STATE;

void leds_init();

void leds_tick();

void leds_update_on_game_state_change(GAME_STATE state);
void leds_update_on_button_press(bool * button_press_flags);

LED_STATE * leds_get_state_array();

#endif
