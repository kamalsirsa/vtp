//
// NodePSM.cpp
//
// Encapsulate behavior for Parallel Scene Manager nodes.
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"


RGBf vtNodeBase::s_white;

///////////////////////////////////////////////////////////////////////
// vtLight
//

void vtLight::SetColor2(const RGBf &color)
{
	SetColor(PSCol4(color.r, color.g, color.b));
}

void vtLight::SetAmbient2(const RGBf &color)
{
}

///////////////////////////////////////////////////////////////////////
// vtCamera
//

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
	SetKind(CAM_Orthographic);
	float fAspect = GetAspect();	// width/height
	float fHeight = fWidth / fAspect;
	PSBox3	vv;

	vv.min.x = -fWidth;
	vv.max.x = fWidth;
	vv.min.y = -fHeight;
	vv.max.y = fHeight;
	SetViewVol(vv);
}


///////////////////////////////////////////////////////////////////////
// Sprite: TODO?
void vtSprite::SetText(const char *msg)
{
}


///////////////////////////////////////////////////////////////////////
// vtGeom
//

void vtGeom::SetMaterials(class vtMaterialArray *mats)
{
	vtGeomBase::SetMaterials(mats);
	SetAppearances(mats->m_pApps);
}

void vtGeom::AddMesh(vtMesh *pMesh, int iMatIdx)
{
	PSSurface* surf = GetSurface();
	if (surf == NULL)
	{
		surf = new PSSurface;
		SetSurface(surf);
	}
	pMesh->SetAppIndex(iMatIdx);
	surf->Append(pMesh);
}

void vtGeom::AddText(vtTextMesh *pMesh, int iMatIdx)
{
	PSSurface* surf = GetSurface();
	if (surf == NULL)
	{
		surf = new PSSurface;
		SetSurface(surf);
	}
	pMesh->SetAppIndex(iMatIdx);
	surf->Append(pMesh);
}

void vtGeom::SetMeshMatIndex(vtMesh *pMesh, int iMatIdx)
{
	pMesh->SetAppIndex(iMatIdx);
}

void vtGeom::RemoveMesh(vtMesh *pMesh)
{
	PSSurface* surf = GetSurface();

	if (surf)
	{
		PSSurface::Iter iter(surf);
		PSGeometry* geo;
		int	idx = -1;

		while (geo = iter.Next())
			if (geo == pMesh)
			{
				idx = iter.GetIndex() - 1;
				break;
			}
		if (idx >= 0)
			surf->RemoveAt(idx);
	}
}

int vtGeom::GetNumMeshes()
{
	PSSurface* surf = GetSurface();

	if (surf)
		return surf->GetSize();
	else
		return 0;
}

vtMesh *vtGeom::GetMesh(int i)
{
	PSSurface* surf = GetSurface();

	if (surf == NULL)
		return NULL;
	return (vtMesh*) surf->GetAt(i);
}


///////////////////////////////////////////////////////////////////////
// vtLOD
//

void vtLOD::SetRanges(float *ranges, int nranges)
{
	if (nranges == 0)
	{
		PSLevelOfDetail::SetRanges(NULL);
		return;
	}
	PSFloatArray* rangearray = new PSFloatArray(nranges);
	for (int i = 0; i < nranges; i++)
		rangearray->SetAt(i, ranges[i]);
	PSLevelOfDetail::SetRanges(rangearray);
}

void vtLOD::SetCenter(FPoint3 &center)
{
	// TODO: figure out what to do here
}


///////////////////////////////////////////////////////////////////////
// vtDynGeom
//

vtDynGeom::vtDynGeom() : vtGeom()
{
	PSSurface* surf = new PSSurface;
	vtDynMesh* mesh = new vtDynMesh(*this);
	surf->Append(mesh);
	SetSurface(surf);
	SetCulling(false);
}

void vtDynGeom::CalcCullPlanes()
{
	vtScene *pScene = vtGetScene();
	vtCamera *pCam = pScene->GetCamera();

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
	PSCamera* cam = PSGetScene()->GetCamera();
	PSSphere sp;
	v2p(sphere, sp);
	return cam->IsVisible(sp);
}


//
// Test a single point against the view volume.
//
//  Result: true if inside, false if outside.
//
bool vtDynGeom::IsVisible(const FPoint3& point) const
{
	unsigned int vis = 0;
	PSCamera* cam = PSGetScene()->GetCamera();
	const PSPlane* cullplanes = cam->GetCullPlanes();

	// cull against standard frustum
	int i;
	for (i = 0; i < 4; i++)
	{
		float dist = cullplanes[i].Distance(v2p(point));
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
	PSCamera* cam = PSGetScene()->GetCamera();
	const PSPlane* cullplanes = cam->GetCullPlanes();

	// cull against standard frustum
	int i;
	for (i = 0; i < 4; i++)
	{
		dist = cullplanes[i].Distance(v2p(point0));
		if (dist > fTolerance)
			outcode0 |= (1 << i);

		dist = cullplanes[i].Distance(v2p(point1));
		if (dist > fTolerance)
			outcode1 |= (1 << i);

		dist = cullplanes[i].Distance(v2p(point2));
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

#if 1
	// cull against standard frustum
	for (int i = 0; i < 4; i++)
	{
		float dist = m_cullPlanes[i].Distance(point);
		if (dist > radius)
			return 0;			// entirely outside this plane
		if (dist < -radius)
			incode |= (1 << i);	// entirely inside this plane
	}
#else
	PSCamera* cam = PSGetScene()->GetCamera();
	const PSPlane* cullplanes = cam->GetCullPlanes();

	// cull against standard frustum
	for (int i = 0; i < 4; i++)
	{
		float dist = cullplanes[i].Distance(v2p(point));
		if (dist > radius)
			return 0;			// entirely outside this plane
		if (dist < -radius)
			incode |= (1 << i);	// entirely inside this plane
	}
#endif
	if (incode == 0x0f)
		return VT_AllVisible;	// entirely inside all planes
	else
		return VT_Visible;
}

int vtDynMesh::GetNumFaces() const
{
	int nvtx = m_DynGeom.GetPolygonCount();
	if (nvtx < 0)
		return 0;
	return nvtx;
}

int vtDynMesh::GetNumVtx() const
{
	int nvtx = m_DynGeom.GetPolygonCount() * 3;
	if (nvtx < 0)
		return 0;
	return nvtx;
}

int vtDynMesh::Cull(const PSMatrix* mtx, PSScene* scene)
{
	PSCamera*	pCam = scene->GetCamera();
	PSVec3		eyepos = pCam->GetCenter();
	IPoint2		window_size = vtGetScene()->GetWindowSize();
	double		fov = pCam->GetFOV();

	m_DynGeom.CalcCullPlanes();

	m_DynGeom.DoCull(p2v(eyepos), window_size, fov);
	return PSGeometry::Cull(mtx, scene);
}

void vtDynMesh::Render(PSScene* scene, const PSAppearances* apps)
{
	m_DynGeom.DoRender();
}

bool vtDynMesh::GetBound(PSBox3* box) const
{
	FBox3 vbox;
	m_DynGeom.DoCalcBoundBox(vbox);
	v2p(vbox, *box);
	return !box->IsEmpty();
}

bool	vtDynMesh::Hit(const PSRay& ray, PSTriHitEvent* hitinfo) const
{
	return PSGeometry::Hit(ray, hitinfo);
}



