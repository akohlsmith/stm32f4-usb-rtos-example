#include <stdbool.h>

#include "cmsis_os.h"
#include "main.h"

#include <usbd_core.h>
#include <usbd_cdc.h>
#include "usbd_cdc_interface.h"
#include "usbd_desc.h"

osThreadId miscThreadHandle;

USBD_HandleTypeDef USBD_Device;
extern USBD_DescriptorsTypeDef VCP_Desc;

static void miscThread(void const *argument);

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
void Error_Handler(void)
{
	while(1) ;
}


#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *	where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	/*
	 * User can add his own implementation to report the file name and line number,
	 * ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line)
	 */

	while (1) ;
}
#endif

/**
 * @brief  System Clock Configuration
 *	The system Clock is configured as follow :
 *		System Clock source	= PLL (HSE)
 *		SYSCLK(Hz)		= 168000000
 *		HCLK(Hz)		= 168000000
 *		AHB Prescaler		= 1
 *		APB1 Prescaler		= 4
 *		APB2 Prescaler		= 2
 *		HSE Frequency(Hz)	= 8000000
 *		PLL_M			= 8
 *		PLL_N			= 336
 *		PLL_P			= 2
 *		PLL_Q			= 7
 *		VDD(V)			= 3.3
 *		Main reg. output	= Scale1 mode
 *		Flash Latency(WS)	= 5
 * @param  None
 * @retval None
 */
static void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	/*
	 * The voltage scaling allows optimizing the power consumption when the device is
	 * clocked below the maximum system frequency, to update the voltage scaling value
	 * regarding system frequency refer to product datasheet.
	 */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}

	/* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
	if (HAL_GetREVID() == 0x1001) {
		/* Enable the Flash prefetch */
		__HAL_FLASH_PREFETCH_BUFFER_ENABLE();
	}
}


void usb_init(void);
static void miscThread(void const *argument)
{
	(void)argument;

	usb_init();

	while (1) {
		osDelay(1500);
		printf("Foo (%lu)\n", osKernelSysTick());
	};
}


void usb_init(void)
{
	USBD_Init(&USBD_Device, &VCP_Desc, 0);
	USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);
	USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);
	USBD_Start(&USBD_Device);
}


int main(void)
{
	HAL_Init();
	SystemClock_Config();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	usb_init();
	setbuf(stdout, NULL);
	printf("Foo\r\n");

	osThreadDef(MISC, miscThread, osPriorityNormal, 0, 1024);

	miscThreadHandle = osThreadCreate(osThread(MISC), NULL);
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */
	while (1) ;
}
