/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : mmc_ll_SPI2.h
 *    Description : Low level MMC SPI diver
 *
 *    History :
 *    1. Date        : October 24, 2007
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 1.1.2.3 $
 **************************************************************************/
extern void Dly100us(void *arg);

#define SD_CP      GPIO_Pin_7
#define SD_WP      GPIO_Pin_6

#define SD_CS      GPIO_Pin_12
#define SD_SCLK    GPIO_Pin_13
#define SD_MOSI    GPIO_Pin_14
#define SD_MISO    GPIO_Pin_15

#if SPI_DMA_ENA > 0
typedef enum _SPI_TransferDir_t {
  SPI_RECEIVE = 0, SPI_TRANSMIT
} SPI_TransferDir_t;

#ifndef DMA_ERRATA
volatile Boolean TransferStatus;     // data transfer state
/*************************************************************************
 * Function Name: SPI2_DmaHandler
 * Parameters: none
 * Return: none
 *
 * Description: Interrupt handler of the SPI2 DMA.
 *
 *************************************************************************/
void SPI2_RxDmaHandler(void)
{
  if (DMA_GetFlagStatus(DMA_FLAG_TE4) == SET)
  {
    DMA_ClearITPendingBit(DMA_IT_GL5);
    DMA_Cmd(DMA_Channel5,DISABLE);
    DMA_Cmd(DMA_Channel4,DISABLE);
  }
  DMA_ClearITPendingBit(DMA_IT_GL4);
  TransferStatus = FALSE;
}

/*************************************************************************
 * Function Name: SPI2_DmaHandler
 * Parameters: none
 * Return: none
 *
 * Description: Interrupt handler of the SPI2 DMA.
 *
 *************************************************************************/
void SPI2_TxDmaHandler(void)
{
  TransferStatus = FALSE;
  DMA_ClearITPendingBit(DMA_IT_GL4);
  DMA_Cmd(DMA_Channel5,DISABLE);
  DMA_Cmd(DMA_Channel4,DISABLE);
}
#endif

/*************************************************************************
 * Function Name: SPI2_DmaTransfer
 * Parameters: pInt8U pData,Int32U Size, SPI_TransferDir_t SPI_TransferDir
 * Return: none
 *
 * Description: DMA transfer
 *
 *************************************************************************/
void SPI2_DmaTransfer(pInt8U pData,Int32U Size, SPI_TransferDir_t SPI_TransferDir)
{
DMA_InitTypeDef  DMA_InitStructure;
Int32U Dummy = 0xFF;

  // Initialize DMA Rx channel
  DMA_DeInit(DMA_Channel4);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (SPI_TransferDir == SPI_RECEIVE)?(Int32U)pData:(Int32U)&Dummy;
  DMA_InitStructure.DMA_BufferSize = Size;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = (SPI_TransferDir == SPI_RECEIVE)?DMA_MemoryInc_Enable:DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  // Init channel
  DMA_Init(DMA_Channel4, &DMA_InitStructure);

  // Initialize DMA Tx channel
  DMA_DeInit(DMA_Channel5);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (SPI_TransferDir == SPI_TRANSMIT)?(Int32U)pData:(Int32U)&Dummy;
  DMA_InitStructure.DMA_BufferSize = Size;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = (SPI_TransferDir == SPI_TRANSMIT)?DMA_MemoryInc_Enable:DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  // Init channel
  DMA_Init(DMA_Channel5, &DMA_InitStructure);
  // Enable SPI2 DMA transfer
  SPI_DMACmd(SPI2,SPI_DMAReq_Rx,ENABLE);
  SPI_DMACmd(SPI2,SPI_DMAReq_Tx,ENABLE);


#ifdef DMA_ERRATA
  ENTR_CRT_SECTION();

  // Enable channel
  DMA_Cmd(DMA_Channel4,ENABLE);
  DMA_Cmd(DMA_Channel5,ENABLE);

  while(1)
  {
    if (  (DMA_GetITStatus(DMA_IT_TE4) == SET)
       || (DMA_GetITStatus(DMA_IT_TE5) == SET))
    {
      DMA_ClearITPendingBit(DMA_IT_GL4 | DMA_IT_GL5);
      DMA_Cmd(DMA_Channel5,DISABLE);
      DMA_Cmd(DMA_Channel4,DISABLE);
      break;
    }
    if (  (DMA_GetITStatus(DMA_IT_TC4) == SET)
       && (DMA_GetITStatus(DMA_IT_TC5) == SET))
    {
      break;
    }
  };
  EXT_CRT_SECTION();
#else
  // set the flag DMA Transfer in progress
  TransferStatus = TRUE;

  DMA_ITConfig(DMA_Channel4, DMA_IT_TC | DMA_IT_TE, ENABLE);
  DMA_ITConfig(DMA_Channel5, DMA_IT_TE, ENABLE);

  // Enable SPI2 DMA transfer
  SPI_DMACmd(SPI2,SPI_DMAReq_Rx,ENABLE);
  SPI_DMACmd(SPI2,SPI_DMAReq_Tx,ENABLE);
  while(TransferStatus);
#endif
  // wait until SPI transmit FIFO isn't empty
  while(SPI_GetFlagStatus(SPI2, SPI_FLAG_TXE)==RESET);
  // wait until SPI receive FIFO isn't empty
  while(SPI_GetFlagStatus(SPI2, SPI_FLAG_RXNE)==SET);

  SPI_DMACmd(SPI2,SPI_DMAReq_Tx,DISABLE);
  SPI_DMACmd(SPI2,SPI_DMAReq_Rx,DISABLE);

}
#endif // SPI_DMA_ENA > 0

