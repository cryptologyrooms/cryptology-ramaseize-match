/* Arduino Library Includes */

#include "Adafruit_NeoPixel.h"
#include "TaskAction.h"

/* Application Includes */

#include "application.h"
#include "led.h"

/* Defines, typedefs, constants */
#define SINGLE_PRESS_COLOR 32,0,0
#define COMPLETED_COLOR 0,32,0
#define OFF_COLOR 0,0,0

static const int PIN = A4;
static const int BLINK_MS = 250;

/* Private Module Variables */

static Adafruit_NeoPixel s_neopixels = Adafruit_NeoPixel(NUMBER_OF_BUTTONS, PIN, NEO_GRB + NEO_KHZ800);
static uint8_t s_complete = 0;

static LED s_leds[NUMBER_OF_ROWS][NUMBER_OF_BUTTONS];

/* Private Module Functions */

static uint8_t ledmap(uint8_t row, uint8_t led)
{
	row = 2 - row;
	if (row == 1) { led = 7-led; }
	return row*8+led;
}

static void set_neopixel(uint8_t row, uint8_t ledn, Adafruit_NeoPixel& neopixels, LED& led)
{
	int r = led.r();
	int g = led.g();
	int b = led.b();

	int neopixel_index = ledmap(row, ledn);

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
	for(uint8_t row=0; row<NUMBER_OF_ROWS; row++)
	{
		for(uint8_t led=0; led<BUTTONS_PER_ROW; led++)
		{
			s_leds[row][led].update(time);

			if (led < s_complete)
			{
				s_leds[row][led].turn_on();
				s_leds[row][led].set_color(COMPLETED_COLOR);
			}

			set_neopixel(
				row, led,
				s_neopixels,
				s_leds[row][led]
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

	for(uint8_t row=0; row<NUMBER_OF_ROWS; row++)
	{
		for(uint8_t led=0; led<BUTTONS_PER_ROW; led++)
		{
			s_leds[row][led].set_color(SINGLE_PRESS_COLOR);
		}
	}

	for(uint8_t row=0; row<NUMBER_OF_ROWS; row++)
	{
		for(uint8_t led=0; led<BUTTONS_PER_ROW; led++)
		{
			s_neopixels.setPixelColor(ledmap(row, led), s_neopixels.Color(COMPLETED_COLOR));
			s_neopixels.show();
			delay(40);
		}
	}
	for(uint8_t row=0; row<NUMBER_OF_ROWS; row++)
	{
		for(uint8_t led=0; led<BUTTONS_PER_ROW; led++)
		{
			s_neopixels.setPixelColor(ledmap(row, led), s_neopixels.Color(OFF_COLOR));
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

void led_manager_set_blink(uint8_t row, uint8_t nblinks)
{
	s_leds[row][s_complete].set_blink(2, 250);
	update(0);
}

void led_manager_service()
{
	s_led_task.tick();
}
