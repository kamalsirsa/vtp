//
// NodeOSG.cpp
//
// Encapsulate behavior for OSG scene graph nodes.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"
#include <osg/Polytope>
#include <osg/LightSource>
#include <osg/Fog>

using namespace osg;

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

	// Tell OSG that we're through with this node
	// The following statement calls unref() on m_pNode, which deletes
	//  the OSG node, which decrements its reference to us, which would
	//  delete us, except for the fact that we artificially increase our
	//  own count.
	m_pNode = NULL;

	// Artificially decrement our own "reference count", should trigger
	// self-deletion
//	unref();

	// This allows us to control deletion
//	delete this;
}

void vtNode::SetEnabled(bool bOn)
{
	m_pNode->setNodeMask(bOn ? 0xffffffff : 0);
}

bool vtNode::GetEnabled()
{
	int mask = m_pNode->getNodeMask();
	return (mask != 0);
}

void vtNode::GetBoundBox(FBox3 &box)
{
	// TODO if needed
//	FBox3 b;
//	m_pTransform->GetBound(&b);
//	box = s2v(b);
}

void vtNode::GetBoundSphere(FSphere &sphere)
{
	BoundingSphere bs = m_pNode->getBound();
	s2v(bs, sphere);
}

vtNodeBase *vtNode::CreateClone()
{
	// TODO
	return new vtNode();
}

void vtNode::SetName2(const char *name)
{
	if (m_pNode != NULL)
		m_pNode->setName((char *)name);
}

const char *vtNode::GetName2()
{
	if (m_pNode != NULL)
		return m_pNode->getName().c_str();
	else
		return NULL;
}

