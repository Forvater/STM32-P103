/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : main.c
 *    Description : Define main module
 *
 *    History :
 *    1. Date        : 19, July 2006
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *  This example project shows how to use the IAR Embedded Workbench
 * for ARM to develop code for the IAR STM32-SK board. It implements USB
 * HID mouse. When host install needed driver a mouse's cursor begin moved
 * in rectangle shape. The WAKE-UP button is used for USB resume when device
 * is suspended.
 *
 *  Controls:
 *   WAKE-UP - USB resume, when device is suspended
 *
 *  Jumpers:
 *   PWR_SEL - depending of power source
 *
 *    $Revision: 19278 $
 **************************************************************************/
#include "includes.h"

#define LOOP_DLY_100US  450

volatile Boolean Update = TRUE;
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
void main(void)
{
NVIC_InitTypeDef NVIC_InitStructure;
TIM1_TimeBaseInitTypeDef TIM1_TimeBaseInitStruct;
GPIO_InitTypeDef GPIO_InitStructure;

Int32U State, Count;
#ifdef DEBUG
 debug();
#endif

  ENTR_CRT_SECTION();

  // Init clock system
  Clk_Init();

  // NVIC init
#ifndef  EMB_FLASH
  /* Set the Vector Table base location at 0x20000000 */
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
  /* Set the Vector Table base location at 0x08000000 */
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

  // Init USB wakeup button (WAKE_UP (PA0))
  // Enable GPIO clock and release reset
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,
                         ENABLE);
  RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA,
                         DISABLE);

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOA,&GPIO_InitStructure);

  // Timer1 Init
  // Enable Timer1 clock and release reset
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
  RCC_APB2PeriphResetCmd(RCC_APB2Periph_TIM1,DISABLE);

  // Set timer period 0.01 sec
  TIM1_TimeBaseInitStruct.TIM1_Prescaler = 720;  // 10us resolution
  TIM1_TimeBaseInitStruct.TIM1_CounterMode = TIM1_CounterMode_Up;
  TIM1_TimeBaseInitStruct.TIM1_Period = 1000;  // 10 ms
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

  // HID USB
  HidInit();
  // Soft connection enable
  USB_ConnectRes(TRUE);

  EXT_CRT_SECTION();

  // LCD Init
/*  HD44780_PowerUpInit();

  // Show message on the LCD
  HD44780_StrShow(1, 1,  "  IAR Systems   ");
  HD44780_StrShow(1, 2,  "   HID Class    ");
  LCD_LIGHT_ON();*/
  while(1)
  {
    if(Update)
    {
      Update = FALSE;
      if(UsbCoreReq(UsbCoreReqDevState) == UsbDevStatusConfigured &&
         !UsbCoreReq(UsbCoreReqDevSusState))
      {
        // Mouse pointer
        switch(State)
        {
        case 0:
          HidMouseSendReport(-2,0,0);
          if(Count-- == 0)
          {
            State++;
            Count = 100;
          }
          break;
        case 1:
          HidMouseSendReport(0,2,0);
          if(Count-- == 0)
          {
            State++;
            Count = 100;
          }
          break;
        case 2:
          HidMouseSendReport(2,0,0);
          if(Count-- == 0)
          {
            State++;
            Count = 100;
          }
          break;
        case 3:
          HidMouseSendReport(0,-2,0);
          if(Count-- == 0)
          {
            State++;
            Count = 100;
          }
          break;
        default:
          State = 0;
          Count = 100;
        }
      }
      else
      {
        // Reset state
        State = 0;
        Count = 100;
      }
      // USB Wakeup
      if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == Bit_SET)
      {
        UsbWakeUp();
      }
    }
  /*  USB_ConnectRes(TRUE);
    for(int i=0;i<50000;i++);
    USB_ConnectRes(TRUE);
    for(int i=0;i<50000;i++);*/
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
