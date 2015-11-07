/*
 * File:        main.c
 * Author:      Sam Miller & Saikrupa Iyer
 * Adapted from:
 *              main.c by
 * Author:      Syed Tahmid Mahbub
 * Target PIC:  PIC32MX250F128B
 */

// graphics libraries
#include "config.h"
#include "tft_master.h"
#include "tft_gfx.h"
// need for rand function
#include <stdlib.h>
#include <math.h>
#include <xc.h>
#include "sgm82_helpers.h"

// threading library
#include "pt_cornell_1_2.h"

#define PWM_RATE 1000 // hz
#define PWM_PERIOD_TICKS 40000 // PB_CLOCK / PWM_RATE

/************************* Serial IO ******************************************/

// Define better printing helpers to use the PT printing interface easily
// These must be used within a Protothread using the pt local variable for
// its protothread structure.

// Setup the protothread structures that are used for the put and get threads.
// These should not be used for any other thread besides spawning
// PT_GetSerialBuffer and PutSerialBuffer
static struct pt pt_get, pt_put;

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


/************************* Variable declarations ******************************/

// === thread structures ============================================
// thread control structs
// note that UART input and output are threads
static struct pt pt_update_tft, pt_serial, pt_pid;

// Iteration vars
unsigned short i, j, k;
// temp vars
char *char_ptr;

// Global control variables
int motor_rpm_goal;
volatile float motor_rpm_actual;
float pid_p = 1;
float pid_i = 0.005;
float pid_d = 1.8;

// Temp vars for capturing serial inputs
float in_float;
int in_int;

// Timing vars
volatile int interval_ticks;  // Timer ticks captured for single blade interval
volatile int timer2_overflow; // Did timer 2 overflow?

// Convenience defines for setting PWM
#define set_motor_pwm(ticks) \
    SetDCOC1PWM(ticks)

#define set_motor_duty(percent) \
    set_motor_pwm(((int)(400 * percent)))

#define set_rpm_pwm(ticks) \
    SetDCOC2PWM(ticks)       

// Constant pre-calculated from: ticks = (40000000/32)*(60/7)*(1/<RPM>)
#define ticks_to_rpm(ticks) \
    (10714285.714 / ((float)(ticks)))

// General purpose string buffer
char buffer[60];

