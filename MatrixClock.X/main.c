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


void rtcc_init() {
    rtccTime tm;
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
        
    RtccOpen(tm.l, dt.l, -500);
}

volatile BOOL enabled = 1;

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
        
    INTEnableSystemMultiVectoredInt();
    
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

void display_date() {
    int secs = 0;
    
    while (TRUE) {
        matrix_fillScreen(COLOR565_BLACK);
        matrix_setCursor(2,2);
        matrix_setTextColor(0x738e);
        matrix_write3x5String("January");

        matrix_setCursor(1,12);
        matrix_setTextColor(0xffff);
        matrix_writeString("24:02");

        if (secs < 30) {
            matrix_drawLine(1,20,secs+1,20,matrix_color444(4,0,0));
        }
        else {
            matrix_drawLine(1,20,30,20,matrix_color444(4,0,0));
            matrix_drawLine(1,21,secs-29,21, matrix_color444(4,0,0));
        }
        
        if (++secs > 59) {
            secs = 0;
        }
        
        matrix_setCursor(0,25);
        matrix_setTextColor(0x738e);
        matrix_write3x5String(" FRI 14 ");

        matrix_swapBuffers(false);
        
        delay_ms(500);
    }
}

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
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
    "SAT",
    "SUN"
};

static const char* days_long[] = {
    "Monday",
    "Tuesday",
    "Wednsday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday"
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
    "NOV",
    "DEC"
};

static const char hr_end_pnts[] = {
    0, -6, //1
    3, -5, //2
    5, -3, //3
};

void get_end_point(int idx, const char* end_pnts, unsigned int num_pnts, int* point) {
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

void display_analog() {
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
    
    matrix_swapBuffers(TRUE);
    
    int i=0,j=5,k=2;
    
    while(TRUE) {
        
        int point_sec[2];
        int point_min[2];
        int point_hr[2];
        
        get_end_point(i, min_end_pnts, 15, point_sec);
        get_end_point(j, min_end_pnts, 15, point_min);
        get_end_point(k, hr_end_pnts, 3, point_hr);
        
        matrix_fillCircle(16,16,14,COLOR565_BLACK);
        matrix_drawLine(16,16,16+point_sec[0],16+point_sec[1],COLOR565_CYAN);
        matrix_drawLine(16,16,16+point_min[0],16+point_min[1],COLOR565_MAGENTA);
        matrix_drawLine(16,16,16+point_hr[0],16+point_hr[1],COLOR565_YELLOW);

        matrix_swapBuffers(TRUE);
        
        delay_ms(900);
                
        if (++i > 59) {
            i = 0;
            if (++j > 59) {
                j = 0;
                if (++k > 11) {
                    k = 0;
                }
            }
        }    
    }
}

unsigned char sec=0,min=0,hr=0;
struct pt pt_update_matrix;

void update_analog_display() {
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
                    
    int point_sec[2];
    int point_min[2];
    int point_hr[2];

    get_end_point(sec, min_end_pnts, 15, point_sec);
    get_end_point(min, min_end_pnts, 15, point_min);
    get_end_point(hr, hr_end_pnts, 3, point_hr);

    matrix_drawLine(16,16,16+point_sec[0],16+point_sec[1],COLOR565_CYAN);
    matrix_drawLine(16,16,16+point_min[0],16+point_min[1],COLOR565_MAGENTA);
    matrix_drawLine(16,16,16+point_hr[0],16+point_hr[1],COLOR565_YELLOW);

    matrix_swapBuffers(FALSE);

    delay_ms(900);

    if (++sec > 59) {
        sec = 0;
        if (++min > 59) {
            min = 0;
            if (++hr > 11) {
                hr = 0;
            }
        }
    }    
}

static PT_THREAD(protothread_update_matrix(struct pt *pt)) {
    PT_BEGIN(pt);
    
    while(TRUE) {
        update_analog_display();
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
