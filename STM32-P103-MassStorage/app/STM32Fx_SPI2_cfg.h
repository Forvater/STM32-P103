/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : STM32Fx_SPI2_cfg.h
 *    Description : SPI 2 configuration file
 *
 *    History :
 *    1. Date        : October 27, 2007
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 1.1.2.1 $
 **************************************************************************/

#include "includes.h"

#ifndef __STM32FX_SPI0_CFG_H
#define __STM32FX_SPI0_CFG_H

// enable block DMA transfer
#define SPI_DMA_ENA       1
// DMA handler interrupt priority level
#define SPI_DMA_INTR_PRIO 3
//  DMA transfer corruption errata
//  Description
//  Data corruption occurs when the CPU executes a write access to the APB bus while the
//  DMA is also performing a write access on the same APB bus.
//  However, the DMA transfer is not corrupted when the CPU performs a read access to the
//  APB bus. Additionally, no data corruption occurs during multi DMA transfers on the same
//  APB bus without CPU write access on this APB bus.
//  Workaround
//  1. Ensure that your application does not execute a write access from the CPU to the APB
//  bus when you have already programmed a DMA write on this APB.
//  2. Use semaphore implementation to avoid this behavior.
#define DMA_ERRATA


#endif //__STM32FX_SPI0_CFG_H
