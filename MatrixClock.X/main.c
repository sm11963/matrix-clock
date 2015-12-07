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
#include "ir_remote.h"
#include "serial_ext.h"
#include "clock_gfx.h"

// === Update TFT Thread ==================================================
// Just updates the TFT Periodically

static struct pt pt_ir;
ir_cmd_t ir_cmd;
BOOL ir_cmd_new = FALSE;

static PT_THREAD(protothread_ir(struct pt *pt)) {
    PT_BEGIN(pt);
    while(TRUE){
        PT_YIELD_UNTIL( pt, ir_ready);
        ir_receive(&ir_cmd);
        ir_cmd_new = TRUE;
    }
    PT_END(pt);
}

void rtcc_init() {
    rtccTime tm;
    rtccDate dt;
    
    // Time this code was finalized
    tm.l=0x00;
    tm.sec=0x00;                                                                 
    tm.min=0x15;
    tm.hour=0x19;

    dt.wday=0x6;
    dt.mday=0x06;
    dt.mon=0x12;
    dt.year=0x15;
    
    RtccOpen(tm.l, dt.l, 0);
}

struct pt pt_update_matrix, pt_serial;

// The original format BCD codified date/time and the decimal versions
rtccTime bcd_tm, dec_tm;
rtccDate bcd_dt, dec_dt;

static PT_THREAD(protothread_update_matrix(struct pt *pt)) {
    static char display_face = 0;
    PT_BEGIN(pt);
    
    while(TRUE) {    
        RtccGetTimeDate(&bcd_tm, &bcd_dt);
        // Convert BCD codified date/time to decimal
        dec_tm = bcdTime2DecTime(bcd_tm);
        dec_dt = bcdDate2DecDate(bcd_dt);
        
        if (dec_tm.min % 30 == 0) {
            pt_printl("gtd");
        }
        
        if (ir_cmd_new && ir_cmd.remote_type == ir_remote_type_apple) { 
            ir_cmd_new = FALSE;
            if (ir_cmd.opcode == apple_remote_opcode_minus) {
                display_face = 0;
            }
            else if (ir_cmd.opcode == apple_remote_opcode_plus) {
                display_face = 1;
            }
            else if (ir_cmd.opcode == apple_remote_opcode_enter) {
                pt_printl("gtd");
            }
        }
        
        matrix_fillScreen(COLOR565_BLACK);
        
        if (display_face == 1) {
            draw_dtime(dec_tm, dec_dt);
        }
        else {
            draw_atime(dec_tm, dec_dt);
        }
        PT_YIELD(pt);
        
        matrix_swapBuffers(FALSE);
    }
    
    PT_END(pt);
}

void setTime(char hour, char min, char sec) {
    static rtccTime tm;

    tm.l=0x00;
    tm.sec=sec;                                                                 
    tm.min=min;
    tm.hour=hour;
    
    RtccSetTime(decTime2BcdTime(tm).l);
}

void setDate(char month, char mday, char year, char wday) {
    rtccDate dt;

    dt.wday=wday;
    dt.mday=mday;
    dt.mon=month;
    dt.year=year;
    
    RtccSetDate(decDate2BcdDate(dt).l);
}

// === Update Serial Thread ==================================================
static PT_THREAD(protothread_serial(struct pt *pt)) {
    PT_BEGIN(pt);
    
    static int in_hr, in_min, in_sec;
    static int in_mday, in_wday, in_mon, in_yr;
    
    pt_printl("gtd"); // Request initial time/date update (Get Time Date)
    
    while (TRUE) {
        PT_SPAWN( pt, &pt_get, PT_GetSerialBuffer(&pt_get) );
        if (sscanf(PT_term_buffer, "t %d:%d:%d", &in_hr, &in_min, &in_sec) == 3) {
            setTime(in_hr, in_min, in_sec);
            pt_printl("ok");
        }
        else if (sscanf(PT_term_buffer, "d %d/%d/%d-%d", &in_mon, &in_mday, &in_yr, &in_wday) == 4) {
            setDate(in_mon, in_mday, in_yr, in_wday);
            pt_printl("ok");
        }
        else {
            pt_printl("BAD CMD");
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
    
    rtcc_init();
    matrix_init(TRUE);
    ir_init();
    
    INTEnableSystemMultiVectoredInt();
    
    PT_setup();
    
    PT_INIT(&pt_update_matrix);
    PT_INIT(&pt_serial);
    PT_INIT(&pt_ir);
    
    while (TRUE) {
        PT_SCHEDULE(protothread_update_matrix(&pt_update_matrix));
        PT_SCHEDULE(protothread_serial(&pt_serial));
        PT_SCHEDULE(protothread_ir(&pt_ir));
    }
}
// === end  ======================================================
