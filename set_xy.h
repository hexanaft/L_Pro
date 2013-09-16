//******************************************************************************
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SET_XY_H
#define __SET_XY_H

#include "stm32f4xx.h"
#include <stdio.h>

#define TTL GPIO_Pin_15

#define SCLK GPIO_Pin_1
#define SYNC GPIO_Pin_2
#define DIN GPIO_Pin_3
#define DIN2 GPIO_Pin_8

void initialization_set_xy( void );
void delayLas(uint32_t counter3);
void delayXY(uint32_t counter2);
void setXY(uint16_t valueX, uint16_t valueY);

#endif /* __SET_XY_H */
//******************************************************************************
