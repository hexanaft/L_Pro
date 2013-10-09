//******************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "discoveryf4utils.h"
#include "uart_to_printf.h"
#include "stm32f4_discovery_sdio_sd.h"
#include "sdio_debug.h"
#include "diskio.h"
#include "ff.h"
#include "ff_debug.h"
#include "set_xy.h"
#include "Random.h"
//******************************************************************************

//******************************************************************************
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "croutine.h"
//******************************************************************************

/**          STM32 SDIO Pin assignment
  *          =========================
  *          +-----------------------------------------------------------+
  *          |                     Pin assignment                        |
  *          +-----------------------------+---------------+-------------+
  *          |  STM32 SDIO Pins            |     SD        |    Pin      |
  *          +-----------------------------+---------------+-------------+	  ______________
  * PC.10    |      SDIO D2                |   D2          |    1        |	 /-Pin 1		|
  * PC.11    |      SDIO D3                |   D3          |    2        |	|-Pin 2			|
  * PD.02    |      SDIO CMD               |   CMD         |    3        |	|-Pin 3			|
  *          |                             |   GND         |    4 (0 V)  |	|-Pin 4			|
  *          |                             |   VCC         |    5 (3.3 V)|	|-Pin 5			|
  * PC.12    |      SDIO CLK               |   CLK         |    6        |	|-Pin 6			|
  *          |                             |   GND         |    7 (0 V)  |	|-Pin 7			|
  * PC.08    |      SDIO D0                |   D0          |    8        |	|-Pin 8			|
  * PC.09    |      SDIO D1                |   D1          |    9        |	|-Pin 9			|
  *          +-----------------------------+---------------+-------------+	+---------------+
  * PC.02	 to GND ( Card Detect )
****/

uint8_t	*Frame = 0;
volatile uint8_t	frp = 0;
uint32_t	*SizeOfFrame;
volatile uint8_t	nSizeOfFrame = 0;

#define STACK_SIZE_MIN	128	/* usStackDepth	- the stack size DEFINED IN WORDS.*/

void vReadSD(void *pvParameters);
void vLedBlinkRed(void *pvParameters);
void vLedBlinkGreen(void *pvParameters);
void vLedBlinkOrange(void *pvParameters);
void vSendUart(void *pvParameters);
void vOutToLaser(void *pvParameters);


void PrintFromMemFrameILDA( uint8_t* frame, uint32_t SizeOfFrame )
{
	uint32_t i=0;
	uint16_t j=0;
	uint16_t k=0;
	uint16_t NumOfFigures=0;
	uint16_t NumOfPoints=0;
	printf("Frame =\n");
	for(i=0;i<SizeOfFrame;i++)
		printf("%u:%X ",i,frame[i]);
	printf("\n");
//	i=0;
//	printf("KolvoByteVFreime = 0x%04X =%u;\n",toshort( &frame[i] ),toshort( &frame[i] ));
//	i+=2;
//	printf("KolvoFigurVFreime = 0x%04X =%u;\n",toshort( &frame[i] ),toshort( &frame[i]));
//	NumOfFigures = toshort( &frame[i] );
//	i+=2;
//	printf("KolvoTochekVFigure = 0x%04X =%u;\n",toshort( &frame[i] ),toshort( &frame[i]));
//	NumOfPoints = toshort( &frame[i] );
//	for(j=0;j<NumOfFigures;j++)
//	{
//		for(k=0;k<NumOfPoints;k++)
//		{
//			i+=2;
//			printf("X = 0x%04X =%5u; ",toshort( &frame[i] ),toshort( &frame[i]));
//			i+=2;
//			printf("Y = 0x%04X =%5u;\n",toshort( &frame[i] ),toshort( &frame[i]));
//		}
//	}
}

typedef struct
{
	uint16_t	NumOfPoints;
	
	uint32_t	*value;
// 	uint16_t	*valueX;
// 	uint16_t	*valueY;
}Figure_t;

typedef struct
{
	uint16_t	NumOfFigures;
	uint16_t	SizeOfFrame;
//	Figure_t 		*figure;
}Frame_t;

typedef struct
{
	uint32_t	NumOfFrames;
	uint32_t	NumOfRepeatFrames;
	uint32_t	DelayBetweenFrames;
	uint32_t	DelayBetweenPoints;
	uint32_t	DelayLazerOnOff;
//	Frame_t 		*frame;
}Head_ilda_t;

