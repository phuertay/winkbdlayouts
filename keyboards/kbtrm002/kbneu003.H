/****************************** Module Header ******************************\
* Module Name: kbneu003.H
*
* keyboard layout header
*
* Copyright (c) 1985-2001, Microsoft Corporation
*
* Various defines for use by keyboard input code.
*
* History:
*
* created by KBDTOOL v3.40 Sun Oct 03 16:03:44 2021
*
\***************************************************************************/

/*
 * kbd type should be controlled by cl command-line argument
 */
#define KBD_TYPE 4

/*
* Include the basis of all keyboard table values
*/
#include "kbd.h"
/***************************************************************************\
* The table below defines the virtual keys for various keyboard types where
* the keyboard differ from the US keyboard.
*
* _EQ() : all keyboard types have the same virtual key for this scancode
* _NE() : different virtual keys for this scancode, depending on kbd type
*
*     +------+ +----------+----------+----------+----------+----------+----------+
*     | Scan | |    kbd   |    kbd   |    kbd   |    kbd   |    kbd   |    kbd   |
*     | code | |   type 1 |   type 2 |   type 3 |   type 4 |   type 5 |   type 6 |
\****+-------+_+----------+----------+----------+----------+----------+----------+*/

#undef  T02
 #define T02 _EQ(                                        '9'                      )
#undef  T03
 #define T03 _EQ(                                        '3'                      )
#undef  T04
 #define T04 _EQ(                                        '1'                      )
#undef  T05
 #define T05 _EQ(                                        '5'                      )
#undef  T06
 #define T06 _EQ(                                        '7'                      )
#undef  T08
 #define T08 _EQ(                                        '2'                      )
#undef  T09
 #define T09 _EQ(                                        '0'                      )
#undef  T0A
 #define T0A _EQ(                                        '4'                      )
#undef  T0B
 #define T0B _EQ(                                        '8'                      )
#undef  T0C
 #define T0C _EQ(                                      OEM_4                      )
#undef  T0D
 #define T0D _EQ(                                      OEM_6                      )
#undef  T10
 #define T10 _EQ(                                        'J'                      )
#undef  T11
 #define T11 _EQ(                                        'G'                      )
#undef  T12
 #define T12 _EQ(                                        'M'                      )
#undef  T13
 #define T13 _EQ(                                        'F'                      )
#undef  T14
 #define T14 _EQ(                                        'W'                      )
#undef  T15
 #define T15 _EQ(                                        'K'                      )
#undef  T16
 #define T16 _EQ(                                     RETURN                      )
#undef  T17
 #define T17 _EQ(                                      OEM_2                      )
#undef  T18
 #define T18 _EQ(                                        'Q'                      )
#undef  T19
 #define T19 _EQ(                                        'Z'                      )
#undef  T1A
 #define T1A _EQ(                                   OEM_PLUS                      )
#undef  T1B
 #define T1B _EQ(                                  OEM_MINUS                      )
#undef  T1E
 #define T1E _EQ(                                        'R'                      )
#undef  T20
 #define T20 _EQ(                                        'N'                      )
#undef  T21
 #define T21 _EQ(                                        'T'                      )
#undef  T22
 #define T22 _EQ(                                        'P'                      )
#undef  T23
 #define T23 _EQ(                                 OEM_PERIOD                      )
#undef  T24
 #define T24 _EQ(                                        'A'                      )
#undef  T25
 #define T25 _EQ(                                        'E'                      )
#undef  T26
 #define T26 _EQ(                                        'I'                      )
#undef  T27
 #define T27 _EQ(                                        'H'                      )
#undef  T28
 #define T28 _EQ(                                      OEM_3                      )
#undef  T29
 #define T29 _EQ(                                      OEM_7                      )
#undef  T2C
 #define T2C _EQ(                                        'X'                      )
#undef  T2D
 #define T2D _EQ(                                        'C'                      )
#undef  T2E
 #define T2E _EQ(                                        'L'                      )
#undef  T2F
 #define T2F _EQ(                                        'D'                      )
#undef  T31
 #define T31 _EQ(                                      OEM_8                      )
#undef  T32
 #define T32 _EQ(                                        'U'                      )
#undef  T33
 #define T33 _EQ(                                        'O'                      )
#undef  T34
 #define T34 _EQ(                                        'Y'                      )
#undef  T35
 #define T35 _EQ(                                        'V'                      )

