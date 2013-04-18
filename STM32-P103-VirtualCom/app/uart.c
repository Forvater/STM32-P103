/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2006
 *
 *    File name   : uart.c
 *    Description : UARTs module
 *
 *    History :
 *    1. Date        : August 11, 2006
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 15135 $
**************************************************************************/
#define UART_GLOBAL
#include "uart.h"

pUartFifo_t pUart1RxFifo; // Pointer to a FIFO Buffer of the UART1 Receive
pUartFifo_t pUart1TxFifo; // Pointer to a FIFO Buffer of the UART1 Transmit
pUartFifo_t pUart2RxFifo; // Pointer to a FIFO Buffer of the UART2 Receive
pUartFifo_t pUart2TxFifo; // Pointer to a FIFO Buffer of the UART2 Transmit
pUartFifo_t pUart3RxFifo; // Pointer to a FIFO Buffer of the UART3 Receive
pUartFifo_t pUart3TxFifo; // Pointer to a FIFO Buffer of the UART3 Transmit

// Hold UART1 Evens (PE, BI, FE, OE)
UartLineEvents_t Uart1LineEvents;
// Hold UART2 Evens (PE, BI, FE, OE)
UartLineEvents_t Uart2LineEvents;
// Hold UART3 Evens (PE, BI, FE, OE)
UartLineEvents_t Uart3LineEvents;

/*************************************************************************
 * Function Name: FifoPush
 * Parameters: pUartFifo_t Fifo, Int8U Data
 *
 * Return: Boolean
 *
 * Description: Push a char in a FIFO. Return TRUE when push is successful
 *  or FALSE when the FIFO is full.
 *
 *************************************************************************/
static Boolean FifoPush(pUartFifo_t Fifo, Int8U Data)
{
Int32U IndxTmp;

  // calculate next push index
  IndxTmp = Fifo->PushIndx + 1;
  IndxTmp = IndxTmp % UART_FIFO_SIZE;

  // Check FIFO state
  if (IndxTmp == Fifo->PopIndx)
  {
    // The FIFO is full
    return(FALSE);
  }
  // Push the data
  Fifo->Buffer[Fifo->PushIndx] = Data;
  // Updating the push's index
  Fifo->PushIndx = IndxTmp;
  return(TRUE);
}

/*************************************************************************
 * Function Name: FifoPop
 * Parameters: pUartFifo_t Fifo, Int8U Data
 *
 * Return: Boolean
 *
 * Description: Pop a char from a FIFO. Return TRUE when pop is successful
 *  or FALSE when the FIFO is empty.
 *
 *************************************************************************/
static Boolean FifoPop(pUartFifo_t Fifo, pInt8U pData)
{
Int32U IndxTmp;

  // Check FIFO state
  if (Fifo->PushIndx == Fifo->PopIndx)
  {
    // The FIFO is empty
    return(FALSE);
  }
  // Calculate the next pop index
  IndxTmp = Fifo->PopIndx + 1;
  IndxTmp = IndxTmp % UART_FIFO_SIZE;
  // Pop the data
  *pData = Fifo->Buffer[Fifo->PopIndx];
  // Updating of the pop's index
  Fifo->PopIndx = IndxTmp;
  return(TRUE);
}


/*************************************************************************
 * Function Name: Uart1Isr
 * Parameters: none
 *
 * Return: none
 *
 * Description: UART 1 interrupt routine
 *
 *************************************************************************/
