#define OSG_COMPILEDYNMESHINST

#include "vtlib/vtlib.h"

#include <stdlib.h>

#include "vtOsgDynMeshBase.h"
#include "vtOsgDynMesh.h"

OSG_USING_NAMESPACE

const OSG::BitVector  vtOsgDynMeshBase::PositionFieldMask = 
(TypeTraits<BitVector>::One << vtOsgDynMeshBase::PositionFieldId);

const OSG::BitVector  vtOsgDynMeshBase::LengthFieldMask = 
(TypeTraits<BitVector>::One << vtOsgDynMeshBase::LengthFieldId);

const OSG::BitVector  vtOsgDynMeshBase::ColorFieldMask = 
(TypeTraits<BitVector>::One << vtOsgDynMeshBase::ColorFieldId);

const OSG::BitVector vtOsgDynMeshBase::MTInfluenceMask = 
(Inherited::MTInfluenceMask) | 
(static_cast<BitVector>(0x0) << Inherited::NextFieldId); 


// Field descriptions

/*! \var Pnt3f           vtOsgDynMeshBase::_mfPosition
	The cubes' positions.
*/
/*! \var Real32          vtOsgDynMeshBase::_mfLength
	The cubes' sizes.
*/
/*! \var Color3f         vtOsgDynMeshBase::_mfColor
	The cubes' colors.
*/

//! vtOsgDynMesh description

FieldDescription *vtOsgDynMeshBase::_desc[] = 
{
	new FieldDescription(MFPnt3f::getClassType(), 
						 "position", 
						 PositionFieldId, PositionFieldMask,
						 false,
						 (FieldAccessMethod) &vtOsgDynMeshBase::getMFPosition),
	new FieldDescription(MFReal32::getClassType(), 
						 "length", 
						 LengthFieldId, LengthFieldMask,
						 false,
						 (FieldAccessMethod) &vtOsgDynMeshBase::getMFLength),
	new FieldDescription(MFColor3f::getClassType(), 
						 "color", 
						 ColorFieldId, ColorFieldMask,
						 false,
						 (FieldAccessMethod) &vtOsgDynMeshBase::getMFColor)
};


FieldContainerType vtOsgDynMeshBase::_type(
										  "vtOsgDynMesh",
										  "MaterialDrawable",
										  NULL,
										  (PrototypeCreateF) &vtOsgDynMeshBase::createEmpty,
										  vtOsgDynMesh::initMethod,
										  _desc,
										  sizeof(_desc));

//OSG_FIELD_CONTAINER_DEF(vtOsgDynMeshBase, vtOsgDynMeshPtr)

/*------------------------------ get -----------------------------------*/

FieldContainerType &vtOsgDynMeshBase::getType(void) 
{
	return _type; 
} 

const FieldContainerType &vtOsgDynMeshBase::getType(void) const 
{
	return _type;
} 


FieldContainerPtr vtOsgDynMeshBase::shallowCopy(void) const 
{ 
	vtOsgDynMeshPtr returnValue; 

	newPtr(returnValue, dynamic_cast<const vtOsgDynMesh *>(this)); 

	return returnValue; 
}

UInt32 vtOsgDynMeshBase::getContainerSize(void) const 
{ 
	return sizeof(vtOsgDynMesh); 
}


#if !defined(OSG_FIXED_MFIELDSYNC)
void vtOsgDynMeshBase::executeSync(      FieldContainer &other,
										 const BitVector      &whichField)
{
	this->executeSyncImpl((vtOsgDynMeshBase *) &other, whichField);
}
#else
void vtOsgDynMeshBase::executeSync(      FieldContainer &other,
										 const BitVector      &whichField,                                    const SyncInfo       &sInfo     )
{
	this->executeSyncImpl((vtOsgDynMeshBase *) &other, whichField, sInfo);
}
void vtOsgDynMeshBase::execBeginEdit(const BitVector &whichField, 
									 UInt32     uiAspect,
									 UInt32     uiContainerSize) 
{
	this->execBeginEditImpl(whichField, uiAspect, uiContainerSize);
}

void vtOsgDynMeshBase::onDestroyAspect(UInt32 uiId, UInt32 uiAspect)
{
	Inherited::onDestroyAspect(uiId, uiAspect);

	_mfPosition.terminateShare(uiAspect, this->getContainerSize());
	_mfLength.terminateShare(uiAspect, this->getContainerSize());
	_mfColor.terminateShare(uiAspect, this->getContainerSize());
}
#endif

/*------------------------- constructors ----------------------------------*/

#ifdef OSG_WIN32_ICL
	#pragma warning (disable : 383)
#endif

vtOsgDynMeshBase::vtOsgDynMeshBase(void) :
_mfPosition               (), 
_mfLength                 (), 
_mfColor                  (), 
Inherited() 
{
}

#ifdef OSG_WIN32_ICL
	#pragma warning (default : 383)
#endif

vtOsgDynMeshBase::vtOsgDynMeshBase(const vtOsgDynMeshBase &source) :
_mfPosition               (source._mfPosition               ), 
_mfLength                 (source._mfLength                 ), 
_mfColor                  (source._mfColor                  ), 
Inherited                 (source)
{
}

/*-------------------------- destructors ----------------------------------*/

vtOsgDynMeshBase::~vtOsgDynMeshBase(void)
{
}