uint32_t toint( uint8_t * buf )
{
	uint8_t i;
	uint32_t tmp1=0;
	
	for(i=0;i<sizeof(tmp1);i++)tmp1 |= (uint32_t)(buf[i])<<(8*i);
	return tmp1;
}

uint16_t toshort( uint8_t * buf )
{
	uint8_t i;
	uint16_t tmp1=0;
	
	for(i=0;i<sizeof(tmp1);i++)tmp1 |= (uint16_t)(buf[i])<<(8*i);
	return tmp1;
}

/*	Вывод прямо с флешки.
	//=========================================================================
	printf("f_open:\n");
	fresult = f_open(&file, filepath, FA_OPEN_EXISTING | FA_READ);
	FR_print_error(fresult);
	
	n=1400;
	while(n--)
	{
		f_lseek(&file,0);
		
		//printf("f_read:\n");
		f_read(&file,(uint8_t*)Zagolovok,sizeof(Zagolovok),0);
		for(i=0;i<(sizeof(Zagolovok)/sizeof(uint32_t));++i)
		{
			//printf("0x%06X =%u;  ",Zagolovok[i],Zagolovok[i]);
		}
		//printf("\n");
		KolvoFreimov = Zagolovok[0];
		
		for(k=0;k<(KolvoFreimov);k++)
		{
			f_read(&file,(void *)&KolvoFigurVFreime,sizeof(KolvoFigurVFreime),0);
			//printf("KolvoFigurVFreime = 0x%04X =%u;\n",KolvoFigurVFreime,KolvoFigurVFreime);
			
			
			for(j=0;j<KolvoFigurVFreime;j++)
			{
				f_read(&file,(void *)&KolvoTochekVFigure,sizeof(KolvoTochekVFigure),0);
				//printf("KolvoTochekVFigure = 0x%04X =%u;\n",KolvoTochekVFigure,KolvoTochekVFigure);
					
				f_read(&file,(void *)&KolvoByteVFigure,sizeof(KolvoByteVFigure),0);
				//printf("KolvoByteVFigure = 0x%04X =%u;\n",KolvoByteVFigure,KolvoByteVFigure);
				
				for(i=0;i<KolvoTochekVFigure;i++)
				{
					f_read(&file,(void *)&valueX,2,0);
					f_read(&file,(void *)&valueY,2,0);
					//printf("valueX = 0x%04X =%6u;\tvalueY = 0x%04X =%6u;\n",valueX,valueX,valueY,valueY);
					setXY(valueX,valueY);
					delayXY(15000);
				}
			}
		}
	}
 	printf("f_close:\n");
 	fresult = f_close(&file);
 	FR_print_error(fresult);
	//=========================================================================

*/





/*
uint32_t ReadToMem( FIL *fp, uint32_t FramePointer, Frame_t* frame )
{
// 	Figure_t	figure;
// 	uint32_t	*points;
// 	uint16_t 	j;
// 	uint16_t 	sizeOfPoints=0;
	
	uint16_t 	i;
	uint16_t	NumOfFigures=0;
	uint16_t	SizeOfFrame=0;
	uint16_t	NumOfPoints=0;
	
	f_lseek(fp,(int)FramePointer);
	
	f_read(fp,(void*)&NumOfFigures,sizeof(uint16_t),0);
	printf("NumOfFigures: %u\n",NumOfFigures);
	FramePointer+=sizeof(uint16_t);
	
	f_read(fp,(void*)&SizeOfFrame,sizeof(uint16_t),&FramePointer);
 	printf("SizeOfFrame:  %u\n",SizeOfFrame);
	FramePointer+=sizeof(uint16_t);
	
	for(i=0;i<NumOfFigures;i++)
	{
		f_lseek(fp,(int)FramePointer+NumOfPoints);
		f_read(fp,&NumOfPoints,sizeof(uint16_t),0);
		printf("NumOfPoints: %u\n",NumOfPoints);
	}
	printf("ReadToMem:%u\n",SizeOfFrame);
	
// 	printf("ReadToMem:%u\n",sizeof(Head_ilda_t));
// 	printf("f_read:\n");
// 	f_lseek(fp,(int)FramePointer);
// 	f_read(fp,(void*)frame->NumOfFigures,sizeof(uint16_t),0);
// 	f_read(fp,(void*)frame->SizeOfFrame,sizeof(uint16_t),0);

// 	printf("frame.NumOfFigures: %u\n",frame->NumOfFigures);
// 	printf("frame.SizeOfFrame:  %u\n",frame->SizeOfFrame);
// 	
// 	for(j=0;j<frame->NumOfFigures;j++)
// 	{
// 		f_read(fp,&figure.NumOfPoints,sizeof(uint16_t),0);
// 		printf("Figure.NumOfPoints: %u\n",figure.NumOfPoints);
// 		
// 		sizeOfPoints = figure.NumOfPoints * sizeof(uint32_t);
// 		points = (uint32_t*)malloc(sizeOfPoints);
// 		if (points == NULL) 
// 		{
// 			printf("dermo!\n");
// 			return (1);
// 		}
// 		f_read(fp,points,sizeOfPoints,0);
// 		for(i=0; i<figure.NumOfPoints;i++)
// 			printf("x: %6u y: %6u\n",(uint16_t)(points[i]>>16),(uint16_t)(points[i]));
// 	}
	return sizeof(Head_ilda_t);
}
*/

