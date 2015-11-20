/*
 * File:   config.h
 * Author: Syed Tahmid Mahbub
 * Modifed by: Bruce Land 
 * Created on October 10, 2014
 * Mod: 24Sept2015
 */

#ifndef CONFIG_H
#define	CONFIG_H

#define _SUPPRESS_PLIB_WARNING 1
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING 1
#include <plib.h>
// serial stuff
#include <stdio.h>

// Ensure the configuration settings are only included for the main application
#ifdef MAIN_APPLICATION

// Set both the system and PB frequency to 40 MHz
#pragma config FNOSC = FRCPLL, POSCMOD = OFF
#pragma config FPLLIDIV = DIV_2, FPLLMUL = MUL_20
#pragma config FPBDIV = DIV_1, FPLLODIV = DIV_2
#pragma config FWDTEN = OFF,  FSOSCEN = OFF, JTAGEN = OFF

#endif /* MAIN_APPLICATION */

// Define clock speed for use by reset of the applications
#define SYSCLK 40000000
#define PBCLK SYSCLK    // FPBDIV is DIV_1

#endif	/* CONFIG_H */
