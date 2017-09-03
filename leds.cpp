/* Arduino Library Includes */

#include "Adafruit_NeoPixel.h"
#include "TaskAction.h"

/* Application Includes */

#include "application.h"
#include "leds.h"

/* Defines, typedefs, constants */
#define SINGLE_PRESS_COLOR 32,0,0
#define COMPLETED_COLOR 0,32,0

static const int PIN = A4;
static const int BLINK_MS = 250;

/* Private Module Variables */

static Adafruit_NeoPixel s_leds = Adafruit_NeoPixel(NUMBER_OF_BUTTONS, PIN, NEO_GRB + NEO_KHZ800);
static uint8_t s_complete = 0;

static int s_row_blink_counter[NUMBER_OF_ROWS];
static bool s_pressed_once[NUMBER_OF_ROWS];

/* Private Module Functions */

static void reset_blink_states()
{
	for(uint8_t row=0; row<NUMBER_OF_ROWS; row++)
	{
		s_pressed_once[row] = false;
		s_row_blink_counter[row] = 0;
	}
}

static uint8_t ledmap(uint8_t row, uint8_t led)
{
	if (row == 1) { led = 7-led; }
	return row*8+led;
}

static void update()
{
	for(uint8_t row=0; row<NUMBER_OF_ROWS; row++)
	{
		for(uint8_t led=0; led<BUTTONS_PER_ROW; led++)
		{
			if (led < s_complete)
			{
				s_leds.setPixelColor(ledmap(row, led), s_leds.Color(COMPLETED_COLOR));
			}
			else if (led == s_complete)
			{
				if (s_row_blink_counter[row])
				{
					s_leds.setPixelColor(ledmap(row, led), s_leds.Color(0,0,0));
				}
				else if (s_pressed_once[row])
				{
					s_leds.setPixelColor(ledmap(row, led), s_leds.Color(SINGLE_PRESS_COLOR));
				}
				else
				{
					s_leds.setPixelColor(ledmap(row, led), s_leds.Color(0,0,0));
				}
			}
			else
			{
				s_leds.setPixelColor(ledmap(row, led), s_leds.Color(0,0,0));
			}
		}
	}

	s_leds.show();
}

static void led_task_fn(TaskAction* this_task)
{
	(void)this_task;
	for(uint8_t row=0; row<NUMBER_OF_ROWS; row++)
	{
		if (s_row_blink_counter[row])
		{
			s_row_blink_counter[row] = max(s_row_blink_counter[row]-10, 0);
		}
	}
	update();
}
static TaskAction s_led_task(led_task_fn, 10, INFINITE_TICKS);

/* Public Module Functions */

void leds_setup()
{
	s_leds.begin();
	s_leds.show();

	for(uint8_t row=0; row<NUMBER_OF_ROWS; row++)
	{
		for(uint8_t led=0; led<BUTTONS_PER_ROW; led++)
		{
			s_leds.setPixelColor(ledmap(row, led), s_leds.Color(COMPLETED_COLOR));
			s_leds.show();
			delay(40);
		}
	}
}

void leds_set_completed_bars(uint8_t set)
{
	s_complete = set;
	reset_blink_states();
	update();
}

void leds_set_blink(uint8_t row)
{
	s_row_blink_counter[row] = BLINK_MS;
	s_pressed_once[row] = true;
	update();
}

void leds_tick()
{
	s_led_task.tick();
}
