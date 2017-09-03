/* Arduino Library Includes */

#include "TaskAction.h"

/* Application Includes */

#include "application.h"
#include "buttons.h"

/* Defines, typedefs, constants */

static const int BUTTONS_PINS[NUMBER_OF_ROWS][BUTTONS_PER_ROW] = {
	11, 10, 9, 8, 7, 6, 5, 4,
	26, 27, 28, 29, 30, 31, 32, 33,
	50, 51, 52, 53, A0, A1, A2, A3
};


static const int DEBOUNCE_COUNT = 5;

/* Private Variables */

static BUTTON s_buttons[NUMBER_OF_ROWS][BUTTONS_PER_ROW];
static int s_pressed_count[3] = {0,0,0};
static bool * sp_button_update_flag;

/* Private Functions */

static void debounce_button(BUTTON& button, bool state)
{
	button.just_pressed = false;
	button.just_released = false;

	if (state)
	{
		button.debounce = min(button.debounce+1, DEBOUNCE_COUNT);
	}
	else
	{
		button.debounce = max(button.debounce-1, 0);	
	}

	if (button.debounce == DEBOUNCE_COUNT)
	{
		button.just_pressed = !button.pressed;
		button.pressed = true;
	}
	else if (button.debounce == 0)
	{
		button.just_released = button.pressed;
		button.pressed = false;
	}
}

static void debounce_task_fn(TaskAction* this_task)
{
	(void)this_task;

	int r;
	int b;
	for (r=0; r<NUMBER_OF_ROWS; r++)
	{
		s_pressed_count[r] = 0;
		for (b=0; b<BUTTONS_PER_ROW; b++)
		{
			debounce_button(s_buttons[r][b], digitalRead(BUTTONS_PINS[r][b])==LOW);
			if (s_buttons[r][b].pressed) { s_pressed_count[r]++; }
		}
	}
	*sp_button_update_flag = true;
}
static TaskAction s_debounce_task(debounce_task_fn, 10, INFINITE_TICKS);

/* Public Functions */

void buttons_setup(bool& button_update_flag)
{
	sp_button_update_flag = &button_update_flag;

	for (uint8_t r=0; r<NUMBER_OF_ROWS; r++)
	{
		for (uint8_t b=0; b<BUTTONS_PER_ROW; b++)
		{
			pinMode(BUTTONS_PINS[r][b], INPUT_PULLUP);
			s_buttons[r][b].just_pressed = false;
			s_buttons[r][b].pressed = false;
			s_buttons[r][b].just_released = false;
			s_buttons[r][b].debounce = 0;
		}
	}
}

void buttons_tick()
{
	s_debounce_task.tick();
}

BUTTON * buttons_get(int r, int b)
{
	return &(s_buttons[r][b]);
}

int buttons_pressed_count(int r)
{
	return s_pressed_count[r];
}

void buttons_fake(uint8_t * button_indexes)
{
	for (uint8_t r=0; r<NUMBER_OF_ROWS; r++)
	{
		s_buttons[r][button_indexes[r]].pressed = true;
		s_buttons[r][button_indexes[r]].just_pressed = true;
		s_buttons[r][button_indexes[r]].just_released = false;
		s_buttons[r][button_indexes[r]].debounce = 0;
		s_pressed_count[r] = 1;
	}
	*sp_button_update_flag = true;
}