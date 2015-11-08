/*
 * File:        main.c
 * Author:      Sam Miller
 * Target PIC:  PIC32MX250F128B
 */

#include "config.h"
#include <stdlib.h>
#include <math.h>
#include <xc.h>
#include "sgm82_helpers.h"

// threading library
#include "pt_cornell_1_2.h"

#define PWM_RATE 1000 // hz
#define PWM_PERIOD_TICKS 40000 // PB_CLOCK / PWM_RATE


int i, j=1;
/************************* Variable declarations ******************************/

void __ISR(_TIMER_2_VECTOR, ipl2) Timer2Interrupt(void) {
    mPORTASetBits(0b11);
    
    for (i=0; i<10; i++) {
        _nop();
    }
    
    j <<= 1;
    if (j == BIT_5) {
        j = BIT_0;
    }
    
    WriteTimer2(0);
    mPORTAClearBits(0b11);
    
 //   mPORTBSetBits(BIT_0);
  //  mPORTBSetBits(BIT_3);
    
    for (i=0; i<32; i++) {
        mPORTBClearBits(0b01111);
        mPORTBSetBits(j);
        mPORTASetBits(BIT_2);
        mPORTAClearBits(BIT_2);
    }
               
    mT2ClearIntFlag();
}

// === Main  ======================================================

void main(void) {
    // Configure the device for maximum performance but do not change the PBDIV
	// Given the options, this function will change the flash wait states, RAM
	// wait state and enable prefetch cache but will not change the PBDIV.
	// The PBDIV value is already set via the pragma FPBDIV option above..
	SYSTEMConfig(SYS_FREQ, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    // === Configure threads ==========
    // turns OFF UART support and debugger pin
    PT_setup();

    // === setup system wide interrupts  ========
    INTEnableSystemMultiVectoredInt();
    
    mPORTASetPinsDigitalOut(0b111);
    // 0,1,2,3 & 5 & 8,9 & 13 14 15 (1110|0011|0010|1111)
    mPORTBSetPinsDigitalOut(0xe32f);
    mPORTBClearBits(0xe32f);
    
    // ================= Setup input capture ==============================
    // Open timer 2 with prescalar 32 and max value 2^16
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_1, 320);
    // Setup interrupts to handle overflow
    ConfigIntTimer2( T2_INT_ON | T2_INT_PRIOR_2 );
    mT2ClearIntFlag();
    

    // round-robin scheduler for threads
    while (1) {
        _nop();
    }
} // main

// === end  ======================================================