//******************************************************************************
void init_timer6()
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
	base_timer.TIM_Prescaler = 42; //- 1; зачем -1 ???  							// 84000000 /840 = 100000
	base_timer.TIM_Period = 1500;	// 1 секунда при 8400 делителе и 10000 таймере. // 100000 /200 = 500 раз в сек будет прерывание
	TIM_TimeBaseInit(TIM6, &base_timer);

	/* Разрешаем прерывание по обновлению (в данном случае -
	* по переполнению) счётчика таймера TIM6.
	*/
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	/* Включаем таймер */
	TIM_Cmd(TIM6, ENABLE);

	/* Разрешаем обработку прерывания по переполнению счётчика
	* таймера TIM6. Так получилось, что это же прерывание
	* отвечает и за опустошение ЦАП.
	*/
	NVIC_EnableIRQ(TIM6_DAC_IRQn);
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

void SetPointFromFrame( void )
{
	static uint32_t Pointer = 4;
	static uint16_t CurrentFigure = 0;
	static uint16_t CurrentPoint = 0;
	static uint16_t NumOfFigures = 0;
	static uint16_t NumOfPoints = 0;
	//uint32_t framerep = 0;
	
	static uint8_t lfrp = 0;
	
	uint16_t valueX = 0;
	uint16_t valueY = 0;
	
	
	if(lfrp != frp)
	{
		lfrp = frp;				//gde ispol'zuetsya?
		Pointer = 4;
		CurrentFigure = 0;
		CurrentPoint = 0;
		NumOfFigures = 0;
		NumOfPoints = 0;
		//PrintFromMemFrameILDA(Frame, SizeOfFrame[nSizeOfFrame]);
	}
	
	if( Pointer >= SizeOfFrame[nSizeOfFrame] )Pointer = 4;
	if (CurrentFigure == 0)
	{
		NumOfFigures = toshort( &Frame[2] );
		//Pointer += 2;
		NumOfPoints = toshort( &Frame[4] );
	}
		//!!!//
	if( CurrentFigure != NumOfFigures )
	{
		if( CurrentPoint != NumOfPoints )
		{
			//LaserOn();
			Pointer += 2;
			valueX = toshort( &Frame[Pointer] );
			Pointer += 2;
			valueY = toshort( &Frame[Pointer] );
			setXY(~valueX, valueY);
			if (CurrentPoint == 0) LaserOn();
			//printf("P:%04u X:%04X-Y:%04X\n",Pointer,valueX,valueY );
			//printf("%u-%u\n",valueX,valueY );
			
			CurrentPoint++;
			//LaserOff();

		}
		else if ( CurrentPoint == NumOfPoints )
		{
			//Pointer += 2;
			//NumOfPoints = toshort( &Frame[Pointer] );
			//
			//if( CurrentPoint == NumOfPoints )
			//{
				CurrentPoint = 0;
				//NumOfPoints = 0;
				CurrentFigure++;
				Pointer += 2;
				NumOfPoints = toshort( &Frame[Pointer] );
				LaserOff();
			//}
		}
		if( CurrentFigure == NumOfFigures ) CurrentFigure = 0;
	}
	//CurrentFigure = 0;
	//else if( CurrentFigure == NumOfFigures ) CurrentFigure = 0;
}

