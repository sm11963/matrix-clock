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
#include "blocking_delay.h"
#include "datetime.h"
#include "analog_clock.h"
#include "pt_cornell_1_2.h"

// Setup the protothread structures that are used for the put and get threads.
// These should not be used for any other thread besides spawning
// PT_GetSerialBuffer and PutSerialBuffer
static struct pt pt_get, pt_put, pt_serial;

// Printf using PT PutSerialBuffer
#define pt_printf(fmt, ...) \
    do { snprintf(PT_send_buffer, max_chars, fmt, __VA_ARGS__);\
         PT_SPAWN( pt, &pt_put, PutSerialBuffer(&pt_put) );\
       } while(0)

// Printf with terminating newline using PT PutSerialBuffer
#define pt_printfl(fmt, ...) \
    do { snprintf(PT_send_buffer, max_chars-2, fmt, __VA_ARGS__); \
         strcat(PT_send_buffer, "\n\r"); \
         PT_SPAWN( pt, &pt_put, PutSerialBuffer(&pt_put) ); \
       } while(0)

// Print simple string using PT PutSerialBuffer
#define pt_print(str) \
    do { strncpy(PT_send_buffer, (str), max_chars-1); \
         PT_send_buffer[max_chars-1] = '\0'; \
         PT_SPAWN( pt, &pt_put, PutSerialBuffer(&pt_put) ); \
       } while(0)

// Print simple string with terminating newline using PT PutSerialBuffer
#define pt_printl(str) \
    do { strncpy(PT_send_buffer, (str), max_chars-3); \
         PT_send_buffer[max_chars-3] = '\0'; \
         strcat(PT_send_buffer, "\n\r"); \
         PT_SPAWN( pt, &pt_put, PutSerialBuffer(&pt_put) ); \
       } while(0)

void rtcc_init() {
    rtccTime tm;
    rtccDate dt;
    
    tm.l=0x00;
    tm.sec=0x50;                                                                 
    tm.min=0x59;
    tm.hour=0x23;

    dt.wday=0x6;
    dt.mday=0x28;
    dt.mon=0x11;
    dt.year=0x15;
    
    RtccOpen(tm.l, dt.l, 0);
}

void draw_dtime(rtccTime dec_tm, rtccDate dec_dt) {
    char time_buf[10];
    
    const char* month_str = months_long[dec_dt.mon - 1];
    
    matrix_fillScreen(COLOR565_BLACK);
    matrix_setCursor(((8-strlen(month_str))<<1),2);
    matrix_setTextColor(matrix_color444(5,5,5));
    matrix_write3x5String(months_long[dec_dt.mon - 1]);

    matrix_setTextColor(0xffff);
    sprintf(time_buf, "%d:%02d", twentyFour2TwelveHour(dec_tm.hour), dec_tm.min);
    matrix_setCursor(((5-strlen(time_buf))*3)+1,12);
    matrix_writeString(time_buf);

    if (dec_tm.sec < 30) {
        matrix_drawLine(1,20,dec_tm.sec+1,20,matrix_color444(1,0,0));
    }
    else {
        matrix_drawLine(1,20,30,20,matrix_color444(1,0,0));
        matrix_drawLine(1,21,dec_tm.sec-29,21, matrix_color444(1,0,0));
    }

    if (++dec_tm.sec > 59) {
        dec_tm.sec = 0;
    }

    matrix_setCursor(((dec_dt.mday > 9) ? 4:6),25);
    matrix_setTextColor(matrix_color444(5,5,5));
    sprintf(time_buf, "%s %d", days_short[dec_dt.wday], dec_dt.mday);
    matrix_write3x5String(time_buf);

    matrix_swapBuffers(FALSE);
}

