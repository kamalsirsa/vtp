//
// NodeOSG.cpp
//
// Encapsulate behavior for OSG scene graph nodes.
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"
#include <osg/Polytope>
#include <osg/LightSource>
#include <osg/Fog>
#include <osg/Projection>
#include <osg/Depth>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgUtil/Optimizer>

using namespace osg;

#define DEBUG_NODE_LOAD	0


///////////////////////////////////////////////////////////////////////
// vtNode
//

vtNode::vtNode()
{
	// Artificially increment our own reference count, so that OSG
	// won't try to delete us when it removes its reference to us.
//	ref();
}

vtNode::~vtNode()
{
}

void vtNode::Release()
{
	// shouldn't happen but... safety check anyway
	if (m_pNode == NULL)
		return;

	// remove the OSG node from its OSG group parent(s)
	unsigned int parents = m_pNode->getNumParents();
	if (parents)
	{
		Group *parent = m_pNode->getParent(0);
		parent->removeChild(m_pNode.get());
	}

	m_pFogStateSet = NULL;
	m_pFog = NULL;

#if DEBUG_NODE_LOAD
	VTLOG("Deleting Node: %lx (\"%s\")\n", this, m_pNode->getName().c_str());
#endif

	// Tell OSG that we're through with this node
	// The following statement calls unref() on m_pNode, which deletes
	//  the OSG node, which decrements its reference to us, which
	//  deletes us.
	m_pNode = NULL;
}

void vtNode::SetEnabled(bool bOn)
{
	m_pNode->setNodeMask(bOn ? 0xffffffff : 0);
}

bool vtNode::GetEnabled() const
{
	int mask = m_pNode->getNodeMask();
	return (mask != 0);
}

void vtNode::SetName2(const char *name)
{
	if (m_pNode != NULL)
		m_pNode->setName((char *)name);
}

const char *vtNode::GetName2() const
{
	if (m_pNode != NULL)
		return m_pNode->getName().c_str();
	else
		return NULL;
}

void vtNode::GetBoundBox(FBox3 &box)
{
	// TODO if needed
//	FBox3 b;
//	m_pTransform->GetBound(&b);
//	box = s2v(b);
}

void vtNode::GetBoundSphere(FSphere &sphere, bool bGlobal)
{
	BoundingSphere bs = m_pNode->getBound();
	s2v(bs, sphere);

	if (bGlobal)
	{
		// We must work our way up the tree to the root, accumulating
		//  the transforms, to get the sphere in the global reference frame.
		vtNode *node = this;
		while (node = node->GetParent(0))
		{
			vtTransform *trans = dynamic_cast<vtTransform *>(node);
			if (trans)
			{
				// Note that this isn't 100% complete; we should be
				//  transforming the radius as well, with scale.
				FMatrix4 mat;
				trans->GetTransform1(mat);
				FPoint3 result;
				mat.Transform(sphere.center, result);
				sphere.center = result;
			}
		}
	}
}

