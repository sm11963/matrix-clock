/* 
 * File:   IR_Test.c
 * Author: Yoshi
 *
 * Created on November 18, 2015, 5:03 PM
 */

#include "config.h"
#include <stdlib.h>
#include <math.h>
#include <xc.h>

// TFT Library
#include "tft_master.h"
#include "tft_gfx.h"

// threading library
#include "pt_cornell_1_2.h"

// Initialization of threads
static struct pt pt_update_tft;

//=========== Variable Declarations ================

// Max pulse length is 65 msec
#define MAXPULSE 65000

// What our timing resolution is
#define RESOLUTION 20

// OP Code Lookup Table
/* OP Codes from the Remote
Vol - : 0x00
Play/Pause: 0x80
Vol +: 0x40
Setup: 0x20
Prev: 0xA0
Stop/Mode: 0x60
CH-: 0x10
Enter/Save: 0x90
CH+: 0x50
0.10+: 0x30
Next: 0xB0
U-turn: 0x70
1: 0x08
2: 0x88
3: 0x48
4: 0x28
5: 0xA8
6: 0x68
7: 0x18
8: 0x98
9: 0x58
*/
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
// Helper to re-write strings on the screen
#define tft_rewriteString(x, y, max_len, color, fmt, ...) \
    tft_setTextColor( (color) ); \
    tft_fillRect( (x), (y), (max_len)*12, 14, ILI9340_BLACK ); \
    tft_setCursor( (x), (y) ); \
    sprintf( buffer, (fmt), ##__VA_ARGS__ ); \
    tft_writeString( buffer )

//============ Helper Functions ===============
// Prints out the pulse times to the TFT
// Side: 0 is first column
//       1 is second column

// Detects an Initialization Pulse
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
        tft_rewriteString(0,cursor,6,ILI9340_GREEN,str);
        
        sprintf(str,"%d",last_pulses[i][1]);
        tft_rewriteString(75,cursor,6,ILI9340_GREEN,str);
  }
}

void printdecpulses(void){
    for(i = 0; i < 40; i++){
        cursor = init_cursor + (i + 1) * 10;
        sprintf(str,"%d",decoded_pulses[i]);
        tft_rewriteString(150,cursor,3,ILI9340_RED,str);
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
int timer;
int counter;
const char * opcode_str;
static PT_THREAD(protothread_tft(struct pt *pt)) {
    // Just write out a header
    PT_BEGIN(pt);
    tft_setCursor(0,0);
    tft_setTextSize(1);
    tft_setTextColor(ILI9340_GREEN);
    tft_writeString("LOW \t      HIGH");
    tft_setTextSize(1);
    while(1){
       // sprintf(str,"%d",counter);
        // tft_rewriteString(100,150,6,ILI9340_RED,str);
        //printpulses();
        decodepulses();
        printdecpulses();
        opcode_str = getPulseCode();
        tft_rewriteString(170,170,6,ILI9340_BLUE,opcode_str);
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
    WriteTimer2(0x0000);            // Reset Timer to 0
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
void __ISR(_TIMER_2_VECTOR, ipl2) Timer2Handler(void){
    if(end_pulse){
        memcpy(*last_pulses,*pulses,sizeof(pulses));
        memset(*pulses, 0, sizeof(pulses));
        end_pulse = 0;
        currentpulse = 0;
        start_decode = 1;
    }
    mT2ClearIntFlag();
    
}


// === Main  ======================================================

void main(void) {
    SYSTEMConfigPerformance(PBCLK);
    
    ANSELA = 0; ANSELB = 0; CM1CON = 0; CM2CON = 0;
    // === Configure threads ==========
    // turns OFF UART support and debugger pin
    PT_setup();

    // === setup system wide interrupts  ========
    INTEnableSystemMultiVectoredInt();
    
    
    // ===================== initialize the display =======================
    tft_init_hw();
    tft_begin();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(0);  // set to 320x240 mode 
    
    // Configure Timer 2
    // Setup timer2 on, interrupts, internal clock, perscalar of 64, toggle rate
    // System CPU is configured to be 40 MHz
    // With Prescalar of 64, this means each timer tick is 1.6e-6 seconds
    // Timer Period is configured to trigger gap time at 20ms (12500 ticks)
    
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_64, 0x30D4);
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_2);
    mT2ClearIntFlag(); // clear int flag
    
    
    
    
    // Set up the input capture
    OpenCapture1(IC_EVERY_EDGE | IC_INT_1CAPTURE | IC_TIMER2_SRC | IC_ON);
    // turn on the interrupt so that every catpure can be recorded
    ConfigIntCapture1(IC_INT_ON | IC_INT_PRIOR_3 | IC_INT_SUB_PRIOR_3);
    INTClearFlag(INT_IC1);
    // connect PIN 15 to IC1 Capture
    PPSInput(3, IC1, RPB13);
    mPORTBSetPinsDigitalIn(BIT_13);  // set pin as input
    
      
    
    // initialize the threads
    PT_INIT(&pt_update_tft);

    // round-robin scheduler for threads
    while (1) {
        PT_SCHEDULE(protothread_tft(&pt_update_tft));
    }
} // main