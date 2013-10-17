//******************************************************************************
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __POINT_TIMER_H
#define __POINT_TIMER_H

#include <inttypes.h>
#include "stm32f4xx.h"
#include "discoveryf4utils.h"
#include "Random.h"
#include "set_xy.h"

extern void SetPointFromFrame( void );

void init_timer6( uint16_t DelayBetweenPoints );
void StopTimer6( void );
void SetPointRandom( void );
void TIM6_DAC_IRQHandler( void );

#endif /* __POINT_TIMER_H */
//******************************************************************************
