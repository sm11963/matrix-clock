#include "sgm82_helpers.h"

#include "tft_master.h"
#include "tft_gfx.h"

unsigned int color_from_rgb(int r, int g, int b) {
    return ((r & 0x1f) << 11) | ((g & 0x3f) << 5) | (b & 0x1f);
} 

void print_lab_header(int lab_num) {
    tft_setCursor(28, 5);
    tft_setTextSize(2);
    tft_setTextColor(color_from_rgb(31,  0,  0));
    tft_writeString("E");
    tft_setTextColor(color_from_rgb(31, 32,  0));
    tft_writeString("C");
    tft_setTextColor(color_from_rgb(31, 63,  0));
    tft_writeString("E ");
    tft_setTextColor(color_from_rgb(16, 63,  0));
    tft_writeString("4");
    tft_setTextColor(color_from_rgb( 0, 63,  0));
    tft_writeString("7");
    tft_setTextColor(color_from_rgb( 0, 63,  16));
    tft_writeString("6");
    tft_setTextColor(color_from_rgb( 0, 63,  31));
    tft_writeString("0 ");
    tft_setTextColor(color_from_rgb( 0, 32,  31));
    tft_writeString("L");
    tft_setTextColor(color_from_rgb( 0,  0,  31));
    tft_writeString("a");
    tft_setTextColor(color_from_rgb(16,  0,  31));
    tft_writeString("b ");
    tft_setTextColor(color_from_rgb(31,  0,  31));
    char buffer[4];
    sprintf(buffer, "%d", lab_num);
    tft_writeString(buffer);
}