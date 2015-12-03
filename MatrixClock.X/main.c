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

static const unsigned char opcode_table[][8] = {
    "Vol-","NULL","NULL","NULL", //0x00 Vol-
    "NULL","NULL","NULL","NULL", 
    "1","NULL","NULL","NULL",    //0x08 1
    "NULL","NULL","NULL","NULL", 
    "CH-","NULL","NULL","NULL",  //0x10 CH-
    "NULL","NULL","NULL","NULL", 
    "7","NULL","NULL","NULL",    //0x18 7 
    "NULL","NULL","NULL","NULL", 
    "Setup","NULL","NULL","NULL",//0x20 Setup
    "NULL","NULL","NULL","NULL",
    "4","NULL","NULL","NULL",    //0x28 4 
    "NULL","NULL","NULL","NULL", 
    "0.10+","NULL","NULL","NULL",//0x30 0.10+
    "NULL","NULL","NULL","NULL", 
    "NULL","NULL","NULL","NULL", 
    "NULL","NULL","NULL","NULL", 
    "Vol+","NULL","NULL","NULL", //0x40 Vol+
    "NULL","NULL","NULL","NULL", 
    "3","NULL","NULL","NULL",    //0x48 3
    "NULL","NULL","NULL","NULL", 
    "CH+","NULL","NULL","NULL",  //0x50 CH+
    "NULL","NULL","NULL","NULL", 
    "9","NULL","NULL","NULL",    //0x58 9
    "NULL","NULL","NULL","NULL", 
    "Stop","NULL","NULL","NULL", //0x60 Stop
    "NULL","NULL","NULL","NULL",
    "6","NULL","NULL","NULL",    //0x68 6
    "NULL","NULL","NULL","NULL",
    "U-turn","NULL","NULL","NULL", //0x70 U-turn
    "NULL","NULL","NULL","NULL",
    "NULL","NULL","NULL","NULL",
    "NULL","NULL","NULL","NULL",
    "Play","NULL","NULL","NULL", //0x80 Play
    "NULL","NULL","NULL","NULL",
    "2","NULL","NULL","NULL",    //0x88 2
    "NULL","NULL","NULL","NULL",
    "Enter","NULL","NULL","NULL", //0x90 Enter
    "NULL","NULL","NULL","NULL",
    "8","NULL","NULL","NULL",    //0x98 8
    "NULL","NULL","NULL","NULL",
    "Prev","NULL","NULL","NULL", //0xA0 Prev
    "NULL","NULL","NULL","NULL",
    "5","NULL","NULL","NULL",    //0xA8 5
    "NULL","NULL","NULL","NULL",
    "Next","NULL","NULL","NULL"  //0xB0 Next
};

// The arrays we are storing the pulses in
int last_pulses[100][2]; // stores the previous pulse
int pulses[100][2]; // pair is 0 is low 1 is high pulse
int currentpulse = 0; // index for pulses we are storing

// Decoding the pulses
int in_pulse = 0;
int decoded_pulses[50];
int opcode; // Holds the opcode's int representation after decode
int start_decode = 0; // Let's us know when its time to decode
char last_opcode_str[7];

// For reseting the pulses on timer overflow (20ms)
int end_pulse = 0; // 0 if we just finished pulse tranvsfer to mem

// Base Location of cursor, for table writing, temp string for writing
int init_cursor = 0;
int cursor;
char str[5];

char buffer[60];

int isINITPulse(int a, int b){
    if(a >= 5650 && a <= 5750){
        if(b >= 2750 && b <= 2850){
            return 1;
        }
    }
    return 0;
}

// Detects a Repeat Pulse
int isREPEATPulse(int a, int b){
    if(a >= 5650 && a <= 5750){
        if(b >= 1350 && b <= 1450){
            return 1;
        }
    }
    return 0;
}


// Decodes Logical Pulses
int isLogicPulse(int a){
    if(a >= 300 && a <= 400){ // 0 logical pulse
        return 0;
    }
    if(a >= 1000 && a <= 1100){ // 1 logical pulse
        return 1;
    }
    return 2;
}

int i;
void printpulses(void) {
    for(i = 0; i < 40; i++){
        cursor = init_cursor + (i + 1) * 10;
        sprintf(str,"%d:%d",i,last_pulses[i][0]);
//        tft_rewriteString(0,cursor,6,ILI9340_GREEN,str);
        
        sprintf(str,"%d",last_pulses[i][1]);
//        tft_rewriteString(75,cursor,6,ILI9340_GREEN,str);
  }
}

void printdecpulses(void){
    for(i = 0; i < 40; i++){
        cursor = init_cursor + (i + 1) * 10;
        sprintf(str,"%d",decoded_pulses[i]);
        //tft_rewriteString(150,cursor,3,ILI9340_RED,str);
    }
}

const char * getPulseCode(void){
    if(decoded_pulses[1] == 99){
        return "Repeat";
    }
    for(i = 2; i <= 9; i++){  // First check to see if the address is 0
        if(decoded_pulses[i]){
            return "Addr Err";
        }
    }
    opcode = 0;
    for(i = 18; i <= 25; i++){
        opcode = opcode << 1;
        opcode = decoded_pulses[i] + opcode;
    }
    return opcode_table[opcode];
}

