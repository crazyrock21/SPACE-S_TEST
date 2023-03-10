#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define	FIRMWARE_VERSION			"0.1.0"
//#define	DEBUG_APP_BUILD

#if 0
#define	DEBUG_EN 
#ifdef	DEBUG_EN
#define	DPRINTF(fmt, args...) fprintf(stdout, fmt, ##args)
#else
#define	DPRINTF(fmt, args...)
#endif
#endif

#define DPRINTF(level, fmt, args...)\
	if (level > 0) \
	fprintf(stdout, fmt, ##args)

#define DPUTCHAR(level, fmt)\
	if (level > 0) \
	putchar(fmt)

#define GPIO_OUTPUT_SIGNAL_LED 	   	4
#define GPIO_OUTPUT_PIN_SEL  		((1ULL<<GPIO_OUTPUT_SIGNAL_LED))

#define LEDC_DIM1_GPIO    		  	(21)
#define LEDC_DIM1_CHANNEL 		  	LEDC_CHANNEL_0

typedef enum {
	LOW,
	HIGH,
} HIGH_LOW;

typedef struct {
	uint8_t status;
	uint8_t pairingCompleteStatus;
	uint8_t deviceId[64];
	uint8_t apSsid[33];
	uint8_t apPassword[65];
	uint8_t endPoint[64];
	uint8_t productName[32];
	uint8_t modelName[32];
	uint8_t macAddr[32];
	uint8_t macAddr2[32];
	uint8_t reserved[12];		// aes128 enc/dec 를 위해 modular 16으로 맞춤
} INFO_PARAM;

INFO_PARAM g_infoParam;

uint8_t g_dbgEn;

int8_t infoparam_write (void);
int8_t infoparam_read (void);
int8_t infoparam_init (void);
int8_t infoparam_blob_init (void);

#endif