// Helper to re-write strings on the screen
#define tft_rewriteString(x, y, max_len, color, fmt, ...) \
    tft_setTextColor( (color) ); \
    tft_fillRect( (x), (y), (max_len)*12, 14, ILI9340_BLACK ); \
    tft_setCursor( (x), (y) ); \
    sprintf( buffer, (fmt), ##__VA_ARGS__ ); \
    tft_writeString( buffer )

// === Update TFT Thread ==================================================
static PT_THREAD(protothread_update_tft(struct pt *pt)) {
    PT_BEGIN(pt);
    tft_setTextSize(2);
    print_lab_header(4);
    
    tft_setTextColor(ILI9340_WHITE);
    
    tft_rewriteString( 10, 60, 7, ILI9340_CYAN, "P Gain:");
    tft_rewriteString( 10, 100, 7, ILI9340_CYAN, "I Gain:");
    tft_rewriteString( 10, 140, 7, ILI9340_CYAN, "D Gain:");
    tft_rewriteString( 10, 180, 13, ILI9340_CYAN, "Target Speed:");    
    tft_rewriteString( 10, 220, 13, ILI9340_CYAN, "Actual Speed:");    

    while (1) {
        
        tft_rewriteString( 10, 80, 20, ILI9340_WHITE, "%f", pid_p);
        tft_rewriteString( 10, 120, 20, ILI9340_WHITE, "%f", pid_i);        
        tft_rewriteString( 10, 160, 20, ILI9340_WHITE, "%f", pid_d);        
        tft_rewriteString( 10, 200, 9, ILI9340_WHITE, "%d RPM", motor_rpm_goal);
        tft_rewriteString( 10, 240, 20, ILI9340_WHITE, "%f RPM", motor_rpm_actual);
        
        PT_YIELD_TIME_msec(200);
    } // END WHILE(1)
    PT_END(pt);
} // Update TFT thread

// === Update Serial Thread ==================================================
static PT_THREAD(protothread_serial(struct pt *pt)) {
    PT_BEGIN(pt);
            
    while (1) {
        PT_SPAWN( pt, &pt_get, PT_GetSerialBuffer(&pt_get) );
        if (num_char > 2) {
            if (sscanf(PT_term_buffer, "s %d", &in_int)) {
                if (in_int >= 0 && in_int <= 3000) {
                    pt_printfl("Changing speed to: %d", in_int);
                    motor_rpm_goal = in_int;
                }
                else {
                    pt_printfl("Invalid motor speed: %d", in_int);
                }
            }
            else if (sscanf(PT_term_buffer, "p %f", &in_float)) {
                pt_printfl("Changing P gain to: %f", in_float);
                pid_p = in_float;
            }
            else if (sscanf(PT_term_buffer, "i %f", &in_float)) {
                pt_printfl("Changing I gain to: %f", in_float);
                pid_i = in_float;
            }
            else if (sscanf(PT_term_buffer, "d %f", &in_float)) {
                pt_printfl("Changing D gain to: %f", in_float);
                pid_d = in_float;
            }
            else {
                pt_printl("Invalid cmd.");
                pt_printl("USAGE: 's <int>', 'p <float>'");
                pt_printl("     'i <float>', 'd <float>'");
            }
        }
        else {
            pt_printl("Invalid cmd.");
            pt_printl("USAGE: 's <int>', 'p <float>'");
            pt_printl("     'i <float>', 'd <float>'");
        }
    } // END WHILE(1)
    PT_END(pt);
} // Update Serial thread

// === Update PID Thread ==================================================
// Updates the PID control 100 times/sec
//
// Relevant variables are as follows;
// int motor_rpm_goal;              The ideal RPM
// float motor_rpm_actual;          Measured RPM from sensor
// float pid_p, pid_d, pid_i;       Each of the PID gains
float c_error = 0;      //              The "P" component of PID calculation
float last_c_error = 0; //              Holds the previous c_error  
float pid_integral = 0; //              The "I" component of the PID calc
float pid_deriv = 0;    //              The "D" component of the PID calc
float output = 0;       //              The output of the PID calc (ranges 0-100)

static PT_THREAD(protothread_pid(struct pt *pt)) {
    PT_BEGIN(pt);
        
    while (1) {
        
        // Calculate the control P error
        c_error = ((float)motor_rpm_goal) - motor_rpm_actual;
        
        // Calculate the integral I error
        if(c_error > 0.0f){
            pid_integral = pid_integral + c_error;
        }
        else{
            pid_integral = 0;
        }
        
        // Calculate the D error
        pid_deriv = c_error - last_c_error;
        
        // Calculate output signal
        output = (pid_p * c_error) + (pid_i * pid_integral) + (pid_d * pid_deriv);
        if(output > 100){
            output = 100;
        } 
        else if (output < 0) {
            output = 0;
        }
        // Set the PWM output to the motor
        set_motor_duty(output);
        
        // Update last_c_error
        last_c_error = c_error;
        
        // Delay 10 msec to give us the 100 updates/sec
        PT_YIELD_TIME_msec(10);
    } // END WHILE(1)
    PT_END(pt);
} // Update PID thread



// 1250 RPM => 8571 ticks
// 250 RPM  => 42857 ticks
// 1000 RPM => 10714 ticks
// 500  RPM => 21429 ticks
// Formula: ticks = (40000000/32)*(60/7)*(1/<RPM>)
//
// Actual RPM PWM output:
// 0%   => 0 RPM
// 25%  => 500 RPM
// 50%  => 1000 RPM
// 100% => 2000 RPM
void __ISR(_INPUT_CAPTURE_4_VECTOR, ipl4) IC4Interrupt(void) {
    // Clear timer first thing to avoid losing timer ticks
    WriteTimer2(0); 

    interval_ticks = mIC4ReadCapture();
    
    // Only update the speed measurement if there wasn't overflow (ie. there
    // was a valid measurement)
    if (timer2_overflow == 0) {
        motor_rpm_actual = ticks_to_rpm(interval_ticks);

        set_rpm_pwm(((int)motor_rpm_actual) * 20);
    }
    
    // Clear the overflow so that if the next measurement does not overflow
    // we correctly use it.
    timer2_overflow = 0;
    mIC4ClearIntFlag();
}

void __ISR(_TIMER_2_VECTOR, ipl5) IRTimerOverflow(void) {
    // If we get overflow we assume the speed is below 250 which we do not
    // measure, therefore assume 0.
    timer2_overflow++;
    motor_rpm_actual = 0;
    set_rpm_pwm(0);
    mT2ClearIntFlag();
}

// === Main  ======================================================

void main(void) {
    SYSTEMConfigPerformance(PBCLK);
    // === Configure threads ==========
    // turns OFF UART support and debugger pin
    PT_setup();

    // === setup system wide interrupts  ========
    INTEnableSystemMultiVectoredInt();
    
    // =============== Setup PWM ===================
    OpenTimer3(T3_ON | T3_SOURCE_INT | T3_PS_1_1, PWM_PERIOD_TICKS);
    
    // Open OC1 for motor PWM
    OpenOC1(OC_ON | OC_TIMER3_SRC | OC_PWM_FAULT_PIN_DISABLE, 0, 0);
    PPSOutput(1, RPA0, OC1); // OC1 -> pin 2 (RA0/RPA0)
    
    // Open OC2 for actual RPM PWM
    OpenOC2(OC_ON | OC_TIMER3_SRC | OC_PWM_FAULT_PIN_DISABLE, 0, 0);
    PPSOutput(2, RPB5, OC2); // OC2 -> pin 14 (RB5/RPB5)
    
    // ================= Setup input capture ==============================
    // Open timer 2 with prescalar 32 and max value 2^16
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_32, 0xffff);
    // Setup interrupts to handle overflow
    ConfigIntTimer2( T2_INT_ON | T2_INT_PRIOR_5 );
    // Setup capture 4 to capture every rising edge and capture timer 2
    OpenCapture4(IC_ON | IC_FEDGE_RISE | IC_CAP_16BIT | IC_TIMER2_SRC | IC_INT_1CAPTURE | IC_EVERY_RISE_EDGE);
    // IC4 <- pin 16 (RB7/RPB7)
    PPSInput(1, IC4, RPB7);
    ConfigIntCapture4( IC_INT_ON | IC_INT_PRIOR_4 );
    mIC4ClearIntFlag();
    mT2ClearIntFlag();
    
    // ===================== initialize the display =======================
    tft_init_hw();
    tft_begin();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(0);  // set to 320x240 mode 
    
    // initialize the threads
    PT_INIT(&pt_update_tft);
    PT_INIT(&pt_serial); 
    PT_INIT(&pt_pid);

    // round-robin scheduler for threads
    while (1) {
        PT_SCHEDULE(protothread_update_tft(&pt_update_tft));
        PT_SCHEDULE(protothread_serial(&pt_serial));
        PT_SCHEDULE(protothread_pid(&pt_pid));
    }
} // main

// === end  ======================================================

