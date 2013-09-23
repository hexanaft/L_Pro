//******************************************************************************
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SET_XY_H
#define __SET_XY_H

#include "stm32f4xx.h"
#include <stdio.h>

#define TTL 		GPIO_Pin_15
#define TTL_GPIO	GPIOA

#define DAC_SCLK	GPIO_Pin_1
#define DAC_SYNC	GPIO_Pin_2
#define DAC_DIN		GPIO_Pin_3
#define DAC_DIN2	GPIO_Pin_8
#define DAC_GPIO	GPIOA
#define RCC_DAC_GPIO 		RCC_AHB1Periph_GPIOA
#define RCC_DAC_GPIO_CMD	RCC_AHB1PeriphClockCmd

void initialization_set_xy( void );
void delayLas(uint32_t counter3);
void delayXY(uint32_t counter2);
void setXY(uint16_t valueX, uint16_t valueY);

#endif /* __SET_XY_H */
//******************************************************************************