/*************************************************************************
 * Function Name: MmcChipSelect
 * Parameters: Boolean Select
 * Return: none
 *
 * Description: Mmc Chip select control
 * Select = true  - Chip is enable
 * Select = false - Chip is disable
 *
 *************************************************************************/
void MmcChipSelect (Boolean Select)
{
  GPIO_WriteBit(GPIOB,SD_CS,Select?Bit_RESET:Bit_SET);
}
/*************************************************************************
 * Function Name: MmcPresent
 * Parameters: none
 * Return: Boolean - true cart present
 *                 - false cart no present
 *
 * Description: Mmc present check
 *
 *************************************************************************/
inline
Boolean MmcPresent (void)
{
  return(GPIO_ReadInputDataBit(GPIOC,SD_CP) == Bit_SET);
}

/*************************************************************************
 * Function Name: MmcWriteProtect
 * Parameters: none
 * Return: Boolean - true cart is protected
 *                 - false cart no protected
 *
 * Description: Mmc Write protect check
 *
 *************************************************************************/
inline
Boolean MmcWriteProtect (void)
{
  return(GPIO_ReadInputDataBit(GPIOC,SD_WP) == Bit_RESET);
}

/*************************************************************************
 * Function Name: MmcSetClockFreq
 * Parameters: Int32U Frequency
 * Return: Int32U
 *
 * Description: Set SPI ckl frequency
 *
 *************************************************************************/
Int32U MmcSetClockFreq (Int32U Frequency)
{
Int32U Div = 2;
Int32U DivVal = 0;
RCC_ClocksTypeDef Clk;

  RCC_GetClocksFreq(&Clk);

  while((Frequency * Div) <=  Clk.PCLK1_Frequency)
  {
    Div <<= 1;
    if (++DivVal == 7)
    {
      break;
    }
  }

  SPI2->CR1 = (SPI2->CR1 & ~(0x7 << 3)) | ((DivVal&0x7) << 3);

  // Return real frequency
  return(Clk.PCLK1_Frequency/Div);
}

/*************************************************************************
 * Function Name: MmcInit
 * Parameters: none
 * Return: none
 *
 * Description: Init SPI, Cart Present, Write Protect and Chip select pins
 *
 *************************************************************************/
