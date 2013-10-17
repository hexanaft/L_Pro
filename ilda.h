//******************************************************************************
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ILDA_H
#define __ILDA_H

#include <inttypes.h>
#include <stdio.h>
#include "ff.h"

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
	uint32_t	DelayLazerOff;
	uint32_t	DelayBetweenFrames;
	uint32_t	DelayBetweenPoints;
	uint32_t	DelayLazerOn;
//	Frame_t 		*frame;
}Head_ilda_t;


void PrintFromMemFrameILDA( uint8_t* frame, uint32_t SizeOfFrame );
uint32_t ReadToMemHeadILDA( FIL *fp, Head_ilda_t* Head_ilda );
void PrintHeadILDA( Head_ilda_t * Head_ilda );
void ReadAllSizeFrame( FIL *fp, uint32_t NumOfFrames, uint32_t * AdrOfFrame, uint32_t * SizeOfFrame );
void ReadToMemFrameILDA( FIL *fp, uint8_t* frame, uint32_t PointerToFrame, uint32_t SizeOfFrame);
uint32_t toint( uint8_t * buf );
uint16_t toshort( uint8_t * buf );

#endif /* __ILDA_H */
//******************************************************************************
