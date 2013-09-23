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

#define STACK_SIZE_MIN	128	/* usStackDepth	- the stack size DEFINED IN WORDS.*/

void vReadSD(void *pvParameters);
void vLedBlinkRed(void *pvParameters);
void vLedBlinkGreen(void *pvParameters);
void vLedBlinkOrange(void *pvParameters);
void vSendUart(void *pvParameters);


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
}File_ilda_t;

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



uint32_t ReadToMemZag( FIL *fp, File_ilda_t* File_ilda )
{
	printf("ReadToMem:%u\n",sizeof(File_ilda_t));
	printf("f_read:\n");
	f_read(fp,(void*)File_ilda,sizeof(File_ilda_t),0);
	
	return sizeof(File_ilda_t);
}

void PrintILDA( File_ilda_t * File_ilda )
{
	printf("File.NumOfFrames:        %u\n",File_ilda->NumOfFrames);
	printf("File.NumOfRepeatFrames:  %u\n",File_ilda->NumOfRepeatFrames);
	printf("File.DelayBetweenFrames: %u\n",File_ilda->DelayBetweenFrames);
	printf("File.DelayBetweenPoints: %u\n",File_ilda->DelayBetweenPoints);
	printf("File.DelayLazerOnOff:    %u\n",File_ilda->DelayLazerOnOff);
}

uint32_t ReadToMem( FIL *fp, uint32_t FramePointer, Frame_t* frame )
{
	Figure_t	figure;
	uint32_t	*points;
	uint16_t 	i,j;
	uint16_t 	sizeOfPoints=0;
	
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
	
// 	printf("ReadToMem:%u\n",sizeof(File_ilda_t));
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
	return sizeof(File_ilda_t);
}
//******************************************************************************
int main(void)
{
	/*!< At this stage the microcontroller clock setting is already configured,
	   this is done through SystemInit() function which is called from startup
	   file (startup_stm32f4xx.s) before to branch to application main.
	   To reconfigure the default setting of SystemInit() function, refer to
	   system_stm32f4xx.c file
	 */

	//=========================================================================
//	uint32_t 	i = 0;
//	uint32_t 	n = 0;
// 	uint32_t 	fsize = 0;
//	uint32_t 	random32bit;
// 	FATFS   	fs;
//	uint8_t 	Buff[24]= {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};
		
//	uint32_t	Zagolovok[5] = {0,0,0,0,0};
//	uint32_t	KolvoFreimov = 0;
//	uint16_t	KolvoFigurVFreime = 0;
//	uint16_t	KolvoTochekVFigure = 0;
//	uint16_t	KolvoByteVFigure = 0;
//	uint16_t	valueX = 0;
//	uint16_t	valueY = 0;
	
//	FIL     	file;
//	FRESULT		fresult;
// 	FILINFO		filinfo;
// 	DIR			dir;
//	TCHAR		buffstr[100];
//	TCHAR		filename[16];
//	TCHAR		filepath[] = {"0:sun.bin"};
//	FILINFO *filinfo;
//	uint8_t *file_in_memory = NULL;
//	File_ilda nfile;
	
//	uint32_t *pint;
	//=========================================================================

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
//	printf("f_mount:\n");
//	fresult = f_mount(0, &fs);
//	FR_print_error(fresult);

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
	
	
	
	
	
	
	
// 	//=========================================================================
// 	printf("f_open:\n");
// 	fresult = f_open(&file, filepath, FA_OPEN_EXISTING | FA_READ);
// 	FR_print_error(fresult);
// 	
// 	printf("f_read:\n");
//  	f_read(&file,(uint8_t*)file_in_memory,fsize,0);
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
// 	
// //	for(i=0;i<(sizeof(Zagolovok)/sizeof(uint32_t));++i)
// //	{
// //		Zagolovok[i] = file_in_memory[n+sizeof(uint32_t)];
// //		printf("0x%06X =%u;  ",Zagolovok[i],Zagolovok[i]);
// //	}
// // 	printf("\n");
// //	KolvoFreimov = Zagolovok[0];
// //	
// //	for(k=0;k<(KolvoFreimov);k++)
// //	{
// //		KolvoFigurVFreime = file_in_memory[n+sizeof(uint16_t)];
// //		printf("KolvoFigurVFreime = 0x%04X =%u;\n",KolvoFigurVFreime,KolvoFigurVFreime);
// //		
// //		for(j=0;j<KolvoFigurVFreime;j++)
// //		{
// //			KolvoTochekVFigure  = file_in_memory[n+sizeof(uint16_t)];
// //			printf("KolvoTochekVFigure = 0x%04X =%u;\n",KolvoTochekVFigure,KolvoTochekVFigure);
// //				
// //			KolvoByteVFigure = file_in_memory[n+sizeof(uint32_t)];
// //			printf("KolvoByteVFigure = 0x%04X =%u;\n",KolvoByteVFigure,KolvoByteVFigure);
// //			
// //			for(i=0;i<KolvoTochekVFigure;i++)
// //			{
// //				valueX = file_in_memory[n+sizeof(uint16_t)];
// //				valueY = file_in_memory[n+sizeof(uint16_t)];
// //				printf("valueX = 0x%04X =%6u;\tvalueY = 0x%04X =%6u;\n",valueX,valueX,valueY,valueY);
// //			}
// //		}
// //	}
// 	
// 	
// 	
// 	
//  	printf("f_close:\n");
//  	fresult = f_close(&file);
//  	FR_print_error(fresult);
// 	//=========================================================================
	
	
	
	
	
	
	
	
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
	
	vTaskStartScheduler();
	//-------------------------------------------------------------------------
}
//******************************************************************************


