#include "ir_remote.h"

#include "config.h"
#include "pt_cornell_1_2.h"
#include "rgb_matrix.h"
#include "matrix_gfx.h"

// The arrays we are storing the pulses in
unsigned short last_pulses[100][2]; // stores the previous pulse
unsigned short pulses[100][2]; // pair is 0 is low 1 is high pulse
volatile unsigned short currentpulse = 0; // index for pulses we are storing

// Decoding the pulses
int in_pulse = 0;
int decoded_pulses[50];
int opcode; // Holds the opcode's int representation after decode
char last_opcode_str[7];

// For reseting the pulses on timer overflow (20ms)
int end_pulse = 0; // 0 if we just finished pulse tranvsfer to mem

// Base Location of cursor, for table writing, temp string for writing
int init_cursor = 0;
int cursor;

int counter;

#define APPLE_REMOTE_ADDR 0b11101110

const char * const apple_remote_mapping[] = {
    "UNK",
    "BACK",
    "FORW",
    "-",
    "+",
    "ENTER",
    "MENU",
};

const char* opcode2str_apple(int opcode) {
    switch (opcode) {
        case 1:
            return apple_remote_mapping[1];
            break;
        case 2:
            return apple_remote_mapping[5];
            break;
        case 4:
            return apple_remote_mapping[6];
            break;
        case 11:
            return apple_remote_mapping[3];
            break;
        case 13:
            return apple_remote_mapping[4];
            break;
        case 14:
            return apple_remote_mapping[2];
            break;
        default:
            return apple_remote_mapping[0];
    }
}


int getAddr() {
    static int addr, i;
    addr = 0;
    for(i = 2; i <= 9; i++){  // First check to see if the address is 0
        addr |= (decoded_pulses[i] << (i-2));
    }
    return addr;
}

int getOpcode() {
    static int opcode, i;
    opcode = 0;
    for(i = 18; i < 22; i++){
        opcode = opcode << 1;
        opcode = decoded_pulses[i] + opcode;
    }
    return opcode;
}

void ir_init() {
    bufferInit(ir_received_ids, 5, char);
    bufferInit(ir_received_times, 5, unsigned long);

    // Configure Timer 3
    // Setup timer3 on, interrupts, internal clock, perscalar of 64, toggle rate
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
}

inline BOOL ir_isINITPulse(int a, int b){
    return (a >= 5650 && a <= 5750) && (b >= 2750 && b <= 2850);
}

// Detects a Repeat Pulse
inline BOOL ir_isREPEATPulse(int a, int b){
    return (a >= 5650 && a <= 5750) && (b >= 1350 && b <= 1450);
}


// Decodes Logical Pulses
inline char ir_isLogicPulse(int a){
    if(a >= 300 && a <= 400){ // 0 logical pulse
        return 0;
    }
    if(a >= 1000 && a <= 1100){ // 1 logical pulse
        return 1;
    }
    return 2;
}

const char *getPulseCode(void){
    static char i;

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
    return ir_opcode_table[opcode];
}

 void matrix_drawIRPulses(void) {
    static unsigned char i;
    static const char num_in_row = 20;
    
    for(i = 0; i < 40; i++) {
        matrix_drawPixel(i % num_in_row, 10 + (i/num_in_row), COLOR565_BLACK);
        
        if (decoded_pulses[i] == 99) {
            matrix_drawPixel(i % num_in_row, 10 + (i / num_in_row), COLOR565_WHITE);
        } else if (decoded_pulses[i] == 52) {
            matrix_drawPixel(i % num_in_row, 10 + (i / num_in_row), COLOR565_BLUE);
        } else if (decoded_pulses[i] == 0) {
            matrix_drawPixel(i % num_in_row, 10 + (i / num_in_row), COLOR565_RED);
        } else if (decoded_pulses[i] == 1) {
            matrix_drawPixel(i % num_in_row, 10 + (i / num_in_row), COLOR565_GREEN);
        } else if (decoded_pulses[i] == 2) {
            matrix_drawPixel(i % num_in_row, 10 + (i / num_in_row), COLOR565_YELLOW);
        }
    }
}


