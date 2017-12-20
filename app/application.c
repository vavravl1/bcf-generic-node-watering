#include <application.h>
#include <sensors.h>
#include <radio.h>

#include "watering.h"

bc_led_t led;
bool led_state = false;

static uint64_t my_id;

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);
void radio_event_handler(bc_radio_event_t event, void *event_param);


void application_init(void)
{
    bc_led_init(&led, BC_GPIO_LED, false, false);

    bc_radio_init(BC_RADIO_MODE_NODE_SLEEPING);
    bc_radio_set_event_handler(radio_event_handler, NULL);

    static bc_button_t button;
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    sensors_init_all();
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_pulse(&led, 100);

        static uint16_t event_count = 0;

        bc_radio_pub_push_button(&event_count);

        event_count++;
	vv_start_water_pump();
    }
}

void radio_event_handler(bc_radio_event_t event, void *event_param)
{
    (void) event_param;

    bc_led_set_mode(&led, BC_LED_MODE_OFF);

    if (event == BC_RADIO_EVENT_ATTACH)
    {
        bc_led_pulse(&led, 1000);
    }
    else if (event == BC_RADIO_EVENT_DETACH)
    {
        bc_led_pulse(&led, 1000);
    }
    else if (event == BC_RADIO_EVENT_INIT_DONE)
    {
        my_id = bc_radio_get_my_id();
	vv_init_watering();
    }
}


