#include "uart_to_printf.h"


//******************************************************************************

void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**************************************************************************************/

void RCC_Configuration(void)
{
  /* --------------------------- System Clocks Configuration -----------------*/
  /* DBG_UART clock enable */
  RCC_DBG_UART_CLK_INIT(RCC_DBG_UART_CLK, ENABLE);

  /* GPIOD clock enable */
  RCC_DBG_UART_GPIO_CLK_INIT(RCC_DBG_UART_GPIO_CLK, ENABLE);
}

/**************************************************************************************/

void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*-------------------------- GPIO Configuration ----------------------------*/
  GPIO_InitStructure.GPIO_Pin = DBG_UART_TX_PIN | DBG_UART_RX_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(DBG_UART_TX_GPIO_PORT, &GPIO_InitStructure);
  GPIO_Init(DBG_UART_RX_GPIO_PORT, &GPIO_InitStructure);

  /* Connect USART pins to AF */
  GPIO_PinAFConfig(DBG_UART_TX_GPIO_PORT, DBG_UART_TX_SOURCE, DBG_UART_GPIO_AF);  // DBG_UART_TX
  GPIO_PinAFConfig(DBG_UART_RX_GPIO_PORT, DBG_UART_RX_SOURCE, DBG_UART_GPIO_AF);  // DBG_UART_RX
}

/**************************************************************************************/

void DBG_UART_Configuration(void)
{
	USART_InitTypeDef USART_InitStructure;

  /* USARTx configuration ------------------------------------------------------*/
  /* USARTx configured as follow:
        - BaudRate = 115200 baud
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(DBG_UART, &USART_InitStructure);

  USART_Cmd(DBG_UART, ENABLE);
}

void UART_Configuration(void)
{
 	NVIC_Configuration(); /* Interrupt Config */
 	RCC_Configuration();
 	GPIO_Configuration();
	DBG_UART_Configuration();
}

//******************************************************************************
// Hosting of stdio functionality through DBG_UART
//******************************************************************************

#include <rt_misc.h>

#pragma import(__use_no_semihosting_swi)

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f)
{
	static int last;

	if ((ch == (int)'\n') && (last != (int)'\r'))
	{
		last = (int)'\r';

  	while(USART_GetFlagStatus(DBG_UART, USART_FLAG_TXE) == RESET);

 	  USART_SendData(DBG_UART, last);
	}
	else
		last = ch;

	while(USART_GetFlagStatus(DBG_UART, USART_FLAG_TXE) == RESET);

  USART_SendData(DBG_UART, ch);

  return(ch);
}

int fgetc(FILE *f)
{
	char ch;

	while(USART_GetFlagStatus(DBG_UART, USART_FLAG_RXNE) == RESET);

	ch = USART_ReceiveData(DBG_UART);

  return((int)ch);
}

int ferror(FILE *f)
{
  /* Your implementation of ferror */
  return EOF;
}

void _ttywrch(int ch)
{
	static int last;

	if ((ch == (int)'\n') && (last != (int)'\r'))
	{
		last = (int)'\r';

  	while(USART_GetFlagStatus(DBG_UART, USART_FLAG_TXE) == RESET);

 	  USART_SendData(DBG_UART, last);
	}
	else
		last = ch;

	while(USART_GetFlagStatus(DBG_UART, USART_FLAG_TXE) == RESET);

  USART_SendData(DBG_UART, ch);
}

void _sys_exit(int return_code)
{
label:  goto label;  /* endless loop */
}

//******************************************************************************

