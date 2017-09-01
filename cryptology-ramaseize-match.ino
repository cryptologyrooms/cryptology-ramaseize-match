/* Arduino Includes */

#include <TaskAction.h>

/* Application Includes */

#include "application.h"
#include "buttons.h"
#include "leds.h"

/* Defines, typedefs, constants */

static const char * SEQUENCE_1 = "ABCDEFGH";
static const char * SEQUENCE_2 = "FACEHDBG";
static const char * SEQUENCE_3 = "DEAFGHCB";

static const char * SEQUENCES[3] = {
	SEQUENCE_1,
	SEQUENCE_2,
	SEQUENCE_3
};

static const int8_t NO_BUTTON_PRESSED = -1;

static const uint8_t RELAY_PIN = 3;

/* Private Variables */

static int8_t s_buttons_pressed[] = {NO_BUTTON_PRESSED, NO_BUTTON_PRESSED, NO_BUTTON_PRESSED};
static uint8_t s_game_state = 0;
static volatile bool s_force_flag = false;

/* Private Functions */

static bool game_state_can_advance(GAME_STATE state, int8_t const * const buttons_pressed, char const * const sequences[3])
{
	if (buttons_pressed[0] == NO_BUTTON_PRESSED) { return false; }
	if (buttons_pressed[1] == NO_BUTTON_PRESSED) { return false; }
	if (buttons_pressed[2] == NO_BUTTON_PRESSED) { return false; }

	bool advance = true;

	int8_t button_states[3] = {
		(int8_t)(sequences[0][buttons_pressed[0]] - 'A'),
		(int8_t)(sequences[1][buttons_pressed[1]] - 'A'),
		(int8_t)(sequences[2][buttons_pressed[2]] - 'A')
	};

	advance &= button_states[0] == state;
	advance &= button_states[1] == state;
	advance &= button_states[2] == state;

	return advance;
}

static void debug_task_fn(TaskAction* task)
{
	(void)task;
	for(uint8_t i =0; i<NUMBER_OF_BUTTONS; i++)
	{
		Serial.print(button_is_pressed(i));
		if (i < NUMBER_OF_BUTTONS-1)
		{
			Serial.print(",");
		}
	}
	Serial.println();

	Serial.print("Buttons: ");
	for(uint8_t i =0; i<3; i++)
	{
		Serial.print(s_buttons_pressed[i]);
		if (i < 2)
		{
			Serial.print(",");
		}
	}
	Serial.println();

	Serial.print("Game state: ");
	Serial.println(s_game_state);

	LED_STATE * p_led_state = leds_get_state_array();
	Serial.print("LED state: ");
	for(uint8_t i =0; i<24; i++)
	{
		Serial.print(p_led_state[i]);
		Serial.print(",");
	}
	Serial.println("");
}
static TaskAction s_debug_task(debug_task_fn, 500, INFINITE_TICKS);

static int8_t get_row_button(uint8_t row)
{
	int8_t row_start = row * 8;
	int8_t number_of_buttons_pressed = 0;
	int8_t button_pressed = NO_BUTTON_PRESSED;

	for (int8_t i = 0; i<8; i++)
	{
		if (button_is_pressed(row_start+i))
		{
			number_of_buttons_pressed++;
			button_pressed = i;
		}
	}
	return number_of_buttons_pressed == 1 ? button_pressed : NO_BUTTON_PRESSED;
}

/*static bool at_least_one_button_change_occurred(bool * change_flags)
{
	return change_flags[0] || change_flags[1] || change_flags[2];
}

static bool at_least_one_button_is_pressed(int8_t * buttons_pressed)
{
	bool pressed = false;
	pressed |= buttons_pressed[0] != NO_BUTTON_PRESSED;
	pressed |= buttons_pressed[1] != NO_BUTTON_PRESSED;
	pressed |= buttons_pressed[2] != NO_BUTTON_PRESSED;
	return pressed;
}*/

static bool at_least_one_button_press_occurred(bool * change_flags, int8_t * buttons_pressed)
{
	for (int8_t i = 0; i<3; i++)
	{
		if (change_flags[i] && (buttons_pressed[i] != NO_BUTTON_PRESSED)) { return true; }
	}
	return false;
}

/* Public Functions */

void setup()
{
	Serial.begin(115200);

	buttons_init();
	leds_init();

	int i, j, k, s;
	for (s=0;s<9;s++)
	{
		for (i=0;i<8;i++)
		{
			for (j=0;j<8;j++)
			{
				for (k=0;k<8;k++)
				{
					s_buttons_pressed[0] = i;
					s_buttons_pressed[1] = j;
					s_buttons_pressed[2] = k;

					if(game_state_can_advance(s, s_buttons_pressed, SEQUENCES))
					{
						Serial.print(s); Serial.print(": ");
						Serial.print(i); Serial.print(',');
						Serial.print(j); Serial.print(',');
						Serial.println(k);
					}
				}
			}
		}
	}

	s_buttons_pressed[0] = NO_BUTTON_PRESSED;
	s_buttons_pressed[1] = NO_BUTTON_PRESSED;
	s_buttons_pressed[2] = NO_BUTTON_PRESSED;

	Serial.println("Done");

	pinMode(RELAY_PIN, OUTPUT);
}

void loop()
{
	bool force_state_jump;
	cli();
	force_state_jump = s_force_flag;
	s_force_flag = false;
	sei();

	int8_t buttons_pressed[3];
	bool button_change_flags[3];
	
	buttons_tick();
	leds_tick();
	s_debug_task.tick();

	buttons_pressed[0] = get_row_button(0);
	buttons_pressed[1] = get_row_button(1);
	buttons_pressed[2] = get_row_button(2);

	button_change_flags[0] = buttons_pressed[0] != s_buttons_pressed[0];
	button_change_flags[1] = buttons_pressed[1] != s_buttons_pressed[1];
	button_change_flags[2] = buttons_pressed[2] != s_buttons_pressed[2];

	s_buttons_pressed[0] = buttons_pressed[0];
	s_buttons_pressed[1] = buttons_pressed[1];
	s_buttons_pressed[2] = buttons_pressed[2];

	if (at_least_one_button_press_occurred(button_change_flags, s_buttons_pressed))
	{
		if (button_change_flags[0]) { Serial.print("BP1"); }
		if (button_change_flags[1]) { Serial.print("BP2"); }
		if (button_change_flags[2]) { Serial.print("BP3"); }
		Serial.println("");
		leds_update_on_button_press(button_change_flags);
	}

	if(game_state_can_advance(s_game_state, s_buttons_pressed, SEQUENCES) || force_state_jump)
	{
		s_game_state++;
		leds_update_on_game_state_change(s_game_state);
	}

	if (s_game_state == 8)
	{
		Serial.println("Game complete!");
		digitalWrite(RELAY_PIN, HIGH);
		while(true){
			leds_tick();
		}
	}
}


void serialEvent()
{
	while(Serial.available())
	{
		char c = Serial.read();
		if (c == 'f')
		{
			s_force_flag = true;
		}
	}
}