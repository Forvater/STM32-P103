/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2006
 *
 *    File name   : cd_class.h
 *    Description : Communication device class module header
 *
 *    History :
 *    1. Date        : June 28, 2006
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 15135 $
**************************************************************************/
#include <includes.h>

#ifndef __CD_CLASS_H
#define __CD_CLASS_H

#ifdef CD_CLASS_GLOBAL
#define CD_CLASS_EXTERN
#else
#define CD_CLASS_EXTERN  extern
#endif

#define CommInterfaceClassCode    2
#define DataInterfaceClassCodes   0xA

// CDC Interfaces definitions
#define CDC_CTRL_INTERFACE_IND        0
#define CDC_DATA_INTERFACE_IND        1

// Parity
#define NOPARITY                0
#define ODDPARITY               1
#define EVENPARITY              2
#define MARKPARITY              3
#define SPACEPARITY             4

// StopBits
#define ONESTOPBIT              0
#define ONE5STOPBITS            1
#define TWOSTOPBITS             2

// Baud rates
#define CBR_110                 110
#define CBR_300                 300
#define CBR_600                 600
#define CBR_1200                1200
#define CBR_2400                2400
#define CBR_4800                4800
#define CBR_9600                9600
#define CBR_14400               14400
#define CBR_19200               19200
#define CBR_38400               38400
#define CBR_56000               56000
#define CBR_57600               57600
#define CBR_115200              115200
#define CBR_128000              128000
#define CBR_256000              256000

#pragma pack(1)
typedef union _CDC_LineState_t
{
  Int8U Data;
  struct
  {
    Int8U  DTR_State  : 1;
    Int8U  RTS_State  : 1;
  };
} CDC_LineState_t, *pCDC_LineState_t;

typedef union _CDC_LineCoding_t
{
  Int8U Data[7];
  struct
  {
    Int32U dwDTERate;
    Int8U  bCharFormat;
    Int8U  bParityType;
    Int8U  bDataBits;
  };
} CDC_LineCoding_t, * pCDC_LineCoding_t;

typedef union _SerialState_t
{
  Int16U Data;
  struct
  {
    Int16U  bRxCarrier  : 1;
    Int16U  bTxCarrier  : 1;
    Int16U  bBreak      : 1;
    Int16U  bRingSignal : 1;
    Int16U  bFraming    : 1;
    Int16U  bParity     : 1;
    Int16U  bOverRun    : 1;
    Int16U              : 9;
  };
} SerialState_t, *pSerialState_t;

typedef struct _SerialStatePacket_t
{
  UsbSetupPacket_t UsbSetupPacket;
  SerialState_t    SerialState;
} SerialStatePacket_t, *pSerialStatePacket_t;

#pragma pack()

/*************************************************************************
 * Function Name: UsbCdcInit
 * Parameters: none
 *
 * Return: none
 *
 * Description: USB communication device class init
 *
 *************************************************************************/
void UsbCdcInit (void);

/*************************************************************************
 * Function Name: UsbCdcConfigure
 * Parameters:  pUsbDevCtrl_t pDev
 *
 * Return: none
 *
 * Description: USB communication device class configure
 *
 *************************************************************************/
void UsbCdcConfigure (pUsbDevCtrl_t pDev);

/*************************************************************************
 * Function Name: IsUsbCdcConfigure
 * Parameters:  none
 *
 * Return: Boolean
 *
 * Description: Return configuration state
 *
 *************************************************************************/
Boolean IsUsbCdcConfigure (void);

/*************************************************************************
 * Function Name: UsbCdcRequest
 * Parameters:  pUsbSetupPacket_t pSetup
 *
 * Return: UsbCommStatus_t
 *
 * Description: The class requests processing
 *
 *************************************************************************/
UsbCommStatus_t UsbCdcRequest (pUsbSetupPacket_t pSetup);

/*************************************************************************
 * Function Name: UsbCdcData
 * Parameters:  USB_Endpoint_t EP
 *
 * Return: none
 *
 * Description: USB Communication Device Class Data receive
 *
 *************************************************************************/
void UsbCdcData (USB_Endpoint_t EP);

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
void UsbCdcReadHandler (USB_Endpoint_t EP);

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
void UsbCdcWriteHandler (USB_Endpoint_t EP);

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
Int32U UsbCdcRead (pInt8U pBuffer, Int32U Size);

/*************************************************************************
 * Function Name: UsbCdcWrite
 * Parameters:  pInt8U pBuffer, Int32U Size
 *
 * Return: CdcStatus_t
 *
 * Description: USB Communication Device Class data send.
 *
 *************************************************************************/
Boolean UsbCdcWrite (pInt8U pBuffer, Int32U Size);

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
Boolean UsbCdcIsNewLineStateSettings(void);

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
CDC_LineState_t UsbCdcGetLineStateSettings(void);

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
void UsbCdcReportHandler (USB_Endpoint_t EP);

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
void UsbCdcReportSerialCommState(SerialState_t SerialState);
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
Boolean UsbCdcGetBreakState(void);
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
Boolean UsbCdcIsNewLineCodingSettings(void);

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
CDC_LineCoding_t UsbCdcGetLineCodingSettings(void);
#endif // CDC_DEVICE_SUPPORT_LINE_CODING > 0

#endif // __CD_CLASS_H