vtNode *vtNode::GetParent(int iParent)
{
	int num = m_pNode->getNumParents();
	if (iParent >= num)
		return NULL;
	osg::Group *parent = m_pNode->getParent(iParent);
	if (!parent)
		return NULL;
	return (vtNode *) (parent->getUserData());
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
 * \param iType Can be GL_LINEAR, GL_EXP or GL_EXP2 for linear or exponential
 *		increase of the fog density.
 */
void vtNode::SetFog(bool bOn, float start, float end, const RGBf &color, int iType)
{
	osg::StateSet *set = GetOsgNode()->getStateSet();
	if (!set)
	{
		m_pFogStateSet = new osg::StateSet;
		set = m_pFogStateSet.get();
		GetOsgNode()->setStateSet(set);
	}

	if (bOn)
	{
		Fog::Mode eType;
		switch (iType)
		{
		case GL_LINEAR: eType = Fog::LINEAR; break;
		case GL_EXP: eType = Fog::EXP; break;
		case GL_EXP2: eType = Fog::EXP2; break;
		default: return;
		}
		m_pFog = new Fog;
		m_pFog->setMode(eType);
		m_pFog->setDensity(0.25f);	// not used for linear
		m_pFog->setStart(start);
		m_pFog->setEnd(end);
		m_pFog->setColor(osg::Vec4(color.r, color.g, color.b, 1));

		set->setAttributeAndModes(m_pFog.get(), StateAttribute::OVERRIDE | StateAttribute::ON);
	}
	else
	{
		// turn fog off
		set->setModeToInherit(GL_FOG);
	}
}

void vtNode::SetOsgNode(Node *n)
{
	m_pNode = n;
	if (m_pNode.valid())
		m_pNode->setUserData((vtNode *)this);
}

// Walk an OSG scenegraph looking for Texture states, and disable mipmap.
class MipmapVisitor : public NodeVisitor
{
public:
	MipmapVisitor() : NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
	virtual void apply(osg::Geode& geode)
	{
		for (unsigned i=0; i<geode.getNumDrawables(); ++i)
		{
			osg::Geometry *geo = dynamic_cast<osg::Geometry *>(geode.getDrawable(i));
			if (!geo) continue;

			StateSet *stateset = geo->getStateSet();
			if (!stateset) continue;

			StateAttribute *state = stateset->getTextureAttribute(0, StateAttribute::TEXTURE);
			if (!state) continue;

			Texture2D *texture = dynamic_cast<Texture2D *>(state);
			if (texture)
				texture->setFilter(Texture::MIN_FILTER, Texture::LINEAR);
		}
		NodeVisitor::apply(geode);
	}
};

// Our own cache of models loaded from OSG
static std::map<vtString, ref_ptr<Node> > m_ModelCache;
bool vtNode::s_bDisableMipmaps = false;

vtNode *vtNode::LoadModel(const char *filename, bool bAllowCache, bool bDisableMipmaps)
{
	// Some of OSG's file readers, such as the Wavefront OBJ reader, have
	//  sensitivity to stdio issues with '.' and ',' in European locales.
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// Workaround for OSG's OBJ-MTL reader which doesn't like backslashes
	vtString fname = filename;
	for (int i = 0; i < fname.GetLength(); i++)
	{
		if (fname.GetAt(i) == '\\') fname.SetAt(i, '/');
	}

	// We must track whether we have loaded this object already; that is, whether
	//  it is in the OSG object cache.  If it is in the cache, then we musn't
	//  apply the rotation below, because it's already been applied to the
	//  one in the cache that we've gotten again.
	Node *node, *existing_node = NULL;
	bool bInCache = (m_ModelCache.count(fname) != 0);
	if (bInCache)
		existing_node = m_ModelCache[fname].get();

	bool bDoLoad = (!bInCache || !bAllowCache);
	if (bDoLoad)
	{
#define HINT osgDB::Registry::CacheHintOptions
		// In case of reloading a previously loaded model, we must empty
		//  our own cache as well as disable OSG's cache.
		osgDB::Registry *reg = osgDB::Registry::instance();
		HINT hint = reg->getUseObjectCacheHint();
		if (bAllowCache)
			hint = (HINT) (hint | (osgDB::Registry::CACHE_NODES));
		else
			hint = (HINT) (hint & ~(osgDB::Registry::CACHE_NODES));
		reg->setUseObjectCacheHint(hint);

		// Now actually request the node from OSG
#if _DEBUG
		VTLOG("[");
#endif
		node = osgDB::readNodeFile((const char *)fname);
#if _DEBUG
		VTLOG("]");
#endif
		if (!node)
			return NULL;
	}
	else
	{
		// Simple case: use cached node
		node = existing_node;
	}

#if DEBUG_NODE_LOAD
	VTLOG("LoadModel: osg node %lx (rc %d), ", node, node->referenceCount());
#endif

	if (bDoLoad)
	{
		// We must insert a 'Normalize' state above the geometry objects
		// that we load, otherwise when they are scaled, the vertex normals
		// will cause strange lighting.  Fortunately, we only need to create
		// a single State object which is shared by all loaded models.
		static 	StateSet *normstate = NULL;
		if (!normstate)
		{
			normstate = new StateSet;
			normstate->setMode(GL_NORMALIZE, StateAttribute::ON);
		}
		node->setStateSet(normstate);

		if (bDisableMipmaps || s_bDisableMipmaps)
		{
			MipmapVisitor visitor;
			node->accept(visitor);
		}

		// We must insert a rotation transform above the model, because OSG's
		//  file loaders (now mostly consistently) tweak the model to put Z
		//  up, and the VTP uses OpenGL coordinates which has Y up.
		float fRotation = -PID2f;

		// Except for the OSG OBJ reader, which needs an additional rotation
		//  offset, since it expect OBJ model to have Y up, but the ones i've
		//  seen have Z up.
		if (osgDB::getLowerCaseFileExtension( filename ) == "obj")
			fRotation = -PIf;

		osg::MatrixTransform *transform = new osg::MatrixTransform;
		transform->setMatrix(osg::Matrix::rotate(fRotation, Vec3(1,0,0)));
		transform->addChild(node);
		// it's not going to change, so tell OSG that it can be optimized
		transform->setDataVariance(osg::Object::STATIC);

		// Now do some OSG voodoo, which should spread the transform downward
		//  through the loaded model, and delete the transform.
		osg::Group *group = new osg::Group;
		group->addChild(transform);

		osgUtil::Optimizer optimizer;
		optimizer.optimize(group, osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS);

		node = group;

#if DEBUG_NODE_LOAD
	VTLOG("group %lx (rc %d), ", node, node->referenceCount());
#endif

		// Store the node in the cache by filename so we'll know next
		//  time that we have already have it
		m_ModelCache[fname] = node;
	}

	// Since there must be a 1-1 correspondence between VTP nodes
	//  at OSG nodes, we can't have multiple VTP nodes sharing a
	//  single OSG node.  So, we can't simply use the OSG node ptr
	//  that we get: we must wrap it.
	osg::Group *container_group = new osg::Group();
	container_group->addChild(node);

#if DEBUG_NODE_LOAD
	VTLOG("container %lx (rc %d), ", container_group, container_group->referenceCount());
#endif

	// The final resulting node is the container of that operation
	vtNode *pNode = new vtNode();
	pNode->SetOsgNode(container_group);
	pNode->SetName2(fname);

#if DEBUG_NODE_LOAD
	VTLOG("VTP node %lx\n", pNode);
#endif

	return pNode;
}

void vtNode::ClearOsgModelCache()
{
	// Each model in the cache, at exit time, should have a refcount
	//  of 1.  Deleting the cache will push them to 0 and delete them.
	m_ModelCache.clear();
}

///////////////////////////////////////////////////////////////////////
// vtGroup
//

vtGroup::vtGroup(bool suppress) : vtNode(), vtGroupBase()
{
	if (!suppress)
	{
		m_pGroup = new Group;
		SetOsgNode(m_pGroup.get());
	}
}

vtGroup::~vtGroup()
{
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
	for (int i = 0; i < rhs->GetNumChildren(); i++)
	{
		vtNode *child = rhs->GetChild(i);
		if (child)
			AddChild(child);
		else
		{
			// Might be an internal (native OSG) node - try to cope with that
			const Group *rhsGroup = rhs->GetOsgGroup();
			Node *pOsgChild = const_cast<Node *>( rhsGroup->getChild(i) );
			if (pOsgChild)
				m_pGroup->addChild(pOsgChild);
		}
	}
}

void vtGroup::Release()
{
	// Check if there are no more external references to this group node.
	// If so, clean up the VTP side of the scene graph.
	int count = m_pGroup.get()->referenceCount();
	if (count == 2)
	{
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
		// Now destroy itself
		m_pGroup = NULL;	// decrease refcount
		vtNode::Release();
	}
	else
	{
		int foo = 1;
	}
}

vtNode *FindNodeByName(vtNode *node, const char *name)
{
	if (!strcmp(node->GetName2(), name))
		return node;

	const vtGroupBase *pGroup = dynamic_cast<const vtGroupBase *>(node);
	if (pGroup)
	{
		for (int i = 0; i < pGroup->GetNumChildren(); i++)
		{
			vtNode *pChild = pGroup->GetChild(i);
			vtNode *pResult = FindNodeByName(pChild, name);
			if (pResult)
				return pResult;
		}
	}
	return NULL;
}

const vtNodeBase *vtGroup::FindDescendantByName(const char *name) const
{
	return FindNodeByName((vtNode *)this, name);
}

void vtGroup::SetOsgGroup(Group *g)
{
	m_pGroup = g;
	SetOsgNode(g);
}

void vtGroup::AddChild(vtNode *pChild)
{
	if (pChild)
		m_pGroup->addChild(pChild->GetOsgNode());
}

void vtGroup::RemoveChild(vtNode *pChild)
{
	if (pChild)
		m_pGroup->removeChild(pChild->GetOsgNode());
}

vtNode *vtGroup::GetChild(int num) const
{
	int children = m_pGroup->getNumChildren();
	if (num >= 0 && num < children)
	{
		Node *pChild = (Node *) m_pGroup->getChild(num);
		return (vtNode *) (pChild->getUserData());
	}
	else
		return NULL;
}

int vtGroup::GetNumChildren() const
{
	// shoudln't happen but... safety check anyway
	if (m_pGroup == NULL)
		return 0;
	return m_pGroup->getNumChildren();
}

bool vtGroup::ContainsChild(vtNode *pNode) const
{
	int i, children = GetNumChildren();
	for (i = 0; i < children; i++)
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
	m_pTransform = new CustomTransform;
	SetOsgGroup(m_pTransform.get());
}

vtTransform::~vtTransform()
{
}

vtNodeBase *vtTransform::Clone()
{
	vtTransform *trans = new vtTransform();
	trans->CopyFrom(this);
	return trans;
}

void vtTransform::CopyFrom(const vtTransform *rhs)
{
	// copy the matrix
	const osg::MatrixTransform *mt = rhs->m_pTransform.get();
	m_pTransform->setMatrix(mt->getMatrix());

	// and the parent members
	vtGroup::CopyFrom(rhs);
}

void vtTransform::Release()
{
	// Check if there are no more external references to this transform node.
	// If so, clean up the VTP side of the scene graph.
	int count = m_pTransform.get()->referenceCount();
	if (count == 3)
	{
		m_pTransform = NULL;
		vtGroup::Release();
	}
	else
	{
		int foo = 1;
	}
}

void vtTransform::Identity()
{
	m_pTransform->setMatrix(Matrix::identity());
}

FPoint3 vtTransform::GetTrans() const
{
	Vec3 v = m_pTransform->getMatrix().getTrans();
	return FPoint3(v[0], v[1], v[2]);
}

void vtTransform::SetTrans(const FPoint3 &pos)
{
	m_pTransform->getMatrix().setTrans(pos.x, pos.y, pos.z);
	m_pTransform->dirtyBound();
}

void vtTransform::Translate1(const FPoint3 &pos)
{
	// OSG 0.8.43 and later
	m_pTransform->postMult(Matrix::translate(pos.x, pos.y, pos.z));
}

void vtTransform::TranslateLocal(const FPoint3 &pos)
{
	// OSG 0.8.43 and later
	m_pTransform->preMult(Matrix::translate(pos.x, pos.y, pos.z));
}

void vtTransform::Rotate2(const FPoint3 &axis, double angle)
{
	// OSG 0.8.43 and later
	m_pTransform->postMult(Matrix::rotate(angle, axis.x, axis.y, axis.z));
}

void vtTransform::RotateLocal(const FPoint3 &axis, double angle)
{
	// OSG 0.8.43 and later
	m_pTransform->preMult(Matrix::rotate(angle, axis.x, axis.y, axis.z));
}

void vtTransform::RotateParent(const FPoint3 &axis, double angle)
{
	// OSG 0.8.43 and later
	Vec3 trans = m_pTransform->getMatrix().getTrans();
	m_pTransform->postMult(Matrix::translate(-trans)*
			  Matrix::rotate(angle, axis.x, axis.y, axis.z)*
			  Matrix::translate(trans));
}

FPoint3 vtTransform::GetDirection() const
{
	const Matrix &xform = m_pTransform->getMatrix();
	const osg_matrix_value *ptr = xform.ptr();
	return FPoint3(-ptr[2], -ptr[6], -ptr[10]);
}

void vtTransform::Scale3(float x, float y, float z)
{
	// OSG 0.8.43 and later
	m_pTransform->preMult(Matrix::scale(x, y, z));
}

void vtTransform::SetTransform1(const FMatrix4 &mat)
{
	Matrix mat_osg;

	ConvertMatrix4(&mat, &mat_osg);

	m_pTransform->setMatrix(mat_osg);
	m_pTransform->dirtyBound();
}

void vtTransform::GetTransform1(FMatrix4 &mat) const
{
	const Matrix &xform = m_pTransform->getMatrix();
	ConvertMatrix4(&xform, &mat);
}

void vtTransform::PointTowards(const FPoint3 &point, bool bPitch)
{
	// OSG 0.8.43 and later
	Matrix matrix = m_pTransform->getMatrix();

	Vec3 trans = matrix.getTrans();

	Vec3 p;
	v2s(point, p);

	Vec3 diff = p - trans;
	float dist = diff.length();

	float theta = atan2f(-diff.z(), diff.x()) - PID2f;
	float phi;
	if (bPitch)
		phi = asinf(diff.y() / dist);
	else
		phi = 0;

	matrix.makeIdentity();
	matrix.preMult(Matrix::rotate(theta, 0.0f, 1.0f, 0.0f));
	matrix.preMult(Matrix::rotate(phi, 1.0f, 0.0f, 0.0f));
	matrix.postMult(Matrix::translate(trans));

	m_pTransform->setMatrix(matrix);
}


///////////////////////////////////////////////////////////////////////
// vtLight
//

vtLight::vtLight()
{
	// Lights can now go into the scene graph in OSG, with LightSource.
	m_pLightSource = new osg::LightSource;

	// A lightsource creates a light, which we will need to get at.
	m_pLight = (osg::Light *) m_pLightSource->getLight();

	SetOsgNode(m_pLightSource.get());
}

vtLight::~vtLight()
{
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
	SetColor(rhs->GetColor());
	SetAmbient(rhs->GetAmbient());
}

void vtLight::Release()
{
	m_pLight = NULL;	// explicit refcount decrement
	m_pLightSource = NULL;
	vtNode::Release();
}

void vtLight::SetColor(const RGBf &color)
{
	m_pLight->setDiffuse(v2s(color));
}

RGBf vtLight::GetColor() const
{
	return s2v(m_pLight->getDiffuse());
}

void vtLight::SetAmbient(const RGBf &color)
{
	m_pLight->setAmbient(v2s(color));
}

RGBf vtLight::GetAmbient() const
{
	return s2v(m_pLight->getAmbient());
}

void vtLight::SetEnabled(bool bOn)
{
/*
	if (bOn)
		m_pLightSource->setLocalStateSetModes(StateAttribute::ON);
	else
		m_pLightSource->setLocalStateSetModes(StateAttribute::OFF);

*/
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

vtCamera::~vtCamera()
{
}

void vtCamera::Release()
{
	vtTransform::Release();
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

void vtCamera::SetHither(float f)
{
	m_fHither = f;
}

float vtCamera::GetHither() const
{
	return m_fHither;
}

void vtCamera::SetYon(float f)
{
	m_fYon = f;
}

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

void vtCamera::ZoomToSphere(const FSphere &sphere)
{
	Identity();
	Translate1(sphere.center);
	Translate1(FPoint3(0.0f, 0.0f, sphere.radius));
}

void vtCamera::SetOrtho(bool bOrtho)
{
	m_bOrtho = bOrtho;
}

bool vtCamera::IsOrtho() const
{
	return m_bOrtho;
}

void vtCamera::SetWidth(float fWidth)
{
	m_fWidth = fWidth;
}

float vtCamera::GetWidth() const
{
	return m_fWidth;
}


///////////////////////////////////////////////////////////////////////
// vtGeom
//

vtGeom::vtGeom() : vtNode()
{
	m_pGeode = new Geode();
	SetOsgNode(m_pGeode.get());
}

vtGeom::~vtGeom()
{
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
	for (int i = 0; i < rhs->GetNumMeshes(); i++)
	{
		vtMesh *mesh = rhs->GetMesh(i);
		if (mesh)
		{
			idx = mesh->GetMatIndex();
			AddMesh(mesh, idx);
		}
		else
		{
			vtTextMesh *tm = rhs->GetTextMesh(i);
			if (tm)
			{
				idx = tm->GetMatIndex();
				AddTextMesh(tm, idx);
			}
		}
	}
}

void vtGeom::Release()
{
	if (m_pGeode != NULL)
	{
		// Release the meshes we contain, which will delete them if there are no
		//  other references to them.
		int i, num = m_pGeode->getNumDrawables();
		for (i = 0; i < num; i++)
		{
			vtMesh *mesh = GetMesh(i);
			if (mesh)
				mesh->Release();
			else
			{
				vtTextMesh *textmesh = GetTextMesh(i);
				if (textmesh)
					textmesh->Release();
			}
		}
		m_pGeode->removeDrawable(0, num);

		// dereference
		m_pGeode = NULL;
	}
	m_pMaterialArray = NULL;

	vtNode::Release();
}

void vtGeom::AddMesh(vtMesh *pMesh, int iMatIdx)
{
	m_pGeode->addDrawable(pMesh->m_pGeometry.get());

	// The vtGeom owns/references the meshes it contains
	pMesh->ref();

	SetMeshMatIndex(pMesh, iMatIdx);
}

void vtGeom::AddTextMesh(vtTextMesh *pTextMesh, int iMatIdx)
{
	// connect the underlying OSG objects
	m_pGeode->addDrawable(pTextMesh->m_pOsgText.get());

	pTextMesh->SetMatIndex(iMatIdx);
	vtMaterial *pMat = GetMaterial(iMatIdx);
	if (pMat)
	{
		StateSet *pState = pMat->m_pStateSet.get();
		pTextMesh->m_pOsgText->setStateSet(pState);
	}
}

void vtGeom::SetMeshMatIndex(vtMesh *pMesh, int iMatIdx)
{
	vtMaterial *pMat = GetMaterial(iMatIdx);
	if (pMat)
	{
		StateSet *pState = pMat->m_pStateSet.get();
		pMesh->m_pGeometry->setStateSet(pState);

		// Try to provide color for un-lit meshes
		if (!pMat->GetLighting())
		{
			// unless it's using vertex colors...
			Geometry::AttributeBinding bd = pMesh->m_pGeometry->getColorBinding();
			if (bd != Geometry::BIND_PER_VERTEX)
			{
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
}

void vtGeom::RemoveMesh(vtMesh *pMesh)
{
	m_pGeode->removeDrawable(pMesh->m_pGeometry.get());

	if (pMesh->_refCount == 2)
	{
		// no more references except its default
		// self-reference and the reflexive reference from its m_pGeometry.
		pMesh->Release();
	}
}

int vtGeom::GetNumMeshes() const
{
	return m_pGeode->getNumDrawables();
}

vtMesh *vtGeom::GetMesh(int i) const
{
	// It is valid to return a non-const pointer to the mesh, since the mesh
	//  can be modified entirely independently of the geometry.
	Drawable *draw = const_cast<Drawable *>( m_pGeode->getDrawable(i) );
	osg::Referenced *ref = draw->getUserData();

	vtMesh *mesh = dynamic_cast<vtMesh*>(ref);
	return mesh;
}

vtTextMesh *vtGeom::GetTextMesh(int i) const
{
	// It is valid to return a non-const pointer to the mesh, since the mesh
	//  can be modified entirely independently of the geometry.
	Drawable *draw = const_cast<Drawable *>( m_pGeode->getDrawable(i) );
	osg::Referenced *ref = draw->getUserData();

	vtTextMesh *mesh = dynamic_cast<vtTextMesh*>(ref);
	return mesh;
}

void vtGeom::SetMaterials(const class vtMaterialArray *mats)
{
	m_pMaterialArray = mats;	// increases reference count
}

const vtMaterialArray *vtGeom::GetMaterials() const
{
	return m_pMaterialArray.get();
}

vtMaterial *vtGeom::GetMaterial(int idx)
{
	if (!m_pMaterialArray.valid())
		return NULL;
	if (idx < 0 || idx >= (int) m_pMaterialArray->GetSize())
		return NULL;
	return m_pMaterialArray->GetAt(idx);
}


///////////////////////////////////////////////////////////////////////
// vtLOD
//

vtLOD::vtLOD() : vtGroup(true)
{
	m_pLOD = new osg::LOD();
	m_pLOD->setCenter(osg::Vec3(0, 0, 0));
	SetOsgGroup(m_pLOD.get());
}

vtLOD::~vtLOD()
{
}

void vtLOD::Release()
{
	m_pLOD = NULL;
	vtGroup::Release();
}

void vtLOD::SetRanges(float *ranges, int nranges)
{
	int i;
	float next;
	for (i = 0; i < nranges; i++)
	{
		if (i < nranges - 1)
			next = ranges[i+1];
		else
			next = 1E10;
		m_pLOD->setRange(i, ranges[i], next);
	}
}

void vtLOD::SetCenter(FPoint3 &center)
{
	Vec3 p;
	v2s(center, p);
	m_pLOD->setCenter(p);
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS


///////////////////////////////////////////////////////////////////////
// OsgDynMesh
//

OsgDynMesh::OsgDynMesh()
{
	// The following line code is a workaround provided by Robert Osfield himself
	// create an empty stateset, to force the traversers
	// to nest any state above it in the inheritance path.
	setStateSet(new StateSet);
}

OsgDynMesh::~OsgDynMesh()
{
}

bool OsgDynMesh::computeBound() const
{
	FBox3 box;
	m_pDynGeom->DoCalcBoundBox(box);

	v2s(box.min, _bbox._min);
	v2s(box.max, _bbox._max);

	_bbox_computed=true;
	return true;
}

State *hack_global_state = NULL;

void OsgDynMesh::drawImplementation(State& state) const
{
	hack_global_state = &state;

	vtScene *pScene = vtGetScene();
	vtCamera *pCam = pScene->GetCamera();

	// setup the culling planes
	m_pDynGeom->m_pPlanes = pScene->GetCullPlanes();

	m_pDynGeom->DoCull(pCam);
	m_pDynGeom->DoRender();

#if 0
	glBegin(GL_TRIANGLES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glEnd();
#endif
}

#endif // DOXYGEN_SHOULD_SKIP_THIS

vtDynGeom::vtDynGeom() : vtGeom()
{
	m_pDynMesh = new OsgDynMesh();
	m_pDynMesh->m_pDynGeom = this;
	m_pDynMesh->setSupportsDisplayList(false);

	m_pGeode->addDrawable(m_pDynMesh);
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
	for (i = 0; i < 4; i++)
	{
		float dist = m_pPlanes[i].Distance(sphere.center);
		if (dist >= sphere.radius)
			return 0;
		if ((dist < 0) &&
			(dist <= sphere.radius))
			vis = (vis << 1) | 1;
	}

	// Notify renderer that object is entirely within standard frustum, so
	// no clipping is necessary.
	if (vis == 0x0F)
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
	unsigned int vis = 0;

	// cull against standard frustum
	int i;
	for (i = 0; i < 4; i++)
	{
		float dist = m_pPlanes[i].Distance(point);
		if (dist > 0.0f)
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
	for (i = 0; i < 4; i++)
	{
		dist = m_pPlanes[i].Distance(point0);
		if (dist > fTolerance)
			outcode0 |= (1 << i);

		dist = m_pPlanes[i].Distance(point1);
		if (dist > fTolerance)
			outcode1 |= (1 << i);

		dist = m_pPlanes[i].Distance(point2);
		if (dist > fTolerance)
			outcode2 |= (1 << i);
	}
	if (outcode0 == 0 && outcode1 == 0 && outcode2 == 0)
		return VT_AllVisible;
	if (outcode0 != 0 && outcode1 != 0 && outcode2 != 0)
	{
		if ((outcode0 & outcode1 & outcode2) != 0)
			return 0;
		else
		{
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
	for (int i = 0; i < 4; i++)
	{
		float dist = m_pPlanes[i].Distance(point);
		if (dist > radius)
			return 0;			// entirely outside this plane
		if (dist < -radius)
			incode |= (1 << i);	// entirely inside this plane
	}
	if (incode == 0x0f)
		return VT_AllVisible;	// entirely inside all planes
	else
		return VT_Visible;
}


///////////////////////////////////////////////////////////////////////
// vtSprite.  TODO: clean up the code, it's very messy.

vtSprite::vtSprite()
{
	m_pMesh = NULL;

	m_geode = new osg::Geode();

	// Turn lighting off for the sprite
	osg::StateSet* stateset = m_geode->getOrCreateStateSet();
	stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

	// To ensure the sprite appears on top we can use osg::Depth to force
	//  the depth fragments to be placed at the front of the screen.
	stateset->setAttribute(new osg::Depth(osg::Depth::LESS,0.0,0.0001));

	osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
	modelview_abs->setReferenceFrame(osg::Transform::RELATIVE_TO_ABSOLUTE);
	modelview_abs->setMatrix(osg::Matrix::identity());
	modelview_abs->addChild(m_geode.get());

	// We can set the projection to pixels (0,width,0,height) or
	//	normalized (0,1,0,1)
	vtScene *pScene = vtGetScene();
	IPoint2 winsize = pScene->GetWindowSize();

	m_projection = new osg::Projection;
	m_projection->setMatrix(osg::Matrix::ortho2D(0, winsize.x, 0, winsize.y));
	m_projection->addChild(modelview_abs);

	SetOsgNode(m_projection.get());
}

vtSprite::~vtSprite()
{
}

void vtSprite::Release()
{
	if (m_geode != NULL)
	{
		// Release the meshes we contain, which will delete them if there are no
		//  other references to them.
		if (m_pMesh)
		{
			m_pMesh->Release();
			m_geode->removeDrawable(0, 1);
		}
		// dereference
		m_geode = NULL;
	}
	// Destroy itself
	m_projection = NULL;	// decrease refcount
	vtNode::Release();
}

void vtSprite::AddMesh(vtMesh *pMesh)
{
	m_geode->addDrawable(pMesh->m_pGeometry.get());

	// The vtSprite owns/references the meshes it contains
	pMesh->ref();
}

void vtSprite::AddTextMesh(vtTextMesh *pTextMesh)
{
	m_geode->addDrawable(pTextMesh->m_pOsgText.get());
}

vtNodeBase *vtSprite::Clone()
{
//	sprite->CopyFrom(this);
	return new vtSprite;
}

void vtSprite::SetText(const char *szText)
{
}

void vtSprite::SetImage(vtImage *pImage)
{
	m_pMesh = new vtMesh(GL_QUADS, VT_TexCoords, 4);
	m_pMesh->AddVertexUV(FPoint3(0,0,0), FPoint2(0,0));
	m_pMesh->AddVertexUV(FPoint3(0.5,0,0), FPoint2(1,0));
	m_pMesh->AddVertexUV(FPoint3(0.5,1,0), FPoint2(1,1));
	m_pMesh->AddVertexUV(FPoint3(0,1,0), FPoint2(0,1));
	m_pMesh->AddQuad(0, 1, 2, 3);

//	vtMaterial *pMat = new vtMaterial();
//	pMat->SetTexture(pImage);
//	StateSet *pState = pMat->m_pStateSet.get();
//	m_geode->setStateSet(pState);

	osg::StateSet* stateset = m_geode->getOrCreateStateSet();

	osg::Texture2D *texture = new osg::Texture2D();
	texture->setImage(pImage->m_pOsgImage.get());
	stateset->setTextureAttributeAndModes(0, texture, StateAttribute::ON);

	osg::BlendFunc *pBlendFunc = new BlendFunc;
	stateset->setAttributeAndModes(pBlendFunc, StateAttribute::ON);

	osg::AlphaFunc *pAlphaFunc = new AlphaFunc;
	pAlphaFunc->setFunction(AlphaFunc::GEQUAL,0.05f);
	stateset->setAttributeAndModes(pAlphaFunc, StateAttribute::ON );
	stateset->setRenderingHint(StateSet::TRANSPARENT_BIN);

	AddMesh(m_pMesh);
	m_pMesh->Release();	// pass ownership
}

void vtSprite::SetPosition(bool bPixels, float l, float t, float r, float b)
{
	if (!m_pMesh)
		return;

	vtScene *pScene = vtGetScene();
	IPoint2 size = pScene->GetWindowSize();

	if (!bPixels)
	{
		l *= size.x;
		r *= size.x;
		t *= size.y;
		b *= size.y;
	}
	m_pMesh->SetVtxPos(0, FPoint3(l, size.y-1 - b, 0));
	m_pMesh->SetVtxPos(1, FPoint3(r, size.y-1 - b, 0));
	m_pMesh->SetVtxPos(2, FPoint3(r, size.y-1 - t, 0));
	m_pMesh->SetVtxPos(3, FPoint3(l, size.y-1 - t, 0));

	m_pMesh->ReOptimize();
}

void vtSprite::SetWindowSize(int x, int y)
{
	m_projection->setMatrix(osg::Matrix::ortho2D(0,x,0,y));
}

