/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : usb_cnfg.h
 *    Description : USB config file
 *
 *    History :
 *    1. Date        : June 16, 2007
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 19125 $
 **************************************************************************/

#include <includes.h>

#ifndef __USB_CNFG_H
#define __USB_CNFG_H

// Enable/Disable trace info

/* USB interrupt priority */
#define USB_INTR_HIGH_PRIORITY    1
#define USB_INTR_LOW_PRIORITY     2

/* USB Events */
#define USB_SOF_EVENT             1
#define USB_ERROR_EVENT           0   // for debug
#define USB_HIGH_PRIORITY_EVENT   0   // ISO and Double buffered bulk
#define USB_PMAOVR_EVENT          0   // for speed up retransfer

/* USB Clock settings */
#define USB_DIVIDER               RCC_USBCLKSource_PLLCLK_1Div5 // when PLL clk 72MHz

/* Max Interfaces number*/
#define USB_MAX_INTERFACE         1

/* Endpoint definitions */
#define MaxIndOfRealizeEp         1
#define Ep0MaxSize                64
#define ReportEpHid               ENP1_IN
#define ReportEpMaxSize           8
#define ReportEpPollingPeriod     2   // resolution 1ms
#define BUTTONS_PER               15  // buttons samples period 15 ms
                                      // at less two sample must be equal before button state change
#define XYZ_PER                   4   // 4 ms

/* Device power atrb  */
#define USB_SELF_POWERED          0
#define USB_REMOTE_WAKEUP         1

/* Class defenitions*/
#define HID_INTERFACE_0           0
#define HID_BOOT_DEVICE           1
#define HID_IDLE_SUPP             1
#define HID_ID_NUMB               0

#define HID_MOUSE_ID              0
/* Other defenitions */

#endif //__USB_CNFG_H