void TIM6_DAC_IRQHandler()
{
	/* Так как этот обработчик вызывается и для ЦАП, нужно проверять,
	* произошло ли прерывание по переполнению счётчика таймера TIM6.
	*/
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
	{
		/* Очищаем бит обрабатываемого прерывания */
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
		
		if( STM_EVAL_PBGetState(BUTTON_USER) == 1 )
		{
			/* Инвертируем состояние светодиодов */
			STM_EVAL_LEDToggle(LED_ORANGE);
			//SetPointRandom();
			
		}
		else
		{
			SetPointFromFrame();
		}
	}
}
//******************************************************************************


//******************************************************************************
int main(void)
{
	/*!< At this stage the microcontroller clock setting is already configured,
	   this is done through SystemInit() function which is called from startup
	   file (startup_stm32f4xx.s) before to branch to application main.
	   To reconfigure the default setting of SystemInit() function, refer to
	   system_stm32f4xx.c file
	 */

	/*!< Most systems default to the wanted configuration, with the noticeable 
		exception of the STM32 driver library. If you are using an STM32 with 
		the STM32 driver library then ensure all the priority bits are assigned 
		to be preempt priority bits by calling 
		NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); before the RTOS is started.
	*/
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );	
	
	//=========================================================================
	STM_EVAL_LEDInit(LED_BLUE);
	STM_EVAL_LEDInit(LED_GREEN);
	STM_EVAL_LEDInit(LED_ORANGE);
	STM_EVAL_LEDInit(LED_RED);
	//=========================================================================

	#ifdef SD_DMA_MODE
	SD_NVIC_Configuration();
	#endif

