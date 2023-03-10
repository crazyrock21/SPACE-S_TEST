/* UART asynchronous example, that uses separate RX and TX tasks

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "string.h"

#include "uart_proc.h"
#include "user_config.h"

static const char *TAG = "uart_rx";

static void uart_rx_task()
{
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
	memset (dtmp, 0, RD_BUF_SIZE);

    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, dtmp, BUF_SIZE, 10 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            ESP_LOGI(TAG, "Read %d bytes", rxBytes);
        //    ESP_LOG_BUFFER_HEXDUMP(TAG, dtmp, rxBytes, ESP_LOG_INFO);

			memcpy (uartFifoBuf.buf + uartFifoBuf.count, dtmp, rxBytes);
			uartFifoBuf.count += rxBytes;

			uartRxData_frameParsing (&uartFifoBuf);
        }
    }
    free(dtmp);
	dtmp = NULL;
	vTaskDelete(NULL);
}

void uart_baud_set (void)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK (uart_param_config(UART_NUM_1, &uart_config));
}

void uart_event_init (void)
{
//    esp_log_level_set(TAG, ESP_LOG_INFO);

	uart_baud_set ();

    //Set UART log level
 //   esp_log_level_set(TAG, ESP_LOG_INFO);
    //Set UART pins (using UART0 default pins ie no changes.)
    ESP_ERROR_CHECK (uart_set_pin(UART_NUM_1, 23, 22, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    //Install UART driver, and get the queue.
    ESP_ERROR_CHECK (uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0));

    //Create a task to handler UART event from ISR
    xTaskCreate(&uart_rx_task, "uart_rx_task", 1024 * 3, NULL, 11, NULL);
}
