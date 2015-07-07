#ifndef __USBD_CDC_IF_H
#define __USBD_CDC_IF_H

#include "usbd_cdc.h"

/* Definition for TIMx clock resources */
#define TIMx                             TIM4
#define TIMx_CLK_ENABLE                  __HAL_RCC_TIM4_CLK_ENABLE
#define TIMx_FORCE_RESET()               __HAL_RCC_USART1_FORCE_RESET()
#define TIMx_RELEASE_RESET()             __HAL_RCC_USART1_RELEASE_RESET()

/* Definition for TIMx's NVIC */
#define TIMx_IRQn                        TIM4_IRQn
#define TIMx_IRQHandler                  TIM4_IRQHandler

/* Periodically, the state of the buffer "UserTxBuffer" is checked. The period depends on CDC_POLLING_INTERVAL */
#define CDC_POLLING_INTERVAL             5 /* in ms. The max is 65 and the min is 1 */

extern USBD_CDC_ItfTypeDef  USBD_CDC_fops;

#endif /* __USBD_CDC_IF_H */
