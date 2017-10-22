#include <application.h>
#include <radio.h>

#include "watering.h"

#define TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL (1 * 60 * 1000)
#define BATTERY_UPDATE_INTERVAL (60 * 60 * 1000)

#define TEMPERATURE_TAG_PUB_VALUE_CHANGE 0.1f
#define TEMPERATURE_TAG_UPDATE_INTERVAL (1 * 1000)

#define HUMIDITY_TAG_PUB_NO_CHANGE_INTEVAL (1 * 60 * 1000)
#define HUMIDITY_TAG_PUB_VALUE_CHANGE 1.0f
#define HUMIDITY_TAG_UPDATE_INTERVAL (1 * 1000)

#define LUX_METER_TAG_PUB_NO_CHANGE_INTEVAL (1 * 60 * 1000)
#define LUX_METER_TAG_PUB_VALUE_CHANGE 5.0f
#define LUX_METER_TAG_UPDATE_INTERVAL (1 * 1000)

#define BAROMETER_TAG_PUB_NO_CHANGE_INTEVAL (1 * 60 * 1000)
#define BAROMETER_TAG_PUB_VALUE_CHANGE 10.0f
#define BAROMETER_TAG_UPDATE_INTERVAL (1 * 1000)

#define CO2_PUB_NO_CHANGE_INTERVAL (1 * 60 * 1000)
#define CO2_PUB_VALUE_CHANGE 50.0f

#if MODULE_POWER
#define MAX_PAGE_INDEX 3
#define CO2_UPDATE_INTERVAL (15 * 1000)
#else
#define MAX_PAGE_INDEX 2
#define CO2_UPDATE_INTERVAL (1 * 60 * 1000)
#endif

bc_led_t led;
bool led_state = false;


static void temperature_tag_init(bc_i2c_channel_t i2c_channel, bc_tag_temperature_i2c_address_t i2c_address, temperature_tag_t *tag);
static void humidity_tag_init(bc_tag_humidity_revision_t revision, bc_i2c_channel_t i2c_channel, humidity_tag_t *tag);
static void lux_meter_tag_init(bc_i2c_channel_t i2c_channel, bc_tag_lux_meter_i2c_address_t i2c_address, lux_meter_tag_t *tag);
static void barometer_tag_init(bc_i2c_channel_t i2c_channel, barometer_tag_t *tag);

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);
void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param);
void humidity_tag_event_handler(bc_tag_humidity_t *self, bc_tag_humidity_event_t event, void *event_param);
void lux_meter_event_handler(bc_tag_lux_meter_t *self, bc_tag_lux_meter_event_t event, void *event_param);
void barometer_tag_event_handler(bc_tag_barometer_t *self, bc_tag_barometer_event_t event, void *event_param);
void co2_event_handler(bc_module_co2_event_t event, void *event_param);
void flood_detector_event_handler(bc_flood_detector_t *self, bc_flood_detector_event_t event, void *event_param);
void pir_event_handler(bc_module_pir_t *self, bc_module_pir_event_t event, void*event_param);
void encoder_event_handler(bc_module_encoder_event_t event, void *event_param);
static void _radio_pub_u16(uint8_t type, uint16_t value);

