/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LASER_TIMER_H
#define __LASER_TIMER_H

#include "stm32f4xx.h"
void init_timer7( uint16_t DelayLazerOn );
void StopTimer7( void );
void init_timer5( uint16_t DelayLazerOn );
void StopTimer5( void );

#endif /* __LASER_TIMER_H */
//******************************************************************************
