#ifndef _VTOSGDYNMESHBASE_H_
	#define _VTOSGDYNMESHBASE_H_
	#ifdef __sgi
		#pragma once
	#endif

	#include "vtOsgDynMeshMyDef.h"

	#include <OpenSG/OSGBaseTypes.h>
	#include <OpenSG/OSGRefPtr.h>
	#include <OpenSG/OSGCoredNodePtr.h>

	#include <OpenSG/OSGMaterialDrawable.h> // Parent

	#include <OpenSG/OSGPnt3fFields.h> // Position type
	#include <OpenSG/OSGReal32Fields.h> // Length type
	#include <OpenSG/OSGColor3fFields.h> // Color type

	#include "vtOsgDynMeshFields.h"

OSG_BEGIN_NAMESPACE

class vtOsgDynMesh;
class BinaryDataHandler;

//! \brief vtOsgDynMesh Base Class.

class /*OSG_MYLIB_DLLMAPPING*/ vtOsgDynMeshBase : public MaterialDrawable {
private:

	typedef MaterialDrawable    Inherited;

	/*==========================  PUBLIC  =================================*/
public:

	typedef vtOsgDynMeshPtr  Ptr;

	enum {
		PositionFieldId = Inherited::NextFieldId,
		LengthFieldId   = PositionFieldId + 1,
		ColorFieldId    = LengthFieldId   + 1,
		NextFieldId     = ColorFieldId    + 1
	};

	static const OSG::BitVector PositionFieldMask;
	static const OSG::BitVector LengthFieldMask;
	static const OSG::BitVector ColorFieldMask;


	static const OSG::BitVector MTInfluenceMask;

	/*---------------------------------------------------------------------*/
	/*! \name                    Class Get                                 */
	/*! \{                                                                 */

	static        FieldContainerType &getClassType    (void); 
	static        UInt32              getClassTypeId  (void); 

	/*! \}                                                                 */
	/*---------------------------------------------------------------------*/
	/*! \name                FieldContainer Get                            */
	/*! \{                                                                 */

	virtual       FieldContainerType &getType  (void); 
	virtual const FieldContainerType &getType  (void) const; 

	virtual       UInt32              getContainerSize(void) const;

	/*! \}                                                                 */
	/*---------------------------------------------------------------------*/
	/*! \name                    Field Get                                 */
	/*! \{                                                                 */

	MFPnt3f             *getMFPosition       (void);
	MFReal32            *getMFLength         (void);
	MFColor3f           *getMFColor          (void);

	Pnt3f               &getPosition       (const UInt32 index);
	MFPnt3f             &getPosition       (void);
	const MFPnt3f             &getPosition       (void) const;
	Real32              &getLength         (const UInt32 index);
	MFReal32            &getLength         (void);
	const MFReal32            &getLength         (void) const;
	Color3f             &getColor          (const UInt32 index);
	MFColor3f           &getColor          (void);
	const MFColor3f           &getColor          (void) const;

	/*! \}                                                                 */
	/*---------------------------------------------------------------------*/
	/*! \name                    Field Set                                 */
	/*! \{                                                                 */


	/*! \}                                                                 */
	/*---------------------------------------------------------------------*/
	/*! \name                       Sync                                   */
	/*! \{                                                                 */

	/*! \}                                                                 */
	/*---------------------------------------------------------------------*/
	/*! \name                   Binary Access                              */
	/*! \{                                                                 */

	virtual UInt32 getBinSize (const BitVector         &whichField);
	virtual void   copyToBin  (      BinaryDataHandler &pMem,
									 const BitVector         &whichField);
	virtual void   copyFromBin(      BinaryDataHandler &pMem,
									 const BitVector         &whichField);


	/*! \}                                                                 */
	/*---------------------------------------------------------------------*/
	/*! \name                   Construction                               */
	/*! \{                                                                 */

	static  vtOsgDynMeshPtr      create          (void); 
	static  vtOsgDynMeshPtr      createEmpty     (void); 

	/*! \}                                                                 */

	/*---------------------------------------------------------------------*/
	/*! \name                       Copy                                   */
	/*! \{                                                                 */

	virtual FieldContainerPtr     shallowCopy     (void) const; 

	/*! \}                                                                 */
	/*=========================  PROTECTED  ===============================*/
protected:

	/*---------------------------------------------------------------------*/
	/*! \name                      Fields                                  */
	/*! \{                                                                 */

	MFPnt3f             _mfPosition;
	MFReal32            _mfLength;
	MFColor3f           _mfColor;

	/*! \}                                                                 */
	/*---------------------------------------------------------------------*/
	/*! \name                   Constructors                               */
	/*! \{                                                                 */

	vtOsgDynMeshBase(void);
	vtOsgDynMeshBase(const vtOsgDynMeshBase &source);

	/*! \}                                                                 */
	/*---------------------------------------------------------------------*/
	/*! \name                   Destructors                                */
	/*! \{                                                                 */

	virtual ~vtOsgDynMeshBase(void); 

	/*! \}                                                                 */
	/*---------------------------------------------------------------------*/
	/*! \name                       Sync                                   */
	/*! \{                                                                 */

	#if !defined(OSG_FIXED_MFIELDSYNC)
	void executeSyncImpl(      vtOsgDynMeshBase *pOther,
							   const BitVector         &whichField);

	virtual void   executeSync(      FieldContainer    &other,
									 const BitVector         &whichField);
	#else
	void executeSyncImpl(      vtOsgDynMeshBase *pOther,
							   const BitVector         &whichField,
							   const SyncInfo          &sInfo     );

	virtual void   executeSync(      FieldContainer    &other,
									 const BitVector         &whichField,
									 const SyncInfo          &sInfo);

	virtual void execBeginEdit     (const BitVector &whichField,
									UInt32     uiAspect,
									UInt32     uiContainerSize);

	void execBeginEditImpl (const BitVector &whichField,
							UInt32     uiAspect,
							UInt32     uiContainerSize);

	virtual void onDestroyAspect(UInt32 uiId, UInt32 uiAspect);
	#endif

	/*! \}                                                                 */
	/*==========================  PRIVATE  ================================*/
private:

	friend class FieldContainer;

	static FieldDescription   *_desc[];
	static FieldContainerType  _type;


	// prohibit default functions (move to 'public' if you need one)
	void operator =(const vtOsgDynMeshBase &source);
};

//---------------------------------------------------------------------------
//   Exported Types
//---------------------------------------------------------------------------


typedef vtOsgDynMeshBase *vtOsgDynMeshBaseP;

typedef osgIF<vtOsgDynMeshBase::isNodeCore,
			  CoredNodePtr<vtOsgDynMesh>,
			  FieldContainer::attempt_to_create_CoredNodePtr_on_non_NodeCore_FC>::
			  _IRet vtOsgDynMeshNodePtr;

typedef RefPtr<vtOsgDynMeshPtr> vtOsgDynMeshRefPtr;

OSG_END_NAMESPACE

	#define OSGCUBESBASE_HEADER_CVSID "@(#)$Id$"

#endif /* _OSGCUBESBASE_H_ */
