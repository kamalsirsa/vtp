
#ifdef RELEASE

double  __cdecl fabs(double);
float fabsf(float _X) {return ((float)fabs((double)_X)); }

#endif

