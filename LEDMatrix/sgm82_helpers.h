/* 
 * File: sgm82_helpers.h
 * Author: Sam Miller (sgm82)
 *   Date: 10/2/15
 */
  
#ifndef SGM82_HELPERS_H
#define	SGM82_HELPERS_H

// Macros for enabling/disabling pullup/pulldown
// PORT B
#define EnablePullDownB(bits) CNPUBCLR=bits; CNPDBSET=bits;
#define DisablePullDownB(bits) CNPDBCLR=bits;
#define EnablePullUpB(bits) CNPDBCLR=bits; CNPUBSET=bits;
#define DisablePullUpB(bits) CNPUBCLR=bits;
//PORT A
#define EnablePullDownA(bits) CNPUACLR=bits; CNPDASET=bits;
#define DisablePullDownA(bits) CNPDACLR=bits;
#define EnablePullUpA(bits) CNPDACLR=bits; CNPUASET=bits;
#define DisablePullUpA(bits) CNPUACLR=bits;

unsigned int color_from_rgb(int r, int g, int b);
void print_lab_header(int lab_num);

#endif	/* SGM82_HELPERS_H */

