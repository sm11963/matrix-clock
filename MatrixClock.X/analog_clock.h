/* 
 * File:   analog_clock.h
 * Author: smiller
 *
 * Created on November 28, 2015, 5:35 PM
 */

#ifndef ANALOG_CLOCK_H
#define	ANALOG_CLOCK_H


// Min/sec/hour hand end point arrays
extern const char min_end_pnts[];
extern const char hr_end_pnts[];

// Accessing analog clock end points for 60/12 position hands
void get_end_pnt60(unsigned char idx, char* ppoint);
void get_end_pnt12(unsigned char idx, char* ppoint);

// Generic index to end point conversion function
void get_end_point(int idx, const char* end_pnts, unsigned int num_pnts, char* point);

#endif	/* ANALOG_CLOCK_H */