void Uart1Isr(void)
{
Int8U Data;
  // Recognizing the interrupt event
  if(USART_GetFlagStatus(USART1,USART_FLAG_ORE))
  {
    USART_ClearFlag(USART1,USART_FLAG_ORE);
    // Overrun Error
    Uart1LineEvents.bOE = TRUE;
  }
  if(USART_GetFlagStatus(USART1,USART_FLAG_FE))
  {
    USART_ClearFlag(USART1,USART_FLAG_FE);
    // Framing Error
    Uart1LineEvents.bFE = TRUE;
  }
  if(USART_GetFlagStatus(USART1,USART_FLAG_NE))
  {
    USART_ClearFlag(USART1,USART_FLAG_NE);
    // Noise Error
    Uart1LineEvents.bFE = TRUE;
  }
  if(USART_GetFlagStatus(USART1,USART_FLAG_PE))
  {
    USART_ClearFlag(USART1,USART_FLAG_PE);
    // Parity Error
    Uart1LineEvents.bPE = TRUE;
  }
  // Push a new data into the receiver buffer
  if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE))
  {
    // Push a new data into the receiver buffer
    if(!FifoPush(pUart1RxFifo,USART_ReceiveData(USART1)))
    {
      // the FIFO is full
      Uart1LineEvents.bOE = TRUE;
    }
  }
  if(   USART_GetFlagStatus(USART1,USART_FLAG_TXE)
     && (USART_GetITStatus (USART1,USART_IT_TXE) == SET))
  {
    if(FifoPop(pUart1TxFifo,&Data))
    {
      USART_SendData(USART1,Data);
    }
    else
    {
      USART_ITConfig(USART1,USART_IT_TXE ,DISABLE);
    }
  }
}

/*************************************************************************
 * Function Name: Uart2Isr
 * Parameters: none
 *
 * Return: none
 *
 * Description: UART 2 interrupt routine
 *
 *************************************************************************/
void Uart2Isr(void)
{
Int8U Data;
  // Recognizing the interrupt event
  if(USART_GetFlagStatus(USART2,USART_FLAG_ORE))
  {
    USART_ClearFlag(USART2,USART_FLAG_ORE);
    // Overrun Error
    Uart2LineEvents.bOE = TRUE;
  }
  if(USART_GetFlagStatus(USART2,USART_FLAG_FE))
  {
    USART_ClearFlag(USART2,USART_FLAG_FE);
    // Framing Error
    Uart2LineEvents.bFE = TRUE;
  }
  if(USART_GetFlagStatus(USART2,USART_FLAG_NE))
  {
    USART_ClearFlag(USART2,USART_FLAG_NE);
    // Noise Error
    Uart2LineEvents.bFE = TRUE;
  }
  if(USART_GetFlagStatus(USART2,USART_FLAG_PE))
  {
    USART_ClearFlag(USART2,USART_FLAG_PE);
    // Parity Error
    Uart2LineEvents.bPE = TRUE;
  }
  // Push a new data into the receiver buffer
  if(USART_GetFlagStatus(USART2,USART_FLAG_RXNE))
  {
    // Push a new data into the receiver buffer
    if(!FifoPush(pUart2RxFifo,USART_ReceiveData(USART2)))
    {
      // the FIFO is full
      Uart2LineEvents.bOE = TRUE;
    }
  }
  if(   USART_GetFlagStatus(USART2,USART_FLAG_TXE)
     && (USART_GetITStatus (USART2,USART_IT_TXE) == SET))
  {
    if(FifoPop(pUart2TxFifo,&Data))
    {
      USART_SendData(USART2,Data);
    }
    else
    {
      USART_ITConfig(USART2,USART_IT_TXE ,DISABLE);
    }
  }
}

/*************************************************************************
 * Function Name: Uart3Isr
 * Parameters: none
 *
 * Return: none
 *
 * Description: UART 3 interrupt routine
 *
 *************************************************************************/
void Uart3Isr(void)
{
Int8U Data;
  // Recognizing the interrupt event
  if(USART_GetFlagStatus(USART3,USART_FLAG_ORE))
  {
    USART_ClearFlag(USART3,USART_FLAG_ORE);
    // Overrun Error
    Uart3LineEvents.bOE = TRUE;
  }
  if(USART_GetFlagStatus(USART3,USART_FLAG_FE))
  {
    USART_ClearFlag(USART3,USART_FLAG_FE);
    // Framing Error
    Uart3LineEvents.bFE = TRUE;
  }
  if(USART_GetFlagStatus(USART3,USART_FLAG_NE))
  {
    USART_ClearFlag(USART3,USART_FLAG_NE);
    // Noise Error
    Uart3LineEvents.bFE = TRUE;
  }
  if(USART_GetFlagStatus(USART3,USART_FLAG_PE))
  {
    USART_ClearFlag(USART3,USART_FLAG_PE);
    // Parity Error
    Uart3LineEvents.bPE = TRUE;
  }
  if(USART_GetFlagStatus(USART3,USART_FLAG_RXNE))
  {
    // Push a new data into the receiver buffer
    if(!FifoPush(pUart3RxFifo,USART_ReceiveData(USART3)))
    {
      // the FIFO is full
      Uart3LineEvents.bOE = TRUE;
    }
  }
  if(   USART_GetFlagStatus(USART3,USART_FLAG_TXE)
     && (USART_GetITStatus (USART3,USART_IT_TXE) == SET))
  {
    if(FifoPop(pUart3TxFifo,&Data))
    {
      USART_SendData(USART3,Data);
    }
    else
    {
      USART_ITConfig(USART3,USART_IT_TXE ,DISABLE);
    }
  }
}

