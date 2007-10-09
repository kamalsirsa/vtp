//
// EnviroGUI.cpp
// GUI-specific functionality of the Enviro class
//
// Copyright (c) 2003-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtdata/vtLog.h"
#include "vtui/Helper.h"
#include "vtui/InstanceDlg.h"
#include "vtui/ProfileDlg.h"

#include "EnviroGUI.h"
#include "EnviroApp.h"
#include "EnviroFrame.h"
#include "canvas.h"
#include "DistanceDlg3d.h"
#include "FeatureTableDlg3d.h"
#include "LayerDlg.h"
#include "StyleDlg.h"

#if WIN32 || !wxUSE_JOYSTICK
  #include "vtui/Joystick.h"
#else
  #include "wx/joystick.h"
#endif

DECLARE_APP(EnviroApp);

//
// This is a 'singleton', the only instance of the global application object
//
EnviroGUI g_App;

// helper
EnviroFrame *GetFrame()
{
	return dynamic_cast<EnviroFrame *>(wxGetApp().GetTopWindow());
}

EnviroGUI::EnviroGUI()
{
	m_pJFlyer = NULL;
}

EnviroGUI::~EnviroGUI()
{
}

void EnviroGUI::ShowPopupMenu(const IPoint2 &pos)
{
	GetFrame()->ShowPopupMenu(pos);
}

void EnviroGUI::SetTerrainToGUI(vtTerrain *pTerrain)
{
	GetFrame()->SetTerrainToGUI(pTerrain);

	if (pTerrain)
	{
		if (m_pJFlyer)
		{
			float speed = pTerrain->GetParams().GetValueFloat(STR_NAVSPEED);
			m_pJFlyer->SetSpeed(speed);
		}
		ShowMapOverview(pTerrain->GetParams().GetValueBool(STR_OVERVIEW));
		ShowCompass(pTerrain->GetParams().GetValueBool(STR_COMPASS));
	}
	else
	{
		ShowMapOverview(false);
		ShowCompass(false);
	}
}

void EnviroGUI::SetFlightSpeed(float speed)
{
	if (m_pJFlyer)
		m_pJFlyer->SetSpeed(speed);
	Enviro::SetFlightSpeed(speed);
}

const char *AppStateNames[] =
{
	"AS_Initializing",
	"AS_Neutral",
	"AS_Orbit",
	"AS_FlyingIn",
	"AS_SwitchToTerrain",
	"AS_Terrain",
	"AS_MovingOut",
	"AS_Error"
};

void EnviroGUI::SetState(AppState s)
{
	// if entering or leaving terrain or orbit state
	AppState previous = m_state;
	m_state = s;

	if (m_state != previous)
	{
		VTLOG("Changing app state from %s to %s\n", AppStateNames[previous],
			AppStateNames[m_state]);
	}

	if ((previous == AS_Terrain && m_state != AS_Terrain) ||
		(previous == AS_Orbit && m_state != AS_Orbit) ||
		(previous != AS_Terrain && m_state == AS_Terrain) ||
		(previous != AS_Orbit && m_state == AS_Orbit))
	{
		GetFrame()->RefreshToolbar();
	}

	if (s == AS_Error)
	{
		// If we encounter an error while trying to open a terrain, don't get
		//  stuck in a progress dialog.
		CloseProgressDialog2();
	}
}

vtString EnviroGUI::GetStringFromUser(const vtString &title, const vtString &msg)
{
	wxString caption(title, wxConvUTF8);
	wxString message(msg, wxConvUTF8);
	wxString str = wxGetTextFromUser(message, caption, _T(""), GetFrame());
	return (vtString) (const char *) str.mb_str(wxConvUTF8);
}

void EnviroGUI::ShowProgress(bool bShow)
{
	if (bShow)
		OpenProgressDialog2(_("Creating Terrain"), false, GetFrame());
	else
		CloseProgressDialog2();
}

void EnviroGUI::SetProgressTerrain(vtTerrain *pTerr)
{
	pTerr->SetProgressCallback(progress_callback_minor);
}

void EnviroGUI::UpdateProgress(const char *msg, int amount1, int amount2)
{
	wxString str(msg, wxConvUTF8);

	// Try to translate it; a translation might be available.
	// If the string is not found in any of the loaded message catalogs,
	// the original string is returned.
	wxString str2 = wxGetTranslation(str);

	UpdateProgressDialog2(amount1, amount2, str2);
}

void EnviroGUI::RefreshLayerView()
{
	LayerDlg *dlg = GetFrame()->m_pLayerDlg;
	dlg->RefreshTreeContents();
}

