//
// NodeSSG.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

void vtNode::SetEnabled(bool bOn)
{
//	m_pModel->SetActive(bOn);	// TODO
	vtEnabledBase::SetEnabled(bOn);
}

void vtNode::SetSsgEntity(ssgEntity *ent)
{
	m_pEntity = ent;
	if (m_pEntity)
		m_pEntity->setUserData(this);
}

//void vtNode::RemoveFromParent()
//{
//	m_pEntity->getParent(0)->removeKid(m_pEntity);
//}

void vtNode::GetBoundBox(FBox3 &box)
{
	// TODO
//	FBox3 b;
//	m_pTransform->GetBound(&b);
//	box = d2v(b);
}

void vtNode::GetBoundSphere(FSphere &sphere)
{
	sgSphere *s = m_pEntity->getBSphere();
	s2v(s, sphere);
}

vtNode *vtNode::CreateClone()
{
	// TODO
	return new vtNode();
}

void vtNode::SetName2(const char *name)
{
	m_pEntity->setName((char *)name);
}

const char *vtNode::GetName2()
{
	return m_pEntity->getName();
}


//////////////////////////////////////////////////////////////////
// Group

vtGroup::vtGroup(bool suppress) : vtNode()
{
	if (suppress)
		m_pBranch = NULL;
	else
		m_pBranch = new ssgBranch();
	SetSsgEntity(m_pBranch);
}

void vtGroup::AddChild(vtNode *pChild)
{
	if (!pChild || !pChild->GetSsgEntity())
		return;

	// assume that this entity is actually a branch node
	((ssgBranch *)m_pEntity)->addKid(pChild->GetSsgEntity());
}

void vtGroup::RemoveChild(vtNode *pChild)
{
	if (!pChild || !pChild->GetSsgEntity())
		return;

	// assume that this entity is actually a branch node
	((ssgBranch *)m_pEntity)->removeKid(pChild->GetSsgEntity());
}

vtNode *vtGroup::GetChild(int num)
{
	ssgEntity *pEnt = m_pBranch->getKid(num);
	if (!pEnt) return NULL;

	ssgBase *pBase = pEnt->getUserData();
//	return (vtNode *) pEnt;
	return (vtNode *) pBase;
}

int vtGroup::GetNumChildren()
{
	return m_pEntity->getNumKids();
}


////////////////////////////////////////////////////////////////////

vtTransform::vtTransform() : vtGroup()
{
	SetSsgEntity(&m_Transform);
}

void vtTransform::Identity()
{
	sgMat4 xform;
	sgMakeIdentMat4(xform);
	m_Transform.setTransform(xform);
}

FPoint3 vtTransform::GetTrans()
{
	sgMat4 xform;
	m_Transform.getTransform(xform);

	FPoint3 p;
	s2v(xform[3], p);
	return p;
}

void vtTransform::SetTrans(const FPoint3 &pos)
{
	sgVec3 pos2;
	v2s(pos, pos2);

	sgMat4 xform;
	m_Transform.getTransform(xform);

	sgCopyVec3(xform[3], pos2);

	m_Transform.setTransform(xform);
}

void vtTransform::Translate1(const FPoint3 &pos)
{
	sgVec3 pos2;
	v2s(pos, pos2);

	sgMat4 xform;
	m_Transform.getTransform(xform);

	sgAddVec3(xform[3], pos2);

	m_Transform.setTransform(xform);
}

void vtTransform::TranslateLocal(const FPoint3 &pos)
{
	sgVec3 pos2;
	v2s(pos, pos2);

	sgMat4 xform;
	m_Transform.getTransform(xform);

	sgMat4 trans;
	sgMakeTransMat4(trans, pos2);

	sgPreMultMat4(xform, trans);

	m_Transform.setTransform(xform);
}

void vtTransform::Rotate2(const FPoint3 &axis, float angle)
{
	sgMat4 xform;
	m_Transform.getTransform(xform);

	sgMat4 rot;
	sgVec3 axis2;
	v2s(axis, axis2);
	sgMakeRotMat4(rot, angle * 180.0f / PIf, axis2);

	sgPostMultMat4(xform, rot);

	m_Transform.setTransform(xform);
}

