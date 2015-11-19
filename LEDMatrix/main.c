/*
 * File:        main.c
 * Author:      Sam Miller
 * Target PIC:  PIC32MX250F128B
 */

#include "config.h"
#include <stdlib.h>
#include <math.h>
#include <xc.h>
#include "rgb_matrix.h"
#include "matrix_gfx.h"

#define ENABLE_MATRIX_PLASMA 1
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
    
        // === setup system wide interrupts  ========
    INTEnableSystemMultiVectoredInt();
    
    
    // ================= Setup input capture ==============================
    
    
    //draw_colorwheel();
    //draw_levels();

    matrix_swapBuffers(1);

    //plasma_loop(1);
    scroll_test_loop();
    //shapes_test_loop();
    
    // round-robin scheduler for threads
    while (1) {
        _nop();
    }
} // main

// === end  ======================================================