void EnviroGUI::UpdateLayerView()
{
	LayerDlg *dlg = GetFrame()->m_pLayerDlg;
	dlg->UpdateTreeTerrain();
}

void EnviroGUI::ShowLayerView()
{
	LayerDlg *dlg = GetFrame()->m_pLayerDlg;
	dlg->Show(true);
}

void EnviroGUI::CameraChanged()
{
	GetFrame()->CameraChanged();
}

void EnviroGUI::EarthPosUpdated()
{
	GetFrame()->EarthPosUpdated(m_EarthPos);
}

void EnviroGUI::ShowDistance(const DPoint2 &p1, const DPoint2 &p2,
							 double fGround, double fVertical)
{
	GetFrame()->m_pDistanceDlg->SetPoints(p1, p2, false);
	GetFrame()->m_pDistanceDlg->SetGroundAndVertical(fGround, fVertical, true);

	if (GetFrame()->m_pProfileDlg)
		GetFrame()->m_pProfileDlg->SetPoints(p1, p2);
}

void EnviroGUI::ShowDistance(const DLine2 &path,
							 double fGround, double fVertical)
{
	GetFrame()->m_pDistanceDlg->SetPath(path, false);
	GetFrame()->m_pDistanceDlg->SetGroundAndVertical(fGround, fVertical, true);

	if (GetFrame()->m_pProfileDlg)
		GetFrame()->m_pProfileDlg->SetPath(path);
}

vtTagArray *EnviroGUI::GetInstanceFromGUI()
{
	return GetFrame()->m_pInstanceDlg->GetTagArray();
}

bool EnviroGUI::OnMouseEvent(vtMouseEvent &event)
{
	return GetFrame()->OnMouseEvent(event);
}

void EnviroGUI::SetupScene3()
{
	GetFrame()->Setup3DScene();

#if wxUSE_JOYSTICK || WIN32
	m_pJFlyer = new vtJoystickEngine;
	m_pJFlyer->SetName2("Joystick");
	vtGetScene()->AddEngine(m_pJFlyer);
	m_pJFlyer->SetTarget(m_pNormalCamera);
#endif
}

void EnviroGUI::SetTimeEngineToGUI(class vtTimeEngine *pEngine)
{
	GetFrame()->SetTimeEngine(pEngine);
}

//////////////////////////////////////////////////////////////////////

bool EnviroGUI::SaveVegetation(bool bAskFilename)
{
	VTLOG1("EnviroGUI::SaveVegetation\n");

	vtTerrain *pTerr = GetCurrentTerrain();
	vtPlantInstanceArray &pia = pTerr->GetPlantInstances();

	vtString fname = pia.GetFilename();

	if (bAskFilename)
	{
		// save current directory
		wxString path = wxGetCwd();

		wxString default_file(StartOfFilename(fname), wxConvUTF8);
		wxString default_dir(ExtractPath(fname, false), wxConvUTF8);

		EnableContinuousRendering(false);
		wxFileDialog saveFile(NULL, _("Save Vegetation Data"), default_dir,
			default_file, _("Vegetation Files (*.vf)|*.vf"), wxFD_SAVE);
		bool bResult = (saveFile.ShowModal() == wxID_OK);
		EnableContinuousRendering(true);
		if (!bResult)
		{
			wxSetWorkingDirectory(path);	// restore
			return false;
		}
		wxString str = saveFile.GetPath();
		fname = str.mb_str(wxConvUTF8);
		pia.SetFilename(fname);
	}
	pia.WriteVF(fname);
	return true;
}