void vtTransform::RotateLocal(const FPoint3 &axis, float angle)
{
	sgMat4 xform;
	m_Transform.getTransform(xform);

	sgMat4 rot;
	sgVec3 axis2;
	v2s(axis, axis2);
	sgMakeRotMat4(rot, angle * 180.0f / PIf, axis2);

	sgPreMultMat4(xform, rot);

	m_Transform.setTransform(xform);
}

void vtTransform::RotateParent(const FPoint3 &axis, float angle)
{
	sgMat4 xform;
	m_Transform.getTransform(xform);

	sgMat4 rot;
	sgVec3 axis2;
	v2s(axis, axis2);
	sgMakeRotMat4(rot, angle * 180.0f / PIf, axis2);

	sgVec3 translation;
	sgCopyVec3(translation, xform[3]);
	sgSetVec3(xform[3], 0.0f, 0.0f, 0.0f);

	sgPostMultMat4(xform, rot);

	sgCopyVec3(xform[3], translation);

	m_Transform.setTransform(xform);
}

void vtTransform::Scale3(float x, float y, float z)
{
	FPoint3 scale(x, y, z);
	sgVec3 scale2;
	v2s(scale, scale2);

	sgMat4 xform;
	m_Transform.getTransform(xform);

	sgScaleVec3(xform[0], scale2[0]);
	sgScaleVec3(xform[1], scale2[1]);
	sgScaleVec3(xform[2], scale2[2]);

	m_Transform.setTransform(xform);
}

void vtTransform::SetTransform1(const FMatrix4 &mat)
{
	sgMat4 xform;
	ConvertMatrix4(&mat, xform);

	m_Transform.setTransform(xform);
}

void vtTransform::GetTransform1(FMatrix4 &mat)
{
	sgMat4 xform;
	m_Transform.getTransform(xform);

	ConvertMatrix4(xform, &mat);
}

/** Rotate this transform so that its -Z axis points toward the given point.
 */
void vtTransform::PointTowards(const FPoint3 &point)
{
	// TODO
}

//////////////////////////////////////////

vtLight::vtLight()
{
	SetSsgEntity(NULL);
	m_pLight = ssgGetLight(0);
}

//////////////////////////////////////////

vtGeom::vtGeom() : vtGeomBase(), vtNode()
{
//	m_pBranch = new ssgBranch();
	SetSsgEntity(&m_Branch);
}

void vtGeom::AddMesh(vtMesh *pMesh, int iMatIdx)
{
	m_Branch.addKid(pMesh->m_pVtx);

	vtMaterial *pMat = GetMaterial(iMatIdx);
	if (pMat)
	{
		ssgSimpleState *pState = (ssgSimpleState *)pMat;
		pMesh->m_pVtx->setState(pState);

		pMesh->m_pVtx->setCullFace(pMat->GetCulling());
	}

	pMesh->m_pVtx->recalcBSphere();

	pMesh->SetMatIndex(iMatIdx);
}

void vtGeom::SetMeshMatIndex(vtMesh *pMesh, int iMatIdx)
{
	vtMaterial *pMat = GetMaterial(iMatIdx);
	if (pMat)
	{
		// TODO
//		StateSet *pState = pMat->m_pStateSet.get();
//		pMesh->m_pGeoSet->setStateSet(pState);
	}
}

void vtGeom::RemoveMesh(vtMesh *pMesh)
{
	// TODO
//	m_pGeode->removeDrawable(pMesh->m_pGeoSet.get());
}

int vtGeom::GetNumMeshes()
{
	return m_Branch.getNumKids();
}

vtMesh *vtGeom::GetMesh(int i)
{
	ssgVtxTable *vtx = (ssgVtxTable *) m_Branch.getKid(i);
	vtMesh *pMesh = (vtMesh *) vtx->getUserData();

	return pMesh;
}


//////////////////////////////////////////