/*************************************************************************
 * Function Name: UartInit
 * Parameters: Int32U IrqSlot
 *
 * Return: Boolean
 *
 * Description: Init UART
 *
 *************************************************************************/
Boolean UartInit(UartNum_t Uart,Int32U IrqSlot)
{
volatile Int8U Tmp;
USART_InitTypeDef  UART_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
GPIO_InitTypeDef GPIO_InitStructure;


  USART_StructInit(&UART_InitStructure);
  UART_InitStructure.USART_Parity = USART_Parity_No;

  switch(Uart)
  {
  case UART_1:
    pUart1RxFifo = (pUartFifo_t)malloc(sizeof(UartFifo_t));
    if(pUart1RxFifo == NULL)
    {
      return(FALSE);
    }
    pUart1TxFifo = (pUartFifo_t)malloc(sizeof(UartFifo_t));
    if(pUart1TxFifo == NULL)
    {
      free(pUart1RxFifo);
      return(FALSE);
    }
    // Init receive and transmit FIFOs
    pUart1RxFifo->PopIndx = pUart1RxFifo->PushIndx = \
    pUart1TxFifo->PopIndx = pUart1TxFifo->PushIndx = 0;

    // Release reset and enable clock
    USART_DeInit(USART1);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    // GPIO Init
    // Enable GPIO clock and release reset
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,
                           ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA,
                           DISABLE);
    GPIO_PinRemapConfig(GPIO_Remap_USART1,DISABLE);

    // Assign PA9 to UART1 (Tx)
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Assign PA10 to UART1 (Rx)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Init UART1
    USART_Init(USART1,&UART_InitStructure);

    // Enable and configure the priority of the UART1 Update IRQ Channel
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART1_INTR_PRI;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ClearFlag(USART1, USART_FLAG_CTS | USART_FLAG_LBD  | USART_FLAG_TXE  |
                            USART_FLAG_TC  | USART_FLAG_RXNE | USART_FLAG_IDLE |
                            USART_FLAG_ORE | USART_FLAG_NE   | USART_FLAG_FE |
                            USART_FLAG_PE);

    // Enable UART1 interrupts
    USART_ITConfig(USART1,USART_IT_PE  ,ENABLE);
    USART_ITConfig(USART1,USART_IT_TC  ,DISABLE);
    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
    USART_ITConfig(USART1,USART_IT_IDLE,DISABLE);
    USART_ITConfig(USART1,USART_IT_LBD ,DISABLE);
    USART_ITConfig(USART1,USART_IT_CTS ,DISABLE);
    USART_ITConfig(USART1,USART_IT_ERR ,DISABLE);

    // Enable the UART1
    USART_Cmd(USART1, ENABLE);
    break;
  case UART_2:
    pUart2RxFifo = (pUartFifo_t)malloc(sizeof(UartFifo_t));
    if(pUart2RxFifo == NULL)
    {
      return(FALSE);
    }
    pUart2TxFifo = (pUartFifo_t)malloc(sizeof(UartFifo_t));
    if(pUart2TxFifo == NULL)
    {
      free(pUart2RxFifo);
      return(FALSE);
    }
    // Init receive and transmit FIFOs
    pUart2RxFifo->PopIndx = pUart2RxFifo->PushIndx = \
    pUart2TxFifo->PopIndx = pUart2TxFifo->PushIndx = 0;

    // Release reset and enable clock
    USART_DeInit(USART2);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // GPIO Init
    // Enable GPIO clock and release reset
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,
                           ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA,
                           DISABLE);
    GPIO_PinRemapConfig(GPIO_Remap_USART2,DISABLE);

    // Assign PA2 to UART2 (Tx)
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Assign PA3 to UART2 (Rx)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Init UART2
    USART_Init(USART2,&UART_InitStructure);

    // Enable and configure the priority of the UART2 Update IRQ Channel
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART2_INTR_PRI;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ClearFlag(USART2, USART_FLAG_CTS | USART_FLAG_LBD  | USART_FLAG_TXE  |
                            USART_FLAG_TC  | USART_FLAG_RXNE | USART_FLAG_IDLE |
                            USART_FLAG_ORE | USART_FLAG_NE   | USART_FLAG_FE |
                            USART_FLAG_PE);

    // Enable UART2 interrupts
    USART_ITConfig(USART2,USART_IT_PE  ,ENABLE);
    USART_ITConfig(USART2,USART_IT_TXE ,ENABLE);
    USART_ITConfig(USART2,USART_IT_TC  ,DISABLE);
    USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
    USART_ITConfig(USART2,USART_IT_IDLE,DISABLE);
    USART_ITConfig(USART2,USART_IT_LBD ,DISABLE);
    USART_ITConfig(USART2,USART_IT_CTS ,DISABLE);
    USART_ITConfig(USART2,USART_IT_ERR ,DISABLE);

    // Enable the UART2
    USART_Cmd(USART2, ENABLE);
    break;
  case UART_3:
    pUart3RxFifo = (pUartFifo_t)malloc(sizeof(UartFifo_t));
    if(pUart3RxFifo == NULL)
    {
      return(FALSE);
    }
    pUart3TxFifo = (pUartFifo_t)malloc(sizeof(UartFifo_t));
    if(pUart3TxFifo == NULL)
    {
      free(pUart3RxFifo);
      return(FALSE);
    }
    // Init receive and transmit FIFOs
    pUart3RxFifo->PopIndx = pUart3RxFifo->PushIndx = \
    pUart3TxFifo->PopIndx = pUart3TxFifo->PushIndx = 0;

    // Release reset and enable clock
    USART_DeInit(USART3);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // GPIO Init
    // Enable GPIO clock and release reset
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO,
                           ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO,
                           DISABLE);
    GPIO_PinRemapConfig(GPIO_PartialRemap_USART3,ENABLE);

    // Assign PC10 to UART3 (Tx)
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Assign PC11 to UART3 (Rx)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Init UART3
    USART_Init(USART3,&UART_InitStructure);

    // Enable and configure the priority of the UART3 Update IRQ Channel
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART3_INTR_PRI;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ClearFlag(USART3, USART_FLAG_CTS | USART_FLAG_LBD  | USART_FLAG_TXE  |
                            USART_FLAG_TC  | USART_FLAG_RXNE | USART_FLAG_IDLE |
                            USART_FLAG_ORE | USART_FLAG_NE   | USART_FLAG_FE |
                            USART_FLAG_PE);

    // Enable UART3 interrupts
    USART_ITConfig(USART3,USART_IT_PE  ,ENABLE);
    USART_ITConfig(USART3,USART_IT_TXE ,DISABLE);
    USART_ITConfig(USART3,USART_IT_TC  ,DISABLE);
    USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
    USART_ITConfig(USART3,USART_IT_IDLE,DISABLE);
    USART_ITConfig(USART3,USART_IT_LBD ,DISABLE);
    USART_ITConfig(USART3,USART_IT_CTS ,DISABLE);
    USART_ITConfig(USART3,USART_IT_ERR ,DISABLE);

    // Enable the UART3
    USART_Cmd(USART3, ENABLE);
    break;
  default:
    return(FALSE);
  }
  return(TRUE);
}

