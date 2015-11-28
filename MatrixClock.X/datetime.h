/* 
 * File:   datetime.h
 * Author: smiller
 *
 * Created on November 28, 2015, 5:25 PM
 */

#ifndef DATETIME_H
#define	DATETIME_H

#define _SUPPRESS_PLIB_WARNING 1
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING 1
#include <peripheral/rtcc.h>

// To string conversion mappings
extern const char* days_short[];
extern const char* days_long[];
extern const char* months_long[];
extern const char* months_short[];

// Time/date conversion helpers
unsigned char bcd2char(unsigned char x);
rtccTime bcdTime2DecTime(rtccTime tm);
rtccDate bcdDate2DecDate(rtccDate dt);
unsigned char twentyFour2TwelveHour(unsigned char hour);

#endif	/* DATETIME_H */

