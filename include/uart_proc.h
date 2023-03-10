#ifndef __IDK_UART_H__ 
#define __IDK_UART_H__

#define BUF_SIZE (256)
#define RD_BUF_SIZE (BUF_SIZE)

/* FIELD VALUE */
#define	UART_STX					0x02
#define UART_ETX					0x03
#define	UART_REQ					0x10			
#define	UART_RES					0x11			
#define	UART_NOTI					0x12			
#define	FIXED_UART_LEN				0x06

/* FIELD ADDRESS */
#define	P_UART_STX					0
#define P_UART_DATA_LEN				1
#define P_UART_CMD_TYPE				2
#define P_UART_CMD					3
#define	P_UART_DATA					4

typedef enum {
	MODE_AUTO1 = 1,
	MODE_AUTO2,
	MODE_ISCHIUM,
	MODE_PERINEUM,
	MODE_BT_ON,
	MODE_BT_OFF,
} RUN_MODE;

typedef enum {
	CMD_START = 1,
	CMD_STOP,
	CMD_PARAM_SET,
	CMD_TEST_SET,
}CMD_KIND;

typedef struct {
	uint16_t count;
	uint8_t buf[BUF_SIZE];
} UART_FIFO_BUF;

UART_FIFO_BUF uartFifoBuf;
struct set_param {
	uint8_t mode;
	uint8_t time;
	uint8_t sonic_power;
	uint8_t speaker_power;
	uint8_t sitz_power;
};

struct set_param g_param;

typedef struct {
	uint8_t stx;
	uint8_t len;
	uint8_t cmd_type;
	uint8_t cmd;
	struct set_param param;
	uint8_t cs;
	uint8_t etx;
} uartFrameFixedData;

void uartTx_operation (uint8_t *dataBuf, size_t dataLength);
void uartRxData_frameParsing (UART_FIFO_BUF *uartRxBuf);
void uart_baud_set (void);
void uartTxData_fixedFrameMakingWithTransmit (CMD_KIND cmdKind, struct set_param *);
void uart_event_init (void);

#endif
