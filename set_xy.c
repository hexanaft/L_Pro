#include "set_xy.h"

void initialization_set_xy( void )
{
	GPIO_InitTypeDef port;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
		
	port.GPIO_Mode = GPIO_Mode_OUT;
	port.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_8 | GPIO_Pin_15;
	port.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &port);
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
	GPIO_ResetBits(GPIOA, SYNC | DIN | DIN2);
	//delay(0xffff);
	/*
	while (count <= 7)
	{
		GPIO_SetBits(GPIOA, SCLK);
		//delay(0xffff);
		GPIO_ResetBits(GPIOA, SCLK);
		//delay(0xffff);
		count++;
	}
	*/
	while (count <= 15)
	{
		if ((buf_func_X & mask) == 0)	GPIO_ResetBits(GPIOA, DIN);
		else GPIO_SetBits(GPIOA, DIN);
		//delay(0xffff);
		if ((buf_func_Y & mask) == 0)	GPIO_ResetBits(GPIOA, DIN2);
		else GPIO_SetBits(GPIOA, DIN2);
		//delay(0xffff);
		mask = mask >> 1;
		GPIO_SetBits(GPIOA, SCLK);
		count++;
		//delay(0xffff);
		GPIO_ResetBits(GPIOA, SCLK);
		//delay(0xffff);
	}
	GPIO_SetBits(GPIOA, SYNC);
	//delay(0xffff);
}

