/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#include "user_config.h"
#include "uart_proc.h"

static const char *TAG = "MAIN";
nvs_handle handle1;

	
int8_t infoparam_read (void)
{
	size_t g_infoParam_len = sizeof (g_infoParam);
	if (nvs_get_blob (handle1, "info_param", &g_infoParam, &g_infoParam_len) == ESP_OK) {
		ESP_LOGI (TAG, "info_param read succeed");

		return ESP_OK;
	}
	else {
		ESP_LOGE (TAG, "info_param read failed");
	}

	return ESP_FAIL;
}

int8_t infoparam_write (void)
{
	if (nvs_set_blob (handle1, "info_param", &g_infoParam, sizeof (g_infoParam)) == ESP_OK) {
		ESP_LOGI (TAG, "info_param change succeed");
		return ESP_OK;
	}
	else {
		ESP_LOGE (TAG, "info_param change failed");
	}

	return ESP_FAIL;
}

int8_t infoparam_init (void)
{
	memset (&g_infoParam, (uint8_t)0, sizeof (g_infoParam));

	if (nvs_set_blob (handle1, "info_param", &g_infoParam, sizeof (g_infoParam)) == ESP_OK) {
		ESP_LOGI (TAG, "info_param init succeed");
		return ESP_OK;
	}
	else {
		ESP_LOGE (TAG, "info_param init failed");
	}

	return ESP_FAIL;
}

void pwm_init (void)
{
    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
        //.duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty		8192 max
        .duty_resolution = LEDC_TIMER_11_BIT, // resolution of PWM duty		2048 max, 20khz 일때는 11bit max
      //  .freq_hz = 5000,                      // frequency of PWM signal		기존 stm32는 20khz 로 구동함, 5khz
        .freq_hz = 60,                      // frequency of PWM signal		기존 stm32는 20khz 로 구동함, 20khz
        .speed_mode = LEDC_LOW_SPEED_MODE,           // timer mode
        .timer_num = LEDC_TIMER_1,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel[2] = {
        {
            .channel    = LEDC_DIM1_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_DIM1_GPIO,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_1
        },
	};

	ledc_channel_config(&ledc_channel[0]);
//	ledc_channel_config(&ledc_channel[1]);
}

void buld_set_intensity (uint32_t duty)
{
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_DIM1_CHANNEL, duty);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_DIM1_CHANNEL);
}

void param_print (void)
{
	DPRINTF (g_dbgEn, "\n=================================================\n");
	DPRINTF (g_dbgEn, "=================================================\n");
}

uint8_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint8_t out_min, uint8_t out_max)
{
	uint8_t val = 0;

	val = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;	// map ()  arduino libray..

	return (val < 1) ? 0 : (val > 99) ? 100 : val;		// constrain ()  arduino library..
}

void timer_task (void *pvParameter)
{
//	static uint32_t i_duty = 0;
//	static uint32_t count = 0;

	while (1) {
		if ((GPIO_REG_READ (GPIO_OUT_REG) >> GPIO_OUTPUT_SIGNAL_LED) & 1U) {		// GPIO OUTPUT 으로 설정된 상태를 읽는 매크로
			gpio_set_level (GPIO_OUTPUT_SIGNAL_LED, LOW);
		}
		else {
			gpio_set_level (GPIO_OUTPUT_SIGNAL_LED, HIGH);
		}

		/*
		buld_set_intensity (i_duty);

		i_duty += 100;
		if (i_duty > 2000) {
			i_duty =0;
		}
		*/

		/*
		if (count == 10) {
			struct set_param *param = (struct set_param *)malloc (sizeof(struct set_param));
			param->mode = MODE_BT_ON;
			param->time = 11;

			uartTxData_fixedFrameMakingWithTransmit (CMD_PARAM_SET, param);

			free (param);
		}
		else if (count == 20) {
			struct set_param *param = (struct set_param *)malloc (sizeof(struct set_param));
			param->mode = MODE_BT_OFF;
			param->time = 11;

			uartTxData_fixedFrameMakingWithTransmit (CMD_PARAM_SET, param);

			free (param);
			count = 0;
		}
		count++;
		*/

		vTaskDelay (1000 / portTICK_RATE_MS);
	}

    vTaskDelete(NULL);
}
		
void gpio_init (void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void app_main(void)
{
	g_dbgEn = 1;
    DPRINTF (g_dbgEn, "SDK version:%s\n", esp_get_idf_version ());
	DPRINTF (g_dbgEn, "SPACE-S_ESP32_v%s\n", FIRMWARE_VERSION);
	DPRINTF (g_dbgEn, "Build Date: %s %s\n", __DATE__, __TIME__);

	gpio_init ();
	gpio_set_level (GPIO_OUTPUT_SIGNAL_LED, LOW);

    ESP_ERROR_CHECK(nvs_flash_init ());
	ESP_ERROR_CHECK (nvs_open ("info_param", NVS_READWRITE, &handle1));

PARAM_RELOAD:
	if (infoparam_read () != ESP_OK) {
		ESP_LOGI (TAG, "NVS Eraseall");
		ESP_ERROR_CHECK (nvs_erase_all (handle1));		// all zero???
		ESP_LOGI (TAG, "System init status");

		if (infoparam_init () == ESP_OK) {
			goto PARAM_RELOAD;
		}
	}
	else {
		ESP_LOGI (TAG, "NVS get infoParam");
		param_print ();
		pwm_init ();

//		buld_set_intensity (300);
//		buld_set_intensity (700);
//		buld_set_intensity (1400);
//		buld_set_intensity (2045);

		if (xTaskCreate (&timer_task, "timer_task", 1024 * 2, NULL, 4, NULL) != pdPASS) {
			ESP_LOGE (TAG, "create timer_mode task failed");
		}

		uart_event_init ();
	}
}
