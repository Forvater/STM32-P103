/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : main.c
 *    Description : Main module
 *
 *    History :
 *    1. Date        : September 18, 2007
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *   This example project shows how to use the IAR Embedded Workbench for ARM
 *  to develop code for the IAR STM32-SK board.
 *   It implements a MMC/SD card drive. The first free drive letters will be
 *  used. For example, if your PC configuration includes two hard disk partitions
 *  (in C:\ and D:\) and a CD-ROM drive (in E:\), the memory card drive will
 *  appear as F:\.
 *  The LCD backlight will indicate drive activity.
 *
 *  Jumpers:
 *   PWR_SEL - depending of power source
 *
 *    $Revision: 1.0 $
 **************************************************************************/
#include "includes.h"

#define TIMER0_TICK_PER_SEC   2
#define UPDATE_SHOW_DLY       ((Int32U)(0.5 * TIMER0_TICK_PER_SEC))

#define LOOP_DLY_100US        450

volatile Boolean Update = FALSE;

#pragma data_alignment=4
__no_init Int8U Lun0Buffer[2048];

const Int8U HexToCharStr [] = "0123456789ABCDEF";
Int32U CriticalSecCntr;

/*************************************************************************
 * Function Name: Timer1IntrHandler
 * Parameters: none
 *
 * Return: none
 *
 * Description: Timer 1 interrupt handler
 *
 *************************************************************************/
void Timer1IntrHandler (void)
{
  // Clear update interrupt bit
  TIM1_ClearITPendingBit(TIM1_FLAG_Update);
  Update = TRUE;
}

/*************************************************************************
 * Function Name: Clk_Init
 * Parameters: Int32U Frequency
 * Return: Int32U
 *
 * Description: Init clock system
 *
 *************************************************************************/
void Clk_Init (void)
{
  // 1. Clocking the controller from internal HSI RC (8 MHz)
  RCC_HSICmd(ENABLE);
  // wait until the HSI is ready
  while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
  RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
  // 2. Enable ext. high frequency OSC
  RCC_HSEConfig(RCC_HSE_ON);
  // wait until the HSE is ready
  while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);
  // 3. Init PLL
  RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_9); // 72MHz
  RCC_PLLCmd(ENABLE);
  // wait until the PLL is ready
  while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
  // 4. Set system clock dividers
  RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
  RCC_ADCCLKConfig(RCC_PCLK2_Div8);
  RCC_PCLK2Config(RCC_HCLK_Div1);
  RCC_PCLK1Config(RCC_HCLK_Div2);
  RCC_HCLKConfig(RCC_SYSCLK_Div1);
#ifdef EMB_FLASH
  // 5. Init Embedded Flash
  // Zero wait state, if 0 < HCLK 24 MHz
  // One wait state, if 24 MHz < HCLK 56 MHz
  // Two wait states, if 56 MHz < HCLK 72 MHz
  // Flash wait state
  FLASH_SetLatency(FLASH_Latency_2);
  // Half cycle access
  FLASH_HalfCycleAccessCmd(FLASH_HalfCycleAccess_Disable);
  // Prefetch buffer
  FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
#endif // EMB_FLASH
  // 5. Clock system from PLL
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
}

/*************************************************************************
 * Function Name: Dly100us
 * Parameters: Int32U Dly
 *
 * Return: none
 *
 * Description: Delay Dly * 100us
 *
 *************************************************************************/
void Dly100us(void *arg)
{
Int32U Dly = (Int32U)arg;
  while(Dly--)
  {
    for(volatile int i = LOOP_DLY_100US; i; i--);
  }
}

/*************************************************************************
 * Function Name: main
 * Parameters: none
 *
 * Return: none
 *
 * Description: main
 *
 *************************************************************************/
