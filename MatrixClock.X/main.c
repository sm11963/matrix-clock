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
#include "pt_cornell_1_2.h"

static const char min_end_pnts[] = {
    0, -12, //1
    1, -12, //2
    2, -12, //3
    3, -11, //4
    5, -11, //5
    6, -10, //6
    7, -10, //7
    8, -9,  //8
    9, -8,  //9
    10, -7, //10
    10, -6, //11
    11, -5, //12
    11, -3, //13
    12, -2, //14
    12, -1  //15
};

static const char* days_short[] = {
    "SUN",
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
    "SAT"
};

static const char* days_long[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednsday",
    "Thursday",
    "Friday",
    "Saturday"
};

static const char* months_long[] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "Septmber",
    "October",
    "November",
    "December"
};

static const char* months_short[] = {
    "JAN",
    "FEB",
    "MAR",
    "APR",
    "MAY",
    "JUN",
    "JUL",
    "AUG",
    "SEP",
    "OCT",
    "NOV",
    "DEC"
};

static const char hr_end_pnts[] = {
    0, -6, //1
    3, -5, //2
    5, -3, //3
};

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

unsigned char bcd2char(unsigned char x) {
    return (x & 0xf) + (((x & 0xf0) >> 4) * 10);
}

unsigned long bcdTime2DecTime(rtccTime tm) {
    tm.sec = bcd2char(tm.sec);
    tm.min = bcd2char(tm.min);
    tm.hour = bcd2char(tm.hour);
    
    return tm.l;
}

unsigned char twentyFour2TwelveHour(unsigned char hour) {
    if (hour == 0) {
        return 12;
    }
    else if (hour > 12) {
        return hour - 12;
    }
    else {
        return hour;
    }
}

unsigned long bcdDate2DecDate(rtccDate dt) {
    dt.mday = bcd2char(dt.mday);
    dt.mon = bcd2char(dt.mon);
    dt.year = bcd2char(dt.year);
    
    return dt.l;
}

void draw_dtime(rtccTime dec_tm, rtccDate dec_dt) {
    char time_buf[10];
    
    const char* month_str = months_long[dec_dt.mon - 1];
    
    matrix_fillScreen(COLOR565_BLACK);
    matrix_setCursor((8-strlen(month_str))<<1,2);
    matrix_setTextColor(matrix_color444(5,5,5));
    matrix_write3x5String(months_long[dec_dt.mon - 1]);

    matrix_setTextColor(0xffff);
    sprintf(time_buf, "%d:%02d", twentyFour2TwelveHour(dec_tm.hour), dec_tm.min);
    matrix_setCursor(((5-strlen(time_buf))*3)+1,11);
    matrix_writeString(time_buf);

    if (dec_tm.sec < 30) {
        matrix_drawLine(1,19,dec_tm.sec+1,19,matrix_color444(4,0,0));
    }
    else {
        matrix_drawLine(1,19,30,19,matrix_color444(4,0,0));
        matrix_drawLine(1,20,dec_tm.sec-29,20, matrix_color444(4,0,0));
    }

    if (++dec_tm.sec > 59) {
        dec_tm.sec = 0;
    }

    matrix_setCursor(0,25);
    matrix_setTextColor(matrix_color444(5,5,5));
    sprintf(time_buf, " %s %-2d ", days_short[dec_dt.wday], dec_dt.mday);
    matrix_write3x5String(time_buf);

    matrix_swapBuffers(FALSE);
}

void get_end_point(int idx, const char* end_pnts, unsigned int num_pnts, char* point) {
    int i = idx % num_pnts;
    int x = end_pnts[i*2];
    int y = end_pnts[(i*2)+1];
    
    const int num_pnts_2x = num_pnts << 1;
    const int num_pnts_3x = (num_pnts << 1) + num_pnts;
    
    if (idx >= num_pnts && idx < num_pnts_2x) {
        swap(x,y);
        x = -x;
    }
    else if (idx >= num_pnts_2x && idx < num_pnts_3x) {
        x = -x;
        y = -y;
    }
    else if (idx >= num_pnts_3x) {
        swap(x,y);
        y = -y;
    }
    
    point[0] = x;
    point[1] = y;
}

struct pt pt_update_matrix;

void draw_atime(rtccTime dec_tm, rtccDate dec_dt) {
    matrix_fillScreen(COLOR565_BLACK);
    
    matrix_drawCircle(16,16,16,COLOR565_BLUE);
    matrix_drawCircle(16,16,15,COLOR565_BLUE);
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

    get_end_point(dec_tm.sec, min_end_pnts, 15, point_sec);
    get_end_point(dec_tm.min, min_end_pnts, 15, point_min);
    get_end_point((dec_tm.hour % 12),  hr_end_pnts,   3, point_hr);

    matrix_drawLine(16,16,16+point_sec[0],16+point_sec[1],COLOR565_CYAN);
    matrix_drawLine(16,16,16+point_min[0],16+point_min[1],COLOR565_MAGENTA);
    matrix_drawLine(16,16,16+point_hr[0],16+point_hr[1],COLOR565_YELLOW);

    matrix_swapBuffers(FALSE);  
}

// The original format BCD codified date/time and the decimal versions
rtccTime bcd_tm, dec_tm;
rtccDate bcd_dt, dec_dt;

static PT_THREAD(protothread_update_matrix(struct pt *pt)) {
    PT_BEGIN(pt);
    
    while(TRUE) {
        //update_analog_display();
        //PT_YIELD_TIME_msec(900);
        //display_time();
        
        RtccGetTimeDate(&bcd_tm, &bcd_dt);
        // Convert BCD codified date/time to decimal
        dec_tm.l = bcdTime2DecTime(bcd_tm);
        dec_dt.l = bcdDate2DecDate(bcd_dt);
        
        draw_dtime(dec_tm, dec_dt);
        PT_YIELD(pt);
        //draw_atime(dec_tm, dec_dt);
    }
    
    PT_END(pt);
}

// === Main  ======================================================
void main(void) {
    // Configure the device for maximum performance but do not change the PBDIV
	// Given the options, this function will change the flash wait states, RAM
	// wait state and enable prefetch cache but will not change the PBDIV.
	// The PBDIV value is already set via the pragma FPBDIV option above..
	SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
   
    INTDisableInterrupts();
    
    rtcc_init();
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
            
//    display_analog();
//    display_date();
//    display_time();
    
    PT_setup();
    
    PT_INIT(&pt_update_matrix);
    
    while (TRUE) {
        PT_SCHEDULE(protothread_update_matrix(&pt_update_matrix));
    }
}
// === end  ======================================================
