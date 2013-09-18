
#include "Random.h"

/**
  * @brief  RNG configuration
  * @param  None
  * @retval None
  *    Get a 32bit Random number     
  *    random32bit = RNG_GetRandomNumber();
  */

void RNG_Config( void )
{  
 /* Enable RNG clock source */
  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);

  /* RNG Peripheral enable */
  RNG_Cmd(ENABLE);
}

uint32_t getRandom( void )
{
	/* Wait until one RNG number is ready */
	while(RNG_GetFlagStatus(RNG_FLAG_DRDY)== RESET){};
	return RNG_GetRandomNumber();
}
