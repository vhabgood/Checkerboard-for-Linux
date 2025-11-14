#ifndef SWITCHES_H
#define SWITCHES_H

// Placeholder for switches.h
// Actual content may need to be determined from original Kingsrow source or documentation.

#include <stdint.h>

typedef uint32_t int32;

// Define POSITION struct to match our pos struct globally for C compatibility
typedef struct POSITION {
    unsigned int bm; // Black men
    unsigned int bk; // Black kings
    unsigned int wm; // White men
    unsigned int wk; // White kings
    int color;
} POSITION;

#endif // SWITCHES_H