/* Arduino Library Includes */

#include "TaskAction.h"

/* Application Includes */

#include "application.h"
#include "buttons.h"
#include "leds.h"

/* Defines, typedefs, constants */

static const char * TOP_ROW_ORDER = "ABCDEFGH";
static const char * MIDDLE_ROW_ORDER = "FACEHDBG";
static const char * BOTTOM_ROW_ORDER = "DEAFGHCB";

static const char * BUTTON_ORDER[NUMBER_OF_ROWS] = {
	TOP_ROW_ORDER,
	MIDDLE_ROW_ORDER,
	BOTTOM_ROW_ORDER
};

static const int GAME_FINISHED = BUTTONS_PER_ROW;

static const int MAGLOCK_PIN = 3;

/* Private Variables */

static bool s_completed_combinations[BUTTONS_PER_ROW] = {false, false, false, false, false, false, false, false};

static volatile bool s_button_update_flag = false;
static volatile bool s_fake_flag = false;

/* Private Functions */

static void maglock_unlock(bool unlock)
{
	digitalWrite(MAGLOCK_PIN, unlock ? LOW : HIGH);
}

static bool all_match(unsigned char const * const c, int n)
{
	for (int i=1; i<n; i++)
	{
		if (c[0] != c[i]) { return false; }
	}
	return true;
}

static int count_true(bool * bools, int n)
{
	int count = 0;
	for (int i=0; i<n; i++)
	{
		if (bools[i]) { count++; }
	}
	return count;
}

static inline unsigned char button_to_letter(int button) { return button + 'A'; }

static int find_letter_in_order(unsigned char letter, char const * const order)
{
	for (int i = 0; i < BUTTONS_PER_ROW; i++)
	{
		if (order[i] == letter) { return i; }
	}
	return -1;
}

static bool exactly_one_button_pressed_in_each_row()
{
	return (buttons_pressed_count(0) == 1) && (buttons_pressed_count(1) == 1) && (buttons_pressed_count(2) == 1);
}

static int get_button_pressed_in_row(int r)
{
	for(int b=0; b<BUTTONS_PER_ROW;b++)
	{
		if (buttons_get(r, b)->pressed) { return b; }
	}
	return -1;
}

static void get_button_index_pressed_in_each_row(int * button_states)
{
	for(int i=0; i<NUMBER_OF_ROWS; i++)
	{
		button_states[i] = get_button_pressed_in_row(i);
	}
}

static void update_game_state(bool * completed_combinations)
{
	int button_states[NUMBER_OF_ROWS];
	unsigned char button_letters[NUMBER_OF_ROWS];

	if (!exactly_one_button_pressed_in_each_row()) { return; }

	get_button_index_pressed_in_each_row(button_states);

	for(int8_t i=0; i<NUMBER_OF_ROWS; i++)
	{
		button_letters[i] = BUTTON_ORDER[i][button_states[i]];//find_letter_in_order(button_states[i], BUTTON_ORDER[i]);
	}

	if (all_match(button_letters, NUMBER_OF_ROWS))
	{
		completed_combinations[button_letters[0] - 'A'] = true;
	}
}

static void handle_game_state(bool * completed_combinations)
{
	static int8_t s_number_complete = -1;

	int8_t number_complete = count_true(completed_combinations, BUTTONS_PER_ROW);

	if (s_number_complete != number_complete)
	{
		s_number_complete = number_complete;
		Serial.print("Complete: ");
		Serial.println(s_number_complete);
		leds_set_completed_bars(number_complete);
	}

	maglock_unlock(number_complete == BUTTONS_PER_ROW);
}

static void handle_led_blink()
{
	for(int8_t r=0; r<NUMBER_OF_ROWS; r++)
	{
		for (int8_t b=0; b<BUTTONS_PER_ROW; b++)
		{
			if (buttons_get(r, b)->just_pressed)
			{
				leds_set_blink(r);
			}	
		}
	}
}

static void debug_task_fn(TaskAction * this_task)
{
	(void)this_task;
	Serial.print("State: ");
	for(int8_t i=0; i < BUTTONS_PER_ROW; i++)
	{
		Serial.print(s_completed_combinations[i] ? "1" : "0");
	}
	Serial.println("");

	Serial.print("Buttons: ");
	for (int8_t r=0; r<NUMBER_OF_ROWS; r++)
	{
		for (int8_t b=0; b<BUTTONS_PER_ROW; b++)
		{
			Serial.print(buttons_get(r,b)->pressed ? "0" : "1");
		}
	}
	Serial.println("");
	
};
static TaskAction s_debug_task(debug_task_fn, 500, INFINITE_TICKS);

static void fake_next_sequence()
{
	int next_combination_index = 0;
	while(s_completed_combinations[next_combination_index])
	{
		next_combination_index++;
		if (next_combination_index == 8) { return; }
	}

	char sequence_letter = 'A' + next_combination_index;
	Serial.print("Faking sequence:"); Serial.print(sequence_letter); 
	Serial.print("(");

	uint8_t fake_buttons[3];
	for (uint8_t i = 0; i<3; i++)
	{
		fake_buttons[i] = find_letter_in_order(sequence_letter, BUTTON_ORDER[i]);
		Serial.print(fake_buttons[i]); Serial.print(',');
	}
	Serial.println(")");

	buttons_fake(fake_buttons);
}

/* Public Functions */

void setup()
{
	Serial.begin(115200);

	leds_setup();
	buttons_setup((bool&)s_button_update_flag);

	pinMode(MAGLOCK_PIN, OUTPUT);
	maglock_unlock(false);
}

void loop()
{
	buttons_tick();
	leds_tick();
	s_debug_task.tick();

	if (s_button_update_flag)
	{
		s_button_update_flag = false;
		handle_led_blink();
		update_game_state(s_completed_combinations);
		handle_game_state(s_completed_combinations);
	}

	if (s_fake_flag)
	{
		s_fake_flag = false;
		fake_next_sequence();
	}
}

void serialEvent()
{
	while (Serial.available())
	{
		char c = Serial.read();
		if (c == 'f')
		{
			s_fake_flag = true;
		}
	}
}