/*
// 	printf("f_mount:\n");
// 	fresult = f_mount(0, &fs);
// 	FR_print_error(fresult);

// 	//=========================================================================
//  	printf("f_stat:\n");
//  	f_stat(filepath, &filinfo);
// 	printf("filesize %s:%u\n",filepath,(uint32_t)(filinfo.fsize));
// 	//=========================================================================
// 	
// 	//=========================================================================
// 	fsize = (uint32_t)(filinfo.fsize);
// 	file_in_memory = (uint8_t*)malloc(fsize * sizeof(uint8_t));
// 	if (file_in_memory == NULL) 
// 	{
// 		printf("dermo!\n");
// 		return (1);
// 	}
// 	else printf("malloc OK!\n");
// 	//=========================================================================
// 	
// 	
	
	
//	SD_test(  );
	
	
	
	//=========================================================================
// 	printf("f_open:\n");
// 	fresult = f_open(&file, filepath, FA_OPEN_EXISTING | FA_READ);
// 	FR_print_error(fresult);
	
// 	printf("f_read:\n");
//  	f_read(&file,(uint8_t*)file_in_memory,602,0);//fsize,0);
// 	for(i=0;i<fsize;++i)printf("0x%02X ",file_in_memory[i]);
// 	printf("\n");
// 	file_in_memory[0] = 0xaa;
// 	file_in_memory[1] = 0xbb;
// 	file_in_memory[2] = 0xcc;
// 	file_in_memory[3] = 0xdd;
// 	
// 	i=0;
// 	KolvoFreimov = (uint32_t)file_in_memory[i];
// 	printf("\nKolvoFreimov = 0x%08X =%u;\n",KolvoFreimov,KolvoFreimov);
// 	
// 	pint = (uint32_t*)(file_in_memory[i]);
// 	
// 	i=0;
// 	for(i=0;i<sizeof(KolvoFreimov);i++)
// 	{
// 		KolvoFreimov |= (uint32_t)(file_in_memory[i])<<(8*i) ;
// 		printf("\nfile_in_memory[i] = 0x%02X =%u;, i=%u\n",file_in_memory[i],file_in_memory[i],i);
// 		printf("\nfile_in_memory[i] = 0x%02X =%u;, i=%u\n",file_in_memory[i]<<(8*i),file_in_memory[i]<<(8*i),i);
// 		printf("\nNumOfFrames = 0x%08X =%u;\n",KolvoFreimov,KolvoFreimov);
// 	}
// 	printf("\nNumOfFrames = 0x%08X =%u;\n",KolvoFreimov,KolvoFreimov);
// 	
	
//	for(i=0;i<(sizeof(Zagolovok)/sizeof(uint32_t));++i)
//	{
//		Zagolovok[i] = file_in_memory[n+sizeof(uint32_t)];
//		printf("0x%06X =%u;  ",Zagolovok[i],Zagolovok[i]);
//	}
// 	printf("\n");
//	KolvoFreimov = Zagolovok[0];
//	
//	for(k=0;k<(KolvoFreimov);k++)
//	{
//		KolvoFigurVFreime = file_in_memory[n+sizeof(uint16_t)];
//		printf("KolvoFigurVFreime = 0x%04X =%u;\n",KolvoFigurVFreime,KolvoFigurVFreime);
//		
//		for(j=0;j<KolvoFigurVFreime;j++)
//		{
//			KolvoTochekVFigure  = file_in_memory[n+sizeof(uint16_t)];
//			printf("KolvoTochekVFigure = 0x%04X =%u;\n",KolvoTochekVFigure,KolvoTochekVFigure);
//				
//			KolvoByteVFigure = file_in_memory[n+sizeof(uint32_t)];
//			printf("KolvoByteVFigure = 0x%04X =%u;\n",KolvoByteVFigure,KolvoByteVFigure);
//			
//			for(i=0;i<KolvoTochekVFigure;i++)
//			{
//				valueX = file_in_memory[n+sizeof(uint16_t)];
//				valueY = file_in_memory[n+sizeof(uint16_t)];
//				printf("valueX = 0x%04X =%6u;\tvalueY = 0x%04X =%6u;\n",valueX,valueX,valueY,valueY);
//			}
//		}
//	}
	
	
	
	
//  	printf("f_close:\n");
//  	fresult = f_close(&file);
//  	FR_print_error(fresult);
	//=========================================================================
	
	
	
	
	
	
	
	
//	//=========================================================================
//	printf("f_open:\n");
//	fresult = f_open(&file, filepath, FA_OPEN_EXISTING | FA_READ);
//	FR_print_error(fresult);
//	
// 	printf("f_read:\n");
// 	f_read(&file,(uint8_t*)Zagolovok,sizeof(Zagolovok),0);
//	for(i=0;i<(sizeof(Zagolovok)/sizeof(uint32_t));++i)
//	{
//		printf("0x%06X =%u;  ",Zagolovok[i],Zagolovok[i]);
//	}
// 	printf("\n");
//	KolvoFreimov = Zagolovok[0];
//	
//	for(k=0;k<(KolvoFreimov);k++)
//	{
//		f_read(&file,(void *)&KolvoFigurVFreime,sizeof(KolvoFigurVFreime),0);
//		printf("KolvoFigurVFreime = 0x%04X =%u;\n",KolvoFigurVFreime,KolvoFigurVFreime);
//		
//		for(j=0;j<KolvoFigurVFreime;j++)
//		{
//			f_read(&file,(void *)&KolvoTochekVFigure,sizeof(KolvoTochekVFigure),0);
//			printf("KolvoTochekVFigure = 0x%04X =%u;\n",KolvoTochekVFigure,KolvoTochekVFigure);
//				
//			f_read(&file,(void *)&KolvoByteVFigure,sizeof(KolvoByteVFigure),0);
//			printf("KolvoByteVFigure = 0x%04X =%u;\n",KolvoByteVFigure,KolvoByteVFigure);
//			
//			for(i=0;i<KolvoTochekVFigure;i++)
//			{
//				f_read(&file,(void *)&valueX,2,0);
//				f_read(&file,(void *)&valueY,2,0);
//				printf("valueX = 0x%04X =%6u;\tvalueY = 0x%04X =%6u;\n",valueX,valueX,valueY,valueY);
//			}
//		}
//	}
//	
// 	printf("f_close:\n");
// 	fresult = f_close(&file);
// 	FR_print_error(fresult);
//	//=========================================================================


//    	printf("Scan files:\n");
//     fresult = scan_files("0:");
//  	FR_print_error(fresult);
	
//	f_mount(0, NULL);
*/
	
	//--------------------- Start FreeRTOS ------------------------------------
	xTaskCreate( vLedBlinkRed, (const signed char*)"Led Blink Task Red", 
		STACK_SIZE_MIN, NULL, tskIDLE_PRIORITY, NULL );
	xTaskCreate( vLedBlinkGreen, (const signed char*)"Led Blink Task Green", 
		STACK_SIZE_MIN, NULL, tskIDLE_PRIORITY, NULL );
	xTaskCreate( vLedBlinkOrange, (const signed char*)"Led Blink Task Orange", 
		STACK_SIZE_MIN, NULL, tskIDLE_PRIORITY, NULL );
	xTaskCreate( vSendUart, (const signed char*)"vSendUart", 
		STACK_SIZE_MIN, NULL, tskIDLE_PRIORITY, NULL );	
 	xTaskCreate( vReadSD, (const signed char*)"vReadSD", 
 		STACK_SIZE_MIN*10, NULL, tskIDLE_PRIORITY, NULL );
	xTaskCreate( vOutToLaser, (const signed char*)"vOutToLaser", 
		STACK_SIZE_MIN*10, NULL, tskIDLE_PRIORITY, NULL );
		
	vTaskStartScheduler();
	//-------------------------------------------------------------------------
}
//******************************************************************************


