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
#include "dragon.h"


#define NUM_STREAMS 200

typedef struct stream {
    INT16 x;          // X position
    INT16 y;          // Y position
    UINT32 hue;
} stream_t;

stream_t streams[NUM_STREAMS];

#define is_stream_valid(streamp) (streams[(streamp)].y != 0x8000)
#define is_stream_invalid(streamp) (streams[(streamp)].y == 0x8000)
#define invalidate_stream(streamp) streams[(streamp)].y = 0x8000

void draw_stream(short x, short y, UINT32 hue) {
    
    if (y < 0 || y - 10 >= _matrix_height) return;
    
    matrix_drawPixel(x, y  , matrix_colorHSV(hue, 255, 255, true));
    matrix_drawPixel(x, y-1, matrix_colorHSV(hue, 240, 80,  true));
    matrix_drawPixel(x, y-2, matrix_colorHSV(hue, 220, 80,  true));
    matrix_drawPixel(x, y-3, matrix_colorHSV(hue, 200, 60,  true));
    matrix_drawPixel(x, y-4, matrix_colorHSV(hue, 180, 50,  true));
    matrix_drawPixel(x, y-5, matrix_colorHSV(hue, 170, 40,  true));
    matrix_drawPixel(x, y-6, matrix_colorHSV(hue, 130, 30,  true));
    matrix_drawPixel(x, y-7, matrix_colorHSV(hue, 115, 20,  true));
    matrix_drawPixel(x, y-8, matrix_colorHSV(hue, 100, 10,   true));
    
    matrix_drawPixel(x, y-9, COLOR565_BLACK);
}

void animation_loop() {
    
    //                     RRRRrGGGGggBBBBb


    
    //matrix_swapBuffers(false);
    
    while (1) {
        int i,j,x;
        unsigned int hue=0;
        
        matrix_fillScreen(COLOR565_BLACK);
        
        matrix_setTextSize(2);

        const char *scroll_text = "Cocktails";
        const int overshoot = -6 * matrix_textsize * strlen(scroll_text);

        for (i = 0; i < 3; i++) {
            for (x = _matrix_width; x > overshoot; x--) {
                hue += 3;
                if (hue >= 1536) hue -= 1536;
                matrix_setTextColor(matrix_colorHSV(hue, 255, 255, true));


                matrix_setCursor(-2, 0);
                matrix_setTextSize(2);
                matrix_writeString("107");

                matrix_fillRect(0, 17, _matrix_width, 16, COLOR565_BLACK);
                matrix_setCursor(x, 17);
                matrix_writeString(scroll_text);
                matrix_swapBuffers(true);
                delay_ms(40);
            }
        }
        
        for (j = 0; j < NUM_STREAMS; j++) {
            invalidate_stream(j);
        }
        
        int active_streams, next;
        active_streams = 0;
        next = 0;
        do {
            short randx = rand() % 32;
            
            if (next < NUM_STREAMS) {
                hue += 30;
                if (hue >= 1536) hue -= 1536;
                
                streams[next++] = (stream_t){ .x = randx, .y = 0, .hue = hue };
                active_streams++;
            }
            
            // Animate streams
            for (j = 0; j < NUM_STREAMS; j++) {
                if (is_stream_valid(j)) {
                
                    streams[j].y++;
                    if (streams[j].y - 10 >= _matrix_height) {
                        invalidate_stream(j);
                        active_streams--;
                    }
                    else {
                        draw_stream(streams[j].x, streams[j].y, streams[j].hue);
                    }
                }
            }
            matrix_swapBuffers(true);
            delay_ms(15);
        } while (active_streams > 0);
                
        matrix_swapBuffers(true);
    }
}

void display_image() {
    char * image_data = header_data;
    unsigned char pixel[3];
    
    int r,c;
    
    matrix_setRotation(1);
    
    while(1) {
        image_data = header_data;
        for (r = 0; r < 32; r++) {
            for (c = 0; c < 32; c++) {
                HEADER_PIXEL(image_data,pixel);
                matrix_drawPixel(r,c,matrix_color888(pixel[0],pixel[1],pixel[2],false));
            }
        }
        matrix_swapBuffers(true);
    }
}

int rtcc_init() {
    rtccTime tm, starttm;
    rtccDate dt;
    
    RtccInit();

    while(RtccGetClkStat() != RTCC_CLK_ON);
    
    tm.l=0x00;
    tm.sec=0x55;                                                                 
    tm.min=0x55;
    tm.hour=0x15;

    dt.wday=0x02;
    dt.mday=0x15;
    dt.mon=0x09;
    dt.year=0x09;
        
    if (RtccOpen(tm.l, dt.l, -500) != RTCC_CLK_ON) {
        // RTCC did not start properly
        return 0;
    }
    
    // get current time
    starttm.l=RtccGetTime();
    
    
    mPORTASetPinsDigitalOut(BIT_0);
    
    while(1) {
        do{
            tm.l=RtccGetTime();
        } while(tm.sec==starttm.sec);
     
        // reset starting time
        starttm.sec = tm.sec;
      
        // toggle leds
        mPORTAToggleBits(BIT_0);
    }
        
    return 0;
}