void ir_decodepulses(void){
    static char i;
    for(i = 0; i < 40; i++){
        if(ir_isREPEATPulse(last_pulses[i][1], last_pulses[i+1][0])){
            decoded_pulses[i+1] = 99; // 99 signifies repeat pulse
            break;
        }
        if(ir_isINITPulse(last_pulses[i][1],last_pulses[i+1][0])){ // First find a begining pulse
            decoded_pulses[i+1] = 52; // 52 signifies init pulse
            in_pulse = 1;
            i++;
            continue;
        }
        if(in_pulse = 1){ // Once we are in the pulse, then decode the pulses via gaps
            decoded_pulses[i] = ir_isLogicPulse(last_pulses[i][0]);
        }
    }
}

unsigned char getPulseId() {
    char opcode, i;

    if (decoded_pulses[1] == 99) {
        return 15;// REPEAT CODE
    }

    for(i = 2; i <= 9; i++) {
        if (decoded_pulses[i]) {
            return 31;// ADDRS Err
        }
    }

    // We are using pulse ID as the native opcode divide by 4
    // Since this reduces the mapping and doesn't lose any info
    for (i = 18; i < 22; i++) {
        opcode = (opcode << 1) + decoded_pulses[i];
    }
    
    return opcode;
}

char ir_receive(ir_cmd_t* cmd_ptr) {
    if (!cmd_ptr) return -1;
    if (!ir_ready) return 1;
    
    ir_decodepulses();
    ir_ready = FALSE;
    
    cmd_ptr->addr = getAddr();
    cmd_ptr->opcode = getOpcode();
    cmd_ptr->is_repeat = (decoded_pulses[1] == 99);
    cmd_ptr->str = opcode2str_apple(cmd_ptr->opcode);
    
    return 0;
}

//=== Capture 1 ISR ================================
void __ISR(_INPUT_CAPTURE_1_VECTOR, IPL3AUTO) C1Handler(void){
    static BOOL pos = 0; // High/Low Voltage
    static capture;

    counter++;
    capture = mIC1ReadCapture();   // Capture time
    WriteTimer3(0);            // Reset Timer to 0
    if(pos == 1){   // If we went HIGH/LOW
        pulses[currentpulse][0] = capture;
    }
    else{           // If we went LOW/HIGH
        pulses[currentpulse][1] = capture;
        currentpulse++;
    }
    pos = !pos;
    end_pulse = TRUE;
    mIC1ClearIntFlag();
}

//=== Timer 2 Overflow Handler
// Will clear the pulses variable to reset for next capture
void __ISR(_TIMER_3_VECTOR, IPL2AUTO) Timer3Handler(void){
    if (end_pulse){
        memcpy(*last_pulses, *pulses, sizeof(pulses));
        memset(*pulses, 0, sizeof(pulses));
        end_pulse = FALSE;
        currentpulse = 0;
        ir_ready = TRUE;
    }
    mT3ClearIntFlag();
}

const char * const ir_opcode_table[] = {
    "Vol-",   //0x00 Vol-
    "NULL",
    "1",         //0x08 1
    "NULL",
    "CH-",     //0x10 CH-
    "NULL",
    "7",         //0x18 7
    "NULL",
    "Setup", //0x20 Setup
    "NULL",
    "4",         //0x28 4
    "NULL",
    "0.10+", //0x30 0.10+
    "NULL",
    "REPEAT", // Designated for repeat code
    "NULL",
    "Vol+",   //0x40 Vol+
    "NULL",
    "3",         //0x48 3
    "NULL",
    "CH+",     //0x50 CH+
    "NULL",
    "9",         //0x58 9
    "NULL",
    "Stop",   //0x60 Stop
    "NULL",
    "6",         //0x68 6
    "NULL",
    "U-turn", //0x70 U-turn
    "NULL",
    "ERR",    // Designated for error
    "NULL",
    "Play",   //0x80 Play
    "NULL",
    "2",         //0x88 2
    "NULL",
    "Enter",  //0x90 Enter
    "NULL",
    "8",         //0x98 8
    "NULL",
    "Prev",   //0xA0 Prev
    "NULL",
    "5",         //0xA8 5
    "NULL",
    "Next",   //0xB0 Next
};
