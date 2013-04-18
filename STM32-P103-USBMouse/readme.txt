########################################################################
#
#                           USBMouse.eww
#
# $Revision: 19278 $
#
########################################################################

DESCRIPTION
===========
   This example project shows how to use the IAR Embedded Workbench
  for ARM to develop code for the IAR STM32-SK board. It implements USB
  HID mouse. When host install needed driver a mouse's cursor begin moved
  in rectangle shape. The WAKE-UP button is used for USB resume when device
  is suspended.
 
   Controls:
    WAKE-UP - USB resume, when device is suspended

COMPATIBILITY
=============
   The USBMouse example project is compatible with, on the IAR-STM32-SK
  evaluation board. By default, the project is configured to use the 
  J-Link JTAG/SWD interface.

CONFIGURATION
=============
   Make sure that the following jumpers are correctly configured on the
  IAR STM32-SK evaluation board:

   Jumpers:
  PWR_SEL - depending of power source

        
GETTING STARTED
===============

  1) Start the IAR Embedded Workbench for ARM.

  2) Select File->Open->Workspace...
     Open the following workspace:

     <installation-root>\arm\examples\ST\
     STM32F10x\IAR-STM32-SK\USBMouse\USBMouse.eww

  3) Run the program.

     The USBMouse application is downloaded to the Embedded RAM or 
     Embedded Flash memory depending of configurations on the evaluation 
     board and executed.
