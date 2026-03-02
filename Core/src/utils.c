//===========================================================================
// File Name : utils.c
//
// Description: Generic helper functions (simple / old compiler friendly)
//
// Author: Preston Elrod
// Date: Feb 2026
//===========================================================================

#include "Core\lib\utils.h"

#define TOSTR_WIDTH  (10)

// internal static buffer (10 chars + null)
static char g_toStringBuf[TOSTR_WIDTH + 1];

// Make buffer = 10 chars wide, right-aligned decimal number, padded with spaces
// Returns pointer to internal static buffer (overwritten every call)
char* ToString(unsigned v)
{
  int i;
  unsigned temp;
  int digits;

  // fill with spaces
  for(i = 0; i < TOSTR_WIDTH; i++) g_toStringBuf[i] = ' ';
  g_toStringBuf[TOSTR_WIDTH] = 0;

  // handle zero
  if(v == 0u)
  {
    g_toStringBuf[TOSTR_WIDTH - 1] = '0';
    return g_toStringBuf;
  }

  // count digits to detect overflow vs width
  temp = v;
  digits = 0;
  while(temp > 0u)
  {
    temp /= 10u;
    digits++;
  }

  // if too many digits, show **********
  if(digits > TOSTR_WIDTH)
  {
    for(i = 0; i < TOSTR_WIDTH; i++) g_toStringBuf[i] = '*';
    g_toStringBuf[TOSTR_WIDTH] = 0;
    return g_toStringBuf;
  }

  // write digits from right to left
  i = TOSTR_WIDTH - 1;
  while((v > 0u) && (i >= 0))
  {
    g_toStringBuf[i] = (char)('0' + (v % 10u));
    v /= 10u;
    i--;
  }

  return g_toStringBuf;
}
