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

// threading library
#include "pt_cornell_1_2.h"

int i, j=1, x, y, hue;
float dy, dx, d;
UINT16 c;
UINT8 sat, val;
INT16 r,g,b;

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
    
    for(y=0; y < MATRIX_WIDTH; y++) {
        dy = 15.5 - (float)y;
        for(x=0; x < MATRIX_HEIGHT; x++) {
            dx = 15.5 - (float)x;
            d  = dx * dx + dy * dy;
            if (d <= (16.5 * 16.5)) { // Inside the circle(ish)?
                hue = (int)((atan2(-dy, dx) + M_PI) * 1536.0 / (M_PI * 2.0));
                d = sqrt(d);
                if(d > 15.5) {
                    // Do a little pseudo anti-aliasing along perimeter
                    sat = 255;
                    val = (int)((1.0 - (d - 15.5)) * 255.0 + 0.5);
                }
                else {
                    // White at center
                    sat = (int)(d / 15.5 * 255.0 + 0.5);
                    val = 255;
                }
                c = matrix_colorHSV(hue, sat, val, 1);
            } else {
                c = 0;
            }
            matrix_drawPixel(x, y, c);
        }
    }    
        // === setup system wide interrupts  ========
    INTEnableSystemMultiVectoredInt();
    
    //matrix_drawChar(0,0,'G',COLOR565_RED,COLOR565_BLACK, 1);
    matrix_setCursor(0, 0);
    matrix_write('G');
    
    // ================= Setup input capture ==============================
    matrix_swapBuffers(1);

    

    // round-robin scheduler for threads
    while (1) {
        _nop();
    }
} // main

// === end  ======================================================

