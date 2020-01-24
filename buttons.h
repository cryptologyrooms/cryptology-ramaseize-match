#ifndef _BUTTONS_H_
#define _BUTTONS_H_

struct button
{
	bool pressed;
	bool just_pressed;
	bool just_released;
	int debounce;
	int row;
	int col;
};
typedef struct button BUTTON;

void buttons_setup(bool& button_update_flag);
void buttons_service();

int buttons_pressed_count(int r);

BUTTON * buttons_get(int r, int b);

void buttons_fake(uint8_t * button_indexes);

void buttons_get_last_three_pressed(BUTTON * p_last_three_buttons[3]);

void buttons_clear_history();

int button_compare_rows(const void * b1, const void * b2);

#endif