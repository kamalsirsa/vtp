//
// SwapEngine.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Enviro.h"
#include "SwapEngine.h"
#include "TerrainSceneWP.h"

SwapEngine::SwapEngine()
{
	m_fadingOut = false;
	m_fadingIn = false;
	m_switch = false;
	m_framecount = 0;
}

void SwapEngine::Eval()
{
	if (m_fadingOut) FadeOut();
	else if (m_fadingIn) FadeIn();

	if (m_switch)
	{
		// change to the other terrain
		g_App.SetTerrain(m_pterrain);

		m_switch = false;   
		m_fadingOut = false;
		m_fadingIn = true;
	}
}

void SwapEngine::FadeOut()
{
	float curtime = vtGetTime();
	curtime -= m_time;
	if (curtime > .02)
	{
		m_framecount++;
		float alpha = (float)m_framecount/30;
		m_mat->SetDiffuse(0.0f, 0.0f, 0.0f, alpha);
		if (m_framecount == 30) 
		{
			m_switch = true;
		}
		m_time = vtGetTime();
	}
}

void SwapEngine::FadeIn()
{
	float curtime = vtGetTime();
	curtime -= m_time;
	if (curtime > .02f)
	{
		m_framecount--;
		float alpha = (float)m_framecount/30;
		m_mat->SetDiffuse(0.0f, 0.0f, 0.0f, alpha);
		if (m_framecount == 0) 
		{
			m_fadingIn = false;
//			m_Sprite.Remove();
		}
		m_time = vtGetTime();
	}
}

void SwapEngine::TriggerHit(vtTerrain* terrain, int jump_index)
{
	if ((!m_fadingOut) && (!m_fadingIn)) //check to see if we are already fading out
	{
		m_pterrain = terrain;
		m_destination_index = jump_index;

#if 0
		m_Sprite.SetName2("Portal Sprite");
//		m_Sprite.MakeGeometry();
		m_Sprite.SetImage(NULL);
		m_Sprite.SetWindowRect(-0.1f, -0.1f, 1.1f, 1.1f);
		vtMaterialArray* apps = m_Sprite.GetMaterials();
		vtMaterial *app = apps->GetAt(0);
		app->Set(APP_Texturing, false);
		app->Set(APP_Transparency, APP_AlphaBlend);
		m_mat = app->GetMaterial();
		m_mat->SetDiffuse(RGBf(0.0f, 0.0f, 0.0f, 0.0f));

		vtScene* scene = vtGetScene();
		vtRoot* model = scene->GetRoot();
		model->AddChild(&m_Sprite);
#endif

		m_time = vtGetTime();
		m_fadingOut = true;
	}
}
