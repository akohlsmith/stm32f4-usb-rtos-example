#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "main.h"

#define DWT_CTRL    (*(volatile uint32_t *)0xe0001000)
#define CYCCNTENA   (1 << 0)
#define DWT_CYCCNT  (*(volatile uint32_t *)0xe0001004)


/* cycles per microsecond */
static uint32_t us_ticks;
static volatile uint32_t ticks = 0;

void delayInit(void)
{
#if 0
	RCC_ClocksTypeDef clocks;

	/* compute the number of system clocks per microsecond */
	RCC_GetClocksFreq(&clocks);
	us_ticks = clocks.SYSCLK_Frequency / 1000000;
#endif
	us_ticks = 168000000 / 1000000;

	/* turn on access to the DWT registers */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

	/* enable the CPU cycle counter */
	DWT_CTRL |= CYCCNTENA;

	/* enable systick here, too */
//	SysTick_Config(SystemCoreClock / 1000);
}

void delayWaitus(uint32_t uS)
{
	static int first = 1;
	uint32_t elapsed = 0;
	uint32_t last_count = DWT_CYCCNT;

	if (first) {
		delayInit();
		first = 0;
	}

	for (;;) {
		uint32_t current_count = DWT_CYCCNT;
		uint32_t elapsed_uS;

		/* measure the time elapsed since the last time we checked */
		elapsed += current_count - last_count;
		last_count = current_count;

		/* convert to microseconds */
		elapsed_uS = elapsed / us_ticks;
		if (elapsed_uS >= uS) {
			break;
		}

		/* reduce the delay by the elapsed time */
		uS -= elapsed_uS;

		/* keep fractional microseconds for the next iteration */
		elapsed %= us_ticks;
	}
}


void delayWaitms(uint32_t mS)
{
	while (mS--) {
		delayWaitus(1000);
	}
}


void HAL_Delay(uint32_t Delay)
{
	delayWaitms(Delay);
}


/* dumps a blob data in hex to the console */
void dump(uint8_t *buf, int len)
{
	int i, offset;
	uint8_t *p;

	offset = 0;
	while (len) {
		printf("\t%3d:", offset);
		for (i=0, p=buf; i<((len < 16) ? len : 16); i++) {
			printf(" %02x", *p++);
		}

		printf("        ");
		for (i=0, p=buf; i<((len < 16) ? len : 16); i++) {
			printf("%c", (isprint(*p)) ? *p : '.');
			p++;
		}

		printf("\n");
		offset += i;
		len -= i;
	};

	printf("\n");
}
