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
static const int MY_DS3231_I2C_SLAVE_ADDRESS = 0x68;

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
    mjd_set_timezone_utc(); // @important for this project!
    mjd_log_time();

    /////ESP_LOGI(TAG, "@doc Wait X seconds after power-on (start logic analyzer, let peripherals become active, ...)");
    /////vTaskDelay(RTOS_DELAY_1SEC);

    /*
     * KConfig
     */
    ESP_LOGI(TAG, "\n*** KConfig:");
    ESP_LOGI(TAG, "  MY_LED_ON_DEVBOARD_GPIO_NUM:    %i", MY_LED_ON_DEVBOARD_GPIO_NUM);
    ESP_LOGI(TAG, "  MY_LED_ON_DEVBOARD_WIRING_TYPE: %i", MY_LED_ON_DEVBOARD_WIRING_TYPE);
    ESP_LOGI(TAG, "  MY_DS3231_I2C_SCL_GPIO_NUM:     %i", MY_DS3231_I2C_SCL_GPIO_NUM);
    ESP_LOGI(TAG, "  MY_DS3231_I2C_SDA_GPIO_NUM:     %i", MY_DS3231_I2C_SDA_GPIO_NUM);

    /*
     * Config
     */
    ESP_LOGI(TAG, "\n*** Config (defines):");
    ESP_LOGI(TAG, "  MY_DS3231_I2C_MASTER_PORT_NUM: %i", MY_DS3231_I2C_MASTER_PORT_NUM);
    ESP_LOGI(TAG, "  MY_DS3231_I2C_SLAVE_ADDRESS:   0x%X", MY_DS3231_I2C_SLAVE_ADDRESS);

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
    mjd_set_timezone_utc(); // @important for this project!

    ESP_LOGI(TAG, "\n*** mjd_ds3231_init()");
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

    ESP_LOGI(TAG, "\n*** 1st mjd_ds3231_get_data()");
    ESP_LOGI(TAG, "      GET datetime from the device after power-on (check that the persisted datetime from the previous session is preserved!)");
    mjd_ds3231_data_t data_first = MJD_DS3231_DATA_DEFAULT();
    f_retval = mjd_ds3231_get_data(&config, &data_first);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_ds3231_get_data() | err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    mjd_ds3231_log_data(data_first);

    vTaskDelay(RTOS_DELAY_2SEC);

    //
    ESP_LOGI(TAG, "\n*** mjd_ds3231_set_datetime(): Thu Dec 31, 2025 23:59:45h");
    mjd_ds3231_data_t data = MJD_DS3231_DATA_DEFAULT();
    data.year = 2025; // OK 2025, NOT-OK: >=2038
    data.month = 12;
    data.day = 31;
    data.hours = 23;
    data.minutes = 59;
    data.seconds = 45;
    /////data.week_day = 5; //1=Sunday not needed

    /*
     // These is a set of invalid property values which should all fail in a unit test.
     data.year = 2999; // Maxval = 2037,2099!
     data.month = 201;
     data.day = 202;
     data.hours = 203;
     data.minutes = 204;
     data.seconds = 205;
     data.week_day = 206;
     */

    f_retval = mjd_ds3231_set_datetime(&config, data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_ds3231_set_datetime() | err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    ESP_LOGI(TAG, "\n*** LOOP");
    uint32_t j;
    for (j = 1; j <= 5; j++) {
        ESP_LOGI(TAG, "ITER#%3u:", j);
        mjd_led_blink_times(MY_LED_ON_DEVBOARD_GPIO_NUM, 1);

        ESP_LOGI(TAG, "mjd_ds3231_get_data()");
        mjd_ds3231_data_t data =
                    { 0 };
        f_retval = mjd_ds3231_get_data(&config, &data);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). mjd_ds3231_get_data() | err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
        mjd_ds3231_log_data(data);

        vTaskDelay(RTOS_DELAY_1SEC);
    }

    vTaskDelay(RTOS_DELAY_2SEC);

    /********************************************************************************
     * RTC Time => MCU Time
     *
     */
    ESP_LOGI(TAG, "\n*** Apply RTC Time => MCU Time");
    mjd_log_time();

    ESP_LOGI(TAG, "mjd_ds3231_set_datetime(): May 21, 2003 01:00:00h");
    mjd_ds3231_data_t data_rtc2mcu = MJD_DS3231_DATA_DEFAULT();
    data_rtc2mcu.year = 2003;
    data_rtc2mcu.month = 5;
    data_rtc2mcu.day = 21;
    data_rtc2mcu.hours = 01;
    data_rtc2mcu.minutes = 00;
    data_rtc2mcu.seconds = 00;
    mjd_ds3231_log_data(data_rtc2mcu);
    f_retval = mjd_ds3231_set_datetime(&config, data_rtc2mcu);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_ds3231_set_datetime() | err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    mjd_ds3231_apply_rtc_time_to_mcu(&config); // MAIN
    mjd_log_time();

    vTaskDelay(RTOS_DELAY_2SEC);

    /********************************************************************************
     * MCU Time => RTC Time
     *
     */
    ESP_LOGI(TAG, "\n*** Save MCU Time => RTC");
    mjd_log_time();
    mjd_ds3231_save_mcu_time_to_rtc(&config); // MAIN

    ESP_LOGI(TAG, "mjd_ds3231_get_data()");
    mjd_ds3231_data_t data_mcu2rtc = MJD_DS3231_DATA_DEFAULT();
    f_retval = mjd_ds3231_get_data(&config, &data_mcu2rtc);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_ds3231_get_data() | err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    mjd_ds3231_log_data(data_mcu2rtc);

    vTaskDelay(RTOS_DELAY_2SEC);

    /********************************************************************************
     * DeInit component
     */
    ESP_LOGI(TAG, "\n*** DeInit DS3231...");

    f_retval = mjd_ds3231_deinit(&config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_ds3231_deinit() | err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /********************************************************************************
     * Post Action: LOG TIME
     *
     */
    ESP_LOGI(TAG, "\n*** Post Action: LOG TIME");
    mjd_log_time();

    /********************************************************************************
     * Post Action: INSTRUCTIONS
     *
     */
    ESP_LOGI(TAG, "\n*** NEXT STEPS:");
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
