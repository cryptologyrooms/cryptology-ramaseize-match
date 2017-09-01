#ifndef _BUTTONS_INIT_H_
#define _BUTTONS_INIT_H_

static const uint8_t NUMBER_OF_BUTTONS = 24;

void buttons_init();
void buttons_tick();

bool button_is_pressed(uint8_t btn);

#endif
