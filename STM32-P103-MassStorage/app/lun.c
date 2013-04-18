/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2005
 *
 *    File name   : lun.c
 *    Description : USB Mass storage device LUNs
 *
 *    History :
 *    1. Datå        : November 15, 2005
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 1.4 $
 **************************************************************************/
#define LUN_GOBALS
#include "lun.h"

LunFpnt_t LunFun [SCSI_LUN_NUMB];

/*************************************************************************
 * Function Name: LunInit
 * Parameters: Int32U LunInd,
 *             DiskInitFpnt_t DiskInitFpnt, DiskStatusFpnt_t DiskStatusFpnt,
 *             DiskIoFpnt_t, DiskIoFpnt
 *
 * Return: none
 *
 * Description: LUN Init
 *
 *************************************************************************/
void LunInit (Int32U LunInd,
              DiskInitFpnt_t DiskInitFpnt, DiskStatusFpnt_t DiskStatusFpnt,
              DiskIoFpnt_t DiskIoFpnt)
{
  LunFun[LunInd].DiskInitFpnt   = DiskInitFpnt;
  LunFun[LunInd].DiskStatusFpnt = DiskStatusFpnt;
  LunFun[LunInd].DiskIoFpnt     = DiskIoFpnt;
}

/*************************************************************************
 * Function Name: LunImp
 * Parameters:  none
 *
 * Return: Boolean 0 - not activity
 *                 1 - activity
 *
 * Description: LUN commands implementation
 *
 *************************************************************************/
