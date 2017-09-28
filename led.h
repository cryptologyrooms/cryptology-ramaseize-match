#ifndef _LED_H_
#define _LED_H_

enum led_state
{
	LED_OFF,
	LED_BLINK_OFF,
	LED_BLINK_ON,
	LED_ON
};

class LED
{
public:
	LED();
	void update(uint16_t time);

	void turn_on();
	void turn_off();

	void set_blink(uint8_t nblinks, uint16_t time);

	bool get_state();
	void set_color(int r, int g, int b);

	int r() { return this->m_r;}
	int g() { return this->m_g;}
	int b() { return this->m_b;}

private:
	void update_blink_timer(uint16_t time);
	void update_blink_counter();

	uint16_t m_blink_reload;
	uint16_t m_blink_timer;
	uint8_t m_blink_counter;
	enum led_state m_state;
	int m_r, m_g, m_b;
};


#endif
