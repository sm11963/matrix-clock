/*
 * File:        main.c
 * Author:      Sam Miller
 * Target PIC:  PIC32MX250F128B
 */
#define ENABLE_MATRIX_PLASMA 1


#include "config.h"
#include <stdlib.h>
#include <math.h>
#include <xc.h>
#include "rgb_matrix.h"
#include "matrix_gfx.h"

#include "matrix_tests.h"

// threading library
#include "pt_cornell_1_2.h"

// === Main  ======================================================

void main(void) {
    // Configure the device for maximum performance but do not change the PBDIV
	// Given the options, this function will change the flash wait states, RAM
	// wait state and enable prefetch cache but will not change the PBDIV.
	// The PBDIV value is already set via the pragma FPBDIV option above..
	SYSTEMConfig(SYS_FREQ, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    
    // === Configure threads ==========
    PT_setup();

    matrix_init(1);
    
    draw_colorwheel();
        // === setup system wide interrupts  ========
    INTEnableSystemMultiVectoredInt();
    
    //matrix_drawChar(0,0,'G',COLOR565_RED,COLOR565_BLACK, 1);
    matrix_setCursor(0, 0);
    matrix_write('G');
    
    // ================= Setup input capture ==============================
    matrix_swapBuffers(1);

    draw_levels();
    plasma_loop(0);

    // round-robin scheduler for threads
    while (1) {
        _nop();
    }
} // main

// === end  ======================================================