//******************************************************************************
void vReadSD(void *pvParameters)
{
	uint32_t 	i = 0;
 	uint32_t 	j = 0;
 	uint32_t 	k = 0;
	uint32_t	Zagolovok[5] = {0,0,0,0,0};
	uint32_t	KolvoFreimov = 0;
	uint16_t	KolvoFigurVFreime = 0;
	uint16_t	KolvoTochekVFigure = 0;
	uint16_t	KolvoByteVFigure = 0;
	uint16_t	valueX = 0;
	uint16_t	valueY = 0;
	
	FATFS   	fs;
	FIL     	file;
	FRESULT		fresult;
	TCHAR		filepath[] = {"0:sun.bin"};

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
		f_read(&file,(void *)&KolvoFigurVFreime,sizeof(KolvoFigurVFreime),0);
		printf("KolvoFigurVFreime = 0x%04X =%u;\n",KolvoFigurVFreime,KolvoFigurVFreime);
		
		for(j=0;j<KolvoFigurVFreime;j++)
		{
			f_read(&file,(void *)&KolvoTochekVFigure,sizeof(KolvoTochekVFigure),0);
			printf("KolvoTochekVFigure = 0x%04X =%u;\n",KolvoTochekVFigure,KolvoTochekVFigure);
				
			f_read(&file,(void *)&KolvoByteVFigure,sizeof(KolvoByteVFigure),0);
			printf("KolvoByteVFigure = 0x%04X =%u;\n",KolvoByteVFigure,KolvoByteVFigure);
			
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
	
	for(;;)
	{
		//=========================================================================
		vTaskDelay( 500 / portTICK_RATE_MS );
	}
}

void vReadSDtmp(void *pvParameters)
{
	File_ilda_t ilda;
	Frame_t		frame;
	FATFS   	fs;
	FIL     	file;
	FRESULT		fresult;
	TCHAR		filepath[] = {"0:sun.bin"};
	uint32_t	*FramePointer;
	uint32_t	nFrame = 0;

 	printf("f_mount:\n");
 	fresult = f_mount(0, &fs);
	FR_print_error(fresult);

	printf("f_open:\n");
	fresult = f_open(&file, filepath, FA_OPEN_EXISTING | FA_READ);
	FR_print_error(fresult);
	
	ReadToMemZag(&file,&ilda);
	PrintILDA(&ilda);
	FramePointer = (uint32_t*)malloc(sizeof(ilda.NumOfFrames) * ilda.NumOfFrames);
	FramePointer[nFrame+1] = ReadToMem(&file,FramePointer[nFrame],&frame);


	printf("f_close:\n");
	fresult = f_close(&file);
	FR_print_error(fresult);
	
	for(;;)
	{
		//=========================================================================
		vTaskDelay( 500 / portTICK_RATE_MS );
	}
}

void vSendUart(void *pvParameters)
{
	UART_Configuration();
	printf("UART_Configuration: DONE\n");
	
	for(;;)
	{
		
		STM_EVAL_LEDToggle(LED_BLUE);
		vTaskDelay( 500 / portTICK_RATE_MS );
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
	RNG_Config();
	for(;;)
	{
		uint16_t tim = (uint16_t)(getRandom()>>22); // 10 bits = 0...1024 mSec
		STM_EVAL_LEDToggle(LED_GREEN);
		vTaskDelay(  tim / portTICK_RATE_MS );
	}
}


void vLedBlinkOrange(void *pvParameters)
{
	for(;;)
	{
		
		STM_EVAL_LEDToggle(LED_ORANGE);
		vTaskDelay( 900 / portTICK_RATE_MS );
	}
}

void vOutToLaser(void *pvParameters)
{
	
	//initialization_set_xy();
	//GPIO_SetBits(GPIOA, TTL);
	
	for(;;)
	{
		
		vTaskDelay( 900 / portTICK_RATE_MS );
	}
}
//******************************************************************************

