#include "matrix_tests.h"
#include "rgb_matrix.h"
#include "matrix_gfx.h"
#include <math.h>

/* Fill screen with a color wheel */
void draw_colorwheel() {
    int x, y, hue;
    float dy, dx, d;
    UINT16 c;
    UINT8 sat, val;
    
    for(y=0; y < _matrix_width; y++) {
        dy = 15.5 - (float)y;
        for(x=0; x < _matrix_height; x++) {
            dx = 15.5 - (float)x;
            d  = dx * dx + dy * dy;
            if (d <= (16.5 * 16.5)) { // Inside the circle(ish)?
                hue = (int)((atan2(-dy, dx) + M_PI) * 1536.0 / (M_PI * 2.0));
                d = sqrt(d);
                if(d > 15.5) {
                    // Do a little pseudo anti-aliasing along perimeter
                    sat = 255;
                    val = (int)((1.0 - (d - 15.5)) * 255.0 + 0.5);
                }
                else {
                    // White at center
                    sat = (int)(d / 15.5 * 255.0 + 0.5);
                    val = 255;
                }
                c = matrix_colorHSV(hue, sat, val, 1);
            } else {
                c = 0;
            }
            matrix_drawPixel(x, y, c);
        }
    }  
}

/* Fill screen with various brightness levels of each color */
void draw_levels() {
    UINT16 i, j, r, b;
    
    for (i=0; i<_matrix_width; i++) {
        for (j=0; j<_matrix_height; j++) {
            r = (i % 16);
            if (j%4 == 0) {
                b = r << 12; // Red
            } else if (j%4 == 1) {
                b = r << 7; // Green
            } else if (j%4 == 2) {
                b = r << 1; // Blue
            } else {
                //b = (r << 12) | (r << 7) | (r << 1); // White
                b = (r << 12) | (r << 7) | (r << 1); // White
            }
            matrix_drawPixel(i,j,b);
        }
    }
}
