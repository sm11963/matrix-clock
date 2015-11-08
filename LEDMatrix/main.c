/*
 * File:        main.c
 * Author:      Sam Miller
 * Target PIC:  PIC32MX250F128B
 */

#include "config.h"
#include <stdlib.h>
#include <math.h>
#include <xc.h>

// threading library
#include "pt_cornell_1_2.h"

int i, j=1;
#define NPLANES 4
#define NROWS 16
#define HEIGHT 32
#define WIDTH 32
#define A_PORTB_BIT BIT_3
#define B_PORTB_BIT BIT_5
#define C_PORTB_BIT BIT_10
#define D_PORTB_BIT BIT_13
#define CLK_PORTA_BIT BIT_2
#define LAT_PORTA_BIT BIT_1
#define OE_PORTA_BIT BIT_0


int buffsize, allocsize, rotation;
INT16 *matrixbuff[2];
BOOL dualbuffers;
INT16 *ptr;
volatile INT16 *buffptr;
volatile int row, plane;
INT16 r,g,b;
INT8 curr_bit, limit, backindex;

void setup_matrix() {
    buffsize = WIDTH * NROWS * 3 * 2;
    allocsize = (dualbuffers) ? buffsize * 2 : buffsize;
    if (NULL == (matrixbuff[0] = (INT16 *)malloc(allocsize))) return;
    memset(matrixbuff[0], 0, allocsize);
    matrixbuff[1] = (dualbuffers) ? &matrixbuff[0][buffsize] : matrixbuff[0];
    
    plane = NPLANES - 1;
    row = NROWS - 1;
    
    mPORTASetPinsDigitalOut(0b111);
    
    mPORTBClearBits(A_PORTB_BIT|B_PORTB_BIT|C_PORTB_BIT|D_PORTB_BIT);
    mPORTBSetPinsDigitalOut(A_PORTB_BIT|B_PORTB_BIT|C_PORTB_BIT|D_PORTB_BIT);
    
    // 0,1,2,3 & 5 & 8,9 & 13 14 15 (1110|0011|0010|1111)
    mPORTBClearBits(0x387);
    mPORTBSetPinsDigitalOut(0x387);
    
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
        *ptr &= ~0x70;           // Plane 0 R,G mask out in one op
        if(r & 1) *ptr |=  0x10; // Plane 0 R: 64 bytes ahead, bit 0
        if(g & 1) *ptr |=  0x20; // Plane 0 G: 64 bytes ahead, bit 1
        if(b & 1) *ptr |=  0x40; // Plane 0 B: 32 bytes ahead, bit 0
        // The remaining three image planes are more normal-ish.
        // Data is stored in the high 6 bits so it can be quickly
        // copied to the DATAPORT register w/6 output lines.
        for(; curr_bit < limit; curr_bit <<= 1) {
            *ptr &= ~0x7;            // Mask out R,G,B in one op
            if(r & curr_bit) *ptr |= 0x1; // Plane N R: bit 2
            if(g & curr_bit) *ptr |= 0x2; // Plane N G: bit 3
            if(b & curr_bit) *ptr |= 0x4; // Plane N B: bit 4
            ptr  += WIDTH;                 // Advance to next bit plane
        }
    } else {
        // Data for the lower half of the display is stored in the upper
        // bits, except for the plane 0 stuff, using 2 least bits.
        ptr = &matrixbuff[backindex][(y - NROWS) * WIDTH * (NPLANES - 1) + x];
        *ptr &= ~0xe000;           // Plane 0 R,G,B mask out in one op
        if(r & 1) *ptr |=  0x2000; // Plane 0 R: 
        if(g & 1) *ptr |=  0x4000; // Plane 0 G:
        if(b & 1) *ptr |=  0x8000; // Plane 0 B:
        for(; curr_bit < limit; curr_bit <<= 1) {
            *ptr &= ~0x380;            // Mask out R,G,B in one op
            if(r & curr_bit) *ptr |= 0x080; // Plane N R: bit 5
            if(g & curr_bit) *ptr |= 0x100; // Plane N G: bit 6
            if(b & curr_bit) *ptr |= 0x200; // Plane N B: bit 7
            ptr  += WIDTH;                 // Advance to next bit plane
        }
    }  
}

/************************* Variable declarations ******************************/

void __ISR(_TIMER_2_VECTOR, IPL5AUTO) LEDMatrixUpdate(void) {
    INT16 * ptr, * end_ptr;
    INT16 i, duration;
    mPORTASetBits(0b11);
    
    duration = 320 << plane;
    
    if (++plane >= NPLANES) {
        plane = 0;
        
        if (++row >= NROWS) {
            row = 0;
            buffptr = matrixbuff[1-backindex];
        }
    }
    else if (plane == 1) {
        mPORTBClearBits(A_PORTB_BIT|B_PORTB_BIT|C_PORTB_BIT|D_PORTB_BIT);
        if (row & BIT_0) mPORTBSetBits(A_PORTB_BIT);
        if (row & BIT_1) mPORTBSetBits(B_PORTB_BIT);
        if (row & BIT_2) mPORTBSetBits(C_PORTB_BIT);
        if (row & BIT_3) mPORTBSetBits(D_PORTB_BIT);
    }
    
    ptr = (INT16 *)buffptr;
    end_ptr = ptr + 32;
    
    WritePeriod2(duration);
    WriteTimer2(0);
    mPORTAClearBits(0b11);
    
    if (plane > 0) {
        for (; ptr< end_ptr; ptr++) {
            mPORTBClearBits(0x387);
            mPORTBSetBits( ptr[i] & 0x387 );
            _nop();
            mPORTASetBits(BIT_2);
            mPORTAClearBits(BIT_2);
        }
        
        buffptr = ptr;
    }
    else {
        for (i=0; i < WIDTH; i++) {
            mPORTBClearBits(0x387);
            mPORTBSetBits( ((ptr[i] >> 6) & 0x380) | ((ptr[i] >> 4) & 0x7) );
            _nop();
            mPORTASetBits(BIT_2);
            mPORTAClearBits(BIT_2);
        }
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
    PT_setup();

    setup_matrix();

    // === setup system wide interrupts  ========
    INTEnableSystemMultiVectoredInt();
    
    for (i=0; i<WIDTH; i++) {
        for (j=0; j<HEIGHT; j++) {
            r = (i % 16);
            if (j%4 == 0) {
                b = r << 12; // Red
            } else if (j%4 == 1) {
                b = r << 7; // Green
            } else if (j%4 == 2) {
                b = r << 1; // Blue
            } else {
                //b = (r << 12) | (r << 7) | (r << 1); // White
                b = (r << 12) | (r << 7) | (r << 1); // White
            }
            drawPixel(i,j,b);
        }
    }
    
    // ================= Setup input capture ==============================

    

    // round-robin scheduler for threads
    while (1) {
        _nop();
    }
} // main

// === end  ======================================================

