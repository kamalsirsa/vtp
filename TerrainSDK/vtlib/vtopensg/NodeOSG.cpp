//
// NodeOSG.cpp
//
// Encapsulate behavior for OpenSG scene graph nodes.
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"

#define DEBUG_NODE_LOAD	0

#include <OpenSG/OSGAttachment.h>
#include <OpenSG/OSGSimpleAttachments.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSceneFileHandler.h>

///////////////////////////////////////////////////////////////////////
// vtNode
//

/**
 * Releases a node.  Use this method instead of C++'s delete operator when
 * you are done with a node.  Internally, the node is reference counted so
 * it is not deleted until all references to it are removed.
 */
void vtNode::Release()
{
/*
#if EXCEPT
	// shouldn't happen but... safety check anyway
	if (m_pNode == NULL)
		return;

	if (m_pNode->referenceCount() == 1)
	{
		m_pFogStateSet = NULL;
		m_pFog = NULL;

#if DEBUG_NODE_LOAD
		VTLOG("Deleting Node: %lx (\"%s\")\n", this, m_pNode->getName().c_str());
#endif
		// Tell OSG that we're through with this node.
		// The following statement calls unref() on m_pNode, which deletes the
		//  OSG node, which decrements its reference to us, which deletes us.
		m_pNode = NULL;
	}
#endif //EXCEPT */
	if( m_pNode == osg::NullFC ) {
		return;
	}
	//TODO check fog states once ported
	//TODO check refcounting
	vtString name = osg::getName(m_pNode);

	osg::subRefCP (m_pNode);

	if( m_pNode == osg::NullFC ) { //if node has been deleted finally, out << debug info
#if DEBUG_NODE_LOAD
		VTLOG("Deleted Node: %lx (\"%s\")\n", this, name.c_str());
#endif
	}
}

void vtNode::SetOsgNode(osg::NodePtr n)
{
	beginEditCP( m_pNode );
	m_pNode = n;
	endEditCP( m_pNode );

	if( m_pNode != osg::NullFC ) {

		osg::VoidPAttachmentPtr a = osg::VoidPAttachment::create();

		beginEditCP(a);
		a->getField().setValue( this );
		endEditCP(a);

		beginEditCP(m_pNode, osg::Node::AttachmentsFieldMask);
		m_pNode->addAttachment(a);
		endEditCP(m_pNode, osg::Node::AttachmentsFieldMask);
	}
}

/**
 * Set the enabled state of this node.  When the node is not enabled, it
 * is not rendered.  If it is a group node, all of the nodes children are
 * also not rendered.
 */
void vtNode::SetEnabled(bool bOn)
{
	if( m_pNode != osg::NullFC ) { //shouldnt happen but who knows
		beginEditCP( m_pNode);
		m_pNode->setActive( bOn );
		endEditCP( m_pNode);
	}
}

/**
 * Return the enabled state of a node.
 */
bool vtNode::GetEnabled() const
{
	if( m_pNode != osg::NullFC ) {
		return m_pNode->getActive();
	} else {
		return false; //shouldnt come here, just for completeness
	}


}

void vtNode::SetName2(const char *name)
{
	if( m_pNode != osg::NullFC ) {
		beginEditCP( m_pNode );
		osg::setName( m_pNode, name );
		endEditCP( m_pNode);
	}
}

const char *vtNode::GetName2() const
{
	if( m_pNode != osg::NullFC ) {
		return osg::getName( m_pNode );
	} else {
		return NULL;
	}
}


/**
 * Calculates the bounding box of the geometry contained in and under
 * this node in the scene graph.  Note that unlike bounding sphere which
 * is cached, this value is calculated every time.
 *
 * \param box Will receive the bounding box.
 */
void vtNode::GetBoundBox(FBox3 &box)
{
	osg::DynamicVolume vol;
	vol = m_pNode->getVolume(false); //no update
	vol.morphToType(osg::DynamicVolume::BOX_VOLUME);
	osg::Pnt3f min, max;
	vol.getBounds(min, max);
	box.max = s2v ( max.subZero() );
	box.min = s2v ( min.subZero() ); 
}

void vtNode::GetBoundSphere(FSphere &sphere, bool bGlobal)
{
	osg::DynamicVolume vol;

	vol = m_pNode->getVolume(false);  //no update of vol is required
	vol.morphToType(osg::DynamicVolume::SPHERE_VOLUME);
	osg::Pnt3f center, min, max;
	osg::Vec3f vec;
	vol.getCenter(center);
	vol.getBounds(min, max);
	vec.setValues(max.x(), max.y(), max.z());
	vec -= osg::Vec3f(min.x(), min.y(), min.z());

	sphere.center = s2v (center.subZero());
	sphere.radius = vec.length()/2;

	if( bGlobal ) {
		LocalToWorld(sphere.center);
	}
}

/**
 * Transform a 3D point from a node's local frame of reference to world
 * coordinates.  This is done by walking the scene graph upwards, applying
 * all transforms that are encountered.
 *
 * \param point A reference to the input point is modified in-place with
 *	world coordinate result.
 */
void vtNode::LocalToWorld(FPoint3 &point)
{
#if 1
	vtNode *node = this;
	while( node = node->GetParent(0) ) {
		vtTransform *trans = dynamic_cast<vtTransform *>(node);
		if( trans ) {
			FMatrix4 mat;
			trans->GetTransform1(mat);
			FPoint3 result;
			mat.Transform(point, result);
			point = result;
		}
	}
#else //TODO native support for OpenSG

#endif

}

/**
 * Return the parent of this node in the scene graph.  If the node is not
 * in the scene graph, NULL is returned.
 *
 * \param iParent If the node has multiple parents, you can specify which
 *	one you want.
 */
vtGroup *vtNode::GetParent(int iParent)
{
	//only one parent for osg::nodes this time
	//TODO check multiple parents of group
	osg::NodePtr parent = m_pNode->getParent();
	if( !parent ) return NULL;

	osg::AttachmentPtr a = parent->findAttachment(osg::VoidPAttachment::getClassType());
	if( a!=osg::NullFC ) {
		osg::VoidPAttachmentPtr m = osg::VoidPAttachmentPtr::dcast(a);
		vtNode *vnode = reinterpret_cast<vtNode *>(m->getField().getValue());
		vtGroup *vgroup = dynamic_cast<vtGroup *>(vnode);
		if( vgroup != NULL ) return vgroup;
	}
	return NULL;
}

RGBf vtNodeBase::s_white(1, 1, 1);

/**
 * Set the Fog state for a node.
 *
 * You can turn fog on or off.  When you turn fog on, it affects this node
 * and all others below it in the scene graph.
 *
 * \param bOn True to turn fog on, false to turn it off.
 * \param start The distance from the camera at which fog starts, in meters.
 * \param end The distance from the camera at which fog end, in meters.  This
 *		is the point at which it becomes totally opaque.
 * \param color The color of the fog.  All geometry will be faded toward this
 *		color.
 * \param Type Can be GL_LINEAR, GL_EXP or GL_EXP2 for linear or exponential
 *		increase of the fog density.
 */
