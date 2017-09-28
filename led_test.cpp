#ifndef ARDUINO

#include <iostream>
#include <cstdint>

#include "led.h"

static void print_led_state(LED& led)
{
	std::cout << "LED state: " << (led.get_state() ? "on" : "off") << std::endl;	
}

int main(int argc, char * argv[])
{
	(void)argc; (void)argv;
	LED led;

	std::cout << "LED Class Testing" << std::endl;
	print_led_state(led);
	std::cout << std::endl;

	std::cout << "Setting single 100ms blink" << std::endl;
	led.set_blink(1, 100);
	print_led_state(led);
	std::cout << std::endl;

	std::cout << "99ms elapsing..." << std::endl;
	led.update(99);
	print_led_state(led);
	std::cout << "1ms elapsing..." << std::endl;
	led.update(1);
	print_led_state(led);
	std::cout << std::endl;

	std::cout << "99ms elapsing..." << std::endl;
	led.update(99);
	print_led_state(led);
	std::cout << "1ms elapsing..." << std::endl;
	led.update(1);
	print_led_state(led);
	std::cout << std::endl;

	std::cout << "99ms elapsing..." << std::endl;
	led.update(99);
	print_led_state(led);
	std::cout << "1ms elapsing..." << std::endl;
	led.update(1);
	print_led_state(led);
	std::cout << std::endl;
	
	std::cout << "Setting double 100ms blink" << std::endl;
	led.set_blink(2, 100);
	print_led_state(led);
	std::cout << std::endl;

	std::cout << "99ms elapsing..." << std::endl;
	led.update(99);
	print_led_state(led);
	std::cout << "1ms elapsing..." << std::endl;
	led.update(1);
	print_led_state(led);
	std::cout << std::endl;

	std::cout << "99ms elapsing..." << std::endl;
	led.update(99);
	print_led_state(led);
	std::cout << "1ms elapsing..." << std::endl;
	led.update(1);
	print_led_state(led);
	std::cout << std::endl;

	std::cout << "99ms elapsing..." << std::endl;
	led.update(99);
	print_led_state(led);
	std::cout << "1ms elapsing..." << std::endl;
	led.update(1);
	print_led_state(led);
	std::cout << std::endl;

	std::cout << "99ms elapsing..." << std::endl;
	led.update(99);
	print_led_state(led);
	std::cout << "1ms elapsing..." << std::endl;
	led.update(1);
	print_led_state(led);
	std::cout << std::endl;

}

#endif