#include "analog_clock.h"

#define swapc(a, b) { char t = a; a = b; b = t; }

/*******************************************************************************
 * Analog clock endpoint mappings
 ******************************************************************************/

const char end_pnts60[] = {
    0, -12, //1
    1, -12, //2
    2, -12, //3
    3, -11, //4
    5, -11, //5
    6, -10, //6
    7, -10, //7
    8, -9,  //8
    9, -8,  //9
    10, -7, //10
    10, -6, //11
    11, -5, //12
    11, -3, //13
    12, -2, //14
    12, -1  //15
};

const char end_pnts12[] = {
    0, -6, //1
    3, -5, //2
    5, -3, //3
};

void get_end_point(int idx, const char* end_pnts, unsigned int num_pnts, char* point) {
    int i = idx % num_pnts;
    int x = end_pnts[i*2];
    int y = end_pnts[(i*2)+1];
    
    const int num_pnts_2x = num_pnts << 1;
    const int num_pnts_3x = (num_pnts << 1) + num_pnts;
    
    if (idx >= num_pnts && idx < num_pnts_2x) {
        swapc(x,y);
        x = -x;
    }
    else if (idx >= num_pnts_2x && idx < num_pnts_3x) {
        x = -x;
        y = -y;
    }
    else if (idx >= num_pnts_3x) {
        swapc(x,y);
        y = -y;
    }
    
    point[0] = x;
    point[1] = y;
}

inline void get_end_pnt60(unsigned char idx, char* ppoint) {
    get_end_point(idx, end_pnts60, 15, ppoint);
}

inline void get_end_pnt12(unsigned char idx, char* ppoint) {
    get_end_point(idx, end_pnts12, 3, ppoint);
}
