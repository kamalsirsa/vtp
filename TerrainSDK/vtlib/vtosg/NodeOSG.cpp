//
// NodeOSG.cpp
//
// Encapsulate behavior for OSG scene graph nodes
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include <osg/ClippingVolume>

using namespace osg;

///////////////////////////////////////////////////////////////////////
// vtNode
//

void vtNode::SetEnabled(bool bOn)
{
	m_pNode->setNodeMask(bOn ? 0xffffffff : 0);
	vtEnabledBase::SetEnabled(bOn);
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

vtNode *vtNode::CreateClone()
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

void vtNode::SetOsgNode(osg::Node *n, bool bGroup )
{
	m_pNode = n;
	if (m_pNode.valid())
		m_pNode->setUserData((vtNode *)this);
	m_bGroup = bGroup;
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

void vtGroup::SetOsgGroup(Group *g)
{
	m_pGroup = g;
	SetOsgNode(g, true);
}

void vtGroup::AddChild(vtNode *pChild)
{
	// Add the child both to our own child array, and the osg group
	m_VtChildren.Append(pChild);
	m_pGroup->addChild(pChild->GetOsgNode());
}

void vtGroup::RemoveChild(vtNode *pChild)
{
	// Remove the child both from our own child array, and the osg group
	int i = m_VtChildren.Find(pChild);
	if (i >= 0)
		m_VtChildren.RemoveAt(i);
	m_pGroup->removeChild(pChild->GetOsgNode());
}

vtNode *vtGroup::GetChild(int num)
{
#if 0
	Node *pChild = (Node *) m_pGroup->getChild(num);
	return (vtNode *) (pChild->_userData);
#else
	if (num < m_VtChildren.GetSize())
		return m_VtChildren.GetAt(num);
	else
		return NULL;
#endif
}

int vtGroup::GetNumChildren()
{
#if 0
	return m_pGroup->getNumChildren();
#else
	return m_VtChildren.GetSize();
#endif
}


///////////////////////////////////////////////////////////////////////
// vtTransform
//

vtTransform::vtTransform() : vtGroup(true), vtTransformBase()
{
	m_pTransform = new Transform;
	m_pTransform->ref();
	SetOsgGroup(m_pTransform);
}

void vtTransform::Identity()
{
	Matrix xform;
	xform.makeIdent();
	m_pTransform->setMatrix(xform);
	m_pTransform->dirtyBound();
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
	Matrix &xform = m_pTransform->getMatrix();
	xform.postTrans(pos.x, pos.y, pos.z);
	m_pTransform->dirtyBound();
}

void vtTransform::TranslateLocal(const FPoint3 &pos)
{
	Matrix &xform = m_pTransform->getMatrix();
	xform.preTrans(pos.x, pos.y, pos.z);
	m_pTransform->dirtyBound();
}

void vtTransform::Rotate2(const FPoint3 &axis, float angle)
{
	Matrix &xform = m_pTransform->getMatrix();
	xform.postRot(-angle * 180.0f / PIf, axis.x, axis.y, axis.z);
	m_pTransform->dirtyBound();
}

void vtTransform::RotateLocal(const FPoint3 &axis, float angle)
{
	Matrix &xform = m_pTransform->getMatrix();
	xform.preRot(-angle * 180.0f / PIf, axis.x, axis.y, axis.z);
	m_pTransform->dirtyBound();
}

void vtTransform::RotateParent(const FPoint3 &axis, float angle)
{
	Matrix &xform = m_pTransform->getMatrix();
	float *mat = (float *) xform._mat[3];
	Vec3 trans(mat[0], mat[1], mat[2]);
	mat[0] = mat[1] = mat[2] = 0.0f;
	xform.postRot(-angle * 180.0f / PIf, axis.x, axis.y, axis.z);
	mat[0] = trans.x();
	mat[1] = trans.y();
	mat[2] = trans.z();
	m_pTransform->dirtyBound();
}

void vtTransform::Scale3(float x, float y, float z)
{
	Matrix &xform = m_pTransform->getMatrix();
	xform.preScale(x, y, z);
	m_pTransform->dirtyBound();
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
	Matrix &xform = m_pTransform->getMatrix();

	float *mat = (float *) xform._mat[3];
	Vec3 trans(mat[0], mat[1], mat[2]);

	Vec3 p;
	v2s(point, p);

	Vec3 diff = p - trans;
	float dist = diff.length();

	float theta = atan2f(-diff.z(), diff.x()) - PID2f;
	float phi = asinf(diff.y() / dist);

	xform.makeIdent();
	xform.preRot(-theta * 180.0f / PIf, 0.0f, 1.0f, 0.0f);
	xform.preRot(-phi * 180.0f / PIf, 1.0f, 0.0f, 0.0f);

	xform.postTrans(trans.x(), trans.y(), trans.z());
	m_pTransform->dirtyBound();
}


///////////////////////////////////////////////////////////////////////
// vtLight
//

vtLight::vtLight()
{
	// lights don't go into the scene graph in OSG (yet?)
//	m_pTransform->addChild(m_pLight);
//	m_pLight = new Light;
	m_pLight = vtGetScene()->m_pOsgSceneView->getLight();
}

void vtLight::SetColor2(const RGBf &color)
{
	m_pLight->setDiffuse(v2s(color));
}

void vtLight::SetAmbient2(const RGBf &color)
{
	m_pLight->setAmbient(v2s(color));
}

vtMovLight::vtMovLight(vtLight *pContained) : vtTransform()
{
	m_pLight = pContained;
//	m_pGroup->addChild(m_pGeom->m_pGeode);	// Light is not a OSG node
	vtGetScene()->AddMovLight(this);
}

///////////////////////////////////////////////////////////////////////
// vtCamera
//

vtCamera::vtCamera() : vtTransform()
{
	m_pOsgCamera = new Camera();

	// Increase reference count so it won't get undesirably deleted later
	m_pOsgCamera->ref();
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

void vtCamera::SetFOV(float fov_x)
{
	float aspect = m_pOsgCamera->calc_aspectRatio();
	float fov_y2 = atan(tan (fov_x/2) / aspect);

	m_pOsgCamera->setPerspective(fov_y2 * 2 * 180.0f / PIf,
		aspect, m_pOsgCamera->zNear(), m_pOsgCamera->zFar());
}

float vtCamera::GetFOV()
{
	float fov_x = m_pOsgCamera->calc_fovx();
	return (fov_x / 180.0f * PIf);
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
// Sprite?
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
	GeoSet *gs = (GeoSet *) m_pGeode->getDrawable(i);
//	vtMesh *pMesh = (vtMesh *) gs->_userData;
	GeoSet2 *gs2 = (GeoSet2 *) gs;
	vtMesh *pMesh = gs2->m_pMesh;
	return pMesh;
}


///////////////////////////////////////////////////////////////////////
// vtLOD
//

void vtLOD::SetRanges(float *ranges, int nranges)
{
	for (int i = 0; i < nranges; i++)
		m_pLOD->setRange(i, ranges[i]);
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

const bool vtDynMesh::computeBound() const
{
	FBox3 box;
	m_pDynGeom->DoCalcBoundBox(box);

	v2s(box.min, _bbox._min);
	v2s(box.max, _bbox._max);

    _bbox_computed=true;
    return true;
}

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
	// "get the view frustum clipping in model coordinates"
	const ClippingVolume &clipvol = pCam->m_pOsgCamera->getClippingVolume();
	const ClippingVolume::PlaneList &planes = clipvol.getPlaneList();

	int i = 0;
	for (ClippingVolume::PlaneList::const_iterator itr=planes.begin();
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

State *hack_global_state = NULL;

void vtDynMesh::drawImmediateMode(State& state)
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



