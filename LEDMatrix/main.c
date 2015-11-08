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

int i, j=1;
int row, plane;
#define NPLANES 4
#define NROWS 16
#define HEIGHT 32
#define WIDTH 32
#define A_PORTB_BIT BIT_3
#define B_PORTB_BIT BIT_5
#define C_PORTB_BIT BIT_8
#define D_PORTB_BIT BIT_9
#define CLK_PORTA_BIT BIT_2
#define LAT_PORTA_BIT BIT_1
#define OE_PORTA_BIT BIT_0


int buffsize, allocsize, rotation;
INT8 *matrixbuff[2];
BOOL dualbuffers;
INT8 *ptr;
INT8 r,g,b;
INT8 curr_bit, limit, backindex;

void setup_matrix() {
    buffsize = WIDTH * NROWS * 3;
    allocsize = (dualbuffers) ? buffsize * 2 : buffsize;
    if (NULL == (matrixbuff[0] = (INT8 *)malloc(allocsize))) return;
    memset(matrixbuff[0], 0, allocsize);
    matrixbuff[1] = (dualbuffers) ? &matrixbuff[0][buffsize] : matrixbuff[0];
    
    plane = NPLANES - 1;
    row = NROWS - 1;
    
    mPORTASetPinsDigitalOut(0b111);
    // 0,1,2,3 & 5 & 8,9 & 13 14 15 (1110|0011|0010|1111)
    mPORTBSetPinsDigitalOut(0xe32f);
    mPORTBClearBits(0xe32f);
    
    // Open timer 2 with prescalar 1 and max value
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_1, 100);
    // Setup interrupts to handle overflow
    ConfigIntTimer2( T2_INT_ON | T2_INT_PRIOR_5 );
    mT2ClearIntFlag();
 }

void drawPixel(int x, int y, int c) {
    if((x < 0) || (x >= WIDTH) || (y < 0) || (y >= HEIGHT)) return;

    // Adafruit_GFX uses 16-bit color in 5/6/5 format, while matrix needs
    // 4/4/4.  Pluck out relevant bits while separating into R,G,B:
    r =  c >> 12;        // RRRRrggggggbbbbb
    g = (c >>  7) & 0xF; // rrrrrGGGGggbbbbb
    b = (c >>  1) & 0xF; // rrrrrggggggBBBBb

    // Loop counter stuff
    curr_bit   = 2;
    limit = 1 << NPLANES;

    if(y < NROWS) {
        // Data for the upper half of the display is stored in the lower
        // bits of each byte.
        ptr = &matrixbuff[backindex][y * WIDTH * (NPLANES - 1) + x]; // Base addr
        // Plane 0 is a tricky case -- its data is spread about,
        // stored in least two bits not used by the other planes.
        ptr[WIDTH*2] &= ~0b00000011;           // Plane 0 R,G mask out in one op
        if(r & 1) ptr[WIDTH*2] |=  0b00000001; // Plane 0 R: 64 bytes ahead, bit 0
        if(g & 1) ptr[WIDTH*2] |=  0b00000010; // Plane 0 G: 64 bytes ahead, bit 1
        if(b & 1) ptr[WIDTH]   |=  0b00000001; // Plane 0 B: 32 bytes ahead, bit 0
        else      ptr[WIDTH]   &= ~0b00000001; // Plane 0 B unset; mask out
        // The remaining three image planes are more normal-ish.
        // Data is stored in the high 6 bits so it can be quickly
        // copied to the DATAPORT register w/6 output lines.
        for(; curr_bit < limit; curr_bit <<= 1) {
            *ptr &= ~0b00011100;            // Mask out R,G,B in one op
            if(r & curr_bit) *ptr |= 0b00000100; // Plane N R: bit 2
            if(g & curr_bit) *ptr |= 0b00001000; // Plane N G: bit 3
            if(b & curr_bit) *ptr |= 0b00010000; // Plane N B: bit 4
            ptr  += WIDTH;                 // Advance to next bit plane
        }
    } else {
        // Data for the lower half of the display is stored in the upper
        // bits, except for the plane 0 stuff, using 2 least bits.
        ptr = &matrixbuff[backindex][(y - NROWS) * WIDTH * (NPLANES - 1) + x];
        *ptr &= ~0b00000011;                  // Plane 0 G,B mask out in one op
        if(r & 1)  ptr[WIDTH] |=  0b00000010; // Plane 0 R: 32 bytes ahead, bit 1
        else       ptr[WIDTH] &= ~0b00000010; // Plane 0 R unset; mask out
        if(g & 1) *ptr        |=  0b00000001; // Plane 0 G: bit 0
        if(b & 1) *ptr        |=  0b00000010; // Plane 0 B: bit 0
        for(; curr_bit < limit; curr_bit <<= 1) {
            *ptr &= ~0b11100000;            // Mask out R,G,B in one op
            if(r & curr_bit) *ptr |= 0b00100000; // Plane N R: bit 5
            if(g & curr_bit) *ptr |= 0b01000000; // Plane N G: bit 6
            if(b & curr_bit) *ptr |= 0b10000000; // Plane N B: bit 7
            ptr  += WIDTH;                 // Advance to next bit plane
        }
    }  
}

/************************* Variable declarations ******************************/

void __ISR(_TIMER_2_VECTOR, IPL5AUTO) LEDMatrixUpdate(void) {
    mPORTASetBits(0b11);
    
    if (++plane >= NPLANES) {
        plane = 0;
        
        if (++row >= NROWS) {
            row = 0;
        }
    }
    else if (plane == 1) {
        mPORTBClearBits(A_PORTB_BIT|B_PORTB_BIT|C_PORTB_BIT|D_PORTB_BIT);
        if (row & BIT_0) mPORTBSetBits(A_PORTB_BIT);
        if (row & BIT_1) mPORTBSetBits(B_PORTB_BIT);
        if (row & BIT_2) mPORTBSetBits(C_PORTB_BIT);
        if (row & BIT_3) mPORTBSetBits(D_PORTB_BIT);
    }
    
    
    WritePeriod2(640);
    WriteTimer2(0);
    mPORTAClearBits(0b11);
    
    for (i=0; i<32; i++) {
        mPORTBClearBits(0b111);
        mPORTBSetBits(plane);
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

    setup_matrix();

    // === setup system wide interrupts  ========
    INTEnableSystemMultiVectoredInt();
    
    // ================= Setup input capture ==============================

    

    // round-robin scheduler for threads
    while (1) {
        _nop();
    }
} // main

// === end  ======================================================

