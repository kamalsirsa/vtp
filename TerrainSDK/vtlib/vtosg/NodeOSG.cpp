//
// NodeOSG.cpp
//
// Encapsulate behavior for OSG scene graph nodes.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include <osg/Polytope>
#include <osg/LightSource>

using namespace osg;

///////////////////////////////////////////////////////////////////////
// vtNode
//

vtNode::~vtNode()
{
}

void vtNode::Destroy()
{
	// Tell OSG that we're through with this node
	// The following statement calls unref() on m_pNode, which deletes
	//  the OSG node, which decrements its reference to us, which would
	//  delete us, except for the fact that we artificially increase our
	//  own count.
	m_pNode = NULL;

	// Artificially decrement our own "reference count", should trigger
	// self-deletion
	unref();

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

void vtNode::SetOsgNode(Node *n)
{
	m_pNode = n;
	if (m_pNode.valid())
	{
		// Artificially increment our own "reference count", so that OSG
		// won't try to delete us when it removes its reference to us.
		ref();
		m_pNode->setUserData((vtNode *)this);
	}
}


///////////////////////////////////////////////////////////////////////
// vtGroup
//

vtGroup::vtGroup(bool suppress) : vtNode(), vtGroupBase()
{
	if (suppress)
		m_pGroup = NULL;
	else
		m_pGroup = new Group;
	SetOsgNode(m_pGroup);
}

void vtGroup::Destroy()
{
	// Destroy children depth-first
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
			pChild->Destroy();
		}
	}
	// Now destroy itself
	vtNode::Destroy();
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
	SetOsgGroup(m_pTransform);
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

void vtLight::SetColor2(const RGBf &color)
{
	m_pLight->setDiffuse(v2s(color));
}

void vtLight::SetAmbient2(const RGBf &color)
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

vtMovLight::vtMovLight(vtLight *pContained) : vtTransform()
{
	m_pLight = pContained;
	AddChild(pContained);
//	m_pGroup->addChild(m_pLight->m_pLightSource);
}


///////////////////////////////////////////////////////////////////////
// vtCamera
//

vtCamera::vtCamera() : vtTransform()
{
	m_pOsgCamera = new Camera();

	// Increase reference count so it won't get undesirably deleted later?
//	m_pOsgCamera->ref();

#if 0
	// Tell OSG to use our transform as the location of the camera
	// (This is no longer possible as of OSG 0.9.3?)
	m_pOsgCamera->attachTransform(Camera::EYE_TO_MODEL,
		&m_pTransform->getMatrix());
#endif
}

vtCamera::~vtCamera()
{
	// don't manually deref?
//	m_pOsgCamera->unref();
}

void vtCamera::SetHither(float f)
{
	float fov1 = m_pOsgCamera->calc_fovx();

	m_pOsgCamera->setNearFar(f, m_pOsgCamera->zFar());

	float fov2 = m_pOsgCamera->calc_fovx();
}

float vtCamera::GetHither()
{
	return m_pOsgCamera->zNear();
}

void vtCamera::SetYon(float f)
{
	m_pOsgCamera->setNearFar(m_pOsgCamera->zNear(), f);
}

float vtCamera::GetYon()
{
	return m_pOsgCamera->zFar();
}

/**
 * Set the camera's horizontal field of view (FOV) in radians.
 */
void vtCamera::SetFOV(float fov_x)
{
	float aspect = m_pOsgCamera->calc_aspectRatio();
	float fov_y2 = atan(tan (fov_x/2) / aspect);

	// osg 0.8.42
//	m_pOsgCamera->setPerspective(fov_y2 * 2 * 180.0f / PIf,
//		aspect, m_pOsgCamera->zNear(), m_pOsgCamera->zFar());

	// OSG 0.8.43 and later
	m_pOsgCamera->setFOV(fov_x * 180.0f / PIf, fov_y2 * 2.0f * 180.0f / PIf,
		m_pOsgCamera->zNear(), m_pOsgCamera->zFar());
}

/**
 * Return the camera's horizontal field of view (FOV) in radians.
 */
