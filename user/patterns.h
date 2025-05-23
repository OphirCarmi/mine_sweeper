#ifndef PATTERNS_H_
#define PATTERNS_H_

#include <stdio.h>
#include <stdlib.h>

// 'X' is out of board or number
// 'Y' is out of board or number or space
// 'S' is where we should flag
// 'E' is where we should reveal
// 'R' can be anything
// 'I' is last flag

static char patterns[][50] = {
    "RRRRRRR"
    "RRRRRRR"
    "RRXXXRR"
    "RR121RR"
    "RRS  RR"
    "RRRRRRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRXXXRR"
    "RRS2XRR"
    "RR 3fRR"
    "RR 2XRR"
    "RRXXXRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRXXXRR"
    "RRS2fRR"
    "RR 3XRR"
    "RR 1XRR"
    "RRXXXRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRXXXRR"
    "RRS3fRR"
    "RR 4fRR"
    "RR 2XRR"
    "RRXXXRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRXXXRR"
    "RRS3fRR"
    "RR 5fRR"
    "RR 3fRR"
    "RRXXXRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRRRRRR"
    "RRYS1XR"
    "RRY 2XR"
    "RR1 2XR"
    "RRRRRRR"
    "RRRRRRR",

    "RRRRRRR"
    "RXXXXXR"
    "RX  2fR"
    "RX I3XR"
    "RX23fXR"
    "RXXfXRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRXXXRR"
    "RRX1 RR"
    "RRX2 RR"
    "RRXfERR"
    "RRRRRRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRRRRRR"
    "RRXfSRR"
    "RRX3 RR"
    "RRX1 RR"
    "RRXXXRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRRRRRR"
    "RRXfSRR"
    "RRX3 RR"
    "RRX1 RR"
    "RRXXXRR"
    "RRRRRRR",

    "RRXXXRR"
    "RRX1 RR"
    "RRX2 RR"
    "RRX2SRR"
    "RRX2 RR"
    "RRXfXRR"
    "RRRRRRR",

    "RRRRRRR"
    "XXXXXXR"
    "X1122fX"
    "X   S3X"
    "RRRR fX"
    "RRRfRRR"
    "RRRRRRR",

    "RRRRRRR"
    "XXXfXXR"
    "f2223fR"
    "X   SXR"
    "RRRRRRR"
    "RRRRRRR"
    "RRRRRRR",

    "RRRRXXX"
    "RRRR 2X"
    "RRE  3f"
    "RRf22fX"
    "RRXXXXR"
    "RRRRRRR"
    "RRRRRRR",

    "RRRRRRR"
    "RXfXRRR"
    "RX2 RRR"
    "RX1 RRR"
    "RXXERRR"
    "RRRRRRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRXX RR"
    "RRX2 RR"
    "RRX2 RR"
    "RRXfERR"
    "RRRRRRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRRRRRR"
    "RRRRRRR"
    "RRRX2ER"
    "RRRX1 R"
    "RRRX1 R"
    "RRRXXXR",

    "RRRRRRR"
    "RRRRRRR"
    "RRSXfRR"
    "RR 4XRR"
    "RR 2fRR"
    "RRRRRRR"
    "RRRRRRR",

    // "RRRRRRR"
    // "RRRRRRR"
    // "R E  XR"
    // "X1111XR"
    // "XXXXXXR"
    // "RRRRRRR"
    // "RRRRRRR",

    "RRRRRRR"
    "RRRRRRR"
    "XXXXXRR"
    "X111XRR"
    "X  E RR"
    "RRRRRRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRfXfRR"
    "RR 4XRR"
    "RR 2fRR"
    "RRE2XRR"
    "RRYXXRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRRRRRR"
    "RXfSRRR"
    "RX3 RRR"
    "RX1 RRR"
    "RXXYRRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRRRRRR"
    "RXffRRR"
    "RX3 RRR"
    "RX1 RRR"
    "RXXERRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRRRRRR"
    "RffXRRR"
    "RX3 RRR"
    "RX1 RRR"
    "RXXERRR"
    "RRRRRRR",

    // "RRRRRRR"
    // "RRRRRRR"
    // "RREEERR"
    // "RRE1ERR"
    // "RREEERR"
    // "RRRRRRR"
    // "RRRRRRR",

    // "RRRRRRR"
    // "RRRRRRR"
    // "RREEERR"
    // "RR 1 RR"
    // "RR 1 RR"
    // "RRRRRRR"
    // "RRRRRRR",

    // "RRRRRRR"
    // "RRRRRRR"
    // "RREEERR"
    // "RRE1 RR"
    // "RRE 1RR"
    // "RRRRRRR"
    // "RRRRRRR",

    "RRRRRRR"
    "RRS  XR"
    "RRf41XR"
    "RRfXXXR"
    "RRRRRRR"
    "RRRRRRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRRXX R"
    "RRYf3 R"
    "RRY2S Y"
    "RRY2 1Y"
    "RRfXYYY"
    "RRRRRRR",

    "RRRRRRR"
    "RRRRRRR"
    "RYE  XR"
    "RY111XR"
    "RYXXXXR"
    "RRRRRRR"
    "RRRRRRR",

    "RRRRRRR"
    "RRXXXXR"
    "RR321XR"
    "RRS  XR"
    "RRRRRRR"
    "RRRRRRR"
    "RRRRRRR",
};
static size_t patterns_len = sizeof(patterns) / sizeof(patterns[0]);