//******************************************************************************

/*
void vReadSD1(void *pvParameters)
{
	uint32_t 	i = 0;
 	uint32_t 	j = 0;
 	uint32_t 	k = 0;
	uint32_t	Zagolovok[5] = {0,0,0,0,0};
	uint32_t	KolvoFreimov = 0;
	uint16_t	KolvoFigurVFreime = 0;
	uint16_t	KolvoTochekVFigure = 0;
	uint16_t	KolvoByteVFigure = 0;
	uint16_t	KolvoByteVFreime = 0;
	uint16_t	valueX = 0;
	uint16_t	valueY = 0;
	
	FATFS   	fs;
	FIL     	file;
	FRESULT		fresult;
	TCHAR		filepath[] = {"0:sun.bin"};

	vTaskDelay( 100 / portTICK_RATE_MS );
	
	printf("f_mount:\n");
	fresult = f_mount(0, &fs);
	FR_print_error(fresult);
	

	//=========================================================================
	printf("f_open:\n");
	fresult = f_open(&file, filepath, FA_OPEN_EXISTING | FA_READ);
	FR_print_error(fresult);
	
	printf("f_read:\n");
	f_read(&file,(uint8_t*)Zagolovok,sizeof(Zagolovok),0);
	for(i=0;i<(sizeof(Zagolovok)/sizeof(uint32_t));++i)
	{
		printf("0x%06X =%u;  ",Zagolovok[i],Zagolovok[i]);
	}
	printf("\n");
	KolvoFreimov = Zagolovok[0];
	
	for(k=0;k<(KolvoFreimov);k++)
	{
		printf("adrKolvoByteVFreime = 0x%04X =%u;\n",f_tell(&file),f_tell(&file));
		f_read(&file,(void *)&KolvoByteVFreime,sizeof(KolvoByteVFreime),0);
		printf("KolvoByteVFreime = 0x%04X =%u;\n",KolvoByteVFreime,KolvoByteVFreime);
		
		f_read(&file,(void *)&KolvoFigurVFreime,sizeof(KolvoFigurVFreime),0);
		printf("KolvoFigurVFreime = 0x%04X =%u;\n",KolvoFigurVFreime,KolvoFigurVFreime);
		
		for(j=0;j<KolvoFigurVFreime;j++)
		{
			f_read(&file,(void *)&KolvoTochekVFigure,sizeof(KolvoTochekVFigure),0);
			printf("KolvoTochekVFigure = 0x%04X =%u;\n",KolvoTochekVFigure,KolvoTochekVFigure);
			
			//f_read(&file,(void *)&KolvoByteVFigure,sizeof(KolvoByteVFigure),0);
			//printf("KolvoByteVFigure = 0x%04X =%u;\n",KolvoByteVFigure,KolvoByteVFigure);
			
			for(i=0;i<KolvoTochekVFigure;i++)
			{
				f_read(&file,(void *)&valueX,2,0);
				f_read(&file,(void *)&valueY,2,0);
				printf("valueX = 0x%04X =%6u;\tvalueY = 0x%04X =%6u;\n",valueX,valueX,valueY,valueY);
			}
		}
	}
		
	printf("f_close:\n");
	fresult = f_close(&file);
	FR_print_error(fresult);
	//=========================================================================
	
	for(;;)
	{
		//=========================================================================
		vTaskDelay( 500 / portTICK_RATE_MS );
	}
}
*/



uint32_t ReadToMemHeadILDA( FIL *fp, Head_ilda_t* Head_ilda )
{
	printf("ReadToMemHeadILDA:%u\n",sizeof(Head_ilda_t));
	printf("f_read:\n");
	f_read(fp,(void*)Head_ilda,sizeof(Head_ilda_t),0);
	
	return sizeof(Head_ilda_t);
}

