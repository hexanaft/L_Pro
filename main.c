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
#include "ilda.h"
#include "LaserTimer.h"
#include "PointTimer.h"
//******************************************************************************

//******************************************************************************
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "croutine.h"
//******************************************************************************

/*******************************************************************************
STM32 SDIO Pin assignment
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
  * PC.07	 to GND ( Card Detect )
	  PD.08 UART TX
******************************************************************************/

#define	SIZE_OF_FRAME_MAX		0x1fff

//*****************************************************************************
// Global variables
FATFS   	fs;
FRESULT		fresult;

Head_ilda_t		Head_ilda;

uint8_t	*Frame = 0;
volatile uint8_t	frp = 0;
volatile uint8_t FrameIsReady = 0;
uint32_t	*SizeOfFrame;
volatile uint8_t	nSizeOfFrame = 0;
volatile uint8_t	FrameFirstShow = 0;
//*****************************************************************************

//*****************************************************************************
#define STACK_SIZE_MIN	512	/* usStackDepth	- the stack size DEFINED IN WORDS.*/
void vReadSD(void *pvParameters);
void vSendUart(void *pvParameters);
void vOutToLaser(void *pvParameters);
//*****************************************************************************

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
	
	//--------------------- Start FreeRTOS ------------------------------------
	xTaskCreate( vSendUart, (const signed char*)"vSendUart", 
		STACK_SIZE_MIN, NULL, tskIDLE_PRIORITY, NULL );	
 	xTaskCreate( vReadSD, (const signed char*)"vReadSD", 
 		STACK_SIZE_MIN*10, NULL, 1 | portPRIVILEGE_BIT, NULL );
	xTaskCreate( vOutToLaser, (const signed char*)"vOutToLaser", 
		STACK_SIZE_MIN*10, NULL, tskIDLE_PRIORITY, NULL );
		
	vTaskStartScheduler();
	//-------------------------------------------------------------------------
}
//******************************************************************************

//pvPortMalloc()
//vPortFree()

//******************************************************************************
// void WaitUntilFrameIsFirstShown( void )
// {
// 	while( FrameFirstShow != 1 )
// 	{
// 		taskYIELD();
// 	};
// 	FrameFirstShow = 0;
// }

// void WaitUntilFrameIsReady( void )
// {
// 	while( FrameIsReady != 1 )
// 	{
// 		taskYIELD();
// 	};
// 	FrameIsReady = 0;
// }

uint8_t ButtonClickUp( void )
{
	
	if( STM_EVAL_PBGetState(BUTTON_USER) ) return 1;
	else return 0;
	
//	delayFr(0xff);
/*
static uint8_t click = 0;
	
if( STM_EVAL_PBGetState(BUTTON_USER) )
	{
		click = 1;
		return 0;
	}
	else
	{
		if( click == 1 )
		{
			click = 0;
			return 1;
		}
	}
	return 0;
	*/
}

void FrameToOut( uint8_t 	* frame )
{
//	uint8_t First;
	Frame = frame;
	printf("Включаем вывод фрейма\n");
	init_timer6( 900 ); //Head_ilda.DelayBetweenPoints );//(130);//
}