Boolean LunImp (Int32U LunInd)
{
static LunState_t LunState;
static Int32U BlockStart, BlockNum;
pLunFpnt_t Lun = &LunFun[LunInd];
pDiskCtrlBlk_t pDiskCrtl;
pMmc3FormatCapResponse_t pFormatCapacity;
Int32U Temp;

  if(pScsiMessage[LunInd] == NULL)
  {
    return(LunState != LunCommandDecode);
  }
  // Get a message
  Int32U Message   = *pScsiMessage[LunInd];
  pInt32U pMessage = pScsiMessage[LunInd]+1;
  // Clear the message pointer
  pScsiMessage[LunInd] = NULL;
  if (Message == LunInitMsg)
  {
    Lun->DiskInitFpnt();
    LunState = LunCommandDecode;
    return(LunState != LunCommandDecode);
  }
  if (Message == LunResetReqMsg)
  {
    LunState = LunCommandDecode;
    return(LunState != LunCommandDecode);
  }
  switch (LunState)
  {
  case LunCommandDecode:
    switch (Message)
    {
    case LunInquiryReqMsg:
      memcpy(Lun0Buffer,MmcDskInquiry,SizeOfInquiryDescMmcDsk);
      ScsiInquiryData(Lun0Buffer,SizeOfInquiryDescMmcDsk);
      break;
    case LunTestUntilReadyReqMsg:
      pDiskCrtl = Lun->DiskStatusFpnt();
      switch (pDiskCrtl->DiskStatus)
      {
      case DiskCommandPass:
        if (pDiskCrtl->MediaChanged)
        {
          ScsiTestUntilReadyData(ScsiMediaChanged);
          pDiskCrtl->MediaChanged = FALSE;
        }
        else
        {
          ScsiTestUntilReadyData(ScsiCommandNoKey);
        }
        break;
      case DiskNotReady:
        ScsiTestUntilReadyData(ScsiMediamNotReady);
        break;
      case DiskNotPresent:
        ScsiTestUntilReadyData(ScsiMediaNotPresent);
        break;
      case DiskChanged:
        ScsiTestUntilReadyData(ScsiMediaChanged);
        break;
      default:
        ScsiTestUntilReadyData(ScsiFatalError);
        break;
      }
      break;
    case LunModeSense6ReqMsg:
      ScsiModeSenseData(Lun->DiskStatusFpnt()->WriteProtect);
      break;
    case LunReadFormatCapacityReqMsg:
      pFormatCapacity = (pMmc3FormatCapResponse_t)Lun0Buffer;

      if(Lun->DiskStatusFpnt()->DiskStatus == DiskCommandPass)
      {
        pFormatCapacity->MaximumDescriptor.DescriptorType    = FormattedMedia;
        // Windows support only 512 bytes sector
        Temp = Lun->DiskStatusFpnt()->BlockSize;
        pFormatCapacity->MaximumDescriptor.BlockLength[0]    = (Temp >> 16) & 0xFF;
        pFormatCapacity->MaximumDescriptor.BlockLength[1]    = (Temp >>  8) & 0xFF;
        pFormatCapacity->MaximumDescriptor.BlockLength[2]    = (Temp      ) & 0xFF;
        Temp = Lun->DiskStatusFpnt()->BlockNumb;
        pFormatCapacity->MaximumDescriptor.NumberofBlocks[0] = (Temp >> 24) & 0xFF;
        pFormatCapacity->MaximumDescriptor.NumberofBlocks[1] = (Temp >> 16) & 0xFF;
        pFormatCapacity->MaximumDescriptor.NumberofBlocks[2] = (Temp >>  8) & 0xFF;
        pFormatCapacity->MaximumDescriptor.NumberofBlocks[3] = (Temp      ) & 0xFF;
      }
      else
      {
        pFormatCapacity->MaximumDescriptor.DescriptorType    = NoMediaPresent;
        pFormatCapacity->MaximumDescriptor.BlockLength[0]    = (2048       >> 16) & 0xFF;
        pFormatCapacity->MaximumDescriptor.BlockLength[1]    = (2048       >>  8) & 0xFF;
        pFormatCapacity->MaximumDescriptor.BlockLength[2]    = (2048            ) & 0xFF;
        pFormatCapacity->MaximumDescriptor.NumberofBlocks[0] = (0xFFFFFFFF >> 24) & 0xFF;
        pFormatCapacity->MaximumDescriptor.NumberofBlocks[1] = (0xFFFFFFFF >> 16) & 0xFF;
        pFormatCapacity->MaximumDescriptor.NumberofBlocks[2] = (0xFFFFFFFF >>  8) & 0xFF;
        pFormatCapacity->MaximumDescriptor.NumberofBlocks[3] = (0xFFFFFFFF      ) & 0xFF;
      }
      ScsiReadFormatCapcityData(Lun0Buffer,sizeof(Mmc3FormatCapResponse_t));
      break;
    case LunReadCapacity10ReqMsg:
      ScsiReadCapacityData(Lun->DiskStatusFpnt()->BlockNumb-1,
                           Lun->DiskStatusFpnt()->BlockSize);
      break;
    case LunRead10ReqMsg:
      BlockStart = *pMessage;
      BlockNum = *++pMessage;
      if ((BlockStart + BlockNum) > Lun->DiskStatusFpnt()->BlockNumb)
      {
        ScsiCmdError(ScsiInvalidCbd,ScsiStallIn);
        break;
      }
      switch(Lun->DiskIoFpnt(Lun0Buffer,BlockStart++,1,DiskRead))
      {
      case DiskCommandPass:
        ScsiReadData(Lun0Buffer,
                     Lun->DiskStatusFpnt()->BlockSize,
                     (--BlockNum == 0));
        if(BlockNum)
        {
          LunState = LunRead;
        }
        break;
      case DiskNotReady:
        // the Media not ready
        ScsiCmdError(ScsiMediamNotReady,ScsiStallIn);
        break;
      case DiskNotPresent:
        // the Media not present
        ScsiCmdError(ScsiMediaNotPresent,ScsiStallIn);
        break;
      default:
        ScsiCmdError(ScsiFatalError,ScsiStallIn);
        break;
      }
      break;
    case LunWrite10ReqMsg:
      BlockStart = *pMessage;
      BlockNum = *++pMessage;

      if ((BlockStart + BlockNum) > Lun->DiskStatusFpnt()->BlockNumb)
      {
        ScsiCmdError(ScsiInvalidCbd,ScsiStallOut);
        break;
      }
      ScsiWriteData(Lun0Buffer,
                    Lun->DiskStatusFpnt()->BlockSize,
                    FALSE);
      LunState = LunWrite;
      break;
    case LunVerify10BytChkReqMsg:
      BlockStart = *pMessage;
      BlockNum = *++pMessage;

      if ((BlockStart + BlockNum) > Lun->DiskStatusFpnt()->BlockNumb)
      {
        ScsiCmdError(ScsiInvalidCbd,ScsiStallOut);
        break;
      }
      ScsiWriteData(Lun0Buffer,
                    Lun->DiskStatusFpnt()->BlockSize,
                    FALSE);
      LunState = LunVerify;
      break;
    case LunVerify10ReqMsg:
      BlockStart = *pMessage;
      BlockNum = *++pMessage;
      if ((BlockStart + BlockNum) > Lun->DiskStatusFpnt()->BlockNumb)
      {
        ScsiCmdError(ScsiInvalidCbd,ScsiStallOut);
        break;
      }
      // Always pass
      ScsiWriteData(NULL,0,TRUE);
      break;
    default:
      // Unknown command
      ScsiCmdError(ScsiUnknowCommand,ScsiStallBoth);
      break;
    }
    break;
  case LunRead:
    if(Message == LunDataReadyMsg)
    {
      switch(Lun->DiskIoFpnt(Lun0Buffer,BlockStart++,1,DiskRead))
      {
      case DiskCommandPass:
        ScsiReadData(Lun0Buffer,
                     Lun->DiskStatusFpnt()->BlockSize,
                     (--BlockNum == 0));
        if(BlockNum == 0)
        {
          LunState = LunCommandDecode;
        }
        break;
      case DiskNotReady:
        // the Media not ready
        ScsiCmdError(ScsiMediamNotReady,ScsiStallIn);
        LunState = LunCommandDecode;
        break;
      case DiskNotPresent:
        // the Media not present
        ScsiCmdError(ScsiMediaNotPresent,ScsiStallIn);
        LunState = LunCommandDecode;
        break;
      default:
        ScsiCmdError(ScsiFatalError,ScsiStallIn);
        LunState = LunCommandDecode;
        break;
      }
    }
    else
    {
      // synchronization lost
      ScsiCmdError(ScsiFatalError,ScsiStallBoth);
      LunState = LunCommandDecode;
    }
    break;
  case LunWrite:
    if(Message == LunDataReadyMsg)
    {
      switch(Lun->DiskIoFpnt(Lun0Buffer,BlockStart++,1,DiskWrite))
      {
      case DiskCommandPass:
        if(--BlockNum == 0)
        {
          ScsiWriteData(NULL,0,TRUE);
          LunState = LunCommandDecode;
        }
        else
        {
        ScsiWriteData(Lun0Buffer,
                      Lun->DiskStatusFpnt()->BlockSize,
                      FALSE);
        }
        break;
      case DiskNotReady:
        // the Media not ready
        ScsiCmdError(ScsiMediamNotReady,ScsiStallOut);
        LunState = LunCommandDecode;
        break;
      case DiskNotPresent:
        // the Media not present
        ScsiCmdError(ScsiMediaNotPresent,ScsiStallOut);
        LunState = LunCommandDecode;
        break;
      default:
        ScsiCmdError(ScsiFatalError,ScsiStallOut);
        LunState = LunCommandDecode;
        break;
      }
    }
    else
    {
      // synchronization lost
      ScsiCmdError(ScsiFatalError,ScsiStallBoth);
      LunState = LunCommandDecode;
    }
    break;
  case LunVerify:
    if(Message == LunDataReadyMsg)
    {
      switch(Lun->DiskIoFpnt(Lun0Buffer,BlockStart++,1,DiskVerify))
      {
      case DiskCommandPass:
        if(--BlockNum == 0)
        {
          ScsiWriteData(NULL,0,TRUE);
          LunState = LunCommandDecode;
        }
        else
        {
        ScsiWriteData(Lun0Buffer,
                      Lun->DiskStatusFpnt()->BlockSize,
                      FALSE);
        }
        break;
      case DiskMiscompareError:
        ScsiCmdError(ScsiMediaNotPresent,ScsiStallOut);
        LunState = LunCommandDecode;
        break;
      case DiskNotReady:
        // the Media not ready
        ScsiCmdError(ScsiMediamNotReady,ScsiStallOut);
        LunState = LunCommandDecode;
        break;
      case DiskNotPresent:
        // the Media not present
        ScsiCmdError(ScsiMediaNotPresent,ScsiStallOut);
        LunState = LunCommandDecode;
        break;
      default:
        ScsiCmdError(ScsiFatalError,ScsiStallOut);
        LunState = LunCommandDecode;
        break;
      }
    }
    else
    {
      // synchronization lost
      ScsiCmdError(ScsiFatalError,ScsiStallBoth);
      LunState = LunCommandDecode;
    }
    break;
  default:
    // something is wrong
    ScsiCmdError(ScsiFatalError,ScsiStallBoth);
    LunState = LunCommandDecode;
    break;
  }
  return(LunState != LunCommandDecode);
}
