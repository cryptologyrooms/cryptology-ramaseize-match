/* Arduino/C/C++ Includes */
#ifdef ARDUINO
#include <Arduino.h>
#else
#include <cstdint>
template <typename T>
static inline T max(T a, T b) { return (a > b) ? a : b; }
template <typename T>
static inline T min(T a, T b) { return (a < b) ? a : b; }
#endif

/* Application Includes */

#include "led.h"

/* Public Class Members */

LED::LED()
{
	m_blink_timer = 0;
	m_blink_counter = 0;
	m_state = LED_OFF;
	m_r = 0; m_g = 0; m_b = 0;
}

void LED::update_blink_timer(uint16_t time)
{
	if (m_blink_timer) { m_blink_timer = max(m_blink_timer-time, 0); };
}

void LED::update_blink_counter()
{
	if (m_blink_timer == 0)
	{
		if (m_blink_counter) { m_blink_counter = max(m_blink_counter-1, 0); };
	}	
}

void LED::update(uint16_t time)
{
	this->update_blink_timer(time);
	switch(m_state)
	{
	case LED_OFF:
	case LED_ON:
		break;
	case LED_BLINK_OFF:
		if (m_blink_timer == 0)
		{
			if(m_blink_counter > 0)
			{
				m_state = LED_BLINK_ON;
				m_blink_timer = m_blink_reload;
			}
			else
			{
				m_state = LED_OFF;
			}
		}
		break;
	case LED_BLINK_ON:
		if (m_blink_timer == 0)
		{
			this->update_blink_counter();
			if(m_blink_counter > 0)
			{
				m_state = LED_BLINK_OFF;
				m_blink_timer = m_blink_reload;
			}
			else
			{
				m_state = LED_OFF;
			}
		}
		break;
	}
}

void LED::set_blink(uint8_t nblinks, uint16_t time)
{
	m_blink_reload = time;
	m_blink_timer = time;
	m_blink_counter = nblinks;
	m_state = LED_BLINK_ON;
}

bool LED::get_state()
{
	return (m_state == LED_ON) || (m_state == LED_BLINK_ON);	
}

void LED::set_color(int r, int g, int b)
{
	m_r = r;
	m_g = g;
	m_b = b;
}

void LED::turn_on()
{
	m_blink_counter = 0;
	m_blink_timer = 0;
	m_state = LED_ON;
}

void LED::turn_off()
{
	m_blink_counter = 0;
	m_blink_timer = 0;
	m_state = LED_OFF;
}
