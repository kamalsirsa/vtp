#ifndef _VTOSGDYNMESHFIELDS_H_
	#define _VTOSGDYNMESHFIELDS_H_
	#ifdef __sgi
		#pragma once
	#endif

	#include <OpenSG/OSGFieldContainerPtr.h>
	#include <OpenSG/OSGNodeCoreFieldDataType.h>

	#include "vtOsgDynMeshFields.h"

	#include <OpenSG/OSGMaterialDrawableFields.h>

OSG_BEGIN_NAMESPACE

class vtOsgDynMesh;

	#if !defined(OSG_DO_DOC)   // created as a dummy class, remove to prevent doubles
//! vtOsgDynMeshPtr

typedef FCPtr<MaterialDrawablePtr, vtOsgDynMesh> vtOsgDynMeshPtr;

	#endif

	#if !defined(OSG_DO_DOC) || (OSG_DOC_LEVEL >= 3)
/*! \ingroup GrpMyFieldTraits
 */
		#if !defined(OSG_DOC_DEV_TRAITS)
/*! \hideinhierarchy */
		#endif

template <>
struct FieldDataTraits<vtOsgDynMeshPtr> : 
public FieldTraitsRecurseMapper<vtOsgDynMeshPtr, true> {
	static DataType             _type;                       

	enum {
		StringConvertable = 0x00
	};
	enum {
		bHasParent        = 0x01
	};

	static DataType &getType (void) { return _type;}

	static char     *getSName(void) { return "SFvtOsgDynMeshPtr";}
	static char     *getMName(void) { return "MFvtOsgDynMeshPtr";}
};

		#if !defined(OSG_DOC_DEV_TRAITS)
/*! \class  FieldTraitsRecurseMapper<vtOsgDynMeshPtr, true>
	\hideinhierarchy
 */
		#endif

	#endif // !defined(OSG_DO_DOC) || (OSG_DOC_LEVEL >= 3)


	#if !defined(OSG_DO_DOC) || defined(OSG_DOC_FIELD_TYPEDEFS)
/*! \ingroup GrpMyFieldSingle */

typedef SField<vtOsgDynMeshPtr> SFvtOsgDynMeshPtr;
	#endif

	#ifndef OSG_COMPILEDYNMESHINST
OSG_DLLEXPORT_DECL1(SField, vtOsgDynMeshPtr, OSG_MYLIB_DLLTMPLMAPPING)
	#endif

	#if !defined(OSG_DO_DOC) || defined(OSG_DOC_FIELD_TYPEDEFS)
/*! \ingroup GrpMyFieldMulti */

typedef MField<vtOsgDynMeshPtr> MFvtOsgDynMeshPtr;
	#endif

	#ifndef OSG_COMPILEDYNMESHINST
OSG_DLLEXPORT_DECL1(MField, vtOsgDynMeshPtr, OSG_MYLIB_DLLTMPLMAPPING)
	#endif

OSG_END_NAMESPACE

	#define OSGDYNMESHFIELDS_HEADER_CVSID "@(#)$Id$"

#endif /* _OSGCUBESFIELDS_H_ */