int main(void)
{
Int32U Dly = UPDATE_SHOW_DLY;
DiskStatusCode_t StatusHold = (DiskStatusCode_t) -1;
Int8U Message[17];
Int32U Tmp, Tmp1;
Boolean nZerro;

NVIC_InitTypeDef NVIC_InitStructure;
TIM1_TimeBaseInitTypeDef TIM1_TimeBaseInitStruct;

extern LunFpnt_t LunFun [SCSI_LUN_NUMB];

#ifdef DEBUG
 debug();
#endif

  ENTR_CRT_SECTION();

  // Init clock system
  Clk_Init();

  // NVIC init
#ifndef  EMB_FLASH
  // Set the Vector Table base location at 0x20000000
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  // VECT_TAB_FLASH
  // Set the Vector Table base location at 0x08000000
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

  // Timer1 Init
  // Enable Timer1 clock and release reset
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
  RCC_APB2PeriphResetCmd(RCC_APB2Periph_TIM1,DISABLE);

  // Set timer period 0.5 sec
  TIM1_TimeBaseInitStruct.TIM1_Prescaler = 720;  // 10us resolution
  TIM1_TimeBaseInitStruct.TIM1_CounterMode = TIM1_CounterMode_Up;
  TIM1_TimeBaseInitStruct.TIM1_Period = 100000/TIMER0_TICK_PER_SEC;
  TIM1_TimeBaseInitStruct.TIM1_ClockDivision = TIM1_CKD_DIV1;
  TIM1_TimeBaseInitStruct.TIM1_RepetitionCounter = 0;
  TIM1_TimeBaseInit(&TIM1_TimeBaseInitStruct);

  // Clear update interrupt bit
  TIM1_ClearITPendingBit(TIM1_FLAG_Update);
  // Enable update interrupt
  TIM1_ITConfig(TIM1_FLAG_Update,ENABLE);

  NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  // Enable timer counting
  TIM1_Cmd(ENABLE);

  // Init USB Mass storage class
  ScsiInit();

  // LUNs Init
  LunInit(MMC_DISK_LUN,MmcDiskInit,MmcGetDiskCtrlBkl,MmcDiskIO);

  EXT_CRT_SECTION();

  // Soft connection enable
  USB_ConnectRes(TRUE);
/*
  // LCD Powerup init
  HD44780_PowerUpInit();
  // Show messages on LCD
  HD44780_StrShow(1, 1,  "IAR Systems ARM ");
  HD44780_StrShow(1, 2,  "USB Mass Storage");
*/
  while(1)
  {
    for(Int32U i = 0; i < SCSI_LUN_NUMB; i++)
    {
      // Implement LUNs messages
      if(LunImp(i))
      {
       // LCD_LIGHT_ON();
      }
      else
      {
      //  LCD_LIGHT_OFF();
      }
    }

    if (Update)
    {
      Update = FALSE;
      // Update MMC/SD card status
      MmcStatusUpdate();
      if(Dly-- == 0)
      {
        // LCD show
        Dly = UPDATE_SHOW_DLY;
        // Current state of MMC/SD show
        pDiskCtrlBlk_t pMMCDiskCtrlBlk = MmcGetDiskCtrlBkl();
        if(StatusHold != pMMCDiskCtrlBlk->DiskStatus)
        {

          StatusHold = pMMCDiskCtrlBlk->DiskStatus;
          switch (pMMCDiskCtrlBlk->DiskStatus)
          {
          case DiskCommandPass:
            switch(pMMCDiskCtrlBlk->DiskType)
            {
            case DiskMMC:
              strcpy((char*)Message,"MMC Card - ");
              break;
            case DiskSD:
              strcpy((char*)Message,"SD Card - ");
              break;
            default:
              strcpy((char*)Message,"Card - ");
            }
            // Calculate MMC/SD size [MB]
            Tmp  = pMMCDiskCtrlBlk->BlockNumb * pMMCDiskCtrlBlk->BlockSize;
            Tmp  = Tmp/1000000;
            Tmp1 = Tmp/1000;
            nZerro = FALSE;
            if(Tmp1)
            {
              Message[strlen((char*)Message)+1] = 0;
              Message[strlen((char*)Message)]   = HexToCharStr[Tmp1];
              Tmp %= 1000;
              nZerro = TRUE;
            }
            Tmp1 = Tmp/100;
            if(Tmp1 || nZerro)
            {
              Message[strlen((char*)Message)+1] = 0;
              Message[strlen((char*)Message)]   = HexToCharStr[Tmp1];
              Tmp %= 100;
              nZerro = TRUE;
            }
            Tmp1 = Tmp/10;
            if(Tmp1 || nZerro)
            {
              Message[strlen((char*)Message)+1] = 0;
              Message[strlen((char*)Message)]   = HexToCharStr[Tmp1];
              Tmp %= 10;
              nZerro = TRUE;
            }
            if(Tmp || nZerro)
            {
              Message[strlen((char*)Message)+1] = 0;
              Message[strlen((char*)Message)]   = HexToCharStr[Tmp];
            }
            strcat((char*)Message,"MB");
            for(Int32U i = strlen((char*)Message); i < 16; ++i)
            {
              Message[i] = ' ';
            }
            Message[strlen((char*)Message)+1] = 0;
            break;
          default:
            strcpy((char*)Message,"Pls, Insert Card");
          }
         // HD44780_StrShow(1, 2, (pInt8S)Message);
        }
      }
    }
  }
}

#ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