void decodepulses(void){
    for(i = 0; i < 40; i++){
        if(isREPEATPulse(last_pulses[i][1], last_pulses[i+1][0])){
            decoded_pulses[i+1] = 99; // 99 signifies repeat pulse
            break;
        }
        if(isINITPulse(last_pulses[i][1],last_pulses[i+1][0])){ // First find a begining pulse
            decoded_pulses[i+1] = 52; // 52 signifies init pulse
            in_pulse = 1;
            i++;
            continue;
        }
        if(in_pulse = 1){ // Once we are in the pulse, then decode the pulses via gaps
            decoded_pulses[i] = isLogicPulse(last_pulses[i][0]);
        }
    }
}

// === Update TFT Thread ==================================================
// Just updates the TFT Periodically

static struct pt pt_ir;
int timer;
int counter;
const char * opcode_str;
static PT_THREAD(protothread_ir(struct pt *pt)) {
    // Just write out a header
    PT_BEGIN(pt);
//    tft_setCursor(0,0);
//    tft_setTextSize(1);
//    tft_setTextColor(ILI9340_GREEN);
//    tft_writeString("LOW \t      HIGH");
//    tft_setTextSize(1);
    while(1){
       // sprintf(str,"%d",counter);
        // tft_rewriteString(100,150,6,ILI9340_RED,str);
        //printpulses();
        decodepulses();
        //printdecpulses();
        opcode_str = getPulseCode();
        matrix_setCursor(0,0);
        matrix_fillScreen(COLOR565_BLACK);
        matrix_write3x5String(opcode_str);
        matrix_swapBuffers(FALSE);
//        tft_rewriteString(170,170,6,ILI9340_BLUE,opcode_str);
        start_decode = 0;
        PT_YIELD_UNTIL(pt, start_decode);
    }
    PT_END(pt);
}

//=== Capture 1 ISR ================================
int capture1;           // Variable that captures Timer
int IR_init;              // Are we in an IR pulse?
int pos = 0;                  // High/Low Voltage

void __ISR(_INPUT_CAPTURE_1_VECTOR, ipl3) C1Handler(void){
    counter++;
    capture1 = mIC1ReadCapture();   // Capture time
    WriteTimer3(0x0000);            // Reset Timer to 0
    if(pos == 1){   // If we went HIGH/LOW
        pulses[currentpulse][0] = capture1;
    }
    else{           // If we went LOW/HIGH
        pulses[currentpulse][1] = capture1;
        currentpulse++;
    }
    pos = !pos;
    end_pulse = 1;
    mIC1ClearIntFlag();
}

//=== Timer 2 Overflow Handler
// Will clear the pulses varible to reset for next capture
void __ISR(_TIMER_3_VECTOR, ipl2) Timer3Handler(void){
    if(end_pulse){
        memcpy(*last_pulses,*pulses,sizeof(pulses));
        memset(*pulses, 0, sizeof(pulses));
        end_pulse = 0;
        currentpulse = 0;
        start_decode = 1;
    }
    mT3ClearIntFlag();
    
}

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
        
        draw_dtime(dec_tm, dec_dt);
        //draw_atime(dec_tm, dec_dt);
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
    
    rtcc_init();
    matrix_init(true);
    
    INTEnableSystemMultiVectoredInt();
    
        // Configure Timer 2
    // Setup timer2 on, interrupts, internal clock, perscalar of 64, toggle rate
    // System CPU is configured to be 40 MHz
    // With Prescalar of 64, this means each timer tick is 1.6e-6 seconds
    // Timer Period is configured to trigger gap time at 20ms (12500 ticks)
    
    OpenTimer3(T3_ON | T3_SOURCE_INT | T3_PS_1_64, 0x30D4);
    ConfigIntTimer3(T3_INT_ON | T3_INT_PRIOR_2);
    mT3ClearIntFlag(); // clear int flag    
    
    // Set up the input capture
    OpenCapture1(IC_EVERY_EDGE | IC_INT_1CAPTURE | IC_TIMER3_SRC | IC_ON);
    // turn on the interrupt so that every catpure can be recorded
    ConfigIntCapture1(IC_INT_ON | IC_INT_PRIOR_3 | IC_INT_SUB_PRIOR_3);
    INTClearFlag(INT_IC1);
    // connect PIN 15 to IC1 Capture
    PPSInput(3, IC1, RPB13);
    mPORTBSetPinsDigitalIn(BIT_13);  // set pin as input
    
    PT_setup();
    
    PT_INIT(&pt_update_matrix);
    PT_INIT(&pt_serial);
    PT_INIT(&pt_ir);
    
    while (TRUE) {
        //PT_SCHEDULE(protothread_update_matrix(&pt_update_matrix));
        //PT_SCHEDULE(protothread_serial(&pt_serial));
        PT_SCHEDULE(protothread_ir(&pt_ir));
    }
}
// === end  ======================================================