void application_init(void)
{
    bc_led_init(&led, BC_GPIO_LED, false, false);

    bc_radio_init();

    static bc_button_t button;
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    //----------------------------

    static temperature_tag_t temperature_tag_0_0;
    temperature_tag_init(BC_I2C_I2C0, BC_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT, &temperature_tag_0_0);

    static temperature_tag_t temperature_tag_0_1;
    temperature_tag_init(BC_I2C_I2C0, BC_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE, &temperature_tag_0_1);

    static temperature_tag_t temperature_tag_1_0;
    temperature_tag_init(BC_I2C_I2C1, BC_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT, &temperature_tag_1_0);

    static temperature_tag_t temperature_tag_1_1;
    temperature_tag_init(BC_I2C_I2C1, BC_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE, &temperature_tag_1_1);

    //----------------------------

    static humidity_tag_t humidity_tag_0_0;
    humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R1, BC_I2C_I2C0, &humidity_tag_0_0);

    static humidity_tag_t humidity_tag_0_2;
    humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R2, BC_I2C_I2C0, &humidity_tag_0_2);

    static humidity_tag_t humidity_tag_0_4;
    humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R3, BC_I2C_I2C0, &humidity_tag_0_4);

    static humidity_tag_t humidity_tag_1_0;
    humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R1, BC_I2C_I2C1, &humidity_tag_1_0);

    static humidity_tag_t humidity_tag_1_2;
    humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R2, BC_I2C_I2C1, &humidity_tag_1_2);

    static humidity_tag_t humidity_tag_1_4;
    humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R3, BC_I2C_I2C1, &humidity_tag_1_4);

    //----------------------------

    static lux_meter_tag_t lux_meter_0_0;
    lux_meter_tag_init(BC_I2C_I2C0, BC_TAG_LUX_METER_I2C_ADDRESS_DEFAULT, &lux_meter_0_0);

    static lux_meter_tag_t lux_meter_0_1;
    lux_meter_tag_init(BC_I2C_I2C0, BC_TAG_LUX_METER_I2C_ADDRESS_ALTERNATE, &lux_meter_0_1);

    static lux_meter_tag_t lux_meter_1_0;
    lux_meter_tag_init(BC_I2C_I2C1, BC_TAG_LUX_METER_I2C_ADDRESS_DEFAULT, &lux_meter_1_0);

    static lux_meter_tag_t lux_meter_1_1;
    lux_meter_tag_init(BC_I2C_I2C1, BC_TAG_LUX_METER_I2C_ADDRESS_ALTERNATE, &lux_meter_1_1);

    //----------------------------

    static barometer_tag_t barometer_tag_0_0;
    barometer_tag_init(BC_I2C_I2C0, &barometer_tag_0_0);

    static barometer_tag_t barometer_tag_1_0;
    barometer_tag_init(BC_I2C_I2C1, &barometer_tag_1_0);

    //----------------------------

    static event_param_t co2_event_param = { .next_pub = 0 };
    bc_module_co2_init();
    bc_module_co2_set_update_interval(CO2_UPDATE_INTERVAL);
    bc_module_co2_set_event_handler(co2_event_handler, &co2_event_param);

    //----------------------------

    static bc_flood_detector_t flood_detector_a;
    static event_param_t flood_detector_a_event_param = {.number = 'a', .value = -1};
    bc_flood_detector_init(&flood_detector_a, BC_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_A);
    bc_flood_detector_set_event_handler(&flood_detector_a, flood_detector_event_handler, &flood_detector_a_event_param);
    bc_flood_detector_set_update_interval(&flood_detector_a, 1000);

    static bc_flood_detector_t flood_detector_b;
    static event_param_t flood_detector_b_event_param = {.number = 'b', .value = -1};
    bc_flood_detector_init(&flood_detector_b, BC_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_B);
    bc_flood_detector_set_event_handler(&flood_detector_b, flood_detector_event_handler, &flood_detector_b_event_param);
    bc_flood_detector_set_update_interval(&flood_detector_b, 1000);

//    static bc_module_pir_t pir;
//    bc_module_pir_init(&pir);
//    bc_module_pir_set_event_handler(&pir, pir_event_handler, NULL);

#if MODULE_POWER
    bc_radio_listen();
    bc_radio_set_event_handler(radio_event_handler, NULL);

    bc_module_power_init();
    bc_led_strip_init(&led_strip.self, bc_module_power_get_led_strip_driver(), &led_strip_buffer);

    bc_module_relay_init(&relay_0_0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT);
    bc_module_relay_init(&relay_0_1, BC_MODULE_RELAY_I2C_ADDRESS_ALTERNATE);

    led_strip.update_task_id = bc_scheduler_register(led_strip_update_task, NULL, BC_TICK_INFINITY);

#else
    #if BATTERY_MINI
        bc_module_battery_init(BC_MODULE_BATTERY_FORMAT_MINI);
    #else
        bc_module_battery_init(BC_MODULE_BATTERY_FORMAT_STANDARD);
    #endif
        bc_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);
#endif

    vv_init_watering();
}

void application_task(void)
{

}

static void temperature_tag_init(bc_i2c_channel_t i2c_channel, bc_tag_temperature_i2c_address_t i2c_address, temperature_tag_t *tag)
{
    memset(tag, 0, sizeof(*tag));

    tag->param.number = (i2c_channel << 7) | i2c_address;

    bc_tag_temperature_init(&tag->self, i2c_channel, i2c_address);

    bc_tag_temperature_set_update_interval(&tag->self, TEMPERATURE_TAG_UPDATE_INTERVAL);

    bc_tag_temperature_set_event_handler(&tag->self, temperature_tag_event_handler, &tag->param);
}