void draw_atime(rtccTime dec_tm, rtccDate dec_dt) {
    matrix_fillScreen(COLOR565_BLACK);
    
    matrix_drawCircle(16,16,16,matrix_color444(0,0,5));
    matrix_drawCircle(16,16,15,matrix_color444(0,0,5));
    matrix_drawPixel(16,1,COLOR565_CYAN);
    matrix_drawPixel(24,3,COLOR565_CYAN);
    matrix_drawPixel(29,8,COLOR565_CYAN);
    matrix_drawPixel(31,16,COLOR565_CYAN);
    matrix_drawPixel(29,24,COLOR565_CYAN);
    matrix_drawPixel(24,29,COLOR565_CYAN);
    matrix_drawPixel(16,31,COLOR565_CYAN);
    matrix_drawPixel(8,29,COLOR565_CYAN);
    matrix_drawPixel(3,24,COLOR565_CYAN);
    matrix_drawPixel(1,16,COLOR565_CYAN);
    matrix_drawPixel(3,8,COLOR565_CYAN);
    matrix_drawPixel(8,3,COLOR565_CYAN);
                    
    char point_sec[2];
    char point_min[2];
    char point_hr[2];
    
    get_end_pnt60(dec_tm.sec, point_sec);
    get_end_pnt60(dec_tm.min, point_min);
    get_end_pnt12((dec_tm.hour % 12), point_hr);

    matrix_drawLine(16,16,16+point_sec[0],16+point_sec[1],matrix_color444(0,2,1));
    matrix_drawLine(16,16,16+point_min[0],16+point_min[1],COLOR565_MAGENTA);
    matrix_drawLine(16,16,16+point_hr[0],16+point_hr[1],COLOR565_YELLOW);

    matrix_swapBuffers(FALSE);  
}

struct pt pt_update_matrix;

// The original format BCD codified date/time and the decimal versions
rtccTime bcd_tm, dec_tm;
rtccDate bcd_dt, dec_dt;

static PT_THREAD(protothread_update_matrix(struct pt *pt)) {
    PT_BEGIN(pt);
    
    while(TRUE) {    
        //RtccGetTimeDate(&bcd_tm, &bcd_dt);
        // Convert BCD codified date/time to decimal
        //dec_tm = bcdTime2DecTime(bcd_tm);
        //dec_dt = bcdDate2DecDate(bcd_dt);
        
        //draw_dtime(dec_tm, dec_dt);
        draw_atime(dec_tm, dec_dt);
        PT_YIELD(pt);
    }
    
    PT_END(pt);
}

// === Update Serial Thread ==================================================
static PT_THREAD(protothread_serial(struct pt *pt)) {
    PT_BEGIN(pt);
    
    static int in_hr, in_min, in_sec;
    static int in_mday, in_wday, in_mon, in_yr;
    
    while (1) {
        PT_SPAWN( pt, &pt_get, PT_GetSerialBuffer(&pt_get) );
        if (sscanf(PT_term_buffer, "t %d:%d:%d", &in_hr, &in_min, &in_sec) == 3) {
            dec_tm.hour = in_hr;
            dec_tm.min = in_min;
            dec_tm.sec = in_sec;
        }
        else if (sscanf(PT_term_buffer, "d %d/%d/%d-%d", &in_mon, &in_mday, &in_yr, &in_wday) == 4) {
            dec_dt.mday = in_mday;
            dec_dt.wday = in_wday;
            dec_dt.mon = in_mon;
            dec_dt.year = in_yr;
        }
        else {
            pt_printl("Invalid");
        }
    } // END WHILE(1)
    PT_END(pt);
} // Update Serial thread

// === Main  ======================================================
void main(void) {
    // Configure the device for maximum performance but do not change the PBDIV
	// Given the options, this function will change the flash wait states, RAM
	// wait state and enable prefetch cache but will not change the PBDIV.
	// The PBDIV value is already set via the pragma FPBDIV option above..
	SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
   
    INTDisableInterrupts();
    
    //rtcc_init();
    matrix_init(true);
    
    INTEnableSystemMultiVectoredInt();
    
    PT_setup();
    
    PT_INIT(&pt_update_matrix);
    PT_INIT(&pt_serial);
    
    while (TRUE) {
        PT_SCHEDULE(protothread_update_matrix(&pt_update_matrix));
        PT_SCHEDULE(protothread_serial(&pt_serial));
    }
}
// === end  ======================================================
