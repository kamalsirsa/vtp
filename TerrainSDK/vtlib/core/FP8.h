/* homegrown floating point: 3 bits exponent, 8 bit mantissa
   maps
    0.. 31  ->     0..  31
   32.. 63  ->    32..  63
   64.. 95  ->    64.. 126, step 2
   96..127  ->   128.. 252, step 4
  128..159  ->   256.. 504, step 8
  160..191  ->   512..1008, step 16
  192..223  ->  1024..2016, step 32
  224..255  ->  2048..4032, step 64
*/

#ifdef _MSC_VER
#  define INLINE __forceinline
#else
#  define INLINE inline
#endif

INLINE unsigned char EncodeFP8( unsigned short nErr )
{
	if( nErr < 64   ) return          nErr;
	if( nErr < 128  ) return (0x40 | (nErr-  64)/2);
	if( nErr < 256  ) return (0x60 | (nErr- 128)/4);
	if( nErr < 512  ) return (0x80 | (nErr- 256)/8);
	if( nErr < 1024 ) return (0xa0 | (nErr- 512)/16);
	if( nErr < 2048 ) return (0xc0 | (nErr-1024)/32);
	if( nErr < 4096 ) return (0xe0 | (nErr-2048)/64);
	                  return  0xff;
}

INLINE unsigned short DecodeFP8( unsigned char nErr )
{
	int a = nErr >> 5;
	int b = nErr & 31;

	if( a < 2 ) {
		return b;
	} else {
		return (b + 32) << (a-1);
	}
}