/*------------------------------ access -----------------------------------*/

UInt32 vtOsgDynMeshBase::getBinSize(const BitVector &whichField)
{
	UInt32 returnValue = Inherited::getBinSize(whichField);

	if( FieldBits::NoField != (PositionFieldMask & whichField) ) {
		returnValue += _mfPosition.getBinSize();
	}

	if( FieldBits::NoField != (LengthFieldMask & whichField) ) {
		returnValue += _mfLength.getBinSize();
	}

	if( FieldBits::NoField != (ColorFieldMask & whichField) ) {
		returnValue += _mfColor.getBinSize();
	}


	return returnValue;
}

void vtOsgDynMeshBase::copyToBin(      BinaryDataHandler &pMem,
									   const BitVector         &whichField)
{
	Inherited::copyToBin(pMem, whichField);

	if( FieldBits::NoField != (PositionFieldMask & whichField) ) {
		_mfPosition.copyToBin(pMem);
	}

	if( FieldBits::NoField != (LengthFieldMask & whichField) ) {
		_mfLength.copyToBin(pMem);
	}

	if( FieldBits::NoField != (ColorFieldMask & whichField) ) {
		_mfColor.copyToBin(pMem);
	}


}

void vtOsgDynMeshBase::copyFromBin(      BinaryDataHandler &pMem,
										 const BitVector    &whichField)
{
	Inherited::copyFromBin(pMem, whichField);

	if( FieldBits::NoField != (PositionFieldMask & whichField) ) {
		_mfPosition.copyFromBin(pMem);
	}

	if( FieldBits::NoField != (LengthFieldMask & whichField) ) {
		_mfLength.copyFromBin(pMem);
	}

	if( FieldBits::NoField != (ColorFieldMask & whichField) ) {
		_mfColor.copyFromBin(pMem);
	}


}

#if !defined(OSG_FIXED_MFIELDSYNC)
void vtOsgDynMeshBase::executeSyncImpl(      vtOsgDynMeshBase *pOther,
											 const BitVector         &whichField)
{

	Inherited::executeSyncImpl(pOther, whichField);

	if( FieldBits::NoField != (PositionFieldMask & whichField) )
		_mfPosition.syncWith(pOther->_mfPosition);

	if( FieldBits::NoField != (LengthFieldMask & whichField) )
		_mfLength.syncWith(pOther->_mfLength);

	if( FieldBits::NoField != (ColorFieldMask & whichField) )
		_mfColor.syncWith(pOther->_mfColor);


}
#else
void vtOsgDynMeshBase::executeSyncImpl(      vtOsgDynMeshBase *pOther,
											 const BitVector         &whichField,
											 const SyncInfo          &sInfo      )
{

	Inherited::executeSyncImpl(pOther, whichField, sInfo);


	if( FieldBits::NoField != (PositionFieldMask & whichField) )
		_mfPosition.syncWith(pOther->_mfPosition, sInfo);

	if( FieldBits::NoField != (LengthFieldMask & whichField) )
		_mfLength.syncWith(pOther->_mfLength, sInfo);

	if( FieldBits::NoField != (ColorFieldMask & whichField) )
		_mfColor.syncWith(pOther->_mfColor, sInfo);


}

void vtOsgDynMeshBase::execBeginEditImpl (const BitVector &whichField, 
										  UInt32     uiAspect,
										  UInt32     uiContainerSize)
{
	Inherited::execBeginEditImpl(whichField, uiAspect, uiContainerSize);

	if( FieldBits::NoField != (PositionFieldMask & whichField) )
		_mfPosition.beginEdit(uiAspect, uiContainerSize);

	if( FieldBits::NoField != (LengthFieldMask & whichField) )
		_mfLength.beginEdit(uiAspect, uiContainerSize);

	if( FieldBits::NoField != (ColorFieldMask & whichField) )
		_mfColor.beginEdit(uiAspect, uiContainerSize);

}
#endif



#include <OpenSG/OSGSFieldTypeDef.inl>
#include <OpenSG/OSGMFieldTypeDef.inl>

OSG_BEGIN_NAMESPACE

#if !defined(OSG_DO_DOC) || defined(OSG_DOC_DEV)
DataType FieldDataTraits<vtOsgDynMeshPtr>::_type("vtOsgDynMeshPtr", "MaterialDrawablePtr");
#endif

OSG_DLLEXPORT_SFIELD_DEF1(vtOsgDynMeshPtr, OSG_MYLIB_DLLTMPLMAPPING);
OSG_DLLEXPORT_MFIELD_DEF1(vtOsgDynMeshPtr, OSG_MYLIB_DLLTMPLMAPPING);

OSG_END_NAMESPACE


/*------------------------------------------------------------------------*/
/*                              cvs id's                                  */

#ifdef OSG_SGI_CC
	#pragma set woff 1174
#endif

#ifdef OSG_LINUX_ICC
	#pragma warning( disable : 177 )
#endif

namespace
{
	static Char8 cvsid_cpp       [] = "@(#)$Id$";
	static Char8 cvsid_hpp       [] = OSGCUBESBASE_HEADER_CVSID;
	static Char8 cvsid_inl       [] = OSGCUBESBASE_INLINE_CVSID;

	static Char8 cvsid_fields_hpp[] = OSGDYNMESHFIELDS_HEADER_CVSID;
}