/*************************************************************************
 * Function Name: UartSetLineCoding
 * Parameters:  UartNum_t Uart, USART_InitTypeDef pUartCoding
 *
 * Return: None
 *
 * Description: Init UART Baud rate, Word width, Stop bits, Parity type
 *
 *************************************************************************/
void UartSetLineCoding(UartNum_t Uart,USART_InitTypeDef UartCoding)
{
USART_InitTypeDef  UART_InitStructure;
USART_TypeDef * pUart;

  USART_StructInit(&UART_InitStructure);

  switch(Uart)
  {
  case UART_1:
    pUart = USART1;
    break;
  case UART_2:
    pUart = USART2;
    break;
  case UART_3:
    pUart = USART3;
    break;
  default:
    return;
  }

  UART_InitStructure.USART_WordLength = UartCoding.USART_WordLength;
  UART_InitStructure.USART_StopBits   = UartCoding.USART_StopBits;
  UART_InitStructure.USART_Parity     = UartCoding.USART_Parity;
  UART_InitStructure.USART_BaudRate   = UartCoding.USART_BaudRate;

  USART_Init(pUart,&UART_InitStructure);
}

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
Int32U UartRead(UartNum_t Uart, pInt8U pBuffer, Int32U BufferSize)
{
Int32U Count;
pUartFifo_t pUartFifo;

  switch(Uart)
  {
  case UART_1:
    pUartFifo = pUart1RxFifo;
    break;
  case UART_2:
    pUartFifo = pUart2RxFifo;
    break;
  case UART_3:
    pUartFifo = pUart3RxFifo;
    break;
  default:
    return(0);
  }

  for (Count = 0; Count < BufferSize; ++Count)
  {
    ENTR_CRT_SECTION();
    if(!FifoPop(pUartFifo,pBuffer+Count))
    {
      EXT_CRT_SECTION();
      break;
    }
    EXT_CRT_SECTION();
  }
  return(Count);
}

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
Int32U UartWrite(UartNum_t Uart, pInt8U pBuffer, Int32U BufferSize)
{
Int32U Count = 0;
pUartFifo_t pUartFifo;
USART_TypeDef * pUart;

  switch(Uart)
  {
  case UART_1:
    pUart = USART1;
    pUartFifo = pUart1TxFifo;
    break;
  case UART_2:
    pUart = USART2;
    pUartFifo = pUart2TxFifo;
    break;
  case UART_3:
    pUart = USART3;
    pUartFifo = pUart3TxFifo;
    break;
  default:
    return(0);
  }

  if(BufferSize != 0)
  {
    ENTR_CRT_SECTION();
    if(pUartFifo->PopIndx == pUartFifo->PushIndx)
    {
      USART_SendData(pUart,*pBuffer);
      USART_ITConfig(pUart,USART_IT_TXE ,ENABLE);
      ++Count;
    }

    for ( ; Count < BufferSize; ++Count)
    {
      if(!FifoPush(pUartFifo,*(pBuffer+Count)))
      {
        break;
      }
    }
    EXT_CRT_SECTION();
  }
  return(Count);
}

