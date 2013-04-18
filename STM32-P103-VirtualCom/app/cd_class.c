/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2006
 *
 *    File name   : cd_class.c
 *    Description : Communication device class module
 *
 *    History :
 *    1. Date        : June 28, 2006
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 15135 $
**************************************************************************/
#define CD_CLASS_GLOBAL
#include "cd_class.h"

#pragma data_alignment=4
static CDC_LineCoding_t CDC_LineCoding;

#if CDC_DEVICE_SUPPORT_LINE_CODING > 0
static volatile Int32U LineCodingDelta;
#endif // CDC_DEVICE_SUPPORT_LINE_CODING > 0

#if CDC_DEVICE_SUPPORT_LINE_STATE > 0
#pragma data_alignment=4
static CDC_LineState_t  CDC_LineState;

static volatile Int32U SerialStateDelta;

static volatile Int32U LineStateDelta;
#endif // CDC_DEVICE_SUPPORT_LINE_STATE > 0

#if CDC_DEVICE_SUPPORT_BREAK > 0
Boolean Break;
#endif // CDC_DEVICE_SUPPORT_BREAK > 0
#pragma data_alignment=4
static UsbSetupPacket_t CdcReqPacket;

static volatile Boolean CDC_Configure;
static Int32U RxSize, RxSizeHold, TxSize;
static volatile Boolean TxDone, RxDone;

#pragma data_alignment=4
static Int8U RxBuffer[CommOutEpMaxSize];

/*************************************************************************
 * Function Name: UsbCdcInit
 * Parameters: none
 *
 * Return: none
 *
 * Description: USB communication device class init
 *
 *************************************************************************/
void UsbCdcInit (void)
{
  // Init CD Class variables
  CDC_Configure               = FALSE;

#if CDC_DEVICE_SUPPORT_BREAK > 0
  Break                       = FALSE;
#endif // CDC_DEVICE_SUPPORT_BREAK > 0

  CDC_LineCoding.dwDTERate    = CDC_DATA_RATE;
  CDC_LineCoding.bDataBits    = CDC_DATA_BITS;
  CDC_LineCoding.bParityType  = CDC_PARITY;
  CDC_LineCoding.bCharFormat  = CDC_STOP_BITS;

#if CDC_DEVICE_SUPPORT_LINE_CODING > 0
  // Update the line coding
  LineCodingDelta             = TRUE;
#endif // CDC_DEVICE_SUPPORT_LINE_CODING > 0

#if CDC_DEVICE_SUPPORT_LINE_STATE > 0
  CDC_LineState.DTR_State     = CDC_LINE_DTR;
  CDC_LineState.RTS_State     = CDC_LINE_RTS;

  // Update the line state
  LineStateDelta              = TRUE;
  SerialStateDelta            = FALSE;
#endif // CDC_DEVICE_SUPPORT_LINE_STATE > 0

  UsbCdcConfigure(NULL);
  UsbCoreInit();
}

/*************************************************************************
 * Function Name: UsbCdcConfigure
 * Parameters:  pUsbDevCtrl_t pDev
 *
 * Return: none
 *
 * Description: USB communication device class configure
 *
 *************************************************************************/
void UsbCdcConfigure (pUsbDevCtrl_t pDev)
{
  if(pDev != NULL && pDev->Configuration)
  {
  #if CDC_DEVICE_SUPPORT_BREAK > 0
    Break = FALSE;
  #endif // CDC_DEVICE_SUPPORT_BREAK > 0
    CDC_Configure = TRUE;
    TxDone = RxDone = FALSE;
    RxSizeHold = RxSize = 0;
    USB_IO_Data(CommOutEp,RxBuffer,sizeof(RxBuffer),(void*)UsbCdcReadHandler);
  }
  else
  {
    TxDone = RxDone = TRUE;
    RxSizeHold = RxSize = TxSize = 0;
    CDC_Configure = FALSE;
  }
}

/*************************************************************************
 * Function Name: IsUsbCdcConfigure
 * Parameters:  none
 *
 * Return: Boolean
 *
 * Description: Return configuration state
 *
 *************************************************************************/
Boolean IsUsbCdcConfigure (void)
{
  return(CDC_Configure);
}

/*************************************************************************
 * Function Name: UsbCdcRequest
 * Parameters:  pUsbSetupPacket_t pSetup
 *
 * Return: UsbCommStatus_t
 *
 * Description: The class requests processing
 *
 *************************************************************************/
