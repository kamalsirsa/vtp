//
// Name: VegFieldsDlg.h
//

#ifndef __VegPointOptions_H__
#define __VegPointOptions_H__

#include "vtdata/vtString.h"

/** This structure stores a description of how an imported file should be
   intepreted as vegetation point data. */
class VegPointOptions
{
public:
	bool bFixedSpecies;
	vtString strFixedSpeciesName;

	int iSpeciesFieldIndex;

	// 0 - species ID (int)
	// 1 - species name (string)
	// 2 - common name (string)
	// 3 - biotype id (int)
	// 4 - biotype name (string)
	int iInterpretSpeciesField;

	bool bHeightRandom;
	int iHeightFieldIndex;
};

#endif // __VegPointOptions_H__