static void humidity_tag_init(bc_tag_humidity_revision_t revision, bc_i2c_channel_t i2c_channel, humidity_tag_t *tag)
{
    uint8_t address;

    memset(tag, 0, sizeof(*tag));

    if (revision == BC_TAG_HUMIDITY_REVISION_R1)
    {
        address = 0x5f;
    }
    else if (revision == BC_TAG_HUMIDITY_REVISION_R2)
    {
        address = 0x40;
    }
    else if (revision == BC_TAG_HUMIDITY_REVISION_R3)
    {
        address = 0x40 | 0x0f; // 0x0f - hack
    }
    else
    {
        return;
    }

    tag->param.number = (i2c_channel << 7) | address;

    bc_tag_humidity_init(&tag->self, revision, i2c_channel, BC_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT);

    bc_tag_humidity_set_update_interval(&tag->self, HUMIDITY_TAG_UPDATE_INTERVAL);

    bc_tag_humidity_set_event_handler(&tag->self, humidity_tag_event_handler, &tag->param);
}

static void lux_meter_tag_init(bc_i2c_channel_t i2c_channel, bc_tag_lux_meter_i2c_address_t i2c_address, lux_meter_tag_t *tag)
{
    memset(tag, 0, sizeof(*tag));

    tag->param.number = (i2c_channel << 7) | i2c_address;

    bc_tag_lux_meter_init(&tag->self, i2c_channel, i2c_address);

    bc_tag_lux_meter_set_update_interval(&tag->self, LUX_METER_TAG_UPDATE_INTERVAL);

    bc_tag_lux_meter_set_event_handler(&tag->self, lux_meter_event_handler, &tag->param);
}

static void barometer_tag_init(bc_i2c_channel_t i2c_channel, barometer_tag_t *tag)
{
    memset(tag, 0, sizeof(*tag));

    tag->param.number = (i2c_channel << 7) | 0x60;

    bc_tag_barometer_init(&tag->self, i2c_channel);

    bc_tag_barometer_set_update_interval(&tag->self, BAROMETER_TAG_UPDATE_INTERVAL);

    bc_tag_barometer_set_event_handler(&tag->self, barometer_tag_event_handler, &tag->param);
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
    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        bc_radio_enroll_to_gateway();

        bc_led_set_mode(&led, BC_LED_MODE_OFF);

        bc_led_pulse(&led, 1000);
    }
}

void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param)
{
    float value;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_TEMPERATURE_EVENT_UPDATE)
    {
        return;
    }

    if (bc_tag_temperature_get_temperature_celsius(self, &value))
    {
        if ((fabs(value - param->value) >= TEMPERATURE_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick()))
        {
            bc_radio_pub_thermometer(param->number, &value);
            param->value = value;
            param->next_pub = bc_scheduler_get_spin_tick() + TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL;

            bc_scheduler_plan_now(0);
        }
    }
}

void humidity_tag_event_handler(bc_tag_humidity_t *self, bc_tag_humidity_event_t event, void *event_param)
{
    float value;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_HUMIDITY_EVENT_UPDATE)
    {
        return;
    }

    if (bc_tag_humidity_get_humidity_percentage(self, &value))
    {
        if ((fabs(value - param->value) >= HUMIDITY_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick()))
        {
            bc_radio_pub_humidity(param->number, &value);
            param->value = value;
            param->next_pub = bc_scheduler_get_spin_tick() + HUMIDITY_TAG_PUB_NO_CHANGE_INTEVAL;

            bc_scheduler_plan_now(0);
        }
    }
}

void lux_meter_event_handler(bc_tag_lux_meter_t *self, bc_tag_lux_meter_event_t event, void *event_param)
{
    float value;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_LUX_METER_EVENT_UPDATE)
    {
        return;
    }

    if (bc_tag_lux_meter_get_illuminance_lux(self, &value))
    {
        if ((fabs(value - param->value) >= LUX_METER_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick()))
        {
            bc_radio_pub_luminosity(param->number, &value);
            param->value = value;
            param->next_pub = bc_scheduler_get_spin_tick() + LUX_METER_TAG_PUB_NO_CHANGE_INTEVAL;

            bc_scheduler_plan_now(0);
        }
    }
}

