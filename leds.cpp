/* Arduino Library Includes */

#include "Adafruit_NeoPixel.h"
#include "TaskAction.h"

/* Application Includes */

#include "application.h"
#include "leds.h"

/* Defines, typedefs, constants */
#define SINGLE_PRESS_COLOR 255,0,0
#define COMPLETED_COLOR 0,255,0

static const int PIN = 0;
static const int BLINK_MS = 250;

/* Private Module Variables */

static Adafruit_NeoPixel s_leds = Adafruit_NeoPixel(NUMBER_OF_BUTTONS, PIN, NEO_GRB + NEO_KHZ800);
static int s_complete = 0;

static int s_row_blink_counter[NUMBER_OF_ROWS];

/* Private Module Functions */
static void update()
{
	for(int row=0; row<BUTTONS_PER_ROW; row++)
	{
		for(int led=0; led<NUMBER_OF_BUTTONS; led++)
		{
			if (led <= s_complete)
			{
				s_leds.setPixelColor(row*8+led, s_leds.Color(COMPLETED_COLOR));
			}
			else
			{
				if (s_row_blink_counter[row])
				{
					s_leds.setPixelColor(row*8+led, s_leds.Color(0,0,0));				
				}
				else
				{
					s_leds.setPixelColor(row*8+led, s_leds.Color(SINGLE_PRESS_COLOR));
				}
			}
		}
	}

	s_leds.show();
}

static void led_task_fn(TaskAction* this_task)
{
	(void)this_task;
	for(int row=0; row<NUMBER_OF_ROWS; row++)
	{
		if (s_row_blink_counter[row])
		{
			s_row_blink_counter[row] = max(s_row_blink_counter[row]-10, 0);
		}
	}
}
static TaskAction s_led_task(led_task_fn, 10, INFINITE_TICKS);

/* Public Module Functions */

void leds_set_completed_bars(int set)
{
	s_complete = set;
	update();
}

void leds_set_blink(int row)
{
	s_row_blink_counter[row] = BLINK_MS;
	update();
}

void leds_tick()
{
	s_led_task.tick();
}