#define ledon() mPORTASetBits(BIT_0)
#define ledoff() mPORTAClearBits(BIT_0)
#define ledtoggle() mPORTAToggleBits(BIT_0)

volatile BOOL enabled = 1;

void Init_Interrupts()
{
    INTEnableSystemMultiVectoredInt();            //Enable Multi vectored Interrupts
   
} //End Init Interrupts

void test() {
    INTDisableInterrupts();

    rtccTime tm,tm1,starttm; // time structure
    rtccDate dt,dt1; // date structure

    mPORTASetPinsDigitalOut(BIT_0);
    
    RtccInit();

    while(RtccGetClkStat() != RTCC_CLK_ON); //espera que el SOSC este corriendo
    
    tm.l=0x00;
    tm.sec=0x30;                                                                 
    tm.min=0x55;
    tm.hour=0x15;

    dt.wday=0x02;
    dt.mday=0x15;
    dt.mon=0x09;
    dt.year=0x09;

    RtccSetTimeDate(tm.l, dt.l);
    
    tm1.l = RtccGetTime();
    dt1.l = RtccGetDate();
    
    RtccOpen(tm.l, dt.l, 0);  //fijo hora, fecha y calibracion

    // another way to see the RTCC is running: check the SYNC bit
    while(mRtccGetSync());    // wait sync to be low
    while(!mRtccGetSync());    // wait to be high
    while(mRtccGetSync());    // wait sync to be low again

    do
    {
        RtccGetTimeDate(&tm, &dt);// get current time and date
    }while((tm.sec&0xf)>0x7);// don't want to have minute or BCD rollover
        
    Init_Interrupts();
    
    // get current time
    starttm.l=RtccGetTime();
    
//    while(1) {
//        do{
//            tm.l=RtccGetTime();
//        } while(tm.sec==starttm.sec);
//     
//        // reset starting time
//        starttm.sec = tm.sec;
//      
//        // toggle leds
//        if (enabled) ledtoggle();
//    }
}

void draw_time(rtccTime tm) {
    char str[16];
    // Erase block
    matrix_drawRect(0,0,32,16,COLOR565_BLACK);
    
    sprintf(str, "%2x:%02x\n:%02x", tm.hour, tm.min, tm.sec);
    
    matrix_setCursor(0,0);
    matrix_setTextWrap(false);
    matrix_writeString(str);
    matrix_swapBuffers(true);
}

void display_time() {
    static rtccTime curr_tm;
    while (true) {
        curr_tm.l = RtccGetTime();
        
        draw_time(curr_tm);
    }
}

void test_chars() {
    unsigned char start_char = '0';
    int yoff = 0;
    int r,c;
    unsigned char curr_char;

    
    while (TRUE) {
        for (yoff=0; yoff < 6; yoff++) {
            curr_char = start_char;
            for (r=0; r < 6; r++) {
                for (c=0; c < 8; c++) {
                    matrix_draw3x5Char(c*4, r*6-yoff, curr_char++, COLOR565_WHITE, COLOR565_BLACK, 1);
                }
            }
            matrix_swapBuffers(FALSE);
            delay_ms(500);
        }
        start_char += 8;
    }
}

// === Main  ======================================================
void main(void) {
    // Configure the device for maximum performance but do not change the PBDIV
	// Given the options, this function will change the flash wait states, RAM
	// wait state and enable prefetch cache but will not change the PBDIV.
	// The PBDIV value is already set via the pragma FPBDIV option above..
	SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    
    //matrix_init(true);
    
    int status;
    test();
    
    
//    status = rtcc_init();
//    if (status != 0) {
//        // error ocurred enable LED and start blinking
//        mPORTASetPinsDigitalOut(BIT_0);
//        while(1) {
//            mPORTAToggleBits(BIT_0);
//            delay_ms(300);
//        }
//    }
    
    INTDisableInterrupts();
    matrix_init(true);
    INTEnableSystemMultiVectoredInt();
    
//    rtccTime tm,starttm;
//    
//    while(1) {
//        do{
//            tm.l=RtccGetTime();
//        } while(tm.sec==starttm.sec);
//     
//        // reset starting time
//        starttm.sec = tm.sec;
//      
//        // toggle leds
//        if (enabled) ledtoggle();
//    }    
            
    //draw_colorwheel();
    //draw_levels();
    //plasma_loop();
    //scroll_test_loop();
    //shapes_test_loop();
    //animation_loop();
    //display_image();
    //display_time();
    test_chars();
}
// === end  ======================================================
