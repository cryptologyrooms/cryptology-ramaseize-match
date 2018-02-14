/* Arduino Library Includes */

#include "Adafruit_NeoPixel.h"
#include "TaskAction.h"

/* Application Includes */

#include "application.h"
#include "led_manager.h"
#include "led.h"

/* Defines, typedefs, constants */
#define RED 32,0,0
#define AMBER 32,15,0
#define GREEN 0,32,0

#define SINGLE_PRESS_COLOR RED
#define COMPLETED_COLOR GREEN
#define BLINK_COLOUR RED
#define OFF_COLOR 0,0,0

static const int PIN = A4;
static const int BLINK_MS = 250;

/* Private Module Variables */

static Adafruit_NeoPixel s_neopixels = Adafruit_NeoPixel(NUMBER_OF_LED_BANKS * LEDS_PER_BANK, PIN, NEO_GRB + NEO_KHZ800);
static uint8_t s_complete = 0;

static LED s_leds[NUMBER_OF_LED_BANKS][LEDS_PER_BANK];

/* Private Module Functions */

static uint8_t ledmap(uint8_t led_bank, uint8_t led)
{
	return (led_bank*LEDS_PER_BANK)+led;
}

static void set_neopixel(uint8_t led_bank, uint8_t ledn, Adafruit_NeoPixel& neopixels, LED& led)
{
	int r = led.r();
	int g = led.g();
	int b = led.b();

	int neopixel_index = ledmap(led_bank, ledn);

	if (led.get_state())
	{
		neopixels.setPixelColor(neopixel_index, neopixels.Color(r, g, b));
	}
	else
	{
		neopixels.setPixelColor(neopixel_index, neopixels.Color(0, 0, 0));
	}
}

static void update(uint16_t time)
{
	for(uint8_t led_bank=0; led_bank<NUMBER_OF_LED_BANKS; led_bank++)
	{
		for(uint8_t led=0; led<LEDS_PER_BANK; led++)
		{
			s_leds[led_bank][led].update(time);

			if (led_bank < s_complete)
			{
				s_leds[led_bank][led].turn_on();
				s_leds[led_bank][led].set_color(COMPLETED_COLOR);
			}

			set_neopixel(
				led_bank, led,
				s_neopixels,
				s_leds[led_bank][led]
			);
		}
	}

	s_neopixels.show();
}

static void led_task_fn(TaskAction* this_task)
{
	(void)this_task;
	update(10);
}
static TaskAction s_led_task(led_task_fn, 10, INFINITE_TICKS);

/* Public Module Functions */

void led_manager_setup()
{
	s_neopixels.begin();
	s_neopixels.show();

	for(uint8_t led_bank=0; led_bank<NUMBER_OF_LED_BANKS; led_bank++)
	{
		for(uint8_t led=0; led<LEDS_PER_BANK; led++)
		{
			s_neopixels.setPixelColor(ledmap(led_bank, led), s_neopixels.Color(COMPLETED_COLOR));
			s_neopixels.show();
			delay(40);
		}
	}

	for(uint8_t led_bank=0; led_bank<NUMBER_OF_LED_BANKS; led_bank++)
	{
		for(uint8_t led=0; led<LEDS_PER_BANK; led++)
		{
			s_neopixels.setPixelColor(ledmap(led_bank, led), s_neopixels.Color(OFF_COLOR));
			s_neopixels.show();
			delay(40);
		}
	}
}

void led_manager_set_completed_bars(uint8_t set)
{
	s_complete = set;
	update(0);
}

void led_manager_set_led_pressed(uint8_t led, int8_t led_bank)
{
	if (led_bank == -1) { led_bank = s_complete; }
	s_leds[led_bank][led].turn_on();
	s_leds[led_bank][led].set_color(SINGLE_PRESS_COLOR);
	update(0);
}

void led_manager_set_led_blink(uint8_t nblinks, uint8_t led, bool invert_blink, bool post_blink_state, int8_t led_bank)
{
	if (led_bank == -1) { led_bank = s_complete; }
	s_leds[led_bank][led].set_color(BLINK_COLOUR);
	s_leds[led_bank][led].set_blink(nblinks, 250, invert_blink, post_blink_state);
	update(0);
}

void led_manager_set_bank_blink(uint8_t nblinks, bool invert_blink, bool post_blink_state, int8_t led_bank)
{
	if (led_bank == -1) { led_bank = s_complete; }

	for(uint8_t led=0; led<LEDS_PER_BANK; led++)
	{
		s_leds[led_bank][led].set_color(BLINK_COLOUR);
		s_leds[led_bank][led].set_blink(nblinks, 250, invert_blink, post_blink_state);
	}
	update(0);
}

void led_manager_service()
{
	s_led_task.tick();
}
