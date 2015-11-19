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

const char str[]  = "Pic32 32x32 RGB LED Matrix Scroll test";

INT8 ball[3][4] = {
      {  3,  0,  1,  1 }, // Initial X,Y pos & velocity for 3 bouncy balls
      { 17, 15,  1, -1 },
      { 27,  4, -1,  1 }
    };

static const UINT16 ballcolor[3] = {
      0x0080, // Green=1
      0x0002, // Blue=1
      0x1000  // Red=1
    };

void scroll_test_loop() {
    int    textX   = _matrix_width,
           textMin = sizeof(str) * -12,
           hue     = 0;
    
    
    matrix_setTextWrap(false); // Allow text to run off right edge
    matrix_setTextSize(2);

    while(true) {
      char i;

      // Clear background
      matrix_fillScreen(0);

      // Bounce three balls around
      for(i=0; i<3; i++) {
        // Draw 'ball'
        matrix_fillCircle(ball[i][0], ball[i][1], 5, pgm_read_word(&ballcolor[i]));
        // Update X, Y position
        ball[i][0] += ball[i][2];
        ball[i][1] += ball[i][3];
        // Bounce off edges
        if((ball[i][0] == 0) || (ball[i][0] == (_matrix_width - 1)))
          ball[i][2] *= -1;
        if((ball[i][1] == 0) || (ball[i][1] == (_matrix_height - 1)))
          ball[i][3] *= -1;
      }

      // Draw big scrolly text on top
      matrix_setTextColor(matrix_colorHSV(hue, 255, 255, true));
      matrix_setCursor(textX, 1);
      matrix_writeString(str);

      // Move text left (w/wrap), increase hue
      if((--textX) < textMin) textX = _matrix_width;
      hue += 7;
      if(hue >= 1536) hue -= 1536;

      // Update display
      matrix_swapBuffers(false);
    }
}