void vtNode::SetFog(bool bOn, float start, float end, const RGBf &color, enum FogType Type)
{
#if EXCEPT
	osg::StateSet *set = GetOsgNode()->getStateSet();
	if( !set ) {
		m_pFogStateSet = new osg::StateSet;
		set = m_pFogStateSet.get();
		GetOsgNode()->setStateSet(set);
	}

	if( bOn ) {
		Fog::Mode eType;
		switch( Type ) {
			case FM_LINEAR: eType = Fog::LINEAR; break;
			case FM_EXP: eType = Fog::EXP; break;
			case FM_EXP2: eType = Fog::EXP2; break;
			default: return;
		}
		m_pFog = new Fog;
		m_pFog->setMode(eType);
		m_pFog->setDensity(0.25f);	// not used for linear
		m_pFog->setStart(start);
		m_pFog->setEnd(end);
		m_pFog->setColor(osg::Vec4(color.r, color.g, color.b, 1));

		set->setAttributeAndModes(m_pFog.get(), StateAttribute::OVERRIDE | StateAttribute::ON);
	} else {
		// turn fog off
		set->setMode(GL_FOG, StateAttribute::OFF);
	}
#endif //EXCEPT
}





// Our own cache of models loaded from OSG
static std::map<vtString, osg::RefPtr<osg::NodePtr> > m_ModelCache;
bool vtNode::s_bDisableMipmaps = false;

/**
 * Load a 3D model from a file.
 *
 * The underlying scenegraph (e.g. OSG) is used to load the model, which is
 * returned as a vtNode.  You can then use this node normally, for example
 * add it to your scenegraph with vtGroup::AddChild(), or to your terrain's
 * subgraph with vtTerrain::AddNode().
 *
 * \param filename The filename to load from.
 * \param bAllowCache Default is true, to allow vtosg to cache models.
 *	This means that if you load from the same filename more than once, you
 *  will get the same model again instantly.  If you don't want this, for
 *	example if the model has changed on disk and you want to force loading,
 *	pass false.
 * \param bDisableMipmaps Pass true to turn off mipmapping in the texture maps
 *	in the loaded model.  Default is false (enable mipmapping).
 *
 * \return A node pointer if successful, or NULL if the load failed.
 */
vtNode *vtNode::LoadModel(const char *filename, bool bAllowCache, bool bDisableMipmaps)
{
	osg::NodePtr node = osg::SceneFileHandler::the().read(filename);
	osg::TransformPtr trans;
	osg::NodePtr container_group = osg::makeCoredNode<osg::Transform>(&trans);
	osg::Quaternion q; q.setValueAsAxisRad(osg::Vec3f(1,0,0), -osg::Pi/2 );
	osg::Matrix mat; mat.setRotate(q);
	beginEditCP(trans);
	trans->setMatrix(mat);
	endEditCP(trans);
	beginEditCP(container_group);
	container_group->addChild(node);
	endEditCP(container_group);


#if DEBUG_NODE_LOAD
	VTLOG("container %lx (rc %d), ", container_group, container_group->referenceCount());
#endif

	// The final resulting node is the container of that operation
	vtGroup *pGroup = new vtGroup(true);
	pGroup->SetOsgNode(container_group);
	pGroup->SetName2(filename);

#if DEBUG_NODE_LOAD
	VTLOG("VTP node %lx\n", pGroup);
#endif
	return pGroup;
}

void vtNode::ClearOsgModelCache()
{
	m_ModelCache.clear();
}

void DecorateVisit(osg::NodePtr node)
{
	if( node == osg::NullFC ) return;

	vtNode *vnode = NULL;
	osg::GroupPtr group ( osg::NullFC );
	vtGroup *vgroup = NULL;

	osg::AttachmentPtr a = node->findAttachment(osg::VoidPAttachment::getClassType());
	if( a!=osg::NullFC ) {
		osg::VoidPAttachmentPtr m = osg::VoidPAttachmentPtr::dcast(a);
		vnode = reinterpret_cast<vtNode *>(m->getField().getValue());
		group = osg::GroupPtr::dcast(node->getCore() );	//is it a group core ?
		if( !vnode ) {
			// needs decorating.  it is a group?
			if( group ) {
				vgroup = new vtGroup(true);
				vgroup->SetOsgNode(node); 
				vnode = vgroup;
			} else {
				// decorate as plain native node
				vnode = new vtNativeNode(node);
			}
		}
		if( group ) {
			for( unsigned int i = 0; i < node->getNChildren(); ++i )
				DecorateVisit(node->getChild(i));
		}
	}
}

void vtNode::DecorateNativeGraph()
{
	DecorateVisit(m_pNode);
}

