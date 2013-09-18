//******************************************************************************
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RANDOM_H
#define __RANDOM_H

#include "stm32f4xx.h"
#include "stm32f4xx_rng.h"

void RNG_Config( void );
uint32_t getRandom( void );

#endif /* __RANDOM_H */
//******************************************************************************
