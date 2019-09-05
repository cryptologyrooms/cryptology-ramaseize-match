/* C/C++ Includes */

#include <stdlib.h>

/*
 * RAAT Includes
 */

#include "raat.hpp"

#include "string-param.hpp"

#include "raat-oneshot-timer.hpp"
#include "raat-oneshot-task.hpp"
#include "raat-task.hpp"

/* Application Includes */

#include "application.h"
#include "buttons.h"
#include "led_manager.h"

/* Defines, typedefs, constants */

static const char * TOP_ROW_ORDER = "ABCDEFGH";
static const char * MIDDLE_ROW_ORDER = "ABCDEFGH";
static const char * BOTTOM_ROW_ORDER = "ABCDEFGH";

static const char * BUTTON_ORDER[NUMBER_OF_BUTTON_ROWS] = {
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

static raat_params_struct * pParams;

/* Private Functions */

static void maglock_unlock(bool unlock)
{
	digitalWrite(MAGLOCK_PIN, unlock ? HIGH : LOW);
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

static void order_by_row(BUTTON * buttons[3])
{
	BUTTON * copy[3] = {buttons[0], buttons[1], buttons[2]};

	for (int i; i<2; i++)
	{
		if (copy[i]->row == 0) { buttons[0] = copy[i]; continue; }
		if (copy[i]->row == 1) { buttons[1] = copy[i]; continue; }
		if (copy[i]->row == 2) { buttons[2] = copy[i]; continue; }
	}
}

static void print_buttons(BUTTON * btns[3])
{
	for (int8_t i=0; i<3; i++)
	{
		if (btns[i])
		{
			Serial.print(btns[i]->row);
			Serial.print(btns[i]->col);
		}
		else
		{
			Serial.print("XX");
		}
		Serial.print(",");
	}
}

static void signal_bad_combination(RGBParam<uint8_t> * pColorParam)
{
	led_manager_set_bank_blink(2, false, false, pColorParam);
}

static void update_pressed_led()
{
	for(int8_t r=0; r<NUMBER_OF_BUTTON_ROWS; r++)
	{
		for(int8_t b=0; b<BUTTONS_PER_ROW; b++)

		if (buttons_get(r, b)->just_pressed)
		{
			led_manager_set_led_blink(1, r, true, true, pParams->pBlinkColour);
		}
	}

}

static void update_game_state(bool * completed_combinations)
{
	BUTTON * p_last_three_buttons[3];

	raat_log(LOG_APP, "Updating state...");

	int button_columns[NUMBER_OF_BUTTON_ROWS];
	unsigned char button_letters[NUMBER_OF_BUTTON_ROWS];

	bool last_three_buttons_are_in_different_rows;

	int last_three_button_rows[3];
	buttons_get_last_three_pressed(p_last_three_buttons);

	if (any_are_null((void**)p_last_three_buttons, 3)) { raat_logln(LOG_APP, " nulls."); return; }

	last_three_button_rows[0] = p_last_three_buttons[0]->row;
	last_three_button_rows[1] = p_last_three_buttons[1]->row;
	last_three_button_rows[2] = p_last_three_buttons[2]->row;

	last_three_buttons_are_in_different_rows = all_unique<int>(last_three_button_rows, 3);

	if (!last_three_buttons_are_in_different_rows) { raat_logln(LOG_APP, " rows diff."); return; }

	qsort(p_last_three_buttons, 3, sizeof(BUTTON*), button_compare_rows);

	raat_log(LOG_APP, "Handling buttons ");
	print_buttons(p_last_three_buttons);
	Serial.println("");

	button_columns[0] = p_last_three_buttons[0]->col;
	button_columns[1] = p_last_three_buttons[1]->col;
	button_columns[2] = p_last_three_buttons[2]->col;
	
	for(int8_t i=0; i<NUMBER_OF_BUTTON_ROWS; i++)
	{
		button_letters[i] = BUTTON_ORDER[i][button_columns[i]];
	}

	if (all_match(button_letters, NUMBER_OF_BUTTON_ROWS))
	{
		if (completed_combinations[button_letters[0] - 'A'])
		{
			raat_logln(LOG_APP, "Existing match %c!", (char)button_letters[0]);
			signal_bad_combination(pParams->pBadCombinationColour);
		}
		else
		{
			raat_logln(LOG_APP, "Match %c!", (char)button_letters[0]);
			completed_combinations[button_letters[0] - 'A'] = true;
		}
	}
	else
	{
		raat_logln(LOG_APP, "No match.");
		signal_bad_combination(pParams->pExistingCombinationColour);
	}
	buttons_clear_history();
}

static void handle_game_state(bool * completed_combinations)
{
	static int8_t s_number_complete = -1;

	int8_t number_complete = count_values<bool>(completed_combinations, true, BUTTONS_PER_ROW);

	if (s_number_complete != number_complete)
	{
		s_number_complete = number_complete;
		raat_logln(LOG_APP, "Complete: %d", s_number_complete);
		led_manager_set_completed_bars(number_complete);
	}

	maglock_unlock(number_complete == BUTTONS_PER_ROW);
}

static void debug_task_fn(RAATTask * pThisTask, void * pTaskData)
{
	(void)pThisTask; (void)pTaskData;

	raat_log(LOG_APP, "State: ");
	for(int8_t i=0; i < BUTTONS_PER_ROW; i++)
	{
		Serial.print(s_completed_combinations[i] ? "1" : "0");
	}
	Serial.println("");

	BUTTON * p_last_three_buttons[3];
	buttons_get_last_three_pressed(p_last_three_buttons);

	raat_log(LOG_APP, "Last 3 buttons: %s, %s, %s");
	for (int8_t i=0; i<3; i++)
	{
		if (p_last_three_buttons[i])
		{
			Serial.print(p_last_three_buttons[i]->row);
			Serial.print(p_last_three_buttons[i]->col);
		}
		else
		{
			Serial.print("XX");
		}
		Serial.print(",");
	}
	Serial.println("");
};
static RAATTask s_debug_task(500, debug_task_fn, NULL);

static void fake_next_sequence(char sequence_letter)
{
	raat_log(LOG_APP, "Faking sequence:"); Serial.print(sequence_letter); 
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

void raat_custom_setup(const raat_devices_struct& devices, const raat_params_struct& params)
{
	pParams = &params;

	led_manager_setup(devices, params);
	buttons_setup((bool&)s_button_update_flag);

	pinMode(MAGLOCK_PIN, OUTPUT);
	maglock_unlock(false);
}

void raat_custom_loop(const raat_devices_struct& devices, const raat_params_struct& params)
{
	buttons_service();
	led_manager_service();

	s_debug_task.run();

	if (s_button_update_flag)
	{
		s_button_update_flag = false;

		update_pressed_led();
		update_game_state(s_completed_combinations);
		handle_game_state(s_completed_combinations);
	}

	if (params.pFakePress->strlen() > 0)
	{
		char sequence_letter[2];
		params.pFakePress->get(sequence_letter);
		params.pFakePress->set("");
		s_fake_flag = false;
		fake_next_sequence(sequence_letter[0]);
	}
}
