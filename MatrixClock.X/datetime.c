#include "datetime.h"

/*******************************************************************************
 * Integer date/time to string conversion mappings
 ******************************************************************************/

const char* days_short[] = {
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
    "SAT",
    "SUN"
};

const char* days_long[] = {
    "Monday",
    "Tuesday",
    "Wednsday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday"
};

const char* months_long[] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "Septmber",
    "October",
    "November",
    "December"
};

const char* months_short[] = {
    "JAN",
    "FEB",
    "MAR",
    "APR",
    "MAY",
    "JUN",
    "JUL",
    "AUG",
    "SEP",
    "OCT",
    "NOV",
    "DEC"
};

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/

unsigned char bcd2char(unsigned char x) {
    return (x & 0xf) + (((x & 0xf0) >> 4) * 10);
}

rtccTime bcdTime2DecTime(rtccTime tm) {
    tm.sec = bcd2char(tm.sec);
    tm.min = bcd2char(tm.min);
    tm.hour = bcd2char(tm.hour);
    
    return tm;
}

rtccDate bcdDate2DecDate(rtccDate dt) {
    dt.mday = bcd2char(dt.mday);
    dt.mon = bcd2char(dt.mon);
    dt.year = bcd2char(dt.year);
    
    return dt;
}

unsigned char twentyFour2TwelveHour(unsigned char hour) {
    if (hour == 0) {
        return 12;
    }
    else if (hour > 12) {
        return hour - 12;
    }
    else {
        return hour;
    }
}