void PrintHeadILDA( Head_ilda_t * Head_ilda )
{
	printf("File.NumOfFrames:        %u\n",Head_ilda->NumOfFrames);
	printf("File.NumOfRepeatFrames:  %u\n",Head_ilda->NumOfRepeatFrames);
	printf("File.DelayBetweenFrames: %u\n",Head_ilda->DelayBetweenFrames);
	printf("File.DelayBetweenPoints: %u\n",Head_ilda->DelayBetweenPoints);
	printf("File.DelayLazerOnOff:    %u\n",Head_ilda->DelayLazerOnOff);
}

void ReadAllSizeFrame( FIL *fp, uint32_t NumOfFrames, uint32_t * AdrOfFrame, uint32_t * SizeOfFrame )
{
	uint16_t SizeOfFrame_tmp = 0;
	uint32_t i = 0;
	
	AdrOfFrame[0] = sizeof(Head_ilda_t);
	printf("AdrNumOfFrame %u:%u byte\n",0,AdrOfFrame[0]);
	
	for(i=1;i<=NumOfFrames;i++)
	{
		f_lseek(fp,(int)AdrOfFrame[i-1]);
		f_read(fp,(void*)&SizeOfFrame_tmp,sizeof(SizeOfFrame_tmp),0);
		SizeOfFrame[i-1] = (uint32_t)SizeOfFrame_tmp + 2; //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		AdrOfFrame[i] = AdrOfFrame[i-1] + SizeOfFrame[i-1];
		printf("AdrNumOfFrame %u:%u byte\n",i,AdrOfFrame[i]);
		printf("SizeOfFrame %u:%u byte\n",i-1,SizeOfFrame[i-1]);
	}
}

void ReadToMemFrameILDA( FIL *fp, uint8_t* frame, uint32_t PointerToFrame, uint32_t SizeOfFrame)
{
 	//printf("PointerToFrame:%u\n",PointerToFrame);
 	f_lseek(fp,(int)PointerToFrame);
 	
 	//printf("f_read:\n");
 	f_read(fp,(void*)frame,SizeOfFrame,0);
}



