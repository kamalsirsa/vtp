//
// vtNode and subclasses
//
// Encapsulates behavior for scene graph nodes
//

#include "vtlib/vtlib.h"

#include <sgl/sglPerspectiveCamera.hpp>
#include <sgl/sglOrthographicCamera.hpp>


void vtNode::SetEnabled(bool bOn)
{
//	m_pModel->SetActive(bOn);	// TODO
	vtEnabledBase::SetEnabled(bOn);
}

void vtNode::GetBoundBox(FBox3 &box)
{
	// TODO
//	FBox3 b;
//	m_pTransform->GetBound(&b);
//	box = d2v(b);
}

void vtNode::GetBoundSphere(FSphere &sphere)
{
	const sglSphereBoundf &s = m_pNode->getBound();
	s2v(s, sphere);
}

vtNodeBase *vtNode::CreateClone()
{
	// TODO
	return new vtNode();
}

void vtNode::SetName2(const char *name)
{
	m_pNode->setName((char *)name);
}

const char *vtNode::GetName2()
{
	std::string foo1 = m_pNode->getName();
	return foo1.c_str();
}

void vtNode::SetSglNode(sglNode *n)
{
	m_pNode = n;
	if (n)
		n->setUserData((sglUserData *)this);
}

////////////////////////////////////////////////////

vtGroup::vtGroup(bool suppress) : vtNode(), vtGroupBase()
{
	if (suppress)
		m_pGroup = NULL;
	else
		m_pGroup = new sglGroup;
	SetSglNode(m_pGroup);
}

void vtGroup::AddChild(vtNodeBase *pChild)
{
	vtNode *pChildNode = dynamic_cast<vtNode *>(pChild);
	if (pChildNode)
		m_pGroup->addChild(pChildNode->GetSglNode());
}

void vtGroup::RemoveChild(vtNodeBase *pChild)
{
	vtNode *pChildNode = dynamic_cast<vtNode *>(pChild);
	if (pChildNode)
		m_pGroup->removeChild(pChildNode->GetSglNode());
}

vtNode *vtGroup::GetChild(int num)
{
	sglNode *pChild = m_pGroup->getChild(num);
	if (pChild)
	{
		vtNode *pNode = (vtNode *) pChild->getUserData();
		return pNode;
	}
	return NULL;
}

int vtGroup::GetNumChildren()
{
	return m_pGroup->getNumChildren();
}


////////////////////////////////////////////////////////

vtTransform::vtTransform() : vtTransformBase()
{
	m_pTrans = new sglTransformf;
}

vtTransform::~vtTransform()
{
	delete m_pTrans;
}

void vtTransform::Identity()
{
	sglMat4f xform;
	xform.buildIdentity();
	m_pTrans->setMatrix(xform);
}

FPoint3 vtTransform::GetTrans()
{
	const sglMat4f &xform = m_pTrans->getMatrix();
	FPoint3 p(xform[3][0], xform[3][1], xform[3][2]);
	return p;
}

void vtTransform::SetTrans(const FPoint3 &pos)
{
	sglMat4f xform = m_pTrans->getMatrix();
	xform[3][0] = pos.x;
	xform[3][1] = pos.y;
	xform[3][2] = pos.z;
	m_pTrans->setMatrix(xform);
}

void vtTransform::Translate1(const FPoint3 &pos)
{
	sglMat4f xform = m_pTrans->getMatrix();
	xform[3][0] += pos.x;
	xform[3][1] += pos.y;
	xform[3][2] += pos.z;
	m_pTrans->setMatrix(xform);
}

void vtTransform::TranslateLocal(const FPoint3 &pos)
{
	sglMat4f xform = m_pTrans->getMatrix();

	sglMat4f trans, result;
	trans.buildTranslation(pos.x, pos.y, pos.z);

	result = trans * xform;
	m_pTrans->setMatrix(result);
}

void vtTransform::Rotate2(const FPoint3 &axis, float angle)
{
	sglMat4f xform = m_pTrans->getMatrix();

	sglMat4f rot, result;
	rot.buildRotation(axis.x, axis.y, axis.z, angle);	// radians!  thank goodness

	result = xform * rot;
	m_pTrans->setMatrix(result);
}

void vtTransform::RotateLocal(const FPoint3 &axis, float angle)
{
	sglMat4f xform = m_pTrans->getMatrix();

	sglMat4f rot, result;
	rot.buildRotation(axis.x, axis.y, axis.z, angle);	// radians!  thank goodness

	result = rot * xform;
	m_pTrans->setMatrix(result);
}

