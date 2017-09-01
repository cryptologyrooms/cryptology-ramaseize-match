/* Arduino Includes */

#include <TaskAction.h>

/* Application Includes */

#include "buttons.h"

/* Defines, typedefs, constants */

static const uint8_t BUTTONS[NUMBER_OF_BUTTONS] = {
	11, 10, 9, 8, 7, 6, 5, 4,
	26, 27, 28, 29, 30, 31, 32, 33,
	50, 51, 52, 53, A0, A1, A2, A3
};

static const uint8_t DEBOUNCE_TICKS = 10;

/* Private Variables */

static int8_t s_debouncers[NUMBER_OF_BUTTONS] = {0,0,0};
static bool s_button_states[NUMBER_OF_BUTTONS] = {false, false, false};

/* Private Functions */
static void debounce_task_fn(TaskAction* task)
{
	(void)task;
	for(uint8_t i=0; i<NUMBER_OF_BUTTONS; i++)
	{
		if (digitalRead(BUTTONS[i]) == LOW)
		{
			s_debouncers[i] = min(s_debouncers[i]+1, DEBOUNCE_TICKS);
		}
		else
		{
			s_debouncers[i] = max(s_debouncers[i]-1, 0);
		}

		if (s_debouncers[i] == DEBOUNCE_TICKS) { s_button_states[i] = true; }
		if (s_debouncers[i] == 0) { s_button_states[i] = false; }
	}
}
static TaskAction s_debounce_task(debounce_task_fn, 5, INFINITE_TICKS);

/* Public Functions */

void buttons_init()
{
	for(uint8_t i=0; i<NUMBER_OF_BUTTONS; i++)
	{
		pinMode(BUTTONS[i], INPUT_PULLUP);
	}
}

void buttons_tick()
{
	s_debounce_task.tick();
}

bool button_is_pressed(uint8_t btn)
{
	return s_button_states[btn];
}
