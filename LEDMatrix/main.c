/*
 * File:        main.c
 * Author:      Sam Miller
 * Target PIC:  PIC32MX250F128B
 */

#include "config.h"
#include <stdlib.h>
#include <math.h>
#include <xc.h>
#include "gamma.h"

// threading library
#include "pt_cornell_1_2.h"

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

int i, j=1, x, y, hue;
float dy, dx, d;
UINT16 c;
UINT8 sat, val;
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

UINT16 colorHSV(long hue, UINT8 sat, UINT8 val, BOOL gflag) {
  static UINT8  r, g, b, lo;
  static UINT16 s1, v1;

  // Hue
  hue %= 1536;             // -1535 to +1535
  if(hue < 0) hue += 1536; //     0 to +1535
  lo = hue & 255;          // Low byte  = primary/secondary color mix
  switch(hue >> 8) {       // High byte = sextant of colorwheel
    case 0 : r = 255     ; g =  lo     ; b =   0     ; break; // R to Y
    case 1 : r = 255 - lo; g = 255     ; b =   0     ; break; // Y to G
    case 2 : r =   0     ; g = 255     ; b =  lo     ; break; // G to C
    case 3 : r =   0     ; g = 255 - lo; b = 255     ; break; // C to B
    case 4 : r =  lo     ; g =   0     ; b = 255     ; break; // B to M
    default: r = 255     ; g =   0     ; b = 255 - lo; break; // M to R
  }

  // Saturation: add 1 so range is 1 to 256, allowig a quick shift operation
  // on the result rather than a costly divide, while the type upgrade to int
  // avoids repeated type conversions in both directions.
  s1 = sat + 1;
  r  = 255 - (((255 - r) * s1) >> 8);
  g  = 255 - (((255 - g) * s1) >> 8);
  b  = 255 - (((255 - b) * s1) >> 8);

  // Value (brightness) & 16-bit color reduction: similar to above, add 1
  // to allow shifts, and upgrade to int makes other conversions implicit.
  v1 = val + 1;
  if(gflag) { // Gamma-corrected color?
    r = gamma[(r * v1) >> 8]; // Gamma correction table maps
    g = gamma[(g * v1) >> 8]; // 8-bit input to 4-bit output
    b = gamma[(b * v1) >> 8];
  } else { // linear (uncorrected) color
    r = (r * v1) >> 12; // 4-bit results
    g = (g * v1) >> 12;
    b = (b * v1) >> 12;
  }
  return (r << 12) | ((r & 0x8) << 8) | // 4/4/4 -> 5/6/5
         (g <<  7) | ((g & 0xC) << 3) |
         (b <<  1) | ( b        >> 3);
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
    
    for(y=0; y < WIDTH; y++) {
        dy = 15.5 - (float)y;
        for(x=0; x < HEIGHT; x++) {
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
                c = colorHSV(hue, sat, val, 1);
            } else {
                c = 0;
            }
            drawPixel(x, y, c);
        }
    }
    
        // === setup system wide interrupts  ========
    INTEnableSystemMultiVectoredInt();
    
    // ================= Setup input capture ==============================

    

    // round-robin scheduler for threads
    while (1) {
        _nop();
    }
} // main

// === end  ======================================================

