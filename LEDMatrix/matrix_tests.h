/* 
 * File:   matrix_tests.h
 * Author: smiller
 *
 * Created on November 19, 2015, 1:34 PM
 */

#ifndef MATRIX_TESTS_H
#define	MATRIX_TESTS_H

/* Fill screen with a color wheel */
void draw_colorwheel();

/* Fill screen with various brightness levels of each color */
void draw_levels();

// Include the plasma test if we elect to use it
#ifdef ENABLE_MATRIX_PLASMA
#include "matrix_plasma.h"
#endif

#endif	/* MATRIX_TESTS_H */

