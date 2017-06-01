#ifndef _BUTTONS_H_
#define _BUTTONS_H_

struct button
{
	bool pressed;
	bool just_pressed;
	bool just_released;
	int debounce;
};
typedef struct button BUTTON;

void buttons_setup();
void buttons_tick();

int buttons_pressed_count(int r);

BUTTON * buttons_get(int r, int b);

#endif
