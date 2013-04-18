/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2006
 *
 *    File name   : uart.h
 *    Description : UARTs module header
 *
 *    History :
 *    1. Date        :  August 11, 2006
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 15135 $
**************************************************************************/
#include <includes.h>

#ifndef __UART_H
#define __UART_H

#ifdef UART_GLOBAL
#define UART_EXTERN
#else
#define UART_EXTERN  extern
#endif

#define UART_FIFO_SIZE        50
#define UART_MAX_BAUD_RATE    256000
#define UART1_INTR_PRI        5
#define UART2_INTR_PRI        6
#define UART3_INTR_PRI        7

// Define UARTs
typedef enum
{
  UART_1 = 0, UART_2, UART_3,
} UartNum_t;

typedef struct _UartFifo_t
{
  Int32U PushIndx;
  Int32U PopIndx;
  Int8U  Buffer[UART_FIFO_SIZE];
} UartFifo_t, *pUartFifo_t;

typedef union _UartModemLineState_t
{
  Int32U Data;
  struct {
    Int32U  bDTR          : 1;
    Int32U  bRTS          : 1;
    Int32U                :30;
  };
} UartModemLineState_t, *pUartModemLineState_t;

typedef union _UartLineEvents_t
{
  Int32U Data;
  struct {
    Int32U              : 1;
    Int32U  bOE         : 1;
    Int32U  bPE         : 1;
    Int32U  bFE         : 1;
    Int32U  bBI         : 1;
    Int32U              :27;
  };
} UartLineEvents_t, *pUartLineEvents_t;

typedef union _UartModemEvents_t
{
  Int32U Data;
  struct {
    Int32U  bDCTS       : 1;
    Int32U  bDDSR       : 1;
    Int32U  bDRI        : 1;
    Int32U  bDDCD       : 1;
    Int32U  bCTS        : 1;
    Int32U  bDSR        : 1;
    Int32U  bRI         : 1;
    Int32U  bDCD        : 1;
    Int32U              :24;
  };
} UartModemEvents_t, *pUartModemEvents_t;

/*************************************************************************
 * Function Name: Uart0Isr
 * Parameters: none
 *
 * Return: none
 *
 * Description: UART 0 interrupt routine
 *
 *************************************************************************/
void Uart0Isr(void);

/*************************************************************************
 * Function Name: Uart1Isr
 * Parameters: none
 *
 * Return: none
 *
 * Description: UART 1 interrupt routine
 *
 *************************************************************************/
void Uart1Isr(void);

/*************************************************************************
 * Function Name: UartInit
 * Parameters: Int32U IrqSlot
 *
 * Return: Boolean
 *
 * Description: Init UART
 *
 *************************************************************************/
Boolean UartInit(UartNum_t Uart,Int32U IrqSlot);

/*************************************************************************
 * Function Name: UartSetLineCoding
 * Parameters:  UartNum_t Uart,USART_InitTypeDef pUartCoding
 *
 * Return: None
 *
 * Description: Init UART Baud rate, Word width, Stop bits, Parity type
 *
 *************************************************************************/
void UartSetLineCoding(UartNum_t Uart,USART_InitTypeDef UartCoding);

/*************************************************************************
 * Function Name: UartRead
 * Parameters:  UartNum_t Uart, pInt8U pBuffer, Int32U BufferSize
 *
 * Return: Int32U
 *
 * Description: Read received data from UART.
 *              Return number of readied characters
 *
 *************************************************************************/
Int32U UartRead(UartNum_t Uart, pInt8U pBuffer, Int32U BufferSize);

/*************************************************************************
 * Function Name: UartWrite
 * Parameters:  UartNum_t Uart, pInt8U pBuffer, Int32U BufferSize
 *
 * Return: Int32U
 *
 * Description: Write a data to UART. Return number of successful
 *  transmitted bytes
 *
 *************************************************************************/
Int32U UartWrite(UartNum_t Uart, pInt8U pBuffer, Int32U BufferSize);

/*************************************************************************
 * Function Name: UartGetUartEvents
 * Parameters:  UartNum_t Uart
 *
 * Return: UartLineEvents_t
 *
 * Description: Get Uart Line events (PE,OE, FE, BI)
 *
 *************************************************************************/
UartLineEvents_t UartGetUartLineEvents (UartNum_t Uart);

/*************************************************************************
 * Function Name: UartSetUartLineState
 * Parameters:  UartNum_t Uart UartNum_t Uart, Boolean Break
 *
 * Return: none
 *
 * Description: Set Uart Break Event
 *
 *************************************************************************/
void UartSetUartLineState (UartNum_t Uart, Boolean Break);

#endif // __UART_H