void vtTransform::RotateParent(const FPoint3 &axis, float angle)
{
	sglMat4f xform = m_pTrans->getMatrix();

	sglVec3f trans(xform[3][0], xform[3][1], xform[3][2]);

	xform[3][0] = xform[3][1] = xform[3][2] = 0.0f;

	sglMat4f delta;
	delta.buildRotation(axis.x, axis.y, axis.z, -angle);
	xform *= delta;

	xform[3][0] = trans[0];
	xform[3][1] = trans[1];
	xform[3][2] = trans[2];
	m_pTrans->setMatrix(xform);
}

void vtTransform::Scale3(float x, float y, float z)
{
	sglMat4f xform = m_pTrans->getMatrix();

	sglMat4f scale, result;
	scale.buildScale(x, y, z);

	result = xform * scale;
	m_pTrans->setMatrix(result);
}

void vtTransform::SetTransform1(const FMatrix4 &mat)
{
	sglMat4f xform;
	ConvertMatrix4(mat, xform);

	m_pTrans->setMatrix(xform);
}

void vtTransform::GetTransform1(FMatrix4 &mat)
{
	sglMat4f xform = m_pTrans->getMatrix();

	ConvertMatrix4(xform, mat);
}

void vtTransform::PointTowards(const FPoint3 &point)
{
	sglMat4f xform = m_pTrans->getMatrix();

	sglVec3f trans(xform[3][0], xform[3][1], xform[3][2]);

	sglVec3f p;
	v2s(point, p);

	sglVec3f diff = p - trans;
	float dist = diff.length();

	float theta = atan2f(-diff[2], diff[0]) - PID2f;
	float phi = asinf(diff[1] / dist);

	xform.buildIdentity();

	sglMat4f delta;
	delta.buildRotation(0.0f, 1.0f, 0.0f, -theta);
	xform *= delta;

	delta.buildRotation(1.0f, 0.0f, 0.0f, -phi);
	xform *= delta;

	delta.buildTranslation(trans[0], trans[1], trans[2]);
	xform *= delta;

	m_pTrans->setMatrix(xform);
}


//////////////////////////////////////////

vtGeom::vtGeom() : vtGeomBase(), vtNode()
{
	m_pGeode = new sglGeode;
	SetSglNode(m_pGeode);
}

void vtGeom::AddMesh(vtMesh *pMesh, int iMatIdx)
{
	m_pGeode->addGeometry(pMesh->m_pGeoSet);
//	pMesh->m_pVtx->recalcBSphere();

	vtMaterial *pMat = GetMaterial(iMatIdx);
	if (pMat)
	{
		pMesh->m_pGeoSet->setState(0, pMat->m_state, NULL);
	}
	pMesh->SetMatIndex(iMatIdx);
}

void vtGeom::SetMeshMatIndex(vtMesh *pMesh, int iMatIdx)
{
	vtMaterial *pMat = GetMaterial(iMatIdx);
	if (pMat)
	{
		// TODO?
	}
	pMesh->SetMatIndex(iMatIdx);
}

void vtGeom::RemoveMesh(vtMesh *pMesh)
{
	m_pGeode->removeGeometry(pMesh->m_pGeoSet);
}

int vtGeom::GetNumMeshes()
{
	return m_pGeode->getNumGeometries();
}

vtMesh *vtGeom::GetMesh(int i)
{
	sglDrawable *geom = m_pGeode->getGeometry(i);
	if (geom)
	{
		vtMesh *pMesh = (vtMesh *) geom->getUserData();
		return pMesh;
	}
	return NULL;
}

//////////////////////////////////////////

void vtLOD::SetRanges(float *ranges, int nranges)
{
	for (int i = 0; i < nranges; i++)
		m_pLOD->setRange(i, ranges[i]);
}

void vtLOD::SetCenter(FPoint3 &center)
{
	sglVec3f center2;
	v2s(center, center2);
	m_pLOD->setCenter(center2);
}

//////////////////////////////////////////////

vtDynDrawable::vtDynDrawable()
{
	m_pDynGeom = NULL;
}

const sglBoxBound &vtDynDrawable::getBound()
{
	FBox3 box;
	m_pDynGeom->DoCalcBoundBox(box);

	m_bbox.setMin(box.min.x, box.min.y, box.min.z);
	m_bbox.setMax(box.max.x, box.max.y, box.max.z);

	return m_bbox;
}

sglCurrState *hack_global_state;