UsbCommStatus_t UsbCdcRequest (pUsbSetupPacket_t pSetup)
{
  CdcReqPacket = *pSetup;
  // Validate Request
  if (CdcReqPacket.mRequestType.Recipient == UsbRecipientInterface)
  {
    switch (CdcReqPacket.bRequest)
    {
    case SET_LINE_CODING:
      if ((CdcReqPacket.wValue.Word == 0) &&
          (CdcReqPacket.wIndex.Word == 0))
      {
        USB_IO_Data(CTRL_ENP_OUT,
                   (pInt8U)&CDC_LineCoding,
                    USB_T9_Size(sizeof(CDC_LineCoding_t),CdcReqPacket.wLength.Word),
                   (void *)UsbCdcData);
        return(UsbPass);
      }
      break;
    case GET_LINE_CODING:
      if ((CdcReqPacket.wValue.Word == 0) &&
          (CdcReqPacket.wIndex.Word == 0))
      {
        USB_IO_Data(CTRL_ENP_IN,
                   (pInt8U)&CDC_LineCoding,
                    USB_T9_Size(sizeof(CDC_LineCoding_t),CdcReqPacket.wLength.Word),
                   (void *)USB_StatusHandler);
        return(UsbPass);
      }
      break;
#if CDC_DEVICE_SUPPORT_LINE_STATE > 0
    case SET_CONTROL_LINE_STATE:
      if ((CdcReqPacket.wLength.Word == 0) &&
          (CdcReqPacket.wIndex.Word == 0))
      {
        CDC_LineState.DTR_State = ((CdcReqPacket.wValue.Lo & 1) != 0);
        CDC_LineState.RTS_State = ((CdcReqPacket.wValue.Lo & 2) != 0);
        // Send AKN
        USB_StatusHandler(CTRL_ENP_IN);
        LineStateDelta = TRUE;
        return(UsbPass);
      }
      break;
#endif // CDC_DEVICE_SUPPORT_LINE_STATE > 0
#if CDC_DEVICE_SUPPORT_BREAK > 0
    case SEND_BREAK:
      Break = TRUE;
#if CDC_DEVICE_SUPPORT_LINE_STATE > 0
      LineStateDelta = TRUE;
      // Send AKN
      USB_StatusHandler(CTRL_ENP_IN);
#endif // CDC_DEVICE_SUPPORT_LINE_STATE > 0
      return(UsbPass);
#endif // CDC_DEVICE_SUPPORT_BREAK > 0
    }
  }
  return(UsbFault);
}

/*************************************************************************
 * Function Name: UsbCdcData
 * Parameters:  USB_Endpoint_t EP
 *
 * Return: none
 *
 * Description: USB Communication Device Class Data receive
 *
 *************************************************************************/
void UsbCdcData (USB_Endpoint_t EP)
{
  if ((CdcReqPacket.bRequest == SET_LINE_CODING) &&
      (CdcReqPacket.wLength.Word != 0))
  {
#if CDC_DEVICE_SUPPORT_LINE_CODING > 0
    LineCodingDelta = TRUE;
#endif // CDC_DEVICE_SUPPORT_LINE_CODING > 0
    USB_StatusHandler(CTRL_ENP_IN);
  }
  USB_T9_ERROR_REQUEST();
}

/*************************************************************************
 * Function Name: UsbCdcReadHandler
 * Parameters: USB_Endpoint_t EP
 *
 * Return: none
 *
 * Description: Called when receive buffer is filled or error appear
 *
 *************************************************************************/
static
void UsbCdcReadHandler (USB_Endpoint_t EP)
{
  assert(EP == CommOutEp);
  RxDone = TRUE;
  RxSize = EpCnfg[EP].Size;
}

/*************************************************************************
 * Function Name: UsbCdcWriteHandler
 * Parameters: USB_Endpoint_t EP
 *
 * Return: none
 *
 * Description: Called when transmitting buffer is empty or error appear
 *
 *************************************************************************/
static
void UsbCdcWriteHandler (USB_Endpoint_t EP)
{
  assert(EP == CommInEp);
  TxDone = TRUE;
  TxSize = EpCnfg[EP].Size;
}

/*************************************************************************
 * Function Name: UsbCdcRead
 * Parameters: pInt8U pBuffer, Int32U Size
 *
 * Return: Int32U
 *
 * Description: USB Communication Device Class data read. Return number
 * of received the bytes.
 *
 *************************************************************************/
Int32U UsbCdcRead (pInt8U pBuffer, Int32U Size)
{
  if (!CDC_Configure)
  {
    // if device isn't configured
    return(0);
  }

  if (Size)
  {
    if (RxDone)
    {
      Size = MIN(Size,RxSize-RxSizeHold);
      memcpy(pBuffer,RxBuffer+RxSizeHold,Size);
      RxSizeHold += Size;
      if (RxSizeHold >= RxSize)
      {
        ENTR_CRT_SECTION();
        RxDone = FALSE;
        RxSizeHold = 0;
        USB_IO_Data(CommOutEp,RxBuffer,sizeof(RxBuffer),(void*)UsbCdcReadHandler);
        EXT_CRT_SECTION();
      }
    }
    else
    {
      Size = 0;
    }
  }
  return(Size);
}

/*************************************************************************
 * Function Name: UsbCdcWrite
 * Parameters:  pInt8U pBuffer, Int32U Size
 *
 * Return: CdcStatus_t
 *
 * Description: USB Communication Device Class data send.
 *
 *************************************************************************/