void vtLOD::SetRanges(float *ranges, int nranges)
{
	m_Selector.setRanges(ranges, nranges);
}

void vtLOD::SetCenter(FPoint3 &center)
{
	// TODO?  Not supported?
}

//////////////////////////////////////////////

#if 0
int __cdecl vtDynGeomCallback(ssgEntity *ent)
{
	vtDynGeom *pHack = (vtDynGeom *)ent;
	pHack->DoRender();
	// return FALSE to prevent the node from being drawn
	return FALSE;
}
#endif

vtDynGeom::vtDynGeom() : vtGeom()
{
//	setCallback(0, vtDynGeomCallback);
	DynLeaf *pDynLeaf = new DynLeaf();
	pDynLeaf->m_pDynGeom = this;
	m_Branch.addKid(pDynLeaf);
}

void DynLeaf::recalcBSphere()
{
	emptyBSphere();
	bbox.empty();

	FBox3 box;
	m_pDynGeom->DoCalcBoundBox(box);
//	bbox.extend(vertices->get(i));
	sgVec3 corner1, corner2;
	v2s(box.min, corner1);
	v2s(box.max, corner2);
	bbox.extend(corner1);
	bbox.extend(corner2);

	extendBSphere(&bbox);
	dirtyBSphere();  /* Cause parents to redo their bspheres */
	bsphere_is_invalid = FALSE;
}

void DynLeaf::print(FILE *fd, char *indent)
{
	// do nothing
    fprintf(fd, "DynLeaf\n");
}

void DynLeaf::cull(sgFrustum *f, sgMat4 m, int test_needed)
{
	// get camera position
	FPoint3 pos(m[3][0], m[3][1], m[3][2]);

	// get window size
	IPoint2 window_size;
	window_size.x = 640;
	window_size.y = 400;

	// get FOV
	SGfloat hfov, vfov;
	f->getFOV(&hfov, &vfov);
	float fov = hfov;

	FMatrix4 mat;
	ConvertMatrix4(m, &mat);

	// setup the culling planes
	m_pDynGeom->CalcCullPlanes(f, &mat);

	m_pDynGeom->DoCull(pos, window_size, fov);

	// the following will trigger the "draw" step if in view
	ssgLeaf::cull(f, m, test_needed);
}