//******************************************************************************
void SetPointFromFrame( void )
{
	static uint16_t	Pointer = 4;
	static uint16_t	CurrentFigure = 0;
	static uint16_t	CurrentPoint = 0;
	static uint16_t	NumOfFigures = 0;
	static uint16_t	NumOfPoints = 0;
	
	uint16_t valueX = 0;
	uint16_t valueY = 0;
	static uint8_t lfrp = 0;

	//FrameIsReady = 0;
	
	
	LaserOn( Head_ilda.DelayLazerOn );
	if(Frame == NULL) 
	{
//		printf("Frame = 0\n");
		return;
	}
	
	if((lfrp != frp) || ( Pointer >= SizeOfFrame[nSizeOfFrame] ))
	{
		lfrp = frp;
		Pointer = 4;
		CurrentFigure = 0;
		CurrentPoint = 0;
		NumOfFigures = 0;
		NumOfPoints = 0;
		//PrintFromMemFrameILDA(Frame, SizeOfFrame[nSizeOfFrame]);
		FrameIsReady = 1;
	}

	if (CurrentFigure == 0)
	{
		NumOfFigures = toshort( &Frame[2] );
		NumOfPoints = toshort( &Frame[4] );
		//printf("NumOfFigures:%u-NumOfPoints:%u\n",NumOfFigures,NumOfPoints);
	}
	
	if( CurrentFigure != NumOfFigures )
	{
		if( CurrentPoint != NumOfPoints )
		{
			Pointer += 2;
			valueX = toshort( &Frame[Pointer] );
			Pointer += 2;
			valueY = toshort( &Frame[Pointer] );
			setXY(~valueX, valueY);
//			printf("P:%04u X:%04X-Y:%04X\n",Pointer,valueX,valueY );
//			printf("Точек всего:%u-Точка:%u-Фигур всего:%u-Фигура:%u\n",NumOfPoints,CurrentPoint,NumOfFigures,CurrentFigure );
			//printf("%u-%u\n",valueX,valueY );
			if (CurrentPoint == 0) LaserOn( Head_ilda.DelayLazerOn );
			CurrentPoint++;
		}
		if ( CurrentPoint == NumOfPoints )
		{
//			printf("NumOfFigures====:%u-NumOfPoints:%u\n",NumOfFigures,NumOfPoints);
//			printf("FrameIsReady:%u \n",FrameIsReady);
			//printf("nSizeOfFrame:%u\n",nSizeOfFrame);
			CurrentPoint = 0;
			CurrentFigure++;
			if (CurrentFigure != NumOfFigures) 
			{
				Pointer += 2;
				NumOfPoints = toshort( &Frame[Pointer] );
//				printf("Фигур всего!===:%u-Фигура:%u\n",NumOfFigures,CurrentFigure );
				//printf("NumOfPoints:%u\n",NumOfPoints);
				//LaserOff( Head_ilda.DelayLazerOff );
			}
			//else //
				if( CurrentFigure == NumOfFigures ) 
			{
//				printf("77777777777FrameIsReady:%u \n",FrameIsReady);
				Pointer = 4;
				CurrentFigure = 0;
				CurrentPoint = 0;
				NumOfFigures = 0;
				NumOfPoints = 0;
				FrameFirstShow = 1;
				FrameIsReady = 1;
				StopTimer6();
			}
		}
	}
}
//******************************************************************************

