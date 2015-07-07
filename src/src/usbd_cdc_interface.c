#include <cmsis_os.h>
#include <stdbool.h>
#include <usbd_cdc.h>
#include "main.h"
#include "usbd_cdc_interface.h"

#define APP_RX_DATA_SIZE  2048
#define APP_TX_DATA_SIZE  2048

USBD_CDC_LineCodingTypeDef LineCoding = {
	115200, /* baud rate*/
	0x00,   /* stop bits-1*/
	0x00,   /* parity - none*/
	0x08    /* nb. of bits 8*/
};

uint8_t UserRxBuffer[APP_RX_DATA_SIZE];/* Received Data over USB are stored in this buffer */
uint8_t UserTxBuffer[APP_TX_DATA_SIZE];/* Received Data over UART (CDC interface) are stored in this buffer */
uint32_t BuffLength;
uint32_t UserTxBufPtrIn = 0;/* Increment this pointer or roll it back to start address when data are received over USART */
uint32_t UserTxBufPtrOut = 0; /* Increment this pointer or roll it back to start address when data are sent over USB */

UART_HandleTypeDef UartHandle;
TIM_HandleTypeDef  TimHandle;

extern USBD_HandleTypeDef  USBD_Device;

static int8_t CDC_Itf_Init(void);
static int8_t CDC_Itf_DeInit(void);
static int8_t CDC_Itf_Control(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Itf_Receive(uint8_t* pbuf, uint32_t *Len);

static void ComPort_Config(void);
static void TIM_Config(void);

USBD_CDC_ItfTypeDef USBD_CDC_fops = {
	CDC_Itf_Init,
	CDC_Itf_DeInit,
	CDC_Itf_Control,
	CDC_Itf_Receive
};


/* initializes the CDC media low level layer */
static int8_t CDC_Itf_Init(void)
{
	TIM_Config();

	if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK) {
		Error_Handler();
	}

	USBD_CDC_SetTxBuffer(&USBD_Device, UserTxBuffer, 0);
	USBD_CDC_SetRxBuffer(&USBD_Device, UserRxBuffer);

	return USBD_OK;
}

static int8_t CDC_Itf_DeInit(void)
{
	return (USBD_OK);
}

/*
 * handle the CDC class requests
 * cmd = command code
 * buf = command parameters
 * len = length of parameter data
 */
static int8_t CDC_Itf_Control (uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
	switch (cmd) {
	case CDC_SEND_ENCAPSULATED_COMMAND:
		break;

	case CDC_GET_ENCAPSULATED_RESPONSE:
		break;

	case CDC_SET_COMM_FEATURE:
		break;

	case CDC_GET_COMM_FEATURE:
		break;

	case CDC_CLEAR_COMM_FEATURE:
		break;

	case CDC_SET_LINE_CODING:
		LineCoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) | (pbuf[2] << 16) | (pbuf[3] << 24));
		LineCoding.format     = pbuf[4];
		LineCoding.paritytype = pbuf[5];
		LineCoding.datatype   = pbuf[6];
		break;

	case CDC_GET_LINE_CODING:
		pbuf[0] = (uint8_t)(LineCoding.bitrate);
		pbuf[1] = (uint8_t)(LineCoding.bitrate >> 8);
		pbuf[2] = (uint8_t)(LineCoding.bitrate >> 16);
		pbuf[3] = (uint8_t)(LineCoding.bitrate >> 24);
		pbuf[4] = LineCoding.format;
		pbuf[5] = LineCoding.paritytype;
		pbuf[6] = LineCoding.datatype;
		break;

	case CDC_SET_CONTROL_LINE_STATE:
		break;

	case CDC_SEND_BREAK:
		break;

	default:
		break;
	}

	return USBD_OK;
}


/* timer callback */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	uint32_t buffptr;
	uint32_t buffsize;

	if (UserTxBufPtrOut != UserTxBufPtrIn) {
		if (UserTxBufPtrOut > UserTxBufPtrIn) {
			buffsize = APP_RX_DATA_SIZE - UserTxBufPtrOut;
		} else {
			buffsize = UserTxBufPtrIn - UserTxBufPtrOut;
		}

		buffptr = UserTxBufPtrOut;
		USBD_CDC_SetTxBuffer(&USBD_Device, (uint8_t*)&UserTxBuffer[buffptr], buffsize);

		if (USBD_CDC_TransmitPacket(&USBD_Device) == USBD_OK) {
			UserTxBufPtrOut += buffsize;
			if (UserTxBufPtrOut == APP_RX_DATA_SIZE) {
				UserTxBufPtrOut = 0;
			}
		}
	}
}


static bool cdc_fifo_put(const char val)
{
	int next;
	bool ret;

	/* calculate next head position to determine if the value will fit */
	next = UserTxBufPtrIn;
	++next;
	if (next == APP_RX_DATA_SIZE) {
		next = 0;
	}

	/* if the FIFO isn't full (next == tail), write the value to the FIFO and update head. */
	if (next != UserTxBufPtrOut) {
		*(uint8_t*)&UserTxBuffer[UserTxBufPtrIn] = val;
		UserTxBufPtrIn = next;
		ret = true;
	} else {
		ret = false;
	}

	return ret;
}

/* writes to the transmitter FIFO. Returns the number of bytes written */
static int cdc_write(const char *buf, int len)
{
	int nwritten;

	nwritten = 0;
	while (len) {
		if (cdc_fifo_put(*buf)) {
			++buf;
			++nwritten;
			--len;
		} else {
			break;
		}
	};

	return nwritten;
}


int __io_putchar(int ch)
{
	if (ch == '\n') {
		char c = '\r';

		cdc_write(&c, 1);
	}

	return cdc_write((char *)&ch, 1);
}


int __io_getchar(void)
{
	return 0;
}


int fputc(int ch, FILE *fp)
{
	return __io_putchar(ch);
}


/* called when data is sent to us from the host */
static int8_t CDC_Itf_Receive(uint8_t* Buf, uint32_t *Len)
{
	return (USBD_OK);
}

static void TIM_Config(void)
{
	TimHandle.Instance = TIM4;

	__HAL_RCC_TIM4_CLK_ENABLE();

	/* Initialize TIM4 peripheral as follow:
	 * Period = 10000 - 1
	 * Prescaler = ((SystemCoreClock/2)/10000) - 1
	 * ClockDivision = 0
	 * Counter direction = Up
	 */
	TimHandle.Init.Period = (CDC_POLLING_INTERVAL*1000) - 1;
	TimHandle.Init.Prescaler = 84-1;
	TimHandle.Init.ClockDivision = 0;
	TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;

	if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK) {
			Error_Handler();
	}

	HAL_NVIC_SetPriority(TIM4_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
}