void MmcInit (void)
{
SPI_InitTypeDef   SPI_InitStructure;
GPIO_InitTypeDef  GPIO_InitStructure;

  // Enable GPIO clocks
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
  // Enable SPI2 Periphery clock
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

  // Deinitializes the SPI2
  SPI_DeInit(SPI2);
  // Release reset of GPIOB, GPIOC
  RCC_APB2PeriphResetCmd(  RCC_APB2Periph_GPIOB
                         | RCC_APB2Periph_GPIOC, DISABLE);

  // Configure SPI2_CLK, SPI2_MOSI, SPI2_nCS1, Card Present and Write Protect pins
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = SD_CS;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = SD_SCLK | SD_MOSI | SD_MISO;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = SD_CP | SD_WP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  // Chip select
  MmcChipSelect(0);

  // Spi init
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI2, &SPI_InitStructure);

  // Enable SPI2 */
  SPI_Cmd(SPI2, ENABLE);

  // Clock Freq. Identification Mode < 400kHz
  MmcSetClockFreq(IdentificationModeClock);

#if SPI_DMA_ENA > 0

  // Enable DMA clock
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA, ENABLE);
  // Clear pending interrupt
  DMA_ClearITPendingBit( DMA_IT_GL4
                       | DMA_IT_GL5);

  // Interrupts DMA enable
  SPI_DMACmd(SPI2,SPI_DMAReq_Rx,DISABLE);
  SPI_DMACmd(SPI2,SPI_DMAReq_Tx,DISABLE);
#ifndef DMA_ERRATA
NVIC_InitTypeDef NVIC_InitStructure;
  // VIC configuration
  NVIC_InitStructure.NVIC_IRQChannel = DMAChannel4_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SPI_DMA_INTR_PRIO;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = DMAChannel5_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SPI_DMA_INTR_PRIO;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
#endif
#endif // SPI_DMA_ENA > 0
}

/*************************************************************************
 * Function Name: MmcTranserByte
 * Parameters: Int8U ch
 * Return: Int8U
 *
 * Description: Transfer byte by SPI
 *
 *************************************************************************/
Int8U MmcTranserByte (Int8U ch)
{
  SPI_SendData(SPI2, ch);
  while(SPI_GetFlagStatus(SPI2, SPI_FLAG_RXNE) == RESET);
  return(SPI_ReceiveData(SPI2));
}

/*************************************************************************
 * Function Name: MmcSendBlock
 * Parameters: pInt8U pData, Int32U Size
 *
 * Return: none
 *
 * Description: Send block by SPI
 *
 *************************************************************************/
void MmcSendBlock (pInt8U pData, Int32U Size)
{
#if SPI_DMA_ENA > 0
  SPI2_DmaTransfer(pData,Size,SPI_TRANSMIT);
#else
Int32U OutCount = Size;
  while (OutCount--)
  {
    SPI_SendData(SPI2, *pData++);
    while(SPI_GetFlagStatus(SPI2, SPI_FLAG_RXNE) == RESET);
    volatile Int32U Dummy = SPI_ReceiveData(SPI2);
  }
#endif // SPI_DMA_ENA > 0
}

/*************************************************************************
 * Function Name: MmcReceiveBlock
 * Parameters: pInt8U pData, Int32U Size
 *
 * Return: none
 *
 * Description: Read block by SPI
 *
 *************************************************************************/
void MmcReceiveBlock (pInt8U pData, Int32U Size)
{
#if SPI_DMA_ENA > 0
  SPI2_DmaTransfer(pData,Size,SPI_RECEIVE);
#else
Int32U InCount = Size;
  while (InCount--)
  {
    SPI_SendData(SPI2, 0xFF);
    while(SPI_GetFlagStatus(SPI2, SPI_FLAG_RXNE) == RESET);
    *pData++ = SPI_ReceiveData(SPI2);
  }
#endif // SPI_DMA_ENA > 0
}

/*************************************************************************
 * Function Name: MmcDly_1ms
 * Parameters: Int32U Delay
 * Return: none
 *
 * Description: Delay [msec]
 *
 *************************************************************************/
void MmcDly_1ms (Int32U Delay)
{
  for(;Delay;--Delay)
  {
    Dly100us((void *)10);
  }
}

