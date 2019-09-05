/* Arduino Library Includes */

#include "Adafruit_NeoPixel.h"

/*
 * RAAT Includes
 */

#include "raat.hpp"
#include "raat-oneshot-timer.hpp"
#include "raat-oneshot-task.hpp"
#include "raat-task.hpp"

#include "raat-util-limited-range-int.hpp"
#include "rgb-param.hpp"

#include "adafruit-neopixel-raat.hpp"

/* Application Includes */

#include "application.h"
#include "led_manager.h"
#include "led.h"

/* Defines, typedefs, constants */

#define OFF_COLOR 0,0,0

static const int PIN = A4;
static const int BLINK_MS = 250;

/* Private Module Variables */

static AdafruitNeoPixelRAAT * s_pNeoPixels;

static uint8_t s_complete = 0;

static LED s_leds[NUMBER_OF_LED_BANKS][LEDS_PER_BANK];

static RGBParam<uint8_t> * s_pCompletedColour;

/* Private Module Functions */

static uint8_t ledmap(uint8_t led_bank, uint8_t led)
{
	return (led_bank*LEDS_PER_BANK)+led;
}

static void set_neopixel(uint8_t led_bank, uint8_t ledn, AdafruitNeoPixelRAAT* pNeopixels, LED& led)
{
	int r = led.r();
	int g = led.g();
	int b = led.b();

	int neopixel_index = ledmap(led_bank, ledn);

	if (led.get_state())
	{
		pNeopixels->setPixelColor(neopixel_index, r, g, b);
	}
	else
	{
		pNeopixels->setPixelColor(neopixel_index, 0, 0, 0);
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
				s_leds[led_bank][led].set_color(s_pCompletedColour->get(eR), s_pCompletedColour->get(eG), s_pCompletedColour->get(eB));
			}

			set_neopixel(
				led_bank, led,
				s_pNeoPixels,
				s_leds[led_bank][led]
			);
		}
	}

	s_pNeoPixels->show();
}

static void led_task_fn(RAATTask* this_task, void * pTaskData)
{
	(void)this_task; (void)pTaskData;
	update(10);
}
static RAATTask s_led_task(led_task_fn, 10, NULL);

/* Public Module Functions */

void led_manager_setup(const raat_devices_struct& devices, const raat_params_struct& params)
{
	s_pNeoPixels = devices.pNeoPixels;

	s_pCompletedColour = params.pCompletedColour;

	for(uint8_t led_bank=0; led_bank<NUMBER_OF_LED_BANKS; led_bank++)
	{
		for(uint8_t led=0; led<LEDS_PER_BANK; led++)
		{
			s_pNeoPixels->setPixelColor(ledmap(led_bank, led), s_pCompletedColour);
			s_pNeoPixels->show();
			delay(40);
		}
	}

	for(uint8_t led_bank=0; led_bank<NUMBER_OF_LED_BANKS; led_bank++)
	{
		for(uint8_t led=0; led<LEDS_PER_BANK; led++)
		{
			s_pNeoPixels->setPixelColor(ledmap(led_bank, led), OFF_COLOR);
			s_pNeoPixels->show();
			delay(40);
		}
	}
}

void led_manager_set_completed_bars(uint8_t set)
{
	s_complete = set;
	update(0);
}

void led_manager_set_led_pressed(uint8_t led, int8_t led_bank, RGBParam<uint8_t> * pColourParam)
{
	if (led_bank == -1) { led_bank = s_complete; }
	s_leds[led_bank][led].turn_on();
	s_leds[led_bank][led].set_color(pColourParam->get(eR), pColourParam->get(eG), pColourParam->get(eB));
	update(0);
}

void led_manager_set_led_blink(uint8_t nblinks, uint8_t led, bool invert_blink, bool post_blink_state, RGBParam<uint8_t> * pColourParam, int8_t led_bank)
{
	if (led_bank == -1) { led_bank = s_complete; }
	s_leds[led_bank][led].set_color(pColourParam->get(eR), pColourParam->get(eG), pColourParam->get(eB));
	s_leds[led_bank][led].set_blink(nblinks, 250, invert_blink, post_blink_state);
	update(0);
}

void led_manager_set_bank_blink(uint8_t nblinks, bool invert_blink, bool post_blink_state, RGBParam<uint8_t> * pColourParam, int8_t led_bank)
{
	if (led_bank == -1) { led_bank = s_complete; }

	for(uint8_t led=0; led<LEDS_PER_BANK; led++)
	{
		s_leds[led_bank][led].set_color(pColourParam->get(eR), pColourParam->get(eG), pColourParam->get(eB));
		s_leds[led_bank][led].set_blink(nblinks, 250, invert_blink, post_blink_state);
	}
	update(0);
}

void led_manager_service()
{
	s_led_task.run();
}
