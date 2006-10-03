
// In Visual C++ 2005 and later, time_t is a 64-bit integer by default.
// That prevents binary compatibility between VC7.x and VC8.0.
// MSDN says: "defining _USE_32BIT_TIME_T changes time_t to __time32_t and
// forces many time functions to call versions that take the 32-bit time_t."
#define _USE_32BIT_TIME_T

#include "wx/wxprec.h"
