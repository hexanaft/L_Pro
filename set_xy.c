#include "set_xy.h"

void initialization_set_xy( void )
{
	GPIO_InitTypeDef	port;
	RCC_DAC_GPIO_CMD(RCC_DAC_GPIO, ENABLE);
		
	port.GPIO_Mode = GPIO_Mode_OUT;
	port.GPIO_Pin = DAC_SCLK | DAC_SYNC | DAC_DIN | DAC_DIN2 | TTL;
	port.GPIO_Speed = GPIO_Speed_50MHz;
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
	uint16_t mask = 0x8000, buf_func_X = 0, buf_func_Y = 0;
	int count = 0;	

	buf_func_X = valueX;
	buf_func_Y = valueY;
	GPIO_ResetBits(DAC_GPIO, DAC_SYNC | DAC_DIN | DAC_DIN2);
	//delay(0xffff);
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
		if ((buf_func_X & mask) == 0)	GPIO_ResetBits(DAC_GPIO, DAC_DIN);
		else GPIO_SetBits(DAC_GPIO, DAC_DIN);
		//delay(0xffff);
		if ((buf_func_Y & mask) == 0)	GPIO_ResetBits(DAC_GPIO, DAC_DIN2);
		else GPIO_SetBits(DAC_GPIO, DAC_DIN2);
		//delay(0xffff);
		mask = mask >> 1;
		GPIO_SetBits(DAC_GPIO, DAC_SCLK);
		count++;
		//delay(0xffff);
		GPIO_ResetBits(DAC_GPIO, DAC_SCLK);
		//delay(0xffff);
	}
	GPIO_SetBits(DAC_GPIO, DAC_SYNC);
	//delay(0xffff);
}