bool EnviroGUI::SaveStructures(bool bAskFilename)
{
	VTLOG1("EnviroGUI::SaveStructures\n");

	vtStructureArray3d *sa = GetCurrentTerrain()->GetStructureLayer();
	vtString fname = sa->GetFilename();
	if (bAskFilename)
	{
		// save current directory
		wxString path = wxGetCwd();

		wxString default_file(StartOfFilename(fname), wxConvUTF8);
		wxString default_dir(ExtractPath(fname, false), wxConvUTF8);

		EnableContinuousRendering(false);
		wxFileDialog saveFile(NULL, _("Save Built Structures Data"),
			default_dir, default_file, _("Structure Files (*.vtst)|*.vtst"),
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		bool bResult = (saveFile.ShowModal() == wxID_OK);
		EnableContinuousRendering(true);
		if (!bResult)
		{
			wxSetWorkingDirectory(path);	// restore
			return false;
		}
		wxString str = saveFile.GetPath();
		fname = str.mb_str(wxConvUTF8);
		sa->SetFilename(fname);
	}
	bool success = false;
	try {
		success = sa->WriteXML(fname);
	}
	catch (xh_io_exception &e)
	{
		string str = e.getFormattedMessage();
		VTLOG("  Error: %s\n", str.c_str());
		wxMessageBox(wxString(str.c_str(), wxConvUTF8), _("Error"));
	}
	return success;
}

void EnviroGUI::OnSetDelete(vtFeatureSet *set)
{
	return GetFrame()->OnSetDelete(set);
}

bool EnviroGUI::IsAcceptable(vtTerrain *pTerr)
{
	return GetFrame()->IsAcceptable(pTerr);
}

void EnviroGUI::ShowMessage(const vtString &str)
{
	VTLOG1("EnviroGUI::ShowMessage '");
	VTLOG1(str);
	VTLOG1("'\n");

	EnableContinuousRendering(false);

	wxString str2(str, wxConvUTF8);
	wxMessageBox(str2);

	EnableContinuousRendering(true);
}

///////////////////////////////////////////////////////////////////////

vtJoystickEngine::vtJoystickEngine()
{
	m_fSpeed = 1.0f;
	m_fLastTime = 0.0f;

	m_pStick = new wxJoystick;
	if (!m_pStick->IsOk())
	{
		delete m_pStick;
		m_pStick = NULL;
	}
}
void vtJoystickEngine::Eval()
{
	if (!m_pStick)
		return;

	float fTime = vtGetTime(), fElapsed = fTime - m_fLastTime;

	vtTransform *pTarget = (vtTransform*) GetTarget();
	if (pTarget)
	{
		wxPoint p = m_pStick->GetPosition();
		int buttons = m_pStick->GetButtonState();
		float dx = ((float)p.x / 32768) - 1.0f;
		float dy = ((float)p.y / 32768) - 1.0f;

		// use a small dead zone to avoid drift
		const float dead_zone = 0.04f;

		if (buttons & wxJOY_BUTTON2)
		{
			// move up down left right
			if (fabs(dx) > dead_zone)
				pTarget->TranslateLocal(FPoint3(dx * m_fSpeed * fElapsed, 0.0f, 0.0f));
			if (fabs(dy) > dead_zone)
				pTarget->Translate1(FPoint3(0.0f, dy * m_fSpeed * fElapsed, 0.0f));
		}
		else if (buttons & wxJOY_BUTTON3)
		{
			// pitch up down, yaw left right
			if (fabs(dx) > dead_zone)
				pTarget->RotateParent(FPoint3(0,1,0), -dx * fElapsed);
			if (fabs(dy) > dead_zone)
				pTarget->RotateLocal(FPoint3(1,0,0), dy * fElapsed);
		}
		else
		{
			// move forward-backward, turn left-right
			if (fabs(dy) > dead_zone)
				pTarget->TranslateLocal(FPoint3(0.0f, 0.0f, dy * m_fSpeed * fElapsed));
			if (fabs(dx) > dead_zone)
				pTarget->RotateParent(FPoint3(0,1,0), -dx * fElapsed);
		}
	}
	m_fLastTime = fTime;
}


///////////////////////////////////////////////////////////////////////
// Helpers

vtAbstractLayer *CreateNewAbstractPointLayer(vtTerrain *pTerr)
{
	// make a new abstract layer (points)
	vtFeatureSetPoint2D *pSet = new vtFeatureSetPoint2D;
	vtTagArray &props = pSet->GetProperties();
	pSet->SetFilename("Untitled.shp");
	pSet->AddField("Label", FT_String);

	// Ask style for the new point layer
	props.SetValueBool("Geometry", false, true);
	props.SetValueBool("Labels", true, true);
	props.SetValueRGBi("LabelColor", RGBi(255,255,0), true);
	props.SetValueFloat("LabelHeight", 10.0f, true);
	props.SetValueInt("TextFieldIndex", 0, true);

	StyleDlg dlg(NULL, -1, _("Style"));
	dlg.SetFeatureSet(pSet);
	dlg.SetOptions(props);
	if (dlg.ShowModal() != wxID_OK)
	{
		delete pSet;
		return NULL;
	}
	dlg.GetOptions(props);

	// wrap the features in an abstract layer
	vtAbstractLayer *pLay = new vtAbstractLayer(pTerr);
	pLay->SetFeatureSet(pSet);

	// add the new layer to the terrain
	pTerr->GetLayers().Append(pLay);
	pTerr->SetAbstractLayer(pLay);

	// and show it in the layers dialog
	GetFrame()->m_pLayerDlg->RefreshTreeContents();	// full refresh

	return pLay;
}