void barometer_tag_event_handler(bc_tag_barometer_t *self, bc_tag_barometer_event_t event, void *event_param)
{
    float pascal;
    float meter;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_BAROMETER_EVENT_UPDATE)
    {
        return;
    }

    if (!bc_tag_barometer_get_pressure_pascal(self, &pascal))
    {
        return;
    }

    if ((fabs(pascal - param->value) >= BAROMETER_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick()))
    {

        if (!bc_tag_barometer_get_altitude_meter(self, &meter))
        {
            return;
        }

        bc_radio_pub_barometer(param->number, &pascal, &meter);
        param->value = pascal;
        param->next_pub = bc_scheduler_get_spin_tick() + BAROMETER_TAG_PUB_NO_CHANGE_INTEVAL;

        bc_scheduler_plan_now(0);
    }
}

void co2_event_handler(bc_module_co2_event_t event, void *event_param)
{
    event_param_t *param = (event_param_t *) event_param;
    float value;

    if (event == BC_MODULE_CO2_EVENT_UPDATE)
    {
        if (bc_module_co2_get_concentration(&value))
        {
            if ((fabs(value - param->value) >= CO2_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick()))
            {
                bc_radio_pub_co2(&value);
                param->value = value;
                param->next_pub = bc_scheduler_get_spin_tick() + CO2_PUB_NO_CHANGE_INTERVAL;

                bc_scheduler_plan_now(0);
            }
        }
    }
}

void flood_detector_event_handler(bc_flood_detector_t *self, bc_flood_detector_event_t event, void *event_param)
{
    if (event == BC_FLOOD_DETECTOR_EVENT_UPDATE)
    {
        if (bc_flood_detector_is_alarm(self) != ((event_param_t *) event_param)->value)
        {
            ((event_param_t *) event_param)->value = bc_flood_detector_is_alarm(self);

            uint8_t buffer[3];
            buffer[0] = RADIO_FLOOD_DETECTOR;
            buffer[1] = ((event_param_t *) event_param)->number;
            buffer[2] = ((event_param_t *) event_param)->value;
            bc_radio_pub_buffer(buffer, sizeof(buffer));
        }
    }
}

void pir_event_handler(bc_module_pir_t *self, bc_module_pir_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_MODULE_PIR_EVENT_MOTION)
    {
        static uint16_t event_count = 0;
        event_count++;

        uint8_t buffer[1 + sizeof(event_count)];

        buffer[0] = RADIO_PIR;

        memcpy(buffer + 1, &event_count, sizeof(event_count));

        bc_radio_pub_buffer(buffer, sizeof(buffer));
    }
}

#if MODULE_POWER
static void radio_event_handler(bc_radio_event_t event, void *event_param)
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
        my_device_address = bc_radio_get_device_address();
    }
}

static void led_strip_update_task(void *param)
{
    (void) param;

    if (!bc_led_strip_is_ready(&led_strip.self))
    {
        bc_scheduler_plan_current_now();
        return;
    }

    switch (led_strip.show) {
        case LED_STRIP_SHOW_COLOR:
        {
            bc_led_strip_effect_stop(&led_strip.self);

            bc_led_strip_fill(&led_strip.self, led_strip.color);

            led_strip.show = LED_STRIP_SHOW_NONE;
            led_strip.last = LED_STRIP_SHOW_COLOR;
            break;
        }
        case LED_STRIP_SHOW_COMPOUND:
        {
            bc_led_strip_effect_stop(&led_strip.self);

            int from = 0;
            int to;
            uint8_t *color;

            for (int i = 0; i < led_strip.compound.length; i += 5)
            {
                color = led_strip.compound.data + i + 1;
                to = from + led_strip.compound.data[i];

                for (;(from < to) && (from < LED_STRIP_COUNT); from++)
                {
                    bc_led_strip_set_pixel_rgbw(&led_strip.self, from, color[0], color[1], color[2], color[3]);
                }

                from = to;
            }

            led_strip.show = LED_STRIP_SHOW_NONE;
            led_strip.last = LED_STRIP_SHOW_COMPOUND;
            break;
        }
        case LED_STRIP_SHOW_EFFECT:
        {
            led_strip.last = LED_STRIP_SHOW_EFFECT;
            return;
        }
        default:
        {
            break;
        }
    }

    bc_led_strip_write(&led_strip.self);

    bc_scheduler_plan_current_relative(250);
}

