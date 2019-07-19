#include "mjd.h"
#include "mjd_ds3231.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig:
 * - LED
 * - sensor GPIO's
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

static const int MY_DS3231_I2C_SCL_GPIO_NUM = CONFIG_MY_DS3231_I2C_SCL_GPIO_NUM;
static const int MY_DS3231_I2C_SDA_GPIO_NUM = CONFIG_MY_DS3231_I2C_SDA_GPIO_NUM;

/*
 * Config:
 * - I2C addr: default 0x68 for DS3231SN and DS3231M
 *
 */
static const int MY_DS3231_I2C_MASTER_PORT_NUM = I2C_NUM_0;
static const int MY_DS3231_I2C_SLAVE_ADDRESS =   0x68;

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_8K (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)


/*
 * TASKS
 */
void main_task(void *pvParameter) {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    /********************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval = ESP_OK;

    /*********************************
     * LOGGING
     * Optional for Production: dump less messages
     * @doc It is possible to lower the log level for specific modules (wifi and tcpip_adapter are strong candidates)
     * @important Disable u8g2_hal DEBUG messages which are too detailed for me.
     */

    /********************************************************************************
     * STANDARD Init
     */
    mjd_log_chip_info();
    mjd_log_memory_statistics();
    mjd_log_time();

    /////ESP_LOGI(TAG, "@doc Wait X seconds after power-on (start logic analyzer, let peripherals become active, ...)");
    /////vTaskDelay(RTOS_DELAY_1SEC);

    /*
     * KConfig
     */
    ESP_LOGI(TAG, "KConfig:");
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_GPIO_NUM:    %i", MY_LED_ON_DEVBOARD_GPIO_NUM);
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_WIRING_TYPE: %i", MY_LED_ON_DEVBOARD_WIRING_TYPE);
    ESP_LOGI(TAG, "MY_DS3231_I2C_SCL_GPIO_NUM:     %i", MY_DS3231_I2C_SCL_GPIO_NUM);
    ESP_LOGI(TAG, "MY_DS3231_I2C_SDA_GPIO_NUM:     %i", MY_DS3231_I2C_SDA_GPIO_NUM);

    /*
     * Config
     */
    ESP_LOGI(TAG, "Config (defines):");
    ESP_LOGI(TAG, "@info MY_DS3231_I2C_MASTER_PORT_NUM: %i", MY_DS3231_I2C_MASTER_PORT_NUM);
    ESP_LOGI(TAG, "@info MY_DS3231_I2C_SLAVE_ADDRESS:   0x%X", MY_DS3231_I2C_SLAVE_ADDRESS);

    /********************************************************************************
     * LED
     */
    mjd_led_config_t led_config =
                { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    /*
     * MAIN
     */
    ESP_LOGI(TAG, "***** mjd_ds3231_init()");
    mjd_ds3231_config_t config = MJD_DS3231_CONFIG_DEFAULT();
    config.i2c_slave_addr = MY_DS3231_I2C_SLAVE_ADDRESS;
    config.i2c_port_num = MY_DS3231_I2C_MASTER_PORT_NUM;
    config.scl_io_num = MY_DS3231_I2C_SCL_GPIO_NUM;
    config.sda_io_num = MY_DS3231_I2C_SDA_GPIO_NUM;
    f_retval = mjd_ds3231_init(&config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_ds3231_init() | err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    ESP_LOGI(TAG, "***** mjd_ds3231_get_data()");
    ESP_LOGI(TAG, "*****   GET datetime from the device after power-on (check datetime from previous session is preserved!)");
    mjd_ds3231_data_t data_first = { 0 };
    f_retval = mjd_ds3231_get_data(&config, &data_first);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_ds3231_get_data() | err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  year %u | month %u | day %u | hours %u | minutes %u | seconds %u", data_first.year, data_first.month,
            data_first.day, data_first.hours, data_first.minutes, data_first.seconds);
    ESP_LOGI(TAG, "  temperature_celsius: %f", data_first.temperature_celsius);

    vTaskDelay(RTOS_DELAY_5SEC);

    //
    ESP_LOGI(TAG, "***** mjd_ds3231_set_datetime()");
    ESP_LOGI(TAG, "*****   SET datetime Thu Dec 31, 2050 23:59:45h");
    mjd_ds3231_data_t data = { 0 };

    data.year = 2050;
    data.month = 12;
    data.day = 31;
    data.hours = 23;
    data.minutes = 59;
    data.seconds = 45;
    data.week_day = 5; //1=Sunday

    // These are temporary unit test values.
    /*
    data.year = 2999; // Maxval = 2099!
    data.month = 201;
    data.day = 202;
    data.hours = 203;
    data.minutes = 204;
    data.seconds = 205;
    data.week_day = 206; //1=Sunday
    */

    f_retval = mjd_ds3231_set_datetime(&config, &data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_ds3231_set_datetime() | err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    ESP_LOGI(TAG, "***** LOOP 5x");
    uint32_t j;
    for (j = 1; j <= 50000; j++) {
        mjd_led_blink_times(MY_LED_ON_DEVBOARD_GPIO_NUM, 1);

        mjd_ds3231_data_t data = { 0 };
        f_retval = mjd_ds3231_get_data(&config, &data);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). mjd_ds3231_get_data() | err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }

        ESP_LOGI(TAG, "LOOP#%3u: year %u | month %u | day %u | hours %u | minutes %u | seconds %u", j, data.year, data.month, data.day,
                data.hours, data.minutes, data.seconds);
        ESP_LOGI(TAG, "    data->temperature_celsius:  %f", data.temperature_celsius);

        vTaskDelay(RTOS_DELAY_1SEC);
}

    /********************************************************************************
     * LOG TIME
     *
     * @doc The MCU datetime is not necessary the same as the datetime that is stored in the DS3231!
     */
    mjd_log_time();

    /********************************************************************************
     * DeInit component
     */
    ESP_LOGI(TAG, "  DeInit DS3231...");

    f_retval = mjd_ds3231_deinit(&config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_ds3231_deinit() | err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    ESP_LOGI(TAG, "==> NEXT STEPS:");
    ESP_LOGI(TAG, "- Reboot the ESP32 device (or power cycle the device).");
    ESP_LOGI(TAG, "- Check that the correct datetime was kept intact on the RTC board.");

    /********************************************************************************
     * LABEL
     */
    cleanup: ;

    /********************************************************************************
     * Task Delete
     * @doc Passing NULL will end the current task
     */
    ESP_LOGI(TAG, "END OF %s()", __FUNCTION__);
    vTaskDelete(NULL);
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_log_memory_statistics();

    /**********
     * CREATE TASK:
     * @important For stability (RMT + Wifi etc.): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate()]
     */
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_8K, NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task has been created, and is running right now");
    }

    /**********
     * END
     */
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}
