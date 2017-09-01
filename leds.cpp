/* Arduino Includes */

#include <TaskAction.h>
#include "Adafruit_NeoPixel.h"

/* Application Includes */

#include "application.h"
#include "leds.h"

/* Defines, typedefs, constants */

static int8_t BLINK_MS = 250;

enum led_event
{
	EVENT_GAME_STATE_UPDATE,
	EVENT_BUTTON_PRESSED,
	EVENT_BLINK_TIMER_EXPIRED
};
typedef enum led_event LED_EVENT;

/* Private Variables */

static Adafruit_NeoPixel s_neopixels = Adafruit_NeoPixel(24, A4, NEO_GRB + NEO_KHZ800);
static LED_STATE s_led_state[3][8];

static int8_t s_led_blink_counter[3];

/* Private Functions */

static void reset_blink_counter(uint8_t row)
{
	s_led_blink_counter[row] = BLINK_MS;
}

static LED_STATE handle_new_game_state(LED_STATE current_state, uint8_t led, GAME_STATE game_state)
{
	LED_STATE new_state = current_state;
	switch(current_state)
	{
	case LED_OFF_1:
		if (led == game_state ) { new_state = LED_OFF_2; }
		break;
	case LED_OFF_2:
	case LED_ON_NOT_OK:
	case LED_BLINK:
		if (game_state >= led) { new_state = LED_ON_OK; }
		break;
	case LED_ON_OK:
		break;
	}

	return new_state;
}

static LED_STATE handle_blink_timer_expired(LED_STATE current_state)
{
	LED_STATE new_state = current_state;
	if (current_state == LED_BLINK)
	{
		new_state = LED_ON_NOT_OK;
	}
	return new_state;
}

static LED_STATE handle_button_press(LED_STATE current_state, uint8_t row)
{
	LED_STATE new_state = current_state;

	switch(current_state)
	{
	case LED_OFF_1:
		break;
	case LED_OFF_2:
		new_state = LED_ON_NOT_OK;
		break;
	case LED_ON_NOT_OK:
		new_state = LED_BLINK;
		reset_blink_counter(row);
		break;
	case LED_BLINK:
		reset_blink_counter(row);
		break;
	case LED_ON_OK:
		break;
	}
	return new_state;
}

static void led_blink_task(TaskAction* task)
{
	(void)task;
	for (uint8_t row = 0; row < 3; row++)
	{
		if (s_led_blink_counter[row])
		{
			s_led_blink_counter[row] -= 10;
			if(s_led_blink_counter[row] == 0)
			{
				for (uint8_t led = 0; led < 8; led++)
				{
					s_led_state[row][led] = handle_blink_timer_expired(s_led_state[row][led]);
				}
			}
		}
	}
}
static TaskAction s_led_blink_task(led_blink_task, 10, INFINITE_TICKS);

static void set_led_on_state(Adafruit_NeoPixel& pixels, uint8_t row, uint8_t led, LED_STATE state)
{
	row = 2-row;
	if (row==1) { led=7-led; }
	switch(state)
	{
	case LED_OFF_1:
	case LED_OFF_2:
		pixels.setPixelColor(row*8+led, 0, 0, 0);
		break;
	case LED_ON_OK:
		pixels.setPixelColor(row*8+led, 0, 255, 0);
		break;
	case LED_ON_NOT_OK:
	case LED_BLINK:
		pixels.setPixelColor(row*8+led, 255, 0, 0);
		break;
	}
}

static void update_leds()
{
	for (uint8_t row = 0; row < 3; row++)
	{
		for (uint8_t led = 0; led < 8; led++)
		{
			set_led_on_state(s_neopixels, row, led, s_led_state[row][led]);
		}
	}
	s_neopixels.show();
}

/* Public Functions */

void leds_update_on_game_state_change(GAME_STATE state)
{
	for (uint8_t row = 0; row < 3; row++)
	{
		for (uint8_t led = 0; led < 8; led++)
		{
			s_led_state[row][led] = handle_new_game_state(s_led_state[row][led], led, state);
		}
	}
}

void leds_update_on_button_press(bool * button_press_flags)
{
	for (uint8_t row = 0; row < 3; row++)
	{
		if (button_press_flags[row])
		{
			for (uint8_t led = 0; led < 8; led++)
			{
				s_led_state[row][led] = handle_button_press(s_led_state[row][led], row);
			}
		}
	}
}

void leds_tick()
{
	s_led_blink_task.tick();
	update_leds();
}

void leds_init()
{
	s_neopixels.begin();
	for (uint8_t row = 0; row < 3; row++)
	{
		for (uint8_t led = 0; led < 8; led++)
		{
			s_led_state[row][led] = (led == 0) ? LED_OFF_2 : LED_OFF_1;
			s_neopixels.setPixelColor(row*8+led, 0, 0, 0);
		}
		s_led_blink_counter[row] = 0;
	}
	s_neopixels.show();
}

LED_STATE * leds_get_state_array()
{
	return (LED_STATE*)s_led_state;
}
