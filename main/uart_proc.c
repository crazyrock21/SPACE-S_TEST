#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_sleep.h"
#include "cJSON.h"
#include "time.h"

#include "user_config.h"
#include "uart_proc.h"


void uartTxData_fixedFrameMakingWithTransmit (CMD_KIND cmdKind, struct set_param *data)
{
	uartFrameFixedData *uartData = (uartFrameFixedData *)malloc (FIXED_UART_LEN);

	uartData->stx = UART_STX;
	uartData->len = sizeof(struct set_param);
	uartData->cmd_type = UART_REQ;
	uartData->cmd = cmdKind;
	memcpy (&uartData->param, data, sizeof(struct set_param));
	uartData->cs = uartData->len + uartData->cmd_type + uartData->cmd + data->mode + data->time + data->sonic_power + data->speaker_power + data->sitz_power;
	uartData->etx = UART_ETX;
		
	uartTx_operation ((uint8_t *)uartData, uartData->len + FIXED_UART_LEN);

	printf (">>>>>>>>>> WIFI to DEVICE >>>>>>>>>>>\n");
	printf ("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", 
			uartData->stx, uartData->len, uartData->cmd_type, uartData->cmd, 
			uartData->param.mode, uartData->param.time, uartData->param.sonic_power, uartData->param.speaker_power, uartData->param.sitz_power, uartData->cs, uartData->etx);
	printf (">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	free (uartData);
}

					
void responseFrameMakingWithTransmit (uint8_t *dataBuf)
{
	*(dataBuf + P_UART_CMD_TYPE) = UART_RES;

	*(dataBuf + 9) = 0;

	for (uint8_t i = 1; i < 9; i++) {
		*(dataBuf + 9) += *(dataBuf + i);
	}

	printf (">>>>>>>>>> WIFI to DEVICE >>>>>>>>>>>\n");
	for (uint8_t i = 0; i < 11; i++) {
		printf ("%02x ", *(dataBuf + i));
	}
	printf ("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	uartTx_operation (dataBuf, 11);
}

void uartRxData_frameParsing (UART_FIFO_BUF *uartRxBuf)
{
	uint8_t *dataBuf = (uint8_t *)malloc (BUF_SIZE);
	uint16_t i = 0;
	uint8_t checkSum = 0;
	uint16_t dataBufCnt = 0;
	uint8_t uartDataLen = 0;
	uint8_t backupCount = 0;

	memcpy (dataBuf, uartRxBuf->buf, uartRxBuf->count);
	backupCount = uartRxBuf->count;

	printf ("<<<<<<<<<< DEVICE to WIFI <<<<<<<<<<<\n");

	for (i = 0; i < uartRxBuf->count; i++)
		printf ("%02x ", *(dataBuf + i));

	do {
		if (*(dataBuf + dataBufCnt + P_UART_STX) == UART_STX) {
			uartDataLen = *(dataBuf + dataBufCnt + P_UART_DATA_LEN);

			checkSum = 0;
			for (i = 0; i < uartDataLen + 3; i++) {
				checkSum += *(dataBuf + dataBufCnt + P_UART_DATA_LEN + i);
			}

			printf ("   Checksum calc: %02x\n", checkSum);
			printf ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");

			if (checkSum == *(dataBuf + dataBufCnt + uartDataLen + 4) && UART_ETX == *(dataBuf + dataBufCnt + uartDataLen + 5)) {
				if (uartRxBuf->count < uartDataLen + 6) {
					uartRxBuf->count = backupCount;
					break;
				}
				if (*(dataBuf + dataBufCnt + P_UART_CMD_TYPE) == UART_REQ) {
					printf ("MCU Request\n");

					memcpy (&g_param, dataBuf + dataBufCnt + P_UART_DATA, sizeof(struct set_param));
					printf ("Mode %d, Time %d, Sonic %d, speaker %d, sitz %d\n", 
							g_param.mode, g_param.time, g_param.sonic_power, g_param.speaker_power, g_param.sitz_power);

					/* ==== application user code start ==== */
					switch (*(dataBuf + dataBufCnt + P_UART_CMD)) {
						case 1:
							{
								printf ("start\n");
							}
							break;
						case 2:
							{
								printf ("stop\n");
							}
							break;
						case 3:
							printf ("param set\n");
							break;
						case 4:
							printf ("test set\n");
							break;
					}
					/* ==== application user code end ==== */
		
					responseFrameMakingWithTransmit (dataBuf);
				}
				else if (*(dataBuf + dataBufCnt + P_UART_CMD_TYPE) == UART_RES) {
					printf ("MCU Response\n");

					switch (*(dataBuf + dataBufCnt + P_UART_CMD)) {
						case 3:
							{
								printf ("param set resp\n");
								switch (*(dataBuf + dataBufCnt + P_UART_DATA)) {
									case 5:
										printf ("bt on resp\n");
										break;
									case 6:
										printf ("bt off resp\n");
										break;
								}
							}
							break;
					}
					printf ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
				}

				dataBufCnt += uartDataLen + 6;
				uartRxBuf->count -= uartDataLen + 6;
			}
			else {
				printf ("\nChecksum error\n");
				if (uartRxBuf->count > 0) {
					uartRxBuf->count--;
					dataBufCnt++;
				}
			}
		}
		else {
			printf ("\nSTX or ETX error\n");
			if (uartRxBuf->count > 0) {
				uartRxBuf->count--;
				dataBufCnt++;
			}
		}
	} while (uartRxBuf->count > 0);

	free (dataBuf);
	printf ("dataBuf memory free %d\n", uartRxBuf->count);
}

void uartTx_operation (uint8_t *dataBuf, size_t dataLength)
{
	uart_write_bytes (UART_NUM_1, (const char *)dataBuf, dataLength);
}


