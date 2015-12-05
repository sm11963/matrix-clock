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

cringbuf_t ir_received_ids;
ulringbuf_t ir_received_times;

extern const char * const ir_opcode_table[];

void ir_init();


#endif	/* IR_REMOTE_H */

