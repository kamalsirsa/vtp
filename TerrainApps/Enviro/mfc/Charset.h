//
// Charset.h : help with CString and character sets
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtdata/vtString.h"

CString FromUTF8(const char *input);
vtString ToUTF8(const CString &cstr);
