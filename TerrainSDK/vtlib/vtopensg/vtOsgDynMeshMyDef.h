#ifndef _VTOSGDYNMESHMYDEF_H_
	#define _VTOSGDYNMESHMYDEF_H_
	#ifdef __sgi
		#pragma once
	#endif

//---------------------------------------------------------------------------
//  Defines
//---------------------------------------------------------------------------

	#if defined(WIN32) && defined(OSG_BUILD_DLL)
		#ifdef OSG_COMPILEMYLIB
			#define OSG_MYLIB_DLLMAPPING     __declspec(dllexport)
			#define OSG_MYLIB_DLLTMPLMAPPING __declspec(dllexport)
		#else
			#define OSG_MYLIB_DLLMAPPING     __declspec(dllimport)
			#define OSG_MYLIB_DLLTMPLMAPPING __declspec(dllimport)
		#endif
	#else
		#define OSG_MYLIB_DLLMAPPING
		#define OSG_MYLIB_DLLTMPLMAPPING
	#endif

#endif /* _OSGMYDEF_H_ */