vtNode *vtNativeNode::FindParentVTNode()
{
	osg::NodePtr node = GetOsgNode();
	while( node ) {
		node = node->getParent();
		osg::AttachmentPtr a = node->findAttachment(osg::VoidPAttachment::getClassType());
		if( a!=osg::NullFC ) {
			osg::VoidPAttachmentPtr m = osg::VoidPAttachmentPtr::dcast(a);
			vtNode *vnode = reinterpret_cast<vtNode *>(m->getField().getValue());
			if( vnode )
				return vnode;
		}
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////
// vtGroup
//

vtGroup::vtGroup(bool suppress) : vtNode(), vtGroupBase()
{
	if( !suppress ) {
		osg::NodePtr node = osg::makeCoredNode<osg::Group>();
		SetOsgNode(node);
	}
}


vtNodeBase *vtGroup::Clone()
{
	vtGroup *group = new vtGroup;
	group->CopyFrom(this);
	return group;
}

void vtGroup::CopyFrom(const vtGroup *rhs)
{

	// Deep copy or shallow copy?
	// Assume shallow copy: duplicate this node and add another reference
	//  to the existing children.
	for( unsigned int i = 0; i < rhs->GetNumChildren(); ++i ) {
		vtNode *child = rhs->GetChild(i);
		if( child )
			AddChild(child);
		else {
			// Might be an internal (native OSG) node - try to cope with that
			const osg::NodePtr rhsGroup = rhs->GetOsgNode();

			osg::NodePtr pOsgChild = rhsGroup->getChild(i);
			if( pOsgChild ) {
				beginEditCP( m_pNode, osg::Node::ChildrenFieldMask );
				m_pNode->addChild(pOsgChild);
				endEditCP( m_pNode, osg::Node::ChildrenFieldMask );
			}
		}
	}
}

void vtGroup::Release()
{
/*
#if EXCEPT
	// Check if there are no more external references to this group node.
	// If so, clean up the VTP side of the scene graph.
	if (m_pNode->referenceCount() == 1)
	{
		// it's over for this node, start the destruction process
		// Release children depth-first
		int children = GetNumChildren();
		vtNode *pChild;

		for (int i = 0; i < children; i++)
		{
			if (NULL == (pChild = GetChild(0)))
			{
				// Probably a raw osg node Group, access it directly.
				Node *node = m_pGroup->getChild(0);
				// This deletes the node as well as there is no outer vtNode
				// holding a reference.
				m_pGroup->removeChild(node);
			}
			else
			{
				m_pGroup->removeChild(pChild->GetOsgNode());
				pChild->Release();
			}
		}
		m_pGroup = NULL;
	}
	// Now release itself
	vtNode::Release();
#endif //EXCEPT
*/

	//TODO check reference counting
	//osg::subRefCP( m_pNode );
	/*if (m_pNode->getPtr().getRefCount()) == 1) {
		int children = GetNumChildren();
		vtNode *pChild;

		for (int i = 0; i < children; ++i)
		{
			if (NULL == (pChild = GetChild(0))) {
				osg::NodePtr node = m_pGroup->GetChild(0);
				beginEditCP( m_pGroup, osg::Node::ChildrenFieldMask );
				m_pGroup->subChild(node);
				endEditCP( m_pGroup, osg::Node::ChildrenFieldMask );
			} else {
				beginEditCP( m_pGroup, osg::Node::ChildrenFieldMask );
				m_pGroup->subChild( pChild->GetOsgNode());
				beginEditCP( m_pGroup, osg::Node::ChildrenFieldMask );
				pChild->Release();
			}
		}
		m_pGroup = osg::NullFC;
	}*/
	vtNode::Release();
}

vtNode *FindNodeByName(vtNode *node, const char *name)
{
	if( !strcmp(node->GetName2(), name) )
		return node;

	const vtGroupBase *pGroup = dynamic_cast<const vtGroupBase *>(node);
	if( pGroup ) {
		for( unsigned int i = 0; i < pGroup->GetNumChildren(); i++ ) {
			vtNode *pChild = pGroup->GetChild(i);
			vtNode *pResult = FindNodeByName(pChild, name);
			if( pResult )
				return pResult;
		}
	}
	return NULL;
}

const vtNode *vtGroup::FindDescendantByName(const char *name) const
{
	return FindNodeByName((vtNode *)this, name);
}

void vtGroup::AddChild(vtNode *pChild)
{
	if( pChild ) {
		beginEditCP( m_pNode, osg::Node::ChildrenFieldMask );
		m_pNode->addChild( pChild->GetOsgNode() );
		endEditCP( m_pNode, osg::Node::ChildrenFieldMask );
	}
}

void vtGroup::RemoveChild(vtNode *pChild)
{
	if( pChild ) {
		beginEditCP( m_pNode, osg::Node::ChildrenFieldMask );
		m_pNode->subChild( pChild->GetOsgNode() );
		endEditCP( m_pNode, osg::Node::ChildrenFieldMask );
	}
}

vtNode *vtGroup::GetChild(unsigned int num) const
{
	osg::UInt32 children = m_pNode->getNChildren();
	if (num >=0 && num < children)
	{
		osg::NodePtr pChild = m_pNode->getChild (num);
		//mw: as for now, void attachments are being used similar to userData
		//mw: in future versions, move on to attachmentPtr 
		osg::AttachmentPtr a = pChild->findAttachment(osg::VoidPAttachment::getClassType());
		if (a!=osg::NullFC)
		{
			osg::VoidPAttachmentPtr m = osg::VoidPAttachmentPtr::dcast(a);
			return reinterpret_cast<vtNode *>(m->getField().getValue());
		}
	}
	return NULL;
}

unsigned int vtGroup::GetNumChildren() const
{
	if (m_pNode == osg::NullFC)
		return 0;
	return m_pNode->getNChildren();
}

bool vtGroup::ContainsChild(vtNode *pNode) const
{
	osg::UInt32 i, children = GetNumChildren();
	for( i = 0; i < children; ++i )
	{
		if (GetChild(i) == pNode)
			return true;
	}
	return false;
}


///////////////////////////////////////////////////////////////////////
// vtTransform
//

vtTransform::vtTransform() : vtGroup(true), vtTransformBase()
{
	osg::NodePtr node = osg::makeCoredNode<osg::Transform>(&m_pTransform);
	SetOsgNode(node);
}

vtNodeBase *vtTransform::Clone()
{
	vtTransform *trans = new vtTransform();
	trans->CopyFrom(this);
	return trans;
}

void vtTransform::CopyFrom(const vtTransform *rhs)
{
	const osg::TransformPtr tfp = rhs->GetOsgTransform();
	beginEditCP(m_pTransform);
	m_pTransform->setMatrix(tfp->getMatrix());
	endEditCP(m_pTransform);
}

void vtTransform::Release()
{
	osg::subRefCP( m_pNode );
	vtGroup::Release();
}

void vtTransform::Identity()
{
	//TODO better way ? 
	osg::Matrix4f mat = osg::Matrix4f::identity();
	beginEditCP(m_pTransform);
	m_pTransform->setMatrix( mat );
	endEditCP(m_pTransform);
}

FPoint3 vtTransform::GetTrans() const
{
	osg::Matrix4f mat = m_pTransform->getMatrix();

	// OpenSG lacks an efficient getTranslate method, so access the array
	osg::Vec3f trans;
	trans[0] = mat[3][0];
	trans[1] = mat[3][1];
	trans[2] = mat[3][2];

	return s2v(trans);
}

void vtTransform::SetTrans(const FPoint3 &pos)
{
	osg::Matrix4f mat = m_pTransform->getMatrix();

	mat.setTranslate(osg::Vec3f(pos.x, pos.y, pos.z));

	beginEditCP(m_pTransform);
	m_pTransform->setMatrix(mat);
	endEditCP(m_pTransform);
}

void vtTransform::Translate1(const FPoint3 &pos)
{
	osg::Matrix4f mat = m_pTransform->getMatrix();

	osg::Matrix4f trans;
	trans.setTranslate( osg::Vec3f(pos.x, pos.y, pos.z) );
	mat.multLeft(trans);

	beginEditCP(m_pTransform);
	m_pTransform->setMatrix(mat);
	endEditCP(m_pTransform);

}

void vtTransform::TranslateLocal(const FPoint3 &pos)
{
	osg::Matrix4f mat = m_pTransform->getMatrix();

	osg::Matrix4f trans;
	trans.setTranslate( osg::Vec3f(pos.x, pos.y, pos.z) );
	mat.mult(trans);

	beginEditCP(m_pTransform);
	m_pTransform->setMatrix(mat);
	endEditCP(m_pTransform);
}

void vtTransform::Rotate2(const FPoint3 &axis, double angle)
{
	osg::Matrix4f mat = m_pTransform->getMatrix();

	osg::Quaternion q(osg::Vec3f( axis.x, axis.y, axis.z), (float) angle );
	osg::Matrix4f rot;  rot.setRotate(q);
	mat.multLeft(rot);

	beginEditCP(m_pTransform);
	m_pTransform->setMatrix(mat);
	endEditCP(m_pTransform);
}

void vtTransform::RotateLocal(const FPoint3 &axis, double angle)
{
	osg::Matrix4f mat = m_pTransform->getMatrix();

	osg::Quaternion q(osg::Vec3f( axis.x, axis.y, axis.z), (float) angle );
	osg::Matrix4f rot;  rot.setRotate(q);
	mat.mult(rot);

	beginEditCP(m_pTransform);
	m_pTransform->setMatrix(mat);
	endEditCP(m_pTransform);
}

void vtTransform::RotateParent(const FPoint3 &axis, double angle)
{
	osg::Matrix4f mat = m_pTransform->getMatrix();

	// remember the translation component
	osg::Vec3f trans;
	trans[0] = mat[3][0];
	trans[1] = mat[3][1];
	trans[2] = mat[3][2];

	// set it to zero
	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;

	// rotate
	osg::Quaternion q (osg::Vec3f( axis.x, axis.y, axis.z), (float) angle);
	osg::Matrix4f rot; rot.setRotate(q);
	mat.multLeft(rot);

	// now restore translation
	mat[3][0] = trans[0];
	mat[3][1] = trans[1];
	mat[3][2] = trans[2];

	beginEditCP(m_pTransform);
	m_pTransform->setMatrix(mat);
	endEditCP(m_pTransform);
}

FQuat vtTransform::GetOrient() const
{
	osg::Matrix4f mat = m_pTransform->getMatrix();
	osg::Vec3f trans, scale;
	osg::Quaternion q, scalerot;
	mat.getTransform(trans, q, scale, scalerot);

	return FQuat(q.x(),q.y(),q.z(),q.w());
}

FPoint3 vtTransform::GetDirection() const
{
	osg::Matrix4f mat = m_pTransform->getMatrix();
	osg::Real32 *ptr = mat.getValues();
	//TODO check if column/row first
	return FPoint3(-ptr[8], -ptr[9], -ptr[10]);
}

void vtTransform::SetDirection(const FPoint3 &point, bool bPitch)
{
	// get current matrix
	FMatrix4 m4;
	GetTransform1(m4);

	// remember where it is now
	FPoint3 trans = m4.GetTrans();

	// orient it in the desired direction
	FMatrix3 m3;
	m3.MakeOrientation(point, bPitch);
	m4.SetFromMatrix3(m3);

	// restore translation
	m4.SetTrans(trans);

	// set current matrix
	SetTransform1(m4);
}

void vtTransform::Scale3(float x, float y, float z)
{
	osg::Matrix4f mat = m_pTransform->getMatrix();
	osg::Matrix4f s;
	s.setScale( x,y,z );
	mat.mult(s);
	beginEditCP(m_pTransform);
	m_pTransform->setMatrix( mat );
	endEditCP(m_pTransform);
}

void vtTransform::SetTransform1(const FMatrix4 &mat)
{
	osg::Matrix4f mat_osg;

	ConvertMatrix4(&mat, &mat_osg);

	beginEditCP(m_pTransform);
	m_pTransform->setMatrix( mat_osg );
	endEditCP(m_pTransform);
}

void vtTransform::GetTransform1(FMatrix4 &mat) const
{
	const osg::Matrix4f xform = m_pTransform->getMatrix();
	ConvertMatrix4(&xform, &mat);
}

void vtTransform::PointTowards(const FPoint3 &point, bool bPitch)
{
	SetDirection(point - GetTrans(), bPitch);
}


///////////////////////////////////////////////////////////////////////
// vtLight
//

vtLight::vtLight()
{
	osg::NodePtr node = osg::makeCoredNode<osg::DirectionalLight>(&m_pLight);
	SetOsgNode(node);
}

vtNodeBase *vtLight::Clone()
{
	vtLight *light = new vtLight();
	light->CopyFrom(this);
	return light;
}

void vtLight::CopyFrom(const vtLight *rhs)
{
	// copy attributes
	SetDiffuse(rhs->GetDiffuse());
	SetAmbient(rhs->GetAmbient());
	SetSpecular(rhs->GetSpecular());
}

void vtLight::Release()
{
/*#if EXCEPT
	// Check if we are completely deferenced
	if (m_pNode->referenceCount() == 1)
		m_pLight = NULL;
	vtNode::Release();
#endif*/
//	osg::subRefCP(m_pLightNode);
	vtNode::Release();
}

void vtLight::SetDiffuse(const RGBf &color)
{
	osg::Color4f diffuse = osg::Color4f(color.r, color.g, color.b, 0.);
	beginEditCP(m_pLight);
	m_pLight->setDiffuse(diffuse);
	endEditCP(m_pLight);
}

RGBf vtLight::GetDiffuse() const
{
	osg::Color4f diffuse = m_pLight->getDiffuse();
	return RGBf(diffuse.red(),diffuse.green(),diffuse.blue());
}

void vtLight::SetSpecular(const RGBf &color)
{
	osg::Color4f specular = osg::Color4f(color.r, color.g, color.b, 0.0);
	beginEditCP(m_pLight);
	m_pLight->setSpecular(specular);
	endEditCP(m_pLight);
}

RGBf vtLight::GetSpecular() const
{
	osg::Color4f specular = m_pLight->getSpecular();
	return RGBf(specular.red(),specular.green(),specular.blue());
}

void vtLight::SetAmbient(const RGBf &color)
{
	osg::Color4f ambient = osg::Color4f(color.r, color.g, color.b, 0.);
	beginEditCP(m_pLight);
	m_pLight->setDiffuse(ambient);
	endEditCP(m_pLight);
}

RGBf vtLight::GetAmbient() const
{
	osg::Color4f ambient = m_pLight->getAmbient();
	return RGBf(ambient.red(),ambient.green(),ambient.blue());
}

void vtLight::SetEnabled(bool bOn)
{
	vtNode::SetEnabled(bOn);
}


///////////////////////////////////////////////////////////////////////
// vtCamera
//

vtCamera::vtCamera() : vtTransform()
{
	m_fHither = 1;
	m_fYon = 100;
	m_fFOV = PIf/3.0f;
	m_bOrtho = false;
	m_fWidth = 1;
}

vtNodeBase *vtCamera::Clone()
{
	vtCamera *newcam = new vtCamera();
	newcam->CopyFrom(this);
	return newcam;
}

void vtCamera::CopyFrom(const vtCamera *rhs)
{
	m_fFOV = rhs->m_fFOV;
	m_fHither = rhs->m_fHither;
	m_fYon = rhs->m_fYon;
	m_bOrtho = rhs->m_bOrtho;
	m_fWidth = rhs->m_fWidth;
	vtTransform::CopyFrom(rhs);
}

/**
 * Set the hither (near) clipping plane distance.
 */
void vtCamera::SetHither(float f)
{
	m_fHither = f;
}

/**
 * Get the hither (near) clipping plane distance.
 */
float vtCamera::GetHither() const
{
	return m_fHither;
}

/**
 * Set the yon (far) clipping plane distance.
 */
void vtCamera::SetYon(float f)
{
	m_fYon = f;
}

/**
 * Get the yon (far) clipping plane distance.
 */
float vtCamera::GetYon() const
{
	return m_fYon;
}

/**
 * Set the camera's horizontal field of view (FOV) in radians.
 */
void vtCamera::SetFOV(float fov_x)
{
	m_fFOV = fov_x;
}

/**
 * Return the camera's horizontal field of view (FOV) in radians.
 */
float vtCamera::GetFOV() const
{
	return m_fFOV;
}

/**
 * Return the camera's vertical field of view (FOV) in radians.
 */
float vtCamera::GetVertFOV() const
{
	IPoint2 size = vtGetScene()->GetWindowSize();
	float aspect = (float) size.x / size.y;

	double a = tan(m_fFOV/2);
	double b = a / aspect;
	return (float) atan(b) * 2;
}

/**
 * Zoom (move) the camera to a sphere, generall the bounding sphere of
 *  something you want to look at.  The camera will be pointing directly
 *  down the -Z axis at the center of the sphere.
 */
void vtCamera::ZoomToSphere(const FSphere &sphere)
{
	Identity();
	Translate1(sphere.center);
	Translate1(FPoint3(0.0f, 0.0f, sphere.radius));
}

/**
 * Set this camera to use an orthographic view.  An orthographic view has
 *  no FOV angle, so Set/GetFOV have no affect.  Instead, use Get/SetWidth
 *  to control the width of the orthogonal view.
 */
void vtCamera::SetOrtho(bool bOrtho)
{
	m_bOrtho = bOrtho;
}

/**
 * Return true if the camera is orthographic.
 */
bool vtCamera::IsOrtho() const
{
	return m_bOrtho;
}

/**
 * Set the view width of an orthographic camera.
 */
void vtCamera::SetWidth(float fWidth)
{
	m_fWidth = fWidth;
}

/**
 * Get the view width of an orthographic camera.
 */
float vtCamera::GetWidth() const
{
	return m_fWidth;
}


///////////////////////////////////////////////////////////////////////
// vtGeom
//

vtGeom::vtGeom() : vtNode()
{
	osg::NodePtr node = osg::makeCoredNode<osg::Group>();
	SetOsgNode(node);
}

vtNodeBase *vtGeom::Clone()
{
	vtGeom *geom = new vtGeom();
	geom->CopyFrom(this);
	return geom;
}

void vtGeom::CopyFrom(const vtGeom *rhs)
{
	// Shallow copy: just reference the meshes and materials of the
	//  geometry that we are copying from.
	SetMaterials(rhs->GetMaterials());
	int idx;
	for( unsigned int i = 0; i < rhs->GetNumMeshes(); i++ ) {
		vtMesh *mesh = rhs->GetMesh(i);
		if( mesh ) {
			idx = mesh->GetMatIndex();
			AddMesh(mesh, idx);
		} else {
			vtTextMesh *tm = rhs->GetTextMesh(i);
			if( tm ) {
				idx = tm->GetMatIndex();
				AddTextMesh(tm, idx);
			}
		}
	}
}

//TODO check release
void vtGeom::Release()
{
#if EXCEPT
	if( m_pNode->referenceCount() == 1 ) {
		// Clean up this geom, it is going away.
		// Release the meshes we contain, which will delete them if there
		//  are no other references to them.
		int i, num = m_pGeode->getNumDrawables();
		for( i = 0; i < num; i++ ) {
			vtMesh *mesh = GetMesh(i);
			if( mesh )
				mesh->Release();
			else {
				vtTextMesh *textmesh = GetTextMesh(i);
				if( textmesh )
					textmesh->Release();
			}
		}
		m_pGeode->removeDrawable(0, num);

		m_pGeode = NULL;
		m_pMaterialArray = NULL;		// dereference
	}
	vtNode::Release();
#endif
}

void vtGeom::AddMesh(vtMesh *pMesh, int iMatIdx)
{
	beginEditCP(m_pNode, osg::Node::ChildrenFieldMask);
	m_pNode->addChild(pMesh->m_pGeometryNode);
	endEditCP(m_pNode, osg::Node::ChildrenFieldMask);

	//TODO add referencing to mesh: addRefCP(ref());

	SetMeshMatIndex(pMesh, iMatIdx);
}

//TODO AddTextMesh
void vtGeom::AddTextMesh(vtTextMesh *pTextMesh, int iMatIdx)
{
	beginEditCP(m_pNode);
	m_pNode->addChild(pTextMesh->m_pOsgTextNode);
	endEditCP(m_pNode);

	/*pTextMesh->SetMatIndex(iMatIdx);
	vtMaterial *pMat = GetMaterial(iMatIdx);
	if (pMat)
	{
		beginEditCP(pTextMesh->m_pOsgText);
		pTextMesh->m_pOsgText->setMaterial(pMat->m_pMaterial);
		endEditCP(pTextMesh->m_pOsgText);
	}*/
}

void vtGeom::SetMeshMatIndex(vtMesh *pMesh, int iMatIdx)
{
#if EXCEPT
	vtMaterial *pMat = GetMaterial(iMatIdx);
	if( pMat ) {
		// Beware: the mesh may already have its own stateset
		StateSet *pState = pMat->m_pStateSet.get();
		StateSet *pStateMesh = pMesh->m_pGeometry->getStateSet();

		if( pStateMesh )
			pStateMesh->merge(*pState);
		else
			pMesh->m_pGeometry->setStateSet(pState);

		// Try to provide color for un-lit meshes
		if( !pMat->GetLighting() ) {
			// unless it's using vertex colors...
			Geometry::AttributeBinding bd = pMesh->m_pGeometry->getColorBinding();
			if( bd != Geometry::BIND_PER_VERTEX ) {
				// not lit, not vertex colors
				// here is a sneaky way of forcing OSG to use the diffuse
				// color for the unlit color

				// This will leave the original color array alllocated in the vtMesh
				Vec4Array *pColors = new Vec4Array;
				pColors->push_back(pMat->m_pMaterial->getDiffuse(Material::FRONT_AND_BACK));
				pMesh->m_pGeometry->setColorArray(pColors);
				pMesh->m_pGeometry->setColorBinding(Geometry::BIND_OVERALL);
			}
		}
	}
	pMesh->SetMatIndex(iMatIdx);
#endif //EXCEPT

	vtMaterial *pMat = GetMaterial(iMatIdx);

	if( pMat ) {

			osg::GeometryPtr geo = pMesh->m_pGeometryCore;
			beginEditCP(geo);
			geo->setMaterial( pMat->m_pMaterial );
			endEditCP(geo);
		
			return; 

	/*	osg::GeometryPtr geo = pMesh->m_pGeometryCore;
		if ( geo->getMaterial() != OSG::NullFC )
		{
			beginEditCP(geo);
			geo->setMaterial( pMat->m_pMaterial );
			endEditCP(geo);
		} 

		//if (pMat->GetLighting()) {
		// Beware: the mesh may already have its own stateset
		osg::StatePtr pState = pMat->GetState();
		//osg::StatePtr pStateMesh = pMesh->m_pGeometryCore->getMaterial()->getState();

		//try to simulate behaviour because there is no merge in opensg besides a merge action
		if( pMesh->m_pGeometryCore->getMaterial() != osg::NullFC ) {
			//merge
			//pStateMesh->merge(*pState); no such thing in OpenSG
			//so figure out how it works in OSG and write this function
			osg::GeometryPtr geo = pMesh->m_pGeometryCore;
			beginEditCP(geo);
			geo->setMaterial( pMat->m_pMaterial );
			endEditCP(geo);

		} else { //this gives correct buildings
			osg::GeometryPtr geo = pMesh->m_pGeometryCore;
			beginEditCP(geo);
			geo->setMaterial( pMat->m_pMaterial );
			endEditCP(geo);
		}
		//} pmat-getlighting
		
		if( !pMat->GetLighting() ) { 
			//no vertex colors
			osg::GeometryPtr geo = pMesh->m_pGeometryCore;
			// colors set ?
			if( !geo->getColors() ) {

				osg::SimpleTexturedMaterialPtr sm =  pMat->m_pMaterial;
				osg::SimpleTexturedMaterialPtr mesh_sm = osg::SimpleTexturedMaterialPtr::dcast(geo->getMaterial());

				//assert it is not null, you never know
				if (sm != osg::NullFC && mesh_sm != osg::NullFC) {
					osg::Color3f diffuse = sm->getDiffuse();
                    beginEditCP(mesh_sm);
					//mesh_sm->setLit(false);
					mesh_sm->setDiffuse(diffuse);
					endEditCP(mesh_sm);
				}
			}
		}*/
	} //pmat
	pMesh->SetMatIndex(iMatIdx);

}

void vtGeom::RemoveMesh(vtMesh *pMesh)
{
/*
#if EXCEPT
	m_pGeode->removeDrawable(pMesh->m_pGeometry.get());

	if (pMesh->_refCount == 2)
	{
		// no more references except its default
		// self-reference and the reflexive reference from its m_pGeometry.
		pMesh->Release();
	}
#endif */
	beginEditCP(m_pNode, osg::Node::ChildrenFieldMask);
	m_pNode->subChild(pMesh->m_pGeometryNode);
	endEditCP(m_pNode, osg::Node::ChildrenFieldMask);
}

unsigned int vtGeom::GetNumMeshes() const
{
	//TODO check that only meshes are counted and ! all children
	//there should be only meshes under this node, but who knows ...
	return m_pNode->getNChildren();
}

vtMesh *vtGeom::GetMesh(int i) const
{
	osg::NodePtr node = m_pNode->getChild(i);
	osg::AttachmentPtr a = node->findAttachment(osg::VoidPAttachment::getClassType());
	if( a!=osg::NullFC ) {
		osg::VoidPAttachmentPtr m = osg::VoidPAttachmentPtr::dcast(a);
		vtMesh *vmesh = reinterpret_cast<vtMesh *>(m->getField().getValue());
		return vmesh;
	}
	return NULL; //shouldnt come here
}

vtTextMesh *vtGeom::GetTextMesh(int i) const
{
	osg::NodePtr node = m_pNode->getChild(i);
	osg::AttachmentPtr a = node->findAttachment(osg::VoidPAttachment::getClassType());
	if( a!=osg::NullFC ) {
		osg::VoidPAttachmentPtr m = osg::VoidPAttachmentPtr::dcast(a);
		vtTextMesh *vmesh = reinterpret_cast<vtTextMesh *>(m->getField().getValue());
		return vmesh;
	}
	return NULL; //shouldnt come here
}

void vtGeom::SetMaterials(const class vtMaterialArray *mats)
{
	m_pMaterialArray = mats;	// increases reference count
}

const vtMaterialArray *vtGeom::GetMaterials() const
{
	return m_pMaterialArray;
}

vtMaterial *vtGeom::GetMaterial(int idx)
{
	if( m_pMaterialArray == NULL ) {
		return NULL;
	}

	if( idx < 0 || idx >= (int) m_pMaterialArray->GetSize() )
		return NULL;

	return m_pMaterialArray->GetAt(idx);
}


///////////////////////////////////////////////////////////////////////
// vtLOD
//

vtLOD::vtLOD() : vtGroup(true)
{
	//m_pNode = osg::makeCoredNode<osg::Group>();
	//SetOsgNode(m_pNode);

	m_pNode = osg::makeCoredNode<osg::DistanceLOD>(&m_pLOD);
	beginEditCP(m_pLOD); 
	m_pLOD->setCenter(osg::Vec3f(0,0,0));   
	endEditCP(m_pLOD);

	SetOsgNode(m_pNode);
}

void vtLOD::Release()
{
/*
#if EXCEPT
	// Check if this node is no longer referenced.
	if (m_pNode->referenceCount() == 1)
		m_pLOD = NULL;
	vtGroup::Release();
#endif
*/
	osg::subRefCP(m_pNode);
	vtGroup::Release();
}

void vtLOD::SetRanges(float *ranges, int nranges)
{
	int i;
	beginEditCP(m_pLOD);
	m_pLOD->getMFRange()->clear();
	endEditCP(m_pLOD);
	for( i = 0; i < nranges; i++ ) {
		beginEditCP(m_pLOD);
		m_pLOD->getMFRange()->push_back(ranges[i]);
		endEditCP(m_pLOD);
	}
	m_pLOD->getMFRange()->push_back(1E10); 
}

void vtLOD::SetCenter(FPoint3 &center)
{
	osg::Vec3f p;
	v2s(center, p);
	beginEditCP(m_pLOD); 
	m_pLOD->setCenter(p);   
	endEditCP(m_pLOD);
}


///////////////////////////////////////////////////////////////////////
// vtDynGeom class

// Static members
vtMaterial *vtDynGeom::s_pCurrentMaterial = NULL;

vtDynGeom::vtDynGeom() : vtGeom()
{

	m_pDynNode = osg::makeCoredNode<osg::vtOsgDynMesh>(&m_pDynMesh);

	osg::setName (m_pDynNode, "vtDynNode");

	beginEditCP(m_pDynMesh);
	//m_pDynMesh->setDynGeom( this ); //use void attachment for this
	//TODO turn off Dlists ? i need to derive from osg::geometry to use it
	//m_pDynMesh->setDlistCache(false);	

	endEditCP(m_pDynMesh);
	osg::setName(m_pDynMesh,"vtOsgDynMesh");

	osg::VoidPAttachmentPtr a = osg::VoidPAttachment::create();
	beginEditCP(a);
	a->getField().setValue( this );
	endEditCP(a);

	beginEditCP(m_pDynMesh, osg::Node::AttachmentsFieldMask);
	m_pDynMesh->addAttachment(a);
	endEditCP(m_pDynMesh, osg::Node::AttachmentsFieldMask);

	//add some dummy data for testing
	/*beginEditCP(m_pDynMesh);
	  
	for ( int i=0; i < 1; ++i )
	{
		m_pDynMesh->getMFPosition()->push_back( osg::Pnt3f(0,0,0) );
		m_pDynMesh->getMFLength()->push_back( 10 );
		m_pDynMesh->getMFColor()->push_back( osg::Color3f( 1,1,1));
	}
	endEditCP(m_pDynMesh); */

	//osg::SimpleTexturedMaterialPtr stm = osg::SimpleTexturedMaterial::create();

	/*GL_NONE, GL_EMISSION, GL_AMBIENT, GL_DIFFUSE,
        GL_SPECULAR and  GL_AMBIENT_AND_DIFFUSE*/
	/*beginEditCP(stm);
	stm->setColorMaterial(GL_MODULATE);
	endEditCP(stm);*/

	//beginEditCP(m_pDynMesh);
	//m_pDynMesh->setMaterial( stm );
	//endEditCP(m_pDynMesh);

	beginEditCP(m_pNode);
	m_pNode->addChild(m_pDynNode);
	endEditCP(m_pNode);

}


/**
 * Test a sphere against the view volume.
 *
 * \return VT_AllVisible if entirely inside the volume,
 *			VT_Visible if partly inside,
 *			otherwise 0.
 */
int vtDynGeom::IsVisible(const FSphere &sphere) const
{
	unsigned int vis = 0;

	// cull against standard frustum
	int i;
	for( i = 0; i < 4; i++ ) {
		float dist = m_pPlanes[i].Distance(sphere.center);
		if( dist >= sphere.radius )
			return 0;
		if( (dist < 0) &&
			(dist <= sphere.radius) )
			vis = (vis << 1) | 1;
	}

	// Notify renderer that object is entirely within standard frustum, so
	// no clipping is necessary.
	if( vis == 0x0F )
		return VT_AllVisible;
	return VT_Visible;
}


/**
 * Test a single point against the view volume.
 *
 * \return true if inside, false if outside.
 */
bool vtDynGeom::IsVisible(const FPoint3& point) const
{
	// cull against standard frustum
	for( unsigned i = 0; i < 4; i++ ) {
		float dist = m_pPlanes[i].Distance(point);
		if( dist > 0.0f )
			return false;
	}
	return true;
}


/**
 * Test a 3d triangle against the view volume.
 *
 * \return VT_AllVisible if entirely inside the volume,
 *			VT_Visible if partly intersecting,
 *			otherwise 0.
 */
int vtDynGeom::IsVisible(const FPoint3& point0,
						 const FPoint3& point1,
						 const FPoint3& point2,
						 const float fTolerance) const
{
	unsigned int outcode0 = 0, outcode1 = 0, outcode2 = 0;
	register float dist;

	// cull against standard frustum
	int i;
	for( i = 0; i < 4; i++ ) {
		dist = m_pPlanes[i].Distance(point0);
		if( dist > fTolerance )
			outcode0 |= (1 << i);

		dist = m_pPlanes[i].Distance(point1);
		if( dist > fTolerance )
			outcode1 |= (1 << i);

		dist = m_pPlanes[i].Distance(point2);
		if( dist > fTolerance )
			outcode2 |= (1 << i);
	}
	if( outcode0 == 0 && outcode1 == 0 && outcode2 == 0 )
		return VT_AllVisible;
	if( outcode0 != 0 && outcode1 != 0 && outcode2 != 0 ) {
		if( (outcode0 & outcode1 & outcode2) != 0 )
			return 0;
		else {
			// tricky case - just be conservative and assume some intersection
			return VT_Visible;
		}
	}
	// not all in, and not all out
	return VT_Visible;
}

/**
 * Test a sphere against the view volume.
 *
 * \return VT_AllVisible if entirely inside the volume,
 *			VT_Visible if partly intersecting,
 *			otherwise 0.
 */
int vtDynGeom::IsVisible(const FPoint3 &point, float radius)
{
	unsigned int incode = 0;

	// cull against standard frustum
	for( int i = 0; i < 4; i++ ) {
		float dist = m_pPlanes[i].Distance(point);
		if( dist > radius )
			return 0;			// entirely outside this plane
		if( dist < -radius )
			incode |= (1 << i);	// entirely inside this plane
	}
	if( incode == 0x0f )
		return VT_AllVisible;	// entirely inside all planes
	else
		return VT_Visible;
}

void vtDynGeom::ApplyMaterial(vtMaterial *mat)
{
	// remove any previously applied material
	UnApplyMaterial();

	osg::StatePtr state = mat->m_pMaterial->getState();
	state->activate(vtGetScene()->GetSceneView()->GetAction());

	// remember it for UnApply later
	s_pCurrentMaterial = mat;
}

void vtDynGeom::UnApplyMaterial()
{
	if (s_pCurrentMaterial)
	{
		osg::StatePtr state = s_pCurrentMaterial->m_pMaterial->getState();
		state->deactivate(vtGetScene()->GetSceneView()->GetAction());
		s_pCurrentMaterial = NULL;
	}
}

///////////////////////////////////////////////////////////
// vtHUD

/**
 * Create a HUD node.  A HUD ("heads-up display") is a group whose whose
 * children are transformed to be drawn in window coordinates, rather
 * than world coordinates.
 *
 * You should only ever create one HUD node in your scenegraph.
 *
 * \param bPixelCoords If true, the child transforms should be interpreted
 *		as pixel coordinates, from (0,0) in the lower-left of the viewpoint.
 *		Otherwise, they are considered in normalized window coordinates,
 *		from (0,0) in the lower-left to (1,1) in the upper right.
 */
vtHUD::vtHUD(bool bPixelCoords) : vtGroup(true)
{
#if EXCEPT
	osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
	modelview_abs->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	modelview_abs->setMatrix(osg::Matrix::identity());

	m_projection = new osg::Projection;
	m_projection->addChild(modelview_abs);
	SetOsgGroup(m_projection);

	// We can set the projection to pixels (0,width,0,height) or
	//	normalized (0,1,0,1)
	m_bPixelCoords = bPixelCoords;
	if( m_bPixelCoords ) {
		IPoint2 winsize = vtGetScene()->GetWindowSize();

		// safety check first, avoid /0 crash
		if( winsize.x != 0 && winsize.y != 0 )
			m_projection->setMatrix(osg::Matrix::ortho2D(0, winsize.x, 0, winsize.y));
	} else {
		// Normalized window coordinates, 0 to 1
		m_projection->setMatrix(osg::Matrix::ortho2D(0, 1, 0, 1));
	}

	// To ensure that the sprite appears on top we can use osg::Depth to
	//  force the depth fragments to be placed at the front of the screen.
	osg::StateSet* stateset = m_projection->getOrCreateStateSet();
	stateset->setAttribute(new osg::Depth(osg::Depth::LESS,0.0,0.0001));

	// A HUD node is unlike other group nodes!
	// The modelview node is the container for the node's children.
	m_pGroup = modelview_abs;

	vtGetScene()->SetHUD(this);
#endif
}

void vtHUD::Release()
{
#if EXCEPT
	// Check if there are no more external references to this HUD node.
	if( m_pNode->referenceCount() == 1 ) {
		m_projection = NULL;
		vtGetScene()->SetHUD(NULL);
	}
	vtGroup::Release();
#endif // EXCEPT
}

vtNodeBase *vtHUD::Clone()
{
	vtHUD *hud = new vtHUD();
	hud->CopyFrom(this);
	return hud;
}

void vtHUD::CopyFrom(const vtHUD *rhs)
{
	// TODO
}

void vtHUD::SetWindowSize(int w, int h)
{
#if EXCEPT
	if( m_bPixelCoords ) {
		if( w != 0 && h != 0 )
			m_projection->setMatrix(osg::Matrix::ortho2D(0, w, 0, h));
	}
#endif
}


///////////////////////////////////////////////////////////////////////
// vtImageSprite

vtImageSprite::vtImageSprite()
{
#if EXCEPT
	m_pMats = NULL;
	m_pGeom = NULL;
	m_pMesh = NULL;
#endif
}

vtImageSprite::~vtImageSprite()
{
	// Do not explicitly free geometry, if it was added to the scene.
}

/**
 * Create a vtImageSprite.
 *
 * \param szTextureName The filename of a texture image.
 * \param bBlending Set to true for alpha-blending, which produces smooth
 *		edges on transparent textures.
 */
bool vtImageSprite::Create(const char *szTextureName, bool bBlending)
{
	vtImage *pImage = vtImageRead(szTextureName);
	if( !pImage )
		return false;
	return Create(pImage, bBlending);
}

/**
 * Create a vtImageSprite.
 *
 * \param pImage A texture image.
 * \param bBlending Set to true for alpha-blending, which produces smooth
 *		edges on transparent textures.
 */
bool vtImageSprite::Create(vtImage *pImage, bool bBlending)
{
	m_Size.x = pImage->GetWidth();
	m_Size.y = pImage->GetHeight();

	// set up material and geometry container
	m_pMats = new vtMaterialArray;
	m_pGeom = new vtGeom;
	m_pGeom->SetMaterials(m_pMats);
	m_pMats->Release();

	m_pMats->AddTextureMaterial(pImage, false, false, bBlending);

	// default position of the mesh is just 0,0-1,1
	m_pMesh = new vtMesh(vtMesh::QUADS, VT_TexCoords, 4);
	m_pMesh->AddVertexUV(FPoint3(0,0,0), FPoint2(0,0));
	m_pMesh->AddVertexUV(FPoint3(1,0,0), FPoint2(1,0));
	m_pMesh->AddVertexUV(FPoint3(1,1,0), FPoint2(1,1));
	m_pMesh->AddVertexUV(FPoint3(0,1,0), FPoint2(0,1));
	m_pMesh->AddQuad(0, 1, 2, 3);
	m_pGeom->AddMesh(m_pMesh, 0);
	m_pMesh->Release();
	return true;
}

/**
 * Set the XY position of the sprite.  These are in world coordinates,
 *  unless this sprite is the child of a vtHUD, in which case they are
 *  pixel coordinates measured from the lower-left corner of the window.
 *
 * \param l Left.
 * \param t Top.
 * \param r Right.
 * \param b Bottom.
 */
void vtImageSprite::SetPosition(float l, float t, float r, float b)
{
	if( !m_pMesh )	 // safety check
		return;
	m_pMesh->SetVtxPos(0, FPoint3(l, b, 0));
	m_pMesh->SetVtxPos(1, FPoint3(r, b, 0));
	m_pMesh->SetVtxPos(2, FPoint3(r, t, 0));
	m_pMesh->SetVtxPos(3, FPoint3(l, t, 0));
	m_pMesh->ReOptimize();
}

/**
 * Set (replace) the image on a sprite that has already been created.
 */
void vtImageSprite::SetImage(vtImage *pImage)
{
	// Sprite must already be created
	if( !m_pMats )
		return;
	vtMaterial *mat = m_pMats->GetAt(0);
	mat->SetTexture(pImage);
}


///////////////////////////////////////////////////////////////////////
// Intersection methods

//EXCEPT #include <osgUtil/IntersectVisitor>

/**
 * Check for surface intersections along a line segment in space.
 *
 * \param pTop The top of the scene graph that you want to search for
 *		intersections.  This can be the root node if you want to seach your
 *		entire scene, or any other node to search a subgraph.
 * \param start The start point (world coordinates) of the line segment.
 * \param end	The end point (world coordinates) of the line segment.
 * \param hitlist The results.  If there are intersections (hits), they are
 *		placed in this list, which is simply a std::vector of vtHit objects.
 *		Each vtHit object gives information about the hit point.
 * \param bLocalCoords Pass true to get your results in local coordinates
 *		(in the frame of the object which was hit).  Otherwise, result points
 *		are in world coordinates.
 *
 * \return The number of intersection hits (size of the hitlist array).
 */
int vtIntersect(vtNode *pTop, const FPoint3 &start, const FPoint3 &end,
				vtHitList &hitlist, bool bLocalCoords)
{

	osg::Line l( v2s(start), v2s(end) );
	osg::NodePtr osgnode = pTop->GetOsgNode();
	osg::IntersectAction *act = osg::IntersectAction::create();
	act->setLine( l );
	act->apply( osgnode );
	if( act->didHit() ) {
		osg::NodePtr hitnode = act->getHitObject();
		osg::Pnt3f hitpoint_l = act->getHitPoint();
		osg::Matrix m;
		hitnode->getToWorld(m);
		osg::Pnt3f hitpoint_g (hitpoint_l);
		m.multMatrixPnt(hitpoint_g);

		vtNode *vnode(0);
		osg::AttachmentPtr a = hitnode->findAttachment(osg::VoidPAttachment::getClassType());
		if( a!=osg::NullFC ) {
			osg::VoidPAttachmentPtr m = osg::VoidPAttachmentPtr::dcast(a);
			vnode = reinterpret_cast<vtNode *>(m->getField().getValue());
		} else {
			vtNativeNode *native = new vtNativeNode(hitnode);
			vnode = native;
		}

		vtHit hit;
		hit.node = vnode;
		if( bLocalCoords ) {
			hit.point = s2v( (osg::Vec3f)hitpoint_l );
		} else {
			hit.point = s2v( (osg::Vec3f)hitpoint_g );
		}
		hit.distance =  (s2v (  (osg::Vec3f)hitpoint_g ) - start).Length();
		hitlist.push_back(hit);
	}
	delete act;
	return hitlist.size();

#if EXCEPT 
	// set up intersect visitor and create the line segment
	osgUtil::IntersectVisitor visitor;
	osgUtil::IntersectVisitor::HitList hlist;

	osg::ref_ptr<osg::LineSegment> segment = new osg::LineSegment;
	segment->set(v2s(start), v2s(end));
	visitor.addLineSegment(segment.get());

	// the accept() method does the intersection testing work
	osg::Node *osgnode = pTop->GetOsgNode();
	osgnode->accept(visitor);

	hlist = visitor.getHitList(segment.get());

	#if 0	// Diagnostic code
	for( osgUtil::IntersectVisitor::HitList::iterator hitr=hlist.begin();
	   hitr!=hlist.end(); ++hitr ) {
		if( hitr->_geode.valid() ) {
			if( hitr->_geode->getName().empty() )
				VTLOG("Geode %lx\n", hitr->_geode.get());
			else
				// the geodes are identified by name.
				VTLOG("Geode '%s'\n", hitr->_geode->getName().c_str());
		} else if( hitr->_drawable.valid() ) {
			VTLOG("Drawable class '%s'\n", hitr->_drawable->className());
		} else {
			osg::Vec3 point = hitr->getLocalIntersectPoint();
			VTLOG("Hitpoint %.1f %.1f %.1f\n", point.x(), point.y(), point.z());
		}
	}
	if( hlist.size() > 0 )
		VTLOG("\n");
	#endif

	// look through the node hits that OSG returned
	for( osgUtil::IntersectVisitor::HitList::iterator hitr=hlist.begin();
	   hitr!=hlist.end(); ++hitr ) {
		if( !hitr->_geode.valid() )
			continue;

		osg::Node *onode = hitr->_geode.get();
		vtNode *vnode = (vtNode *) (hitr->_geode->getUserData());

		if( vnode == NULL ) {
			// a bit radical here - wrap the OSG node in a VTLIB wrapper
			//  on the fly.  hope it doesn't get confused with refcounts.
			vtNativeNode *native = new vtNativeNode(onode);
			vnode = native;
		}
		// put it on the list of hit results
		vtHit hit;
		hit.node = vnode;
		if( bLocalCoords ) {
			hit.point = s2v(hitr->getLocalIntersectPoint());
		} else {
			hit.point = s2v(hitr->getWorldIntersectPoint());
		}
		hit.distance = (s2v(hitr->getWorldIntersectPoint()) - start).Length();
		hitlist.push_back(hit);
	}
	std::sort(hitlist.begin(), hitlist.end());
	return hitlist.size();
#endif //EXCEPT
	return 0;
}



