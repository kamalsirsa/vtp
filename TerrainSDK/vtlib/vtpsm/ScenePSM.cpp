//
// ScenePSM.cpp
//
// Implementation of vtScene for the PSM library
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/engine.h"
#include <time.h>

#define	SCENE_WireFrame	(OBJ_LASTUSED<<1)

extern void _psm_MakeWorld();

// There is always and only one global vtScene object
vtScene g_Scene;

vtScene *vtGetScene()
{
	return &g_Scene;
}

vtScene::vtScene()
{
	m_pPSS = NULL;
}

void vtScene::DoUpdate()
{
	DoEngines();
}

void vtScene::SetCamera(vtCamera* c)
{
	m_pCamera = c;
	if (m_pPSS) m_pPSS->SetCamera(c);
}

bool vtScene::Init()
{
	// Create the main frame window
	PSWorld3D* world = PSWorld3D::Get();

	if (world == NULL)				// create PSM World
		_psm_MakeWorld();
	world = PSWorld3D::Get();
	if (world == NULL)
		return false;
	if (!world->OnInit())			// initialize PSM World
		return false;

	SetCamera(new vtCamera());

	return true;
}

void vtScene::AllocInternalScene()
{
	m_pPSS = new PSScene();
	m_pPSS->ClearFlags(SCENE_WireFrame);
}

void vtScene::SetWindowSize(int w, int h)
{
	if (!m_pPSS)
		return;
	PSBox2	vport = m_pPSS->GetViewport();
	vport.max.x = vport.min.x + w;
	vport.max.y = vport.min.y + h;
	m_pPSS->SetViewport(vport);
}

IPoint2	 vtScene::GetWindowSize()
{
	if (!m_pPSS)
		return IPoint2(0, 0);
	PSBox2	vport = m_pPSS->GetViewport();
	int x = vport.Width();
	int y = vport.Height();
	return IPoint2(x, y);
}

float vtGetTime()
{
	float sec;

	// clock(), while ANSI C, is generally a low-res timer on most systems.
	//   Use a high-res timer if one exists.
#ifdef __FreeBSD__
	struct rusage ru;
	if (getrusage(RUSAGE_SELF, &ru))
		sec = -1.0 / 1000000;
	else
		sec = (ru.ru_utime.tv_sec  + ru.ru_stime.tv_sec ) +
			  (ru.ru_utime.tv_usec + ru.ru_stime.tv_usec) / 1000000.0;
#else
	sec = (float) clock() / CLOCKS_PER_SEC;
#endif

	return sec;
}

double vtScene::frameSeconds()
{
	PSSceneStats* stats = m_pPSS->GetStats();
	double	framesec = stats->EndTime - stats->StartTime;
	return  framesec;
}

/**
 * Convert window coordinates (in pixels) to a ray from the camera
 * in world coordinates.
 */
bool vtScene::CameraRay(const IPoint2 &win, FPoint3 &pos, FPoint3 &dir)
{
	PSVec3		far_point;
	PSCamera*	cam = GetCamera();
	PSMatrix	trans;
	PSBox3		vv = cam->GetViewVol();
	IPoint2		wdim = GetWindowSize();
	PSVec3		pfrom, ppos, pdir;

	pfrom.Set(0, 0, 0);
	ppos.x = vv.min.x + win.x * vv.Width() / wdim.x;
	ppos.y = vv.max.y - win.y * vv.Height() / wdim.y;
	ppos.z = -vv.min.z;

	cam->TotalTransform(&trans);
	pfrom *= trans;
	ppos *= trans;
	pdir = ppos - pfrom;
	pdir.Normalize();

	p2v(pfrom, pos);
	p2v(pdir, dir);

	return true;
}

void vtScene::SetGlobalWireframe(bool bWire)
{
	if (bWire)
	{
		PSAppearances::Apply(m_pPSS->GetDynamic(), APPEAR_Shading, APPEAR_Wire);
		m_pPSS->SetFlags(SCENE_WireFrame);
	}
	else
	{
		PSAppearances::Apply(m_pPSS->GetDynamic(), APPEAR_Shading, APPEAR_Smooth);
		m_pPSS->ClearFlags(SCENE_WireFrame);
	}
}

bool vtScene::GetGlobalWireframe()
{
	return m_pPSS->IsSet(SCENE_WireFrame);
}


vtNodeBase *vtLoadModel(const char *filename)
{
	vtpsNode<PSExtModel, vtTransformBase>* extnode = new vtpsNode<PSExtModel, vtTransformBase>;
	extnode->Load(filename, PSGetScene());
	return (vtNodeBase*) extnode;
}

RGBf vtScene::s_white(1, 1, 1);

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
void vtScene::SetFog(bool bOn, float start, float end, const RGBf &color, int iType)
{
	PSFog* fog = m_pPSS->GetFog();
	if (bOn)
	{
		if (fog == NULL)
		{
			fog = new PSFog;
			m_pPSS->SetFog(fog);
		}
		switch (iType)
		{
			case GL_LINEAR: iType = FOG_Linear; break;
			case GL_EXP: iType = FOG_Exponential; break;
			case GL_EXP2: iType = FOG_Exponential2; break;
			default: return;
		}
		fog->SetKind(iType);
		fog->SetDensity(0.25f);	// not used for linear
		fog->SetStart(start);
		fog->SetEnd(end);
		fog->SetColor(PSCol4(color.r, color.g, color.b, 1));
	}
	else
		m_pPSS->SetFog(NULL);
}

