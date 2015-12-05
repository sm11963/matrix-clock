/* 
 * File:   ir_remote.h
 * Author: smiller
 *
 * Created on December 5, 2015, 2:33 AM
 */

#ifndef IR_REMOTE_H
#define	IR_REMOTE_H

#include "ringbuffer.h"

ringBuffer_typedef(char, cringbuf_t);
ringBuffer_typedef(unsigned long, ulringbuf_t);

typedef struct {
    unsigned short opcode;
    unsigned short addr;
    unsigned char is_repeat;
    const char *str;
} ir_cmd_t;

cringbuf_t ir_received_ids;
ulringbuf_t ir_received_times;

volatile unsigned char ir_ready;

extern const char * const ir_opcode_table[];

void ir_init();
int getOpcode();
const char* opcode2str_apple(int opcode);
void ir_decodepulses();
char ir_receive(ir_cmd_t* cmd_ptr);

#endif	/* IR_REMOTE_H */

