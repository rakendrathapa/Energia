/*
 ************************************************************************
 *	WInterrupts.c
 *
 *	Arduino core files for MSP430
 *		Copyright (c) 2012 Robert Wessels. All right reserved.
 *
 *
 ***********************************************************************
  Derived from:

  WInterrupts.c Part of the Wiring project - http://wiring.uniandes.edu.co

  Copyright (c) 2004-05 Hernando Barragan

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  Modified 24 November 2006 by David A. Mellis
  Modified 1 August 2010 by Mark Sproul
 */

#include <inttypes.h>
#include <stdio.h>
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"
#include "driverlib/gpio.h"
#include "wiring_private.h"
#include "driverlib/rom.h"

uint8_t portBits[7] = {0}; //index 0 = nothing
void (*portFuncs[7])(void) = {0}; //index 0 = nothing

void gpioHandler(void) {
	uint8_t activePort = 0;
	uint32_t portBase = 0;
	if(HWREG(NVIC_ACTIVE0) & 0x4000) activePort = PF;
	for(;!(HWREG(NVIC_ACTIVE0) & (1 << (activePort-1)));activePort++);
    portBase = (uint32_t) portBASERegister(activePort);
    ROM_GPIOPinIntClear(portBase, portBits[activePort]);
    (*portFuncs[activePort])();
}

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode) {
	uint8_t bit = digitalPinToBitMask(interruptNum);
    uint8_t port = digitalPinToPort(interruptNum);
    uint32_t portBase = (uint32_t) portBASERegister(port);
    int lm4fMode;

    switch(mode) {
    case LOW:
        lm4fMode = GPIO_LOW_LEVEL;
        break;
    case CHANGE:
        lm4fMode = GPIO_BOTH_EDGES;
        break;
    case RISING:
        lm4fMode = GPIO_RISING_EDGE;
        break;
    case FALLING:
        lm4fMode = GPIO_FALLING_EDGE;
        break;
    default:
        return;
    }

    ROM_IntMasterDisable();
    ROM_GPIOPinIntClear(portBase, bit);
    ROM_GPIOIntTypeSet(portBase, bit, lm4fMode);
    portBits[port] = bit;
    portFuncs[port] = userFunc;
    GPIOPortIntRegister(portBase, &gpioHandler);
    ROM_GPIOPinIntEnable(portBase, bit);
    ROM_IntMasterEnable();

}

void detachInterrupt(uint8_t interruptNum) {
    uint8_t bit = digitalPinToBitMask(interruptNum);
    uint8_t port = digitalPinToPort(interruptNum);
    uint32_t portBase = (uint32_t) portBASERegister(port);
    if (port == NOT_A_PIN) return;
    GPIOPinIntDisable(portBase, bit);

}
