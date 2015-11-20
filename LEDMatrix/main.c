/*
 * File:        main.c
 * Author:      Sam Miller
 * Target PIC:  PIC32MX250F128B
 */
// Declare this file as main application for the pre-processor
#define MAIN_APPLICATION

#include <stdlib.h>
#include <math.h>
#include <xc.h>

#include "config.h"
#include "rgb_matrix.h"
#include "matrix_gfx.h"
#include "matrix_tests.h"
#include "blocking_delay.h"

#define NUM_STREAMS 200

typedef struct stream {
    short x;          // X position
    short y;          // Y position
} stream_t;

stream_t streams[NUM_STREAMS];

#define is_stream_valid(i) (streams[(i)].x != 0x8000)
#define is_stream_invalid(i) (streams[(i)].x == 0x8000)
#define invalidate_stream(i) streams[(i)].x = 0x8000

void draw_stream(short x, short y, short h, 
                 UINT32 hue, UINT8 sat) {
    unsigned char i;
    short max_y;
    
    max_y = y+h;
    if (max_y < 0 || y > _matrix_height-1) return;
    
    
    for (i=0; i < h; i++) {
        matrix_drawPixel(x, y-i, matrix_colorHSV(hue, sat, 255 - (i*25) , true));
    }
}

void animation_loop() {
    int i, j;
   
    draw_stream(0,30,8,750,255);
    
    draw_stream(3,30,8,250,255);
    
    draw_stream(7,30,8,500,255);
    
    draw_stream(18,30,8,1000,255);
    
     for (i=6; i<_matrix_width-6; i++) {
        for (j=6; j<_matrix_height-6; j++) {
            matrix_drawPixel(i, j, 0b0001000000000000);
        }
     }

    
    matrix_swapBuffers(false);
    
    while (false) {
        matrix_setCursor(-2, 0);
        matrix_setTextSize(2);
        matrix_writeString("107");
        
        matrix_setTextSize(2);

        const char *scroll_text = "Cocktails";
        const int overshoot = -6 * matrix_textsize * strlen(scroll_text);
        int x = _matrix_width;
        for (; x > overshoot; x--) {
            matrix_fillRect(0, 17, _matrix_width, 16, COLOR565_BLACK);
            matrix_setCursor(x, 17);
            matrix_writeString(scroll_text);
            matrix_swapBuffers(true);
            delay_ms(40);
        }
        
        int j;
        int next = 0;
        for (x = NUM_STREAMS; x > 0; x--) {
            short randx = rand() % 32;
            
            streams[next++] = (stream_t){ .x = randx, .y = _matrix_height };
            
            // Animate streams
            for (j = 0; j < NUM_STREAMS; j++) {
                if (is_stream_invalid(j)) break;
                
                
            }
        }
                
        matrix_swapBuffers(true);
    }
}

// === Main  ======================================================
void main(void) {
    // Configure the device for maximum performance but do not change the PBDIV
	// Given the options, this function will change the flash wait states, RAM
	// wait state and enable prefetch cache but will not change the PBDIV.
	// The PBDIV value is already set via the pragma FPBDIV option above..
	SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    

    matrix_init(true);
    
    // === setup system wide interrupts  ========
    INTEnableSystemMultiVectoredInt();
            
    //draw_colorwheel();
    //draw_levels();
    //plasma_loop();
    //scroll_test_loop();
    //shapes_test_loop();
    animation_loop();
}
// === end  ======================================================
