#include "set_xy.h"

void LaserOn( void )
{
	delayLas(25000);
	GPIO_SetBits(TTL_GPIO, TTL);
}

void LaserOff( void )
{
	delayLas(25000);
	GPIO_ResetBits(TTL_GPIO, TTL);
}

void initialization_set_xy( void )
{
	GPIO_InitTypeDef	port;
	RCC_DAC_GPIO_CMD(RCC_DAC_GPIO, ENABLE);
		
	port.GPIO_Mode = GPIO_Mode_OUT;
	port.GPIO_Pin = DAC_SCLK | DAC_SYNC | DAC_DIN | DAC_DIN2 | TTL;
	port.GPIO_Speed = GPIO_Speed_100MHz;//GPIO_Speed_50MHz;
	port.GPIO_OType = GPIO_OType_PP;
	port.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(DAC_GPIO, &port);
}

void delayLas(uint32_t counter3)
{
	while (counter3--)
	{
		counter3++;
		counter3--;
		counter3++;
		counter3--;
		counter3++;
		counter3--;
		counter3++;
		counter3--;
	}
}

void delayXY(uint32_t counter2)
{
	while (counter2--)
	{}
}

void setXY(uint16_t valueX, uint16_t valueY)
{
	uint16_t mask = 0x8000;
	int count = 0;	

	GPIO_ResetBits(DAC_GPIO, DAC_DIN);
	GPIO_ResetBits(DAC_GPIO, DAC_DIN2);
	GPIO_ResetBits(DAC_GPIO, DAC_SYNC);
	/*
	while (count <= 7)
	{
		GPIO_SetBits(DAC_GPIO, DAC_SCLK);
		//delay(0xffff);
		GPIO_ResetBits(DAC_GPIO, DAC_SCLK);
		//delay(0xffff);
		count++;
	}
	*/
	while (count <= 15)
	{
		if ((valueX & mask) == 0)	GPIO_ResetBits(DAC_GPIO, DAC_DIN);
		else GPIO_SetBits(DAC_GPIO, DAC_DIN);

		if ((valueY & mask) == 0)	GPIO_ResetBits(DAC_GPIO, DAC_DIN2);
		else GPIO_SetBits(DAC_GPIO, DAC_DIN2);

		mask = mask >> 1;
		count++;
		
		GPIO_SetBits(DAC_GPIO, DAC_SCLK);
		//delayXY(0x0f);
	
		GPIO_ResetBits(DAC_GPIO, DAC_SCLK);
		//delayXY(0x0f);
	}
	GPIO_SetBits(DAC_GPIO, DAC_SYNC);

}