float vtCamera::GetFOV()
{
	float fov_x = m_pOsgCamera->calc_fovx();

	// osg 0.8.42 and osg 0.8.43
	return (fov_x / 180.0f * PIf);
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

void vtCamera::SetOrtho(float fWidth)
{
	float fAspect = m_pOsgCamera->calc_aspectRatio();	// width/height
	float fHeight = fWidth / fAspect;
	m_pOsgCamera->setOrtho(-fWidth/2, fWidth, -fHeight/2, fHeight,
		m_pOsgCamera->zNear(), m_pOsgCamera->zFar());
}


///////////////////////////////////////////////////////////////////////
// Sprite: TODO?
void vtSprite::SetText(const char *msg)
{
}


///////////////////////////////////////////////////////////////////////
// vtGeom
//

vtGeom::vtGeom() : vtGeomBase(), vtNode()
{
	m_pGeode = new Geode();
	SetOsgNode(m_pGeode);
}

void vtGeom::AddMesh(vtMesh *pMesh, int iMatIdx)
{
	m_pGeode->addDrawable(pMesh->m_pGeoSet.get());

	SetMeshMatIndex(pMesh, iMatIdx);
}

void vtGeom::AddText(vtTextMesh *pMesh, int iMatIdx)
{
	m_pGeode->addDrawable(pMesh->m_pOsgText.get());

	vtMaterial *pMat = GetMaterial(iMatIdx);
	if (pMat)
	{
		StateSet *pState = pMat->m_pStateSet.get();
		pMesh->m_pOsgText->setStateSet(pState);
	}
}

void vtGeom::SetMeshMatIndex(vtMesh *pMesh, int iMatIdx)
{
	vtMaterial *pMat = GetMaterial(iMatIdx);
	if (pMat)
	{
		StateSet *pState = pMat->m_pStateSet.get();
		pMesh->m_pGeoSet->setStateSet(pState);

		// Try to provide color for un-lit meshes
		if (!pMat->GetLighting())
		{
			// unless it's using vertex colors...
			GeoSet::BindingType bd = pMesh->m_pGeoSet->getColorBinding();
			if (bd != GeoSet::BIND_PERVERTEX)
			{
				RGBAf color = pMat->GetDiffuse();
				Vec4 *lp = new Vec4;
				lp->set(color.r, color.g, color.b, color.a);

				pMesh->m_pGeoSet->setColors(lp);
				pMesh->m_pGeoSet->setColorBinding(GeoSet::BIND_OVERALL);
			}
		}
	}
	pMesh->SetMatIndex(iMatIdx);
}

void vtGeom::RemoveMesh(vtMesh *pMesh)
{
	m_pGeode->removeDrawable(pMesh->m_pGeoSet.get());
}

int vtGeom::GetNumMeshes()
{
	return m_pGeode->getNumDrawables();
}

vtMesh *vtGeom::GetMesh(int i)
{
	// Unfortunately we can't just store a backpointer to the vtMesh in the
	// GeoSet, since it does not have a mechanism to support this.  Instead,
	// we use a subclass called "GeoSet2" which provides the backpointer.
	Drawable *draw = m_pGeode->getDrawable(i);
	GeoSet2 *gs2 = dynamic_cast<GeoSet2*>(draw);
	if (gs2)
		return gs2->m_pMesh;
	else
		return NULL;
}


///////////////////////////////////////////////////////////////////////
// vtLOD
//

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


///////////////////////////////////////////////////////////////////////
// vtDynGeom
//

vtDynMesh::vtDynMesh()
{
	// The following line code is a workaround provided by Robert Osfield himself
	// create an empty stateset, to force the traversers
	// to nest any state above it in the inheritance path.
	setStateSet(new StateSet);
}

bool vtDynMesh::computeBound() const
{
	FBox3 box;
	m_pDynGeom->DoCalcBoundBox(box);

	v2s(box.min, _bbox._min);
	v2s(box.max, _bbox._max);

	_bbox_computed=true;
	return true;
}

State *hack_global_state = NULL;

void vtDynGeom::CalcCullPlanes()
{
	vtScene *pScene = vtGetScene();
	vtCamera *pCam = pScene->GetCamera();

#if 0
	// Non-API-Specific code - will work correctly as long as the Camera
	// methods are fully functional.
	FMatrix4 mat;
	pCam->GetTransform1(mat);

	IPoint2 window_size = pScene->GetWindowSize();
	assert(( window_size.x > 0 ) && ( window_size.y > 0 ));

	double fov = pCam->GetFOV();

	double aspect = (float)window_size.y / window_size.x;
	double hither = pCam->GetHither();

	double a = hither * tan(fov / 2);
	double b = a * aspect;

	FPoint3 vec_l(-a, 0, -hither);
	FPoint3 vec_r(a, 0, -hither);
	FPoint3 vec_t(0, b, -hither);
	FPoint3 vec_b(0, -b, -hither);

	vec_l.Normalize();
	vec_r.Normalize();
	vec_t.Normalize();
	vec_b.Normalize();

	FPoint3 up(0.0f, 1.0f, 0.0f);
	FPoint3 right(1.0f, 0.0f, 0.0f);

	FPoint3 temp;

	FPoint3 center;
	FPoint3 norm_l, norm_r, norm_t, norm_b;

	temp = up.Cross(vec_l);
	mat.TransformVector(temp, norm_l);

	temp = vec_r.Cross(up);
	mat.TransformVector(temp, norm_r);

	temp = right.Cross(vec_t);
	mat.TransformVector(temp, norm_t);

	temp = vec_b.Cross(right);
	mat.TransformVector(temp, norm_b);

	mat.Transform(FPoint3(0.0f, 0.0f, 0.0f), center);

	// set up m_cullPlanes in world coordinates!
	m_cullPlanes[0].Set(center, norm_l);
	m_cullPlanes[1].Set(center, norm_r);
	m_cullPlanes[2].Set(center, norm_t);
	m_cullPlanes[3].Set(center, norm_b);
#else
	// Get the view frustum clipping directly from OSG

	// OSG 0.8.44
//	const ClippingVolume &clipvol = pCam->m_pOsgCamera->getClippingVolume();
	// OSG 0.8.45
//	const ClippingVolume &clipvol = hack_global_state->getClippingVolume();
	// OSG 0.9.0
	const Polytope &clipvol1 = pCam->m_pOsgCamera->getViewFrustum();
//	const Polytope &clipvol2 = hack_global_state->getViewFrustum();

	// clipvol1 is the global camera frustum (in world coordinates)
	// clipvol2 is the camera's frustum after it's been transformed to the
	//		local coordinates.
	const Polytope::PlaneList &planes = clipvol1.getPlaneList();

	int i = 0;
	for (Polytope::PlaneList::const_iterator itr=planes.begin();
		itr!=planes.end(); ++itr)
	{
		// make a copy of the clipping plane
		Plane plane = *itr;

		// extract the OSG plane to our own structure
		Vec4 pvec = plane.asVec4();
		m_cullPlanes[i++].Set(-pvec.x(), -pvec.y(), -pvec.z(), -pvec.w());
	}
#endif
}

void vtDynMesh::drawImplementation(State& state) const
{
	hack_global_state = &state;

	vtScene *pScene = vtGetScene();
	vtCamera *pCam = pScene->GetCamera();

	FPoint3 eyepos = pCam->GetTrans();
	IPoint2 window_size = pScene->GetWindowSize();
	double fov = pCam->GetFOV();

	// setup the culling planes
	m_pDynGeom->CalcCullPlanes();

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

vtDynGeom::vtDynGeom() : vtGeom()
{
	m_pDynMesh = new vtDynMesh();
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
		float dist = m_cullPlanes[i].Distance(sphere.center);
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
		float dist = m_cullPlanes[i].Distance(point);
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
		dist = m_cullPlanes[i].Distance(point0);
		if (dist > fTolerance)
			outcode0 |= (1 << i);

		dist = m_cullPlanes[i].Distance(point1);
		if (dist > fTolerance)
			outcode1 |= (1 << i);

		dist = m_cullPlanes[i].Distance(point2);
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
		float dist = m_cullPlanes[i].Distance(point);
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

