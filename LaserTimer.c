#include "discoveryf4utils.h"
#include "LaserTimer.h"
#include "set_xy.h"

//volatile uint8_t LaserStatusGlobe = 1;

extern uint8_t LaserOnGlobe;
extern uint8_t LaserOffGlobe;

//******************************************************************************
void init_timer7(uint16_t DelayLazerOn )
{
	static TIM_TimeBaseInitTypeDef		base_timer;
	static uint8_t										TimerIsInit = 0;
	
	if( TimerIsInit == 0)
	{
		TimerIsInit = 1;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
		TIM_TimeBaseStructInit(&base_timer);
		base_timer.TIM_Prescaler = 42 - 1;
		base_timer.TIM_Period = DelayLazerOn; // 1 mSec
		TIM_TimeBaseInit(TIM7, &base_timer);
		NVIC_EnableIRQ(TIM7_IRQn);
		TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
		//TIM_SetCounter(TIM7,0);
		TIM_Cmd(TIM7, ENABLE);
	}
	else
	{
		base_timer.TIM_Period = DelayLazerOn;
		//TIM_TimeBaseInit(TIM7, &base_timer);
		//TIM_SetCounter(TIM7,0);
		//TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
		TIM_Cmd(TIM7, ENABLE);
	}
}

void StopTimer7( void )
{
	TIM_Cmd(TIM7, DISABLE);
}

void TIM7_IRQHandler()
{
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
	{
		TIM_Cmd(TIM7, DISABLE);
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
		//TIM_Cmd(TIM7, DISABLE);

		//STM_EVAL_LEDToggle(LED_RED);
		//STM_EVAL_LEDOn(LED_RED);
		
		//NVIC_DisableIRQ(TIM7_IRQn);
		//STM_EVAL_LEDToggle(LED_BLUE);

		//StopTimer7();
		//GPIO_ToggleBits(TTL_GPIO, TTL);
		//printf("777_LaserStatusGlobe:        %u\n",LaserStatusGlobe);
		if ((LaserOnGlobe) == 1) 
			{
				GPIO_SetBits(TTL_GPIO, TTL);
				LaserOnGlobe = 0;
			}
		else ;//if (LaserStatusGlobe == 2) GPIO_ResetBits(TTL_GPIO, TTL);
	}
}


void init_timer5(uint16_t DelayLazerOff )
{
	static TIM_TimeBaseInitTypeDef		base_timer;
	static uint8_t										TimerIsInit5 = 0;
	
	if( TimerIsInit5 == 0)
	{
		TimerIsInit5 = 1;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
		TIM_TimeBaseStructInit(&base_timer);
		base_timer.TIM_Prescaler = 42 - 1;
		base_timer.TIM_Period = DelayLazerOff; // 1 mSec
		TIM_TimeBaseInit(TIM5, &base_timer);
		NVIC_EnableIRQ(TIM5_IRQn);
		TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
		//TIM_SetCounter(TIM7,0);
		TIM_Cmd(TIM5, ENABLE);
	}
	else
	{
		base_timer.TIM_Period = DelayLazerOff;
		//TIM_TimeBaseInit(TIM7, &base_timer);
		//TIM_SetCounter(TIM7,0);
		//TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
		TIM_Cmd(TIM5, ENABLE);
	}
}

void StopTimer5( void )
{
	TIM_Cmd(TIM5, DISABLE);
}

void TIM5_IRQHandler()
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
	{
		TIM_Cmd(TIM5, DISABLE);
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
		//TIM_Cmd(TIM7, DISABLE);

		//STM_EVAL_LEDToggle(LED_RED);
		//STM_EVAL_LEDOn(LED_RED);
		
		//NVIC_DisableIRQ(TIM7_IRQn);
		//STM_EVAL_LEDToggle(LED_ORANGE);
		//STM_EVAL_LEDToggle(LED_GREEN);
		//StopTimer7();
		//GPIO_ToggleBits(TTL_GPIO, TTL);
		//printf("555_LaserStatusGlobe:        %u\n",LaserStatusGlobe);
		if ((LaserOffGlobe) == 1)
		{
			GPIO_ResetBits(TTL_GPIO, TTL);
			LaserOffGlobe = 0;
		}
		else ;
	}
}
//******************************************************************************
