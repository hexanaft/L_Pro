#include "ilda.h"

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

void PrintFromMemFrameILDA( uint8_t* frame, uint32_t SizeOfFrame )
{
	uint32_t i=0;
	uint8_t j=0;
	printf("Pechataem sam freim:\n");
	for(i=0;i<SizeOfFrame;i++)
	{
		printf("%02X ",frame[i]);
		if( ++j == 8 ){j=0;printf("\n");}
		//printf("%3u:%2X ",i,frame[i]);
	}
	printf("\n");
}

uint32_t ReadToMemHeadILDA( FIL *fp, Head_ilda_t* Head_ilda )
{
	printf("ReadToMemHeadILDA:%u\n",sizeof(Head_ilda_t));
	printf("f_read:\n");
	f_read(fp,(void*)Head_ilda,sizeof(Head_ilda_t),0);
	
	return sizeof(Head_ilda_t);
}

void PrintHeadILDA( Head_ilda_t * Head_ilda )
{
	printf("Pechataem zagolovok:\n");
	printf("	NumOfFrames:        %u\n",Head_ilda->NumOfFrames);
	printf("	DelayLazerOff:      %u\n",Head_ilda->DelayLazerOff);
	printf("	DelayBetweenFrames: %u\n",Head_ilda->DelayBetweenFrames);
	printf("	DelayBetweenPoints: %u\n",Head_ilda->DelayBetweenPoints);
	printf("	DelayLazerOn:       %u\n",Head_ilda->DelayLazerOn);
}

void ReadAllSizeFrame( FIL *fp, uint32_t NumOfFrames, uint32_t * AdrOfFrame, uint32_t * SizeOfFrame )
{
	uint16_t SizeOfFrame_tmp = 0;
	uint32_t i = 0;
	
	AdrOfFrame[0] = sizeof(Head_ilda_t);
	//printf("AdrNumOfFrame %u:%u byte\n",0,AdrOfFrame[0]);
	
	for(i=1;i<=NumOfFrames;i++)
	{
		f_lseek(fp,(int)AdrOfFrame[i-1]);
		f_read(fp,(void*)&SizeOfFrame_tmp,sizeof(SizeOfFrame_tmp),0);
		SizeOfFrame[i-1] = (uint32_t)SizeOfFrame_tmp + 2; //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		AdrOfFrame[i] = AdrOfFrame[i-1] + SizeOfFrame[i-1];
		//printf("AdrNumOfFrame %u:%u byte\n",i,AdrOfFrame[i]);
		//printf("SizeOfFrame %u:%u byte\n",i-1,SizeOfFrame[i-1]);
	}
}

void ReadToMemFrameILDA( FIL *fp, uint8_t* frame, uint32_t PointerToFrame, uint32_t SizeOfFrame)
{
 	//printf("PointerToFrame:%u\n",PointerToFrame);
 	f_lseek(fp,(int)PointerToFrame);
 	
 	//printf("f_read:\n");
 	f_read(fp,(void*)frame,SizeOfFrame,0);
}

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