void DynLeaf::draw()
{
	m_pDynGeom->DoRender();
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
void vtDynGeom::CalcCullPlanes(sgFrustum *pFrust, const FMatrix4* trans)
{
	// TODO
#if 0
	const Box3 vv = pCam->GetCullVol();

	FPoint3 c_loc (0, 0, 0);
	FPoint3 c_wvpr(0.0f, 0.0f, -vv.min.z);
	FPoint3 c_wvpn (0.0f, 0.0f, 1.0f);
	FPoint3 c_wvpu (0, 1.0f, 0.0f);

	// trans is the transform for world -> camera
	// invert it for camera -> world
	FMatrix4 camworld;
	camworld.Invert(*trans);

	// transform all the camera information into world frame
	FPoint3 loc, wvpr, wvpn, wvpu;
	camworld.Transform(c_loc, loc);
	camworld.TransformVector(c_wvpr, wvpr);
	camworld.TransformVector(c_wvpn, wvpn);
	camworld.TransformVector(c_wvpu, wvpu);

	FPoint3 uxn = wvpu.Cross(wvpn);

	FPoint3 p0, pa, tmp;
	FPoint3 lmv;

	float scll = vv.min.x; //cam.view.left;
	float sclr = vv.max.x; //cam.view.right;

	tmp.x = (wvpr.x + scll * uxn.x);		// vector to left edge
	tmp.y = (wvpr.y + scll * uxn.y);
	tmp.z = (wvpr.z + scll * uxn.z);
	pa =wvpu.Cross(tmp);
	pa.Normalize();
	cullPlanes[0].Set(loc, pa);	// left plane

	tmp.x = (wvpr.x + sclr * uxn.x);		// vector to right edge
	tmp.y = (wvpr.y + sclr * uxn.y);
	tmp.z = (wvpr.z + sclr * uxn.z);
	pa= tmp.Cross(wvpu);
	pa.Normalize();
	cullPlanes[1].Set(loc, pa);	// right plane

	tmp.x = (wvpr.x + vv.max.y * wvpu.x);		// vector to top edge
	tmp.y = (wvpr.y + vv.max.y * wvpu.y);
	tmp.z = (wvpr.z + vv.max.y * wvpu.z);
	pa= uxn.Cross(tmp);
	pa.Normalize( );
	cullPlanes[2].Set(loc, pa);	// top plane

	tmp.x = (wvpr.x + vv.min.y * wvpu.x);	// vector to bot edge
	tmp.y = (wvpr.y + vv.min.y * wvpu.y);
	tmp.z = (wvpr.z + vv.min.y * wvpu.z);
	pa= tmp.Cross(uxn);
	pa.Normalize();
	cullPlanes[3].Set(loc, pa);	// bottom plane
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
		float dist = cullPlanes[i].Distance(sphere.center);
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
		float dist = cullPlanes[i].Distance(point);
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
		dist = cullPlanes[i].Distance(point0);
		if (dist > fTolerance)
			outcode0 |= (1 << i);

		dist = cullPlanes[i].Distance(point1);
		if (dist > fTolerance)
			outcode1 |= (1 << i);

		dist = cullPlanes[i].Distance(point2);
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

int vtDynGeom::IsVisible(const FPoint3 &point, float radius)
{
	unsigned int incode = 0;

	// cull against standard frustum
	for (int i = 0; i < 4; i++)
	{
		float dist = cullPlanes[i].Distance(point);
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


///////////////////////////////////////

void ConvertMatrix4(const sgMat4 mat_sg, FMatrix4 *mat)
{
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
		{
			mat->Set(i, j, mat_sg[i][j]);
		}
}

void ConvertMatrix4(const FMatrix4 *mat, sgMat4 mat_sg)
{
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
		{
			mat_sg[i][j] = mat->Get(i, j);
		}
}

//////////////////////////////////////////////

void vtCamera::SetHither(float f)
{
	float h, y;
	ssgContext *con = ssgGetCurrentContext();

	con->getNearFar(&h, &y);
	h = f;
	con->setNearFar(h, y);
}

float vtCamera::GetHither()
{
	float h, y;
	ssgContext *con = ssgGetCurrentContext();

	con->getNearFar(&h, &y);
	return h;
}

void vtCamera::SetYon(float f)
{
	float h, y;
	ssgContext *con = ssgGetCurrentContext();

	con->getNearFar(&h, &y);
	y = f;
	con->setNearFar(h, y);
}

float vtCamera::GetYon()
{
	float h, y;
	ssgContext *con = ssgGetCurrentContext();

	con->getNearFar(&h, &y);
	return y;
}

#if 0
FPoint2 vtCamera::GetHitherSize()
{
	ssgContext *con = ssgGetCurrentContext();
	sgFrustum *frust = con->getFrustum();

	FPoint2 size;
	size.x = frust->getLeft() * 2.0f;
	size.y = frust->getBot() * 2.0f;
	return size;
}
#endif

void vtCamera::SetFOV(float f)
{
	float w, h;
	ssgContext *con = ssgGetCurrentContext();

	con->getFOV(&w, &h);
	w = f;
	con->setFOV(w, f);
}

float vtCamera::GetFOV()
{
	float w, h;
	ssgContext *con = ssgGetCurrentContext();

	con->getFOV(&w, &h);
	return w;
}

void vtCamera::ZoomToSphere(const FSphere &sphere)
{
	Identity();
	Translate1(sphere.center);
	Translate1(FPoint3(0.0f, 0.0f, sphere.radius));
}

void vtCamera::SetOrtho(float fWidth)
{
	// TODO: change to an orthographic camera
}


//////////////////////////////////////////////

void vtLight::SetColor2(const RGBf &color)
{
	// TODO
}

void vtLight::SetAmbient2(const RGBf &color)
{
}

//////////////////////////////////////////////

void vtSprite::SetText(const char *msg)
{
}