void vReadSD(void *pvParameters)
{
	Head_ilda_t Head_ilda;
	FATFS   	fs;
	FIL     	file;
	FRESULT		fresult;
	TCHAR		filepath[] = {"0:test2.bin"};

	uint32_t	*PointerToFrame;
	//uint32_t	*SizeOfFrame;

//	uint16_t	word;
//	uint32_t	dword;

	uint8_t		i = 0;
	uint8_t		time = 0;
	uint8_t 	* frame[2] = {NULL,NULL};

	uint8_t		click = 0;

	//+++++++++++++++++++++++++++++++++++
	vTaskDelay( 100 / portTICK_RATE_MS );
 	
	printf("f_mount:\n");
 	fresult = f_mount(0, &fs);
	FR_print_error(fresult);

	printf("f_open:\n");
	fresult = f_open(&file, filepath, FA_OPEN_EXISTING | FA_READ);
	FR_print_error(fresult);
	
	ReadToMemHeadILDA(&file,&Head_ilda);
	PrintHeadILDA(&Head_ilda);
	
	///////////////////////////////////////////////////////////////////////////
	//=========================================================================
	printf("ReadToMemNumOfFrame:%u byte\n",Head_ilda.NumOfFrames);
	printf("malloc PointerToFrame:%u byte\n",Head_ilda.NumOfFrames);
	PointerToFrame = (uint32_t*)malloc(Head_ilda.NumOfFrames * sizeof(uint32_t));
	if (PointerToFrame == NULL) 
	{
		printf("malloc dermo!\n");
		return;
	}
	else printf("malloc OK!\n");
	//=========================================================================
	
	//=========================================================================
	printf("malloc SizeOfFrame:%u byte\n",Head_ilda.NumOfFrames);
	SizeOfFrame = (uint32_t*)malloc(Head_ilda.NumOfFrames * sizeof(uint32_t));
	if (PointerToFrame == NULL) 
	{
		printf("malloc dermo!\n");
		return;
	}
	else printf("malloc OK!\n");
	//=========================================================================
	
	ReadAllSizeFrame(&file,Head_ilda.NumOfFrames,PointerToFrame,SizeOfFrame);
	
	printf("\nQWE:\n");
	for(i=0;i<Head_ilda.NumOfFrames;i++)
		printf("AdrNumOfFrame %u:%u byte\n",i,PointerToFrame[i]);
	for(i=0;i<Head_ilda.NumOfFrames;i++)
		printf("SizeOfFrame %u:%u byte\n",i,SizeOfFrame[i]);
	///////////////////////////////////////////////////////////////////////////
	
	i=0;
	if(frame[0] != NULL)free((void*)frame[0]);
	//=========================================================================
	printf("SizeOfFrame:%u byte\n",SizeOfFrame[i]);
	printf("malloc Frame:%u byte\n",SizeOfFrame[i]);
	frame[0] = (uint8_t*)malloc(SizeOfFrame[i] * sizeof(uint8_t));
	if (frame[0] == NULL) 
	{
		printf("malloc dermo!\n");
		return;
	}
	else printf("malloc OK!\n");
	//=========================================================================
	
	ReadToMemFrameILDA(&file,frame[0],PointerToFrame[i],SizeOfFrame[i]);
	PrintFromMemFrameILDA(frame[0],SizeOfFrame[i]);
	Frame = frame[0];
	init_timer6();

	for(;;)
	{
		if( STM_EVAL_PBGetState(BUTTON_USER) == 1 )
		{
			click = 1;
		}
		else{
			if( click == 1 )
			{
				click = 0;
				
				frp++;
				if(frp > 1)frp=0;
				
				i++;
				if(i>=Head_ilda.NumOfFrames)i=0;
				printf("NumOfFrame:%3u, frp = %3u\n",i,frp);
				nSizeOfFrame = i;
				
				//if(Frame != NULL)
				free((void*)frame[frp]);
				
				//=========================================================================
				printf("SizeOfFrame:%u byte\n",SizeOfFrame[i]);
				printf("malloc Frame:%u byte\n",SizeOfFrame[i]);
				frame[frp] = (uint8_t*)malloc(SizeOfFrame[i] * sizeof(uint8_t));
				if (frame[frp] == NULL) 
				{
					printf("malloc dermo!\n");
					return;
				}
				else printf("malloc OK!\n");
				//=========================================================================
				//ReadToMemFrameILDA(&file,Frame,PointerToFrame[i],SizeOfFrame[i]);
				f_lseek(&file,(int)PointerToFrame[i]);
				f_read(&file,(void*)frame[frp],SizeOfFrame[i],0);
				//PrintFromMemFrameILDA(frame[frp],SizeOfFrame[i]);
				Frame = frame[frp];
			}
		}
		//=========================================================================
		vTaskDelay( 10 / portTICK_RATE_MS );
	}
		
	//=========================================================================
	printf("f_close:\n");
	fresult = f_close(&file);
	FR_print_error(fresult);
	
}

void vSendUart(void *pvParameters)
{
	uint32_t second=0;
	UART_Configuration();
	vTaskDelay( 10 / portTICK_RATE_MS );
	printf("UART_Configuration_task: DONE\n");
	
	for(;;)
	{
		second+=10;
		//printf("Blik Blue %u sec\n",second);
		STM_EVAL_LEDToggle(LED_BLUE);
		vTaskDelay( 10000 / portTICK_RATE_MS );
	}
}

void vLedBlinkRed(void *pvParameters)
{
	for(;;)
	{
		STM_EVAL_LEDToggle(LED_RED);
		vTaskDelay( 750 / portTICK_RATE_MS );
	}
}

void vLedBlinkGreen(void *pvParameters)
{
	//RNG_Config();
	uint16_t tim = 100;
	for(;;)
	{
		//uint16_t tim = (uint16_t)(getRandom()>>22); // 10 bits = 0...1024 mSec
		STM_EVAL_LEDToggle(LED_GREEN);
		vTaskDelay(  tim / portTICK_RATE_MS );
	}
}


void vLedBlinkOrange(void *pvParameters)
{
	for(;;)
	{
		
		//STM_EVAL_LEDToggle(LED_ORANGE);
		vTaskDelay( 900 / portTICK_RATE_MS );
	}
}

void vOutToLaser(void *pvParameters)
{
	RNG_Config();
	initialization_set_xy();
	LaserOff();

	STM_EVAL_PBInit(BUTTON_USER,BUTTON_MODE_GPIO);
	
	//init_timer6();
	for(;;)
	{
	
		//====================================
		vTaskDelay( 1000 / portTICK_RATE_MS );
	}
}
//******************************************************************************