void vtDynDrawable::drawGeometry(sglVec2f *tex_coords) const
{
	if (!m_pDynGeom)
		return;

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

void vtDynDrawable::addStats(sglStats &stats) const
{
	// TODO
}

bool vtDynDrawable::isValid() const
{
	return (m_pDynGeom != NULL);
}

vtDynGeom::vtDynGeom() : vtGeom()
{
	m_pDynDrawable = new vtDynDrawable();
	m_pDynDrawable->m_pDynGeom = this;

	// TODO
//	m_pGeode->addGeometry(m_pDynDrawable);
}

///////////////////////////////////////
//
// Calculate the culling planes, for the camera this frame,
// in world coordinates.
//
// This way, each point or sphere doesn't have to be converted
// to Camera coordinates before testing against the view volume
//
// We don't bother to calculate the near and far planes, since they
// aren't important for dynamic terrain.
//
void vtDynGeom::CalcCullPlanes()
{
	vtScene *pScene = vtGetScene();
	vtCamera *pCam = pScene->GetCamera();

#if 1
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
	// TODO? get planes directly from SGL
#endif
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

int vtDynGeom::IsVisible(FPoint3 point, float radius)
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


//////////////////////////////////////////////

vtCamera::vtCamera() : vtTransform()
{
	m_psglPerspCamera = new sglPerspectiveCamera();
	m_psglOrthoCamera = NULL;
	m_psglCamera = m_psglPerspCamera;

	m_psglViewPlatform = new sglViewPlatform(*m_psglCamera);
	m_pTrans->addChild(m_psglViewPlatform);
}

void vtCamera::SetHither(float f)
{
	double fovy, aspect, near_clip, far_clip;
	m_psglPerspCamera->getFOV(fovy, aspect, near_clip, far_clip);
	near_clip = f;
	m_psglPerspCamera->setFOV(fovy, aspect, near_clip, far_clip);
}

float vtCamera::GetHither()
{
	double fovy, aspect, near_clip, far_clip;
	m_psglPerspCamera->getFOV(fovy, aspect, near_clip, far_clip);
	return near_clip;
}

void vtCamera::SetYon(float f)
{
	double fovy, aspect, near_clip, far_clip;
	m_psglPerspCamera->getFOV(fovy, aspect, near_clip, far_clip);
	far_clip = f;
	m_psglPerspCamera->setFOV(fovy, aspect, near_clip, far_clip);
}

float vtCamera::GetYon()
{
	double fovy, aspect, near_clip, far_clip;
	m_psglPerspCamera->getFOV(fovy, aspect, near_clip, far_clip);
	return near_clip;
}

void vtCamera::SetFOV(float f)
{
	double fovy, aspect, near_clip, far_clip;
	m_psglPerspCamera->getFOV(fovy, aspect, near_clip, far_clip);
	fovy = f * aspect;
	m_psglPerspCamera->setFOV(fovy, aspect, near_clip, far_clip);
}

float vtCamera::GetFOV()
{
	double fovy, aspect, near_clip, far_clip;
	m_psglPerspCamera->getFOV(fovy, aspect, near_clip, far_clip);
	return fovy / aspect;
}

void vtCamera::ZoomToSphere(const FSphere &sphere)
{
	Identity();
	Translate1(sphere.center);
	Translate1(FPoint3(0.0f, 0.0f, sphere.radius));
}

void vtCamera::SetOrtho(float fWidth)
{
	m_psglOrthoCamera = new sglOrthographicCamera();
	m_psglCamera = m_psglOrthoCamera;
	m_psglViewPlatform->setCamera(*m_psglCamera);
}

//////////////////////////////////////////

vtLight::vtLight()
{
	m_pLight = new sglDirectionalLight;
	m_pLight->setDiffuse(1., 1., 1., 1.);
	m_pLight->setDirection(sglVec3f(0., 0., 1.));
	SetSglNode(m_pLight);
}

void vtLight::SetColor2(RGBf color)
{
	sglVec4f color2;
	v2s(color, color2);
	m_pLight->setDiffuse(color2);
}

void vtLight::SetAmbient2(RGBf &color)
{
	sglVec4f color2;
	v2s(color, color2);
	m_pLight->setAmbient(color2);
}

vtMovLight::vtMovLight(vtLight *pContained) : vtTransform()
{
	m_pLight = pContained;
	AddChild(m_pLight);
}


////////////////////////

// Sprite?
void vtSprite::SetText(const char *msg)
{
}