static int8_t rotations[][sizeof(patterns[0]) - 1] = {
    // normal
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
     17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
     34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48},
    // rot 90
    {6, 13, 20, 27, 34, 41, 48, 5, 12, 19, 26, 33, 40, 47, 4, 11, 18,
     25, 32, 39, 46, 3, 10, 17, 24, 31, 38, 45, 2, 9, 16, 23, 30, 37,
     44, 1, 8, 15, 22, 29, 36, 43, 0, 7, 14, 21, 28, 35, 42},
    // rot 180
    {48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
     31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15,
     14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
    // rot 270
    {42, 35, 28, 21, 14, 7, 0, 43, 36, 29, 22, 15, 8, 1, 44, 37, 30,
     23, 16, 9, 2, 45, 38, 31, 24, 17, 10, 3, 46, 39, 32, 25, 18, 11,
     4, 47, 40, 33, 26, 19, 12, 5, 48, 41, 34, 27, 20, 13, 6},
    // flip horiz
    {42, 43, 44, 45, 46, 47, 48, 35, 36, 37, 38, 39, 40, 41, 28, 29, 30,
     31, 32, 33, 34, 21, 22, 23, 24, 25, 26, 27, 14, 15, 16, 17, 18, 19,
     20, 7, 8, 9, 10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6},
    // flip horiz rot 90
    {48, 41, 34, 27, 20, 13, 6, 47, 40, 33, 26, 19, 12, 5, 46, 39, 32,
     25, 18, 11, 4, 45, 38, 31, 24, 17, 10, 3, 44, 37, 30, 23, 16, 9,
     2, 43, 36, 29, 22, 15, 8, 1, 42, 35, 28, 21, 14, 7, 0},
    // flip horiz rot 180
    {6, 5, 4, 3, 2, 1, 0, 13, 12, 11, 10, 9, 8, 7, 20, 19, 18,
     17, 16, 15, 14, 27, 26, 25, 24, 23, 22, 21, 34, 33, 32, 31, 30, 29,
     28, 41, 40, 39, 38, 37, 36, 35, 48, 47, 46, 45, 44, 43, 42},
    // flip horiz rot 270
    {0, 7, 14, 21, 28, 35, 42, 1, 8, 15, 22, 29, 36, 43, 2, 9, 16,
     23, 30, 37, 44, 3, 10, 17, 24, 31, 38, 45, 4, 11, 18, 25, 32, 39,
     46, 5, 12, 19, 26, 33, 40, 47, 6, 13, 20, 27, 34, 41, 48},
};

static size_t rotations_len = sizeof(rotations) / sizeof(rotations[0]);

#endif  // PATTERNS_H_