void vtNode::SetOsgNode(Node *n)
{
	m_pNode = n;
	if (m_pNode.valid())
		m_pNode->setUserData((vtNode *)this);
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

vtNodeBase *vtGroup::FindDescendantByName(const char *name)
{
	if (!strcmp(GetName2(), name))
		return (dynamic_cast<vtNode *>(this));

	vtGroupBase *pGroup = dynamic_cast<vtGroupBase *>(this);
	if (pGroup)
	{
		for (int i = 0; i < pGroup->GetNumChildren(); i++)
		{
			vtNodeBase *pChild = pGroup->GetChild(i);
			vtGroup *pGroupChild = dynamic_cast<vtGroup *>(pChild);
			if (pGroupChild)
			{
				vtNodeBase *pResult = pGroupChild->FindDescendantByName(name);
				if (pResult)
					return pResult;
			}
		}
	}
	return NULL;
}

void vtGroup::Release()
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

void vtGroup::SetOsgGroup(Group *g)
{
	m_pGroup = g;
	SetOsgNode(g);
}

void vtGroup::AddChild(vtNodeBase *pChild)
{
	vtNode *pChildNode = dynamic_cast<vtNode *>(pChild);
	if (pChildNode)
		m_pGroup->addChild(pChildNode->GetOsgNode());
}

void vtGroup::RemoveChild(vtNodeBase *pChild)
{
	vtNode *pChildNode = dynamic_cast<vtNode *>(pChild);
	if (pChildNode)
		m_pGroup->removeChild(pChildNode->GetOsgNode());
}

vtNode *vtGroup::GetChild(int num)
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

int vtGroup::GetNumChildren()
{
	// shoudln't happen but... safety check anyway
	if (m_pGroup == NULL)
		return 0;
	return m_pGroup->getNumChildren();
}

bool vtGroup::ContainsChild(vtNodeBase *pNode)
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

void vtTransform::Release()
{
	m_pTransform = NULL;
	vtGroup::Release();
}

void vtTransform::Identity()
{
	m_pTransform->setMatrix(Matrix::identity());
}

FPoint3 vtTransform::GetTrans()
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

void vtTransform::Rotate2(const FPoint3 &axis, float angle)
{
	// OSG 0.8.43 and later
	m_pTransform->postMult(Matrix::rotate(angle, axis.x, axis.y, axis.z));
}

void vtTransform::RotateLocal(const FPoint3 &axis, float angle)
{
	// OSG 0.8.43 and later
	m_pTransform->preMult(Matrix::rotate(angle, axis.x, axis.y, axis.z));
}

void vtTransform::RotateParent(const FPoint3 &axis, float angle)
{
	// OSG 0.8.43 and later
	Vec3 trans = m_pTransform->getMatrix().getTrans();
	m_pTransform->postMult(Matrix::translate(-trans)*
			  Matrix::rotate(angle, axis.x, axis.y, axis.z)*
			  Matrix::translate(trans));
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

void vtTransform::GetTransform1(FMatrix4 &mat)
{
	Matrix &xform = m_pTransform->getMatrix();
	ConvertMatrix4(&xform, &mat);
}

void vtTransform::PointTowards(const FPoint3 &point)
{
	// OSG 0.8.43 and later
	Matrix matrix = m_pTransform->getMatrix();

	Vec3 trans = matrix.getTrans();

	Vec3 p;
	v2s(point, p);

	Vec3 diff = p - trans;
	float dist = diff.length();

	float theta = atan2f(-diff.z(), diff.x()) - PID2f;
	float phi = asinf(diff.y() / dist);

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

void vtLight::SetAmbient(const RGBf &color)
{
	m_pLight->setAmbient(v2s(color));
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

void vtCamera::SetHither(float f)
{
	m_fHither = f;
}

float vtCamera::GetHither()
{
	return m_fHither;
}

void vtCamera::SetYon(float f)
{
	m_fYon = f;
}

float vtCamera::GetYon()
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
float vtCamera::GetFOV()
{
	return m_fFOV;
}

void vtCamera::GetDirection(FPoint3 &dir)
{
	FMatrix4 mat;
	GetTransform1(mat);
	FPoint3 forward(0, 0, -1);
	mat.TransformVector(forward, dir);
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

bool vtCamera::IsOrtho()
{
	return m_bOrtho;
}

void vtCamera::SetWidth(float fWidth)
{
	m_fWidth = fWidth;
}

float vtCamera::GetWidth()
{
	return m_fWidth;
}


///////////////////////////////////////////////////////////////////////
// Sprite: TODO?
void vtSprite::SetText(const char *msg)
{
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

void vtGeom::Release()
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
	m_pMaterialArray = NULL;
	m_pGeode = NULL;

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
	m_pGeode->addDrawable(pTextMesh->m_pOsgText.get());

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
}

int vtGeom::GetNumMeshes()
{
	return m_pGeode->getNumDrawables();
}

vtMesh *vtGeom::GetMesh(int i)
{
	Drawable *draw = m_pGeode->getDrawable(i);
	osg::Referenced *ref = draw->getUserData();

	vtMesh *mesh = dynamic_cast<vtMesh*>(ref);
	return mesh;
}

vtTextMesh *vtGeom::GetTextMesh(int i)
{
	Drawable *draw = m_pGeode->getDrawable(i);
	osg::Referenced *ref = draw->getUserData();

	vtTextMesh *mesh = dynamic_cast<vtTextMesh*>(ref);
	return mesh;
}

void vtGeom::SetMaterials(class vtMaterialArray *mats)
{
	m_pMaterialArray = mats;	// increases reference count
}

vtMaterialArray	*vtGeom::GetMaterials()
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

	FPoint3 eyepos = pCam->GetTrans();
	IPoint2 window_size = pScene->GetWindowSize();
	double fov = pCam->GetFOV();

	// setup the culling planes
	m_pDynGeom->m_pPlanes = pScene->GetCullPlanes();

	m_pDynGeom->DoCull(eyepos, window_size, fov);
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


//
// Test a sphere against the view volume.
//
// Result: VT_AllVisible if entirely inside the volume,
//			VT_Visible if partly inside,
//			otherwise 0.
//
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


//
// Test a single point against the view volume.
//
//  Result: true if inside, false if outside.
//
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


//
// Test a 3d triangle against the view volume.
//
// Result: VT_AllVisible if entirely inside the volume,
//			VT_Visible if partly intersecting,
//			otherwise 0.
//
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

//
// Test a sphere against the view volume.
//
// Result: VT_AllVisible if entirely inside the volume,
//			VT_Visible if partly intersecting,
//			otherwise 0.
//
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

