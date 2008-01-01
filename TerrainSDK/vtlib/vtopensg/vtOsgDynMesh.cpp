#define OSG_COMPILEMYLIB

#include "vtlib/vtlib.h"
//#include "vtdata/vtLog.h"

#include <stdlib.h>
#include <stdio.h>

/*#ifdef OSG_STREAM_IN_STD_NAMESPACE
#include <iostream>
#else
#include <iostream.h>
#endif*/

#include <iostream>

#include <OpenSG/OSGGL.h>


#include <OpenSG/OSGAction.h>
#include <OpenSG/OSGDrawAction.h>
#include <OpenSG/OSGRenderAction.h>
#include <OpenSG/OSGState.h>
#include <OpenSG/OSGMaterial.h>
#include <OpenSG/OSGAttachment.h>
#include <OpenSG/OSGSimpleAttachments.h>

#include <OpenSG/OSGSimpleSceneManager.h>
#include "vtOsgDynMesh.h"

OSG_USING_NAMESPACE


/***************************************************************************\
 *                            Description                                  *
\***************************************************************************/

/*! \class osg::vtOsgDynMesh

A test node for directly rendering to OpenGL.


*/

/***************************************************************************\
 *                               Types                                     *
\***************************************************************************/

/***************************************************************************\
 *                           Class variables                               *
\***************************************************************************/

char vtOsgDynMesh::cvsid[] = "@(#)$Id$";

/***************************************************************************\
 *                           Class methods                                 *
\***************************************************************************/

/*-------------------------------------------------------------------------*\
 -  public                                                                 -
\*-------------------------------------------------------------------------*/

/***************************************************************************\
 *                           Class methods                                 *
\***************************************************************************/

/*-------------------------------------------------------------------------*\
 -  public                                                                 -
\*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*\
 -  protected                                                              -
\*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*\
 -  private                                                                -
\*-------------------------------------------------------------------------*/

/** \brief initialize the static features of the class, e.g. action callbacks
 */

void vtOsgDynMesh::initMethod (void)
{
	DrawAction::registerEnterDefault( getClassType(),
									  osgTypedMethodFunctor2BaseCPtrRef<Action::ResultE, MaterialDrawablePtr,
									  CNodePtr, Action *>(&MaterialDrawable::drawActionHandler));

	RenderAction::registerEnterDefault( getClassType(),
										osgTypedMethodFunctor2BaseCPtrRef<Action::ResultE, MaterialDrawablePtr,
										CNodePtr, Action *>(&MaterialDrawable::renderActionHandler));
}

/***************************************************************************\
 *                           Instance methods                              *
\***************************************************************************/

/*-------------------------------------------------------------------------*\
 -  public                                                                 -
\*-------------------------------------------------------------------------*/


/*------------- constructors & destructors --------------------------------*/

/** \brief Constructor
 */

vtOsgDynMesh::vtOsgDynMesh(void) :
Inherited()
{

}

/** \brief Copy Constructor
 */

vtOsgDynMesh::vtOsgDynMesh(const vtOsgDynMesh &source) :
Inherited(source)
{
}

/** \brief Destructor
 */

vtOsgDynMesh::~vtOsgDynMesh(void)
{
}


/** \brief react to field changes
 */

void vtOsgDynMesh::changed(BitVector, UInt32)
{
}

/*------------------------------- dump ----------------------------------*/

/** \brief output the instance for debug purposes
 */

void vtOsgDynMesh::dump(       UInt32    uiIndent, 
							   const BitVector bvFlags) const
{
	SLOG << "Dump vtOsgDynMesh NI" << std::endl;
}



Action::ResultE vtOsgDynMesh::drawPrimitives(DrawActionBase * action)
{
	vtScene *pScene = vtGetScene();
	vtCamera *pCam = pScene->GetCamera();

	//alternatively
	osg::AttachmentPtr a = findAttachment(osg::VoidPAttachment::getClassType());
	if( a!=osg::NullFC ) {
		osg::VoidPAttachmentPtr m = osg::VoidPAttachmentPtr::dcast(a);
		vtDynGeom *vtgeom = reinterpret_cast<vtDynGeom *>(m->getField().getValue());

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glColor3f(1,1,1);	

		vtgeom->m_pPlanes = pScene->GetCullPlanes();
		vtgeom->DoCull(pCam);
		vtgeom->DoRender();

		// unapply any remaining material (OpenSG needs to do this manually)
		vtgeom->UnApplyMaterial();

		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	return Action::Continue;
}


void vtOsgDynMesh::adjustVolume(Volume & volume)
{   
	volume.setValid();
	volume.setEmpty();

	FBox3 box;
	//TODO get rid of the if then
	osg::AttachmentPtr a = findAttachment(osg::VoidPAttachment::getClassType());
	if( a!=osg::NullFC ) {
		osg::VoidPAttachmentPtr m = osg::VoidPAttachmentPtr::dcast(a);
		vtDynGeom *vtgeom = reinterpret_cast<vtDynGeom *>(m->getField().getValue());
		if( vtgeom != NULL ) {
			vtgeom->DoCalcBoundBox(box);

			//TODO check if center is needed
			volume.extendBy( Pnt3f(v2s(box.min))) ;
			volume.extendBy( Pnt3f(v2s(box.max))) ;
		}
	}
}

/*-------------------------------------------------------------------------*\
 -  protected                                                              -
\*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*\
 -  private                                                                -
\*-------------------------------------------------------------------------*/

