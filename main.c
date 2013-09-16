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
#include "stm32f4xx_rng.h"
#include "set_xy.h"
//******************************************************************************

typedef struct
{
	uint32_t	NumOfPoints;
	
	uint16_t	*valueX;
	uint16_t	*valueY;
}Figure;

typedef struct
{
	uint32_t	NumOfFigures;
	uint32_t	SizeOfFrame;
	Figure 		*figure;
}Frame;

typedef struct
{
	uint32_t	NumOfFrames;
	uint32_t	NumOfRepeatFrames;
	uint32_t	DelayBetweenFrames;
	uint32_t	DelayBetweenPoints;
	uint32_t	DelayLazerOnOff;
	Frame 		*frame;
}File_ilda;

/**
  * @brief  RNG configuration
  * @param  None
  * @retval None
  *    Get a 32bit Random number     
  *    random32bit = RNG_GetRandomNumber();
  */
void RNG_Config(void)
{  
 /* Enable RNG clock source */
  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);

  /* RNG Peripheral enable */
  RNG_Cmd(ENABLE);
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
 	uint32_t 	i = 0;
 	uint32_t 	j = 0;
 	uint32_t 	k = 0;
 	uint32_t 	n = 0;
 	uint32_t 	fsize = 0;
//	uint32_t 	random32bit;
 	FATFS   	fs;
//	uint8_t 	Buff[24]= {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};
		
	uint32_t	Zagolovok[5] = {0,0,0,0,0};
	uint32_t	KolvoFreimov = 0;
	uint16_t	KolvoFigurVFreime = 0;
	uint16_t	KolvoTochekVFigure = 0;
	uint16_t	KolvoByteVFigure = 0;
	uint16_t	valueX = 0;
	uint16_t	valueY = 0;
	
	FIL     	file;
	FRESULT		fresult;
 	FILINFO		filinfo;
// 	DIR			dir;
//	TCHAR		buffstr[100];
//	TCHAR		filename[16];
	TCHAR		filepath[] = {"0:sun.bin"};
//	FILINFO *filinfo;
	uint8_t *file_in_memory = NULL;
	File_ilda nfile;
	
	uint32_t *pint;
	//=========================================================================
	
//	init_mempool(&malloc_mempool, sizeof(malloc_mempool));
	
	
	
	//=========================================================================
	STM_EVAL_LEDInit(LED_BLUE);
	STM_EVAL_LEDInit(LED_GREEN);
	STM_EVAL_LEDInit(LED_ORANGE);
	STM_EVAL_LEDInit(LED_RED);
	//=========================================================================
	
	UART_Configuration();
	#ifdef SD_DMA_MODE
	SD_NVIC_Configuration();
	#endif
	
	
//	initialization_set_xy();
//	GPIO_SetBits(GPIOA, TTL);
	
	
	
	printf("f_mount:\n");
	fresult = f_mount(0, &fs);
	FR_print_error(fresult);

	//=========================================================================
 	printf("f_stat:\n");
 	f_stat(filepath, &filinfo);
	printf("filesize %s:%u\n",filepath,(uint32_t)(filinfo.fsize));
	//=========================================================================
	
	
	
	
	//=========================================================================
	fsize = (uint32_t)(filinfo.fsize);
	file_in_memory = (uint8_t*)malloc(fsize * sizeof(uint8_t));
	if (file_in_memory == NULL) 
	{
		printf("dermo!\n");
		return (1);
	}
	else printf("malloc OK! = %u\n",sizeof(*file_in_memory));
	//=========================================================================
	
	
	
	
	
	
	
	//=========================================================================
	printf("f_open:\n");
	fresult = f_open(&file, filepath, FA_OPEN_EXISTING | FA_READ);
	FR_print_error(fresult);
	
	printf("f_read:\n");
 	f_read(&file,(uint8_t*)file_in_memory,fsize,0);
	for(i=0;i<fsize;++i)printf("0x%02X ",file_in_memory[i]);
	printf("\n");
	file_in_memory[0] = 0xaa;
	file_in_memory[1] = 0xbb;
	file_in_memory[2] = 0xcc;
	file_in_memory[3] = 0xdd;
	
	i=0;
	KolvoFreimov = (uint32_t)file_in_memory[i];
	printf("\nKolvoFreimov = 0x%08X =%u;\n",KolvoFreimov,KolvoFreimov);
	
	pint = (uint32_t*)(file_in_memory[i]);
	
	i=0;
	for(i=0;i<sizeof(KolvoFreimov);i++)
	{
		KolvoFreimov |= (uint32_t)(file_in_memory[i])<<(8*i) ;
		printf("\nfile_in_memory[i] = 0x%02X =%u;, i=%u\n",file_in_memory[i],file_in_memory[i],i);
		printf("\nfile_in_memory[i] = 0x%02X =%u;, i=%u\n",file_in_memory[i]<<(8*i),file_in_memory[i]<<(8*i),i);
		printf("\nNumOfFrames = 0x%08X =%u;\n",KolvoFreimov,KolvoFreimov);
	}
	printf("\nNumOfFrames = 0x%08X =%u;\n",KolvoFreimov,KolvoFreimov);
	
	
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
	
	
	
	
 	printf("f_close:\n");
 	fresult = f_close(&file);
 	FR_print_error(fresult);
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
	
	f_mount(0, NULL);
	
	
	while(1) /* Infinite loop */
	{
		//STM_EVAL_LEDToggle(LED_BLUE);
		//STM_EVAL_LEDToggle(LED_GREEN);
		STM_EVAL_LEDToggle(LED_ORANGE);
		STM_EVAL_LEDToggle(LED_RED);
		for(i=0;i<0x01FFFFFF;++i){__NOP();};
		//printf("k = %i\n",k++);
	}
}
//******************************************************************************