/*************************************************************************
 * Function Name: UartGetUartEvents
 * Parameters:  UartNum_t Uart
 *
 * Return: UartLineEvents_t
 *
 * Description: Get Uart Line events (PE,OE, FE, BI)
 *
 *************************************************************************/
UartLineEvents_t UartGetUartLineEvents (UartNum_t Uart)

{
UartLineEvents_t  LineEvents;
  LineEvents.Data = 0;
  switch(Uart)
  {
  case UART_1:
    LineEvents.Data = AtomicExchange(LineEvents.Data,
                                     &Uart1LineEvents.Data);
    break;
  case UART_2:
    LineEvents.Data = AtomicExchange(LineEvents.Data,
                                     &Uart2LineEvents.Data);
    break;
  case UART_3:
    LineEvents.Data = AtomicExchange(LineEvents.Data,
                                     &Uart3LineEvents.Data);
    break;
  }
  return(LineEvents);
}

/*************************************************************************
 * Function Name: UartSetUartLineState
 * Parameters:  UartNum_t Uart UartNum_t Uart, Boolean Break
 *
 * Return: none
 *
 * Description: Set Uart Break Event
 *
 *************************************************************************/
void UartSetUartLineState (UartNum_t Uart, Boolean Break)
{
USART_TypeDef * pUart;

  if(Break == FALSE)
  {
    return;
  }

  switch(Uart)
  {
  case UART_1:
    pUart = USART1;
    break;
  case UART_2:
    pUart = USART2;
    break;
  case UART_3:
    pUart = USART3;
    break;
  default:
    return;
  }
  USART_SendBreak(pUart);
}