void bc_radio_on_buffer(uint64_t *peer_device_address, uint8_t *buffer, size_t *length)
{
    (void) peer_device_address;
    if (*length < (1 + sizeof(uint64_t)))
    {
        return;
    }

    uint64_t device_address;
    uint8_t *pointer = buffer + sizeof(uint64_t) + 1;

    memcpy(&device_address, buffer + 1, sizeof(device_address));

    if (device_address != my_device_address)
    {
        return;
    }

    switch (buffer[0]) {
        case RADIO_LED_SET:
        {
            if (*length != (1 + sizeof(uint64_t) + 1))
            {
                return;
            }
            led_state = buffer[sizeof(uint64_t) + 1];
            bc_led_set_mode(&led, led_state ? BC_LED_MODE_ON : BC_LED_MODE_OFF);
            _radio_pub_state(RADIO_LED, led_state);
            break;
        }
        case RADIO_RELAY_0_SET:
        case RADIO_RELAY_1_SET:
        {
            if (*length != (1 + sizeof(uint64_t) + 1))
            {
                return;
            }
            bc_module_relay_set_state(buffer[0] == RADIO_RELAY_0_SET ? &relay_0_0 : &relay_0_1, buffer[sizeof(uint64_t) + 1]);
            _radio_pub_state(buffer[0] == RADIO_RELAY_0_SET ? RADIO_RELAY_0 : RADIO_RELAY_1, buffer[sizeof(uint64_t) + 1]);
            break;
        }
        case RADIO_RELAY_0_PULSE_SET:
        case RADIO_RELAY_1_PULSE_SET:
        {
            if (*length != (1 + sizeof(uint64_t) + 1 + 4))
            {
                return;
            }
            uint32_t duration; // Duration is 4 byte long in a radio packet, but 8 bytes as a bc_relay_pulse parameter.
            memcpy(&duration, &buffer[sizeof(uint64_t) + 2], sizeof(uint32_t));
            bc_module_relay_pulse(buffer[0] == RADIO_RELAY_0_PULSE_SET ? &relay_0_0 : &relay_0_1, buffer[sizeof(uint64_t) + 1], (bc_tick_t)duration);
            break;
        }
        case RADIO_RELAY_0_GET:
        case RADIO_RELAY_1_GET:
        {
            bc_module_relay_state_t state = bc_module_relay_get_state(buffer[0] == RADIO_RELAY_0_GET ? &relay_0_0 : &relay_0_1);
            if (state != BC_MODULE_RELAY_STATE_UNKNOWN)
            {
                _radio_pub_state(buffer[0] == RADIO_RELAY_0_GET ? RADIO_RELAY_0 : RADIO_RELAY_1, state == BC_MODULE_RELAY_STATE_TRUE ? true : false);
            }
            break;
        }
        case RADIO_RELAY_POWER_SET:
        {
            if (*length != (1 + sizeof(uint64_t) + 1))
            {
                return;
            }
            bc_module_power_relay_set_state(*pointer);
            _radio_pub_state(RADIO_RELAY_POWER, *pointer);
            break;
        }
        case RADIO_RELAY_POWER_GET:
        {
            _radio_pub_state(RADIO_RELAY_POWER, bc_module_power_relay_get_state());
            break;
        }
        case RADIO_LED_STRIP_COLOR_SET:
        {    // HEAD(1B); ADDRESS(8B); COLOR(4B)
            if (*length != (1 + sizeof(uint64_t) + 4))
            {
                return;
            }

            led_strip.color = 0;

            led_strip.color |= ((uint32_t) *pointer++) << 24;
            led_strip.color |= ((uint32_t) *pointer++) << 16;
            led_strip.color |= ((uint32_t) *pointer++) << 8;
            led_strip.color |= ((uint32_t) *pointer);

            led_strip.show = LED_STRIP_SHOW_COLOR;
            bc_scheduler_plan_now(led_strip.update_task_id);
            break;
        }
        case RADIO_LED_STRIP_BRIGHTNESS_SET:
        {
            // HEAD(1B); ADDRESS(8B); BRIGHTNESS(1B)
            if (*length != (1 + sizeof(uint64_t) + 1))
            {
                return;
            }

            uint8_t brightness = (uint16_t)buffer[sizeof(uint64_t) + 1] * 255 / 100;

            bc_led_strip_set_brightness(&led_strip.self, brightness);

            led_strip.show = led_strip.last;
            bc_scheduler_plan_now(led_strip.update_task_id);
            break;
        }
        case RADIO_LED_STRIP_COMPOUND_SET:
        {
            // HEAD(1B); ADDRESS(8B); OFFSET(1B), COUNT(1B), COLOR(4B), COUNT(1B), COLOR(4B), ...
            if (*length < (1 + sizeof(uint64_t) + 1))
            {
                return;
            }

            bc_led_strip_effect_stop(&led_strip.self);

            int offset = buffer[sizeof(uint64_t) + 1];

            memcpy(led_strip.compound.data + offset, buffer + sizeof(uint64_t) + 2, sizeof(led_strip.compound.data) - offset);

            led_strip.compound.length = offset + (int) *length;

            led_strip.show = LED_STRIP_SHOW_COMPOUND;

            bc_scheduler_plan_now(led_strip.update_task_id);
            break;
        }
        case RADIO_LED_STRIP_EFFECT_SET:
        {
            //TYPE(1B); WAIT(2B); COLOR(4B)
            if (*length < (1 + sizeof(uint64_t) + 1 + sizeof(uint16_t) + sizeof(uint32_t)))
            {
                return;
            }

            uint16_t wait;
            memcpy(&wait, buffer + 1 + sizeof(uint64_t) + 1, sizeof(wait));
            uint32_t color;
            memcpy(&color, buffer + 1 + sizeof(uint64_t) + 1 + sizeof(wait), sizeof(color));

            switch (buffer[sizeof(uint64_t) + 1]) {
                case RADIO_LED_STRIP_EFFECT_TYPE_TEST:
                {
                    bc_led_strip_effect_test(&led_strip.self);
                    break;
                }
                case RADIO_LED_STRIP_EFFECT_TYPE_RAINBOW:
                {
                    bc_led_strip_effect_rainbow(&led_strip.self, wait);
                    break;
                }
                case RADIO_LED_STRIP_EFFECT_TYPE_RAINBOW_CYCLE:
                {
                    bc_led_strip_effect_rainbow_cycle(&led_strip.self, wait);
                    break;
                }
                case RADIO_LED_STRIP_EFFECT_TYPE_THEATER_CHASE_RAINBOW:
                {
                    bc_led_strip_effect_theater_chase_rainbow(&led_strip.self, wait);
                    break;
                }
                case RADIO_LED_STRIP_EFFECT_TYPE_COLOR_WIPE:
                {
                    bc_led_strip_effect_color_wipe(&led_strip.self, color, wait);
                    break;
                }
                case RADIO_LED_STRIP_EFFECT_TYPE_THEATER_CHASE:
                {
                    bc_led_strip_effect_theater_chase(&led_strip.self, color, wait);
                    break;
                }
                default:
                    return;
            }
            led_strip.show = LED_STRIP_SHOW_EFFECT;
            break;
        }
        case RADIO_LED_STRIP_THERMOMETER_SET:
        {
            if (*length < (1 + sizeof(uint64_t) + sizeof(float) + 1 + 1))
            {
                return;
            }
            bc_led_strip_effect_stop(&led_strip.self);
            memcpy(&led_strip.thermometer.temperature, buffer + 1 + sizeof(uint64_t), sizeof(float));
            led_strip.thermometer.min = buffer[1 + sizeof(uint64_t) + sizeof(float)];
            led_strip.thermometer.max = buffer[1 + sizeof(uint64_t) + sizeof(float) + 1];
            led_strip.show = LED_STRIP_SHOW_THERMOMETER;
            bc_scheduler_plan_now(led_strip.update_task_id);
            break;
        }
        default:
        {
            break;
        }
    }
}

static void _radio_pub_state(uint8_t type, bool state)
{
    uint8_t buffer[2];
    buffer[0] = type;
    buffer[1] = state;
    bc_radio_pub_buffer(buffer, sizeof(buffer));
}
#else

void battery_event_handler(bc_module_battery_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    float voltage;

    if (bc_module_battery_get_voltage(&voltage))
    {
        bc_radio_pub_battery((BATTERY_MINI ? 1 : 0), &voltage);
    }
}

#endif // MODULE_POWER

static void _radio_pub_u16(uint8_t type, uint16_t value)
{
    uint8_t buffer[1 + sizeof(value)];
    buffer[0] = type;
    memcpy(buffer + 1, &value, sizeof(value));
    bc_radio_pub_buffer(buffer, sizeof(buffer));
}

