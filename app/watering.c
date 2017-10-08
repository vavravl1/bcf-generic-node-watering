#include "watering.h"
#include <bc_gpio.h>
#include <bc_scheduler.h>
#include <bc_button.h>
#include <bc_usb_cdc.h>
#include <bc_adc.h>
#include <bc_radio.h>

#define WATERING_PROCESS_DELAY 10000
#define WATERING_HUMIDITY_THRESHOLD 155
#define MEASURE_HUMIDITY_DELAY 1000
#define PUMP_ON_TIME 5000

void _watering_process_task();
void _init_water_level_power();
void _init_water_level_input();
void _measure_humidity_task();
void _measure_water_level_task();
void _stop_measuring_water_level_task();
void _stop_measuring_humidity_task();
void _stop_water_pump_task();
void _radio_pub_watering(uint8_t event_type, uint8_t value);


// -------------------------------------------------------------
// Intializers
void _vv_init_water_level() {
    _init_water_level_power();
    _init_water_level_input();
    vv_watering_self._measure_water_level_task_id = bc_scheduler_register(_measure_water_level_task, NULL, 100);    
    vv_watering_self._stop_measuring_water_level_task_id = bc_scheduler_register(_stop_measuring_water_level_task, NULL, 100);    
}

void _vv_init_usb_logging() {
     bc_usb_cdc_init();
}

void _vv_init_humidity() {
    vv_watering_self._measured_humidity = 255;
    bc_adc_init(PORT_HUMIDITY_MEASURE, BC_ADC_FORMAT_8_BIT);
    _init_humidity_power_input();
    vv_watering_self._measure_humidity_task_id = bc_scheduler_register(_measure_humidity_task, NULL, 100);
    vv_watering_self._stop_measuring_humidity_task_id = bc_scheduler_register(_stop_measuring_humidity_task, NULL, 100);
}

void _vv_init_water_pump() {
    bc_gpio_init(PORT_WATER_PUMP);
    bc_gpio_set_mode(PORT_WATER_PUMP, BC_GPIO_MODE_OUTPUT);
    _stop_water_pump_task();
    vv_watering_self._stop_water_pump_task_id = bc_scheduler_register(_stop_water_pump_task, NULL, 0);
}

void _vv_init_watering_process() {
    vv_watering_self._watering_process_task_id = bc_scheduler_register(_watering_process_task, NULL, 100);
}

void _init_water_level_power() {
    bc_gpio_init(PORT_WATER_LEVEL_POWER);
    bc_gpio_set_mode(PORT_WATER_LEVEL_POWER, BC_GPIO_MODE_OUTPUT);
}

void _init_humidity_power_input() {
    bc_gpio_init(PORT_HUMIDITY_POWER);
    bc_gpio_set_mode(PORT_HUMIDITY_POWER, BC_GPIO_MODE_OUTPUT);
}

void _init_water_level_input() {
    bc_gpio_init(PORT_WATER_LEVEL_INPUT);
    bc_gpio_set_mode(PORT_WATER_LEVEL_INPUT, BC_GPIO_MODE_INPUT);
    bc_gpio_set_pull(PORT_WATER_LEVEL_INPUT, BC_GPIO_PULL_DOWN);
}

// -------------------------------------------------------------
// Main Intializer
void vv_init_watering() {
#ifdef VV_DEBUG    
    _vv_init_usb_logging();
#endif    
    _vv_init_water_level();
    _vv_init_humidity();
    _vv_init_water_pump();
    _vv_init_watering_process();
}

void vv_log_to_usb(char* message, const char * format, ... ) {
    char buffer[100];
    snprintf(buffer, 100, message, format);    
#ifdef VV_DEBUG        
    bc_usb_cdc_write(buffer, strlen(buffer));
#endif    
}

void _measure_water_level_task() {
    bc_gpio_set_output(PORT_WATER_LEVEL_POWER, 1);
    bc_scheduler_plan_relative(vv_watering_self._stop_measuring_water_level_task_id, 25);
    bc_scheduler_plan_relative(vv_watering_self._measure_water_level_task_id, MEASURE_HUMIDITY_DELAY);
}

void _stop_measuring_water_level_task() {
    vv_watering_self._measured_water_level = !bc_gpio_get_input(PORT_WATER_LEVEL_INPUT);
    vv_log_to_usb("Measured water_level %d\n", vv_watering_self._measured_water_level);    
    _radio_pub_watering(RADIO_WATER_LEVEL, vv_watering_self._measured_water_level);    

    bc_gpio_set_output(PORT_WATER_LEVEL_POWER, 0);
}

// -------------------------------------------------------------
// Tasks
void _measure_humidity_task() {
    bc_gpio_set_output(PORT_HUMIDITY_POWER, 1);
    bc_scheduler_plan_relative(vv_watering_self._stop_measuring_humidity_task_id, 25);
    bc_scheduler_plan_relative(vv_watering_self._measure_humidity_task_id, MEASURE_HUMIDITY_DELAY);
}

void _stop_measuring_humidity_task() {
    bc_adc_read(PORT_HUMIDITY_MEASURE, &vv_watering_self._measured_humidity);
    vv_log_to_usb("Measured humidity %d\n", vv_watering_self._measured_humidity);    
    _radio_pub_watering(RADIO_HUMIDITY, vv_watering_self._measured_humidity);

    bc_gpio_set_output(PORT_HUMIDITY_POWER, 0);   
}

void vv_start_water_pump() {
    bc_gpio_set_output(PORT_WATER_PUMP, 0);
    bc_scheduler_plan_relative(vv_watering_self._stop_water_pump_task_id, PUMP_ON_TIME);
    _radio_pub_watering(RADIO_PUMP, 1);        
}

void _stop_water_pump_task() {
    bc_gpio_set_output(PORT_WATER_PUMP, 1);
    _radio_pub_watering(RADIO_PUMP, 0);        
}

void _radio_pub_watering(uint8_t event_type, uint8_t value) {
    uint8_t buffer[2];
    buffer[0] = event_type;
    buffer[1] = value;
    bc_radio_pub_buffer(buffer, sizeof(buffer));
}

void _watering_process_task() {
    uint8_t water_level_state = vv_watering_self._measured_water_level;
    uint8_t humidity_low = vv_watering_self._measured_humidity > WATERING_HUMIDITY_THRESHOLD;

    if(water_level_state && humidity_low) {
	vv_start_water_pump();
    } else {
	_radio_pub_watering(RADIO_PUMP, 0);        	
    }
    bc_scheduler_plan_relative(vv_watering_self._watering_process_task_id, WATERING_PROCESS_DELAY);
}



