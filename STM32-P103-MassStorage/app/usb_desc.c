/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : usb_desc.c
 *    Description : usb decriptors module
 *
 *    History :
 *    1. Date        : 22, July 2007
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: $
 **************************************************************************/
#include "usb_desc.h"

const UsbStandardDeviceDescriptorStr_t UsbStandardDeviceDescriptorStr =
{
  sizeof(UsbStandardDeviceDescriptorStr_t),   // bLength
  UsbDescriptorDevice,                        // bDescriptorType
  0x200,                                      // bcdUSB
  0,                                          // bDeviceClass
  0,                                          // bDeviceSubClass
  0,                                          // bDeviceProtocol
  Ep0MaxSize,                                 // bMaxPacketSize0
  0xFFFF,                                     // idVendor
  0x1017,                                     // idProduct
  0x0000,                                     // bcdDevice
  iManufacturerStr,                           // iManufacturer
  iProductStr,                                // iProduct
  iSerialNumberStr,                           // iSerialNumber
  1,                                          // bNumConfigurations
};

#pragma data_alignment=4
static const Int8U UsbFS_Cfg0[] =
{
// Configuration Descriptor
  sizeof(UsbStandardConfigurationDescriptor_t),
  UsbDescriptorConfiguration,
  (1*sizeof(UsbStandardConfigurationDescriptor_t)+
   1*sizeof(UsbStandardInterfaceDescriptor_t)+
   2*sizeof(UsbStandardEpDescriptor_t)),
  (1*sizeof(UsbStandardConfigurationDescriptor_t)+
   1*sizeof(UsbStandardInterfaceDescriptor_t)+
   2*sizeof(UsbStandardEpDescriptor_t)) >> 8,
  1,
  1,
  0,
  UsbConfigurationCommmonAttr,
  UsbConfigPower_mA(100),
  sizeof(UsbStandardInterfaceDescriptor_t),
  UsbDescriptorInterface,
  0,
  0,
  2,
  UsbDeviceClassStorage,
  MscSubClassScsi,
  MscProtocolBulkOnly,
  0,
  sizeof(UsbStandardEpDescriptor_t),
  UsbDescriptorEp,
  UsbEpIn(BulkInEp>>1),
  UsbEpTransferBulk,
  BulkInEpMaxSize,
  BulkInEpMaxSize>>8,
  0,
  sizeof(UsbStandardEpDescriptor_t),
  UsbDescriptorEp,
  UsbEpOut(BulkOutEp>>1),
  UsbEpTransferBulk,
  BulkOutEpMaxSize,
  BulkOutEpMaxSize>>8,
  0,
  0,
};

static const Int8U * const UsbFS_CfgArray[] =
{
  UsbFS_Cfg0,
  NULL,
};

static const UsbEP_ExtDesc_t UsbEPExt_Cfg0EP1 =
{
  1,                     // Configuration
  UsbEpIn(BulkInEp>>1),  // EP address
  {
    EP_SLOT1, FALSE
  }
};

static const UsbEP_ExtDesc_t UsbEPExt_Cfg0EP2 =
{
  1,                     // Configuration
  UsbEpOut(BulkOutEp>>1),  // EP address
  {
    EP_SLOT2, FALSE
  }
};

static const UsbEP_ExtDesc_t * const UsbFS_EPExtArray[] =
{
  &UsbEPExt_Cfg0EP1,
  &UsbEPExt_Cfg0EP2,
  NULL,
};

#pragma data_alignment=4
const Int8U UsbLanguagesStr [] =
{
  // Length of LanguagesStr + 2
  4,
  // Desciptor Type
  UsbDescriptorString,
  // Languages ID
  // Languages1 ID English
  0x09,0x04,
};

#pragma data_alignment=4
const Int8U ManufacturerStrLan1 [] =
{
  24, // length
  UsbDescriptorString,  // Descriptor
  'I',0,'A',0,'R',0,' ',0,'S',0,'y',0,'s',0,'t',0,'e',0,'m',0,'s',0
};

#pragma data_alignment=4
const Int8U ProductStrLan1 [] =
{
  64, //length
  UsbDescriptorString, // Descriptor
  'I',0,'A',0,'R',0,' ',0,'S',0,'T',0,'M',0,'3',0,'2',0,'F',0,' ',0,'M',0,'a',0,'s',0,'s',0,' ',0,'s',0,'t',0,'o',0,'r',0,'a',0,'g',0,'e',0,' ',0,'e',0,'x',0,'a',0,'m',0,'p',0,'l',0,'e',0 // Str
};


#pragma data_alignment=4
const Int8U SerialNumberStrLan1 [] =
{
  26, //length
  UsbDescriptorString, // Descriptor
  '0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'1',0, // Str
};

const Int8U * const UsbLanguages1Strings[] =
{
  // iManufacturerStr
  ManufacturerStrLan1,
  // iProductStr
  ProductStrLan1,
  // iSerialNumberStr
  SerialNumberStrLan1,
  // Terminator
  NULL
};

static const Int8U * const * const UsbFS_StringArray[] =
{
  UsbLanguages1Strings,
  NULL,
};

const void * const UsbDescArray[] =
{
  UsbFS_CfgArray,
  UsbFS_EPExtArray,
  UsbLanguagesStr,
  UsbFS_StringArray,
  NULL,
};
