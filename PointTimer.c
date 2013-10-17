
#include "PointTimer.h"

//******************************************************************************
void init_timer6( uint16_t DelayBetweenPoints )
{
	TIM_TimeBaseInitTypeDef base_timer;
	
	/* Не забываем затактировать таймер, частота таймера = APB2 * 2 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	/* Инициализируем базовый таймер: 84000000 тактовая частота шины таймера
	* делитель 42000, период 500 мс.
	* Другие параметры структуры TIM_TimeBaseInitTypeDef
	* не имеют смысла для базовых таймеров.
	*/
	TIM_TimeBaseStructInit(&base_timer);
	/* Делитель учитывается как TIM_Prescaler + 1, поэтому отнимаем 1 */
	base_timer.TIM_Prescaler = 84 - 1; //- 1; зачем -1 ???  							// 84000000 /840 = 100000
	base_timer.TIM_Period = DelayBetweenPoints*0.55;//*2;	// 1 секунда при 8400 делителе и 10000 таймере. // 100000 /200 = 500 раз в сек будет прерывание
	TIM_TimeBaseInit(TIM6, &base_timer);

	/* Разрешаем прерывание по обновлению (в данном случае -
	* по переполнению) счётчика таймера TIM6.
	*/
	NVIC_EnableIRQ(TIM6_DAC_IRQn);
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	/* Включаем таймер */
	TIM_Cmd(TIM6, ENABLE);

	/* Разрешаем обработку прерывания по переполнению счётчика
	* таймера TIM6. Так получилось, что это же прерывание
	* отвечает и за опустошение ЦАП.
	*/
	
}

void StopTimer6( void )
{
	//NVIC_DisableIRQ(TIM6_DAC_IRQn);
	TIM_Cmd(TIM6, DISABLE);
	//TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	STM_EVAL_LEDToggle(LED_ORANGE);
	//delayFr(0xfff);
	//LaserOff( Head_ilda.DelayLazerOff );
}


void SetPointRandom( void )
{
	uint32_t value=0;
	uint16_t valueX=0;
	uint16_t valueY=0;
	
	value = getRandom(); // 10 bits = 0...1024 mSec
	valueX = (uint16_t) value;
	valueY = (uint16_t) (value>>16);
	setXY(valueX, valueY);
	//printf("%u-%u\n",valueX,valueY );
}

void TIM6_DAC_IRQHandler( void )
{
	/* Так как этот обработчик вызывается и для ЦАП, нужно проверять,
	* произошло ли прерывание по переполнению счётчика таймера TIM6.
	*/
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
	{
		/* Очищаем бит обрабатываемого прерывания */
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
		
		SetPointFromFrame();
		STM_EVAL_LEDToggle(LED_RED);
	
// 		if( STM_EVAL_PBGetState(BUTTON_USER) == 1 )
// 		{
// 			/* Инвертируем состояние светодиодов */
// 			//STM_EVAL_LEDToggle(LED_ORANGE);
// 			//SetPointRandom();
// 			
// 		}
// 		else
// 		{
// 			SetPointFromFrame();
// 			STM_EVAL_LEDToggle(LED_RED);
// 		}
		
	}
}