Boolean UsbCdcWrite (pInt8U pBuffer, Int32U Size)
{
  if (!CDC_Configure)
  {
    // if device isn't configured
    return(0);
  }

  if(Size)
  {
    while(1)
    {
      ENTR_CRT_SECTION();
      TxDone = FALSE;
      if (USB_IO_Data(CommInEp,pBuffer,Size,(void*)UsbCdcWriteHandler) != NOT_READY)
      {
        EXT_CRT_SECTION();
        break;
      }
      EXT_CRT_SECTION();
    }

    while(!TxDone);
    Size = TxSize;
  }
  return(Size);
}

#if CDC_DEVICE_SUPPORT_LINE_STATE > 0
/*************************************************************************
 * Function Name: UsbCdcIsNewLineStateSettings
 * Parameters:  none
 *
 * Return: Boolean
 *
 * Description: Is there a new modem settings received?
 * RTS and DTR signals
 *
 *************************************************************************/
Boolean UsbCdcIsNewLineStateSettings(void)
{
Boolean StateHold;
  StateHold = (AtomicExchange(FALSE,(pInt32U)&LineStateDelta) != 0);
  return(StateHold);
}

/*************************************************************************
 * Function Name: UsbCdcGetLineStateSettings
 * Parameters:  none
 *
 * Return: CDC_LineState_t
 *
 * Description: Return the Line Signals states structure
 * RTS and DTR signals
 *
 *************************************************************************/
CDC_LineState_t UsbCdcGetLineStateSettings(void)
{
  return(CDC_LineState);
}

/*************************************************************************
 * Function Name: UsbCdcReportHandler
 * Parameters: USB_Endpoint_t EP
 *
 * Return: none
 *
 * Description: Called when Line status report was send
 *
 *************************************************************************/
static
void UsbCdcReportHandler (USB_Endpoint_t EP)
{
  assert(EP == ReportEp);
  SerialStateDelta = FALSE;
}

/*************************************************************************
 * Function Name: UsbCdcReportSerialCommState
 * Parameters: SerialState_t SerialState
 *
 * Return: none
 *
 * Description: Report the current state of serial communication channel
 * Overrun Error,  Parity Error, Framing Error, Ring Signal, Break,
 * Tx Carrier, Rx Carrier
 *
 *************************************************************************/
void UsbCdcReportSerialCommState(SerialState_t SerialState)
{
SerialStatePacket_t SerialStatePacket;
  if(CDC_Configure == FALSE)
  {
    return;
  }

  SerialStatePacket.UsbSetupPacket.mRequestType.mRequestTypeData = 0xA1;
  SerialStatePacket.UsbSetupPacket.bRequest = SERIAL_STATE;
  SerialStatePacket.UsbSetupPacket.wValue.Word = 0;
  SerialStatePacket.UsbSetupPacket.wIndex.Word = CDC_DATA_INTERFACE_IND;
  SerialStatePacket.UsbSetupPacket.wLength.Word = sizeof(SerialState_t);
  SerialStatePacket.SerialState = SerialState;

  // Disable interrupt and save current state of the interrupt flags
  while(1)
  {
    ENTR_CRT_SECTION();
    SerialStateDelta = TRUE;
    if (USB_IO_Data( ReportEp,
                    (pInt8U)&SerialStatePacket,
                     sizeof(SerialState_t),
                    (void*)UsbCdcReportHandler) != NOT_READY)
    {
      EXT_CRT_SECTION();
      break;
    }
    EXT_CRT_SECTION();
  }

  while(SerialStateDelta);
}

#endif // CDC_DEVICE_SUPPORT_LINE_STATE > 0

#if CDC_DEVICE_SUPPORT_BREAK > 0
/*************************************************************************
 * Function Name: UsbCdcGetBreakState
 * Parameters:  none
 *
 * Return: Boolean
 *
 * Description: Return Break event state
 *
 *************************************************************************/
Boolean UsbCdcGetBreakState(void)
{
Boolean BreakHold;
  BreakHold = Break;
  Break = 0;
  return(BreakHold != 0);
}
#endif // CDC_DEVICE_SUPPORT_BREAK > 0

#if CDC_DEVICE_SUPPORT_LINE_CODING > 0
/*************************************************************************
 * Function Name: UsbCdcIsNewLineCtrlSettings
 * Parameters:  none
 *
 * Return: Boolean
 *
 * Description: Is there a new line settings received?
 * Baud rate, Data bits, Stop bits and Parity
 *
 *************************************************************************/
Boolean UsbCdcIsNewLineCodingSettings(void)
{
Boolean StateHold;
  StateHold = (AtomicExchange(FALSE,(pInt32U)&LineCodingDelta) != 0);
  return(StateHold);
}

/*************************************************************************
 * Function Name: UsbCdcGetLineCodingSettings
 * Parameters:  none
 *
 * Return: CDC_LineCoding_t
 *
 * Description: Return the Line Coding structure
 * Baud rate, Data bits, Stop bits and Parity
 *
 *************************************************************************/
CDC_LineCoding_t UsbCdcGetLineCodingSettings(void)
{
  return(CDC_LineCoding);
}
#endif // CDC_DEVICE_SUPPORT_LINE_CODING > 0
