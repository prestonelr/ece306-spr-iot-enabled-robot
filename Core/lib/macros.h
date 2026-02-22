//===========================================================================
// File Name : macros.h
//
// Description: This file contains the macros used by all the other files
//
// Author: Preston Elrod
// Date: Feb 2026
// Compiler: Built with Code Composer Version: CCS12.4.0.00007_win64
//===========================================================================

#define USE_GPIO (0x00)
#define USE_SMCLK (0x01)
#define ALWAYS                  (1)
#define RESET_STATE             (0)
#define RED_LED              (0x01) // RED LED 0
#define GRN_LED              (0x40) // GREEN LED 1
#define TEST_PROBE           (0x01) // 0 TEST PROBE
#define TRUE                 (0x01) //

int SMCLK_CONFIG; // uses gpio by default


#define P4PUD (P4OUT) // interupts
#define RESET_REGISTER (0x00)
