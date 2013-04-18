########################################################################
#
#                           VirtualCom.eww
#
# $Revision: 1.0 $
#
########################################################################

DESCRIPTION
===========
   This example project shows how to use the IAR Embedded Workbench
  for ARM to develop code for the IAR STM32-SK board.
   It implements USB CDC (Communication Device Class) device and install
  it like a Virtual COM port. UART3 is used for physical implementation
  of the RS232 port.

COMPATIBILITY
=============
   The USB CDC example project is compatible with, on the IAR-STM32-SK
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
     STM32F10x\IAR-STM32-SK\VirtualCom\VirtualCom.eww

  3) Run the program.

     The USB CDC application is downloaded to the Embedded Flash memory
     on the evaluation board and executed.