//******************************************************************************
void SetFileToOut( const TCHAR * filepath )
{
	FIL     	file;
	uint8_t 	* frame[2] = {NULL,NULL};
	uint32_t	* PointerToFrame;
	uint16_t	nFrp = 0;
	uint8_t		counter = 0;
	
	uint8_t		FrameLokal[SIZE_OF_FRAME_MAX];

	//+++++++++++++++++++++++++++++++++++
	printf("Открываем файл: %s\n",filepath);
	fresult = f_open(&file, filepath, FA_OPEN_EXISTING | FA_READ);
	if(fresult)FR_print_error(fresult);
	
	ReadToMemHeadILDA(&file,&Head_ilda);
	PrintHeadILDA(&Head_ilda);
	
	///////////////////////////////////////////////////////////////////////////
	//=========================================================================
	PointerToFrame = (uint32_t*)pvPortMalloc(Head_ilda.NumOfFrames * sizeof(uint32_t));
	if (PointerToFrame == NULL) 
	{
		printf("pvPortMalloc PointerToFrame:%u byte\n",Head_ilda.NumOfFrames);
		printf("pvPortMalloc dermo!\n");
		return;
	}
	//else printf("pvPortMalloc OK!\n");
	//=========================================================================
	
	//=========================================================================
	SizeOfFrame = (uint32_t*)pvPortMalloc(Head_ilda.NumOfFrames * sizeof(uint32_t));
	if (SizeOfFrame == NULL) 
	{
		printf("pvPortMalloc SizeOfFrame:%u byte\n",Head_ilda.NumOfFrames);
		printf("pvPortMalloc dermo!\n");
		return;
	}
	//else printf("pvPortMalloc OK!\n");
	//=========================================================================
	
	// Считываем размеры и адреса всех фреймов
	printf("Считываем размеры и адреса всех фреймов\n");
	ReadAllSizeFrame(&file,Head_ilda.NumOfFrames,PointerToFrame,SizeOfFrame);

	// Печатаем размеры и адреса фреймов
	printf("Печатаем размеры и адреса фреймов:\n");
	for(nFrp=0;nFrp<Head_ilda.NumOfFrames;nFrp++)
		printf("	AdrNumOfFrame %u:%u byte\n",nFrp,PointerToFrame[nFrp]);
	for(nFrp=0;nFrp<Head_ilda.NumOfFrames;nFrp++)
		printf("	SizeOfFrame %u:%u byte\n",nFrp,SizeOfFrame[nFrp]);

	///////////////////////////////////////////////////////////////////////////
	
	nFrp = 0;
	frp = 0;
	FrameFirstShow = 0;
	FrameIsReady = 1;
	//printf("Пошли в вайл\n");
	
	for(nFrp=0;nFrp<=(Head_ilda.NumOfFrames);nFrp++)
	{
		printf("	AdrNumOfFrame %u:%X byte\n",nFrp,PointerToFrame[nFrp]);
		printf("	SizeOfFrame %u:%u byte\n",nFrp,SizeOfFrame[nFrp]);
		//=========================================================================
		ReadToMemFrameILDA(&file,FrameLokal,PointerToFrame[nFrp],SizeOfFrame[nFrp]);
		PrintFromMemFrameILDA(FrameLokal,SizeOfFrame[nFrp]);
	}
	
	while( counter < 500)
	{
		vTaskDelay( 900 / portTICK_RATE_MS );
		printf("Вернулись из втаска\n");
		printf("FrameIsReady:%u \n",FrameIsReady);
		if (FrameIsReady == 1)
		{
			printf("Фрейм:%3u, frp = %3u SizeOfFrame:%u byte\n",nFrp,frp,SizeOfFrame[nFrp]);
			
			printf("Очищаем память frp =%i, adr = %X\n",frp,(void*)frame[frp]);
			vPortFree((void*)frame[frp]); //if(frame[frp] != NULL)
			//=========================================================================
			frame[frp] = (uint8_t*)pvPortMalloc(SizeOfFrame[nFrp] * sizeof(uint8_t));
			
			//PrintFromMemFrameILDA(frame[frp],SizeOfFrame[nFrp]);
			
			if (frame[frp] == NULL)
			{
				printf("SizeOfFrame:%u byte\n",SizeOfFrame[nFrp]);
				printf("pvPortMalloc dermo!\n");
				return;
			}
			//else printf("pvPortMalloc OK!\n");
			printf("Выделили память frp =%i, adr = %X\n",frp,(void*)frame[frp]);
			//=========================================================================
			ReadToMemFrameILDA(&file,frame[frp],PointerToFrame[nFrp],SizeOfFrame[nFrp]);
			//PrintFromMemFrameILDA(frame[frp],SizeOfFrame[nFrp]);

			Frame = frame[frp];
			nSizeOfFrame = nFrp;
			
			frp++;
			if(frp > 1)frp=0;
			
			++nFrp;
			if(nFrp >= Head_ilda.NumOfFrames)nFrp=0;
			
			counter++;
			//if( Head_ilda.NumOfFrames > 1 )
			//printf("\n");
			//printf("Включаем вывод фрейма\n");
			
			FrameIsReady = 0;
			init_timer6( 900 ); //Head_ilda.DelayBetweenPoints );//(130);//
		}
		else printf("else\n");
	}
	
	//=========================================================================
	StopTimer6();
	
	vPortFree((void*)frame[0]);
	vPortFree((void*)frame[1]);
	vPortFree(SizeOfFrame);
	vPortFree(PointerToFrame);
	
	*SizeOfFrame = NULL;
	*PointerToFrame = NULL;
	nFrp = 0;
	*frame[0] = NULL;
	*frame[1] = NULL;
	
	frp = 0;
	nSizeOfFrame = 0;
	
	printf("f_close:\n");
	fresult = f_close(&file);
	FR_print_error(fresult);
}
//******************************************************************************

void vReadSD(void *pvParameters)
{
	vTaskDelay( 30 / portTICK_RATE_MS );
 	
	printf("f_mount:\n");
 	fresult = f_mount(0, &fs);
	FR_print_error(fresult);

	for(;;)
	{
		SetFileToOut( "0:Horse.bin" );
		//SetFileToOut( "0:test2.bin" );
		//SetFileToOut( "0:Horse33.bin" );
		//SetFileToOut( "0:CanGoose.bin" );
		//SetFileToOut( "0:krest_30.bin" );
		//SetFileToOut( "0:beer.bin" );
		//SetFileToOut( "0:Horse2.bin" );
		//SetFileToOut( "0:test2.bin" );
	}
}

void vSendUart(void *pvParameters)
{
	uint32_t second=0;
	UART_Configuration();
	vTaskDelay( 30 / portTICK_RATE_MS );
	printf("UART_Configuration_task: DONE\n");
	
	for(;;)
	{
		second+=10;
		//printf("Blik Blue %u sec\n",second);
		//STM_EVAL_LEDToggle(LED_BLUE);
		vTaskDelay( 300 / portTICK_RATE_MS );
	}
}


void vOutToLaser(void *pvParameters)
{
	RNG_Config();
	initialization_set_xy();
	STM_EVAL_PBInit(BUTTON_USER,BUTTON_MODE_GPIO);
	STM_EVAL_LEDOff(LED_RED);
	STM_EVAL_LEDOff(LED_BLUE);
	
	for(;;)
	{
		//init_timer7( 1000 );
		//====================================
		vTaskDelay( 5000 / portTICK_RATE_MS );
	}
	
}
//******************************************************************************


