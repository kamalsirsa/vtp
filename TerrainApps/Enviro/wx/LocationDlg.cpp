//
// Name:		LocationDlg.cpp
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "LocationDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/image.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtui/wxString2.h"
#include "vtdata/FilePath.h"
#include "canvas.h"
#include "LocationDlg.h"

void BlockingMessageBox(const wxString &msg)
{
	EnableContinuousRendering(false);
	wxMessageBox(msg);
	EnableContinuousRendering(true);
}

// WDR: class implementations

//----------------------------------------------------------------------------
// LocationDlg
//----------------------------------------------------------------------------

// WDR: event table for LocationDlg

BEGIN_EVENT_TABLE(LocationDlg,AutoDialog)
	EVT_LISTBOX( ID_LOCLIST, LocationDlg::OnLocList )
	EVT_BUTTON( ID_RECALL, LocationDlg::OnRecall )
	EVT_BUTTON( ID_STORE, LocationDlg::OnStore )
	EVT_BUTTON( ID_STOREAS, LocationDlg::OnStoreAs )
	EVT_BUTTON( ID_SAVE, LocationDlg::OnSave )
	EVT_BUTTON( ID_LOAD, LocationDlg::OnLoad )
	EVT_LISTBOX_DCLICK( ID_LOCLIST, LocationDlg::OnListDblClick )
	EVT_BUTTON( ID_REMOVE, LocationDlg::OnRemove )

	EVT_BUTTON( ID_NEW_ANIM, LocationDlg::OnNewAnim )
	EVT_BUTTON( ID_SAVE_ANIM, LocationDlg::OnSaveAnim )
	EVT_BUTTON( ID_LOAD_ANIM, LocationDlg::OnLoadAnim )
	EVT_BUTTON( ID_PLAY, LocationDlg::OnPlay )
	EVT_BUTTON( ID_RECORD1, LocationDlg::OnRecord1 )
	EVT_BUTTON( ID_STOP, LocationDlg::OnStop )
	EVT_LISTBOX( ID_ANIMS, LocationDlg::OnAnim )
	EVT_CHECKBOX( ID_LOOP, LocationDlg::OnCheckbox )
	EVT_CHECKBOX( ID_CONTINUOUS, LocationDlg::OnCheckbox )
	EVT_CHECKBOX( ID_SMOOTH, LocationDlg::OnCheckbox )
	EVT_CHECKBOX( ID_POS_ONLY, LocationDlg::OnCheckbox )
	EVT_BUTTON( ID_RESET, LocationDlg::OnReset )
	EVT_SLIDER( ID_SPEEDSLIDER, LocationDlg::OnSpeedSlider )
	EVT_TEXT( ID_SPEED, LocationDlg::OnText )
	EVT_RADIOBUTTON( ID_RECORD_LINEAR, LocationDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RECORD_INTERVAL, LocationDlg::OnRadio )
	EVT_CHECKBOX( ID_ACTIVE, LocationDlg::OnActive )
	EVT_SLIDER( ID_ANIM_POS, LocationDlg::OnAnimPosSlider )
END_EVENT_TABLE()

LocationDlg::LocationDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_bActive = true;
	m_bLoop = true;
	m_bContinuous = false;
	m_bSmooth = true;
	m_bPosOnly = false;
	m_iAnim = -1;
	m_iPos = 0;
	m_fSpeed = 1.0f;
	m_bSetting = false;
	m_fRecordSpacing = 1.0f;
	m_bRecordLinear = true;
	m_bRecordInterval = false;

	LocationDialogFunc( this, TRUE );

	m_pSaver = NULL;
	m_pAnimPaths = NULL;

	m_pLocList = GetLoclist();

	AddValidator(ID_ACTIVE, &m_bActive);
	AddValidator(ID_LOOP, &m_bLoop);
	AddValidator(ID_CONTINUOUS, &m_bContinuous);
	AddValidator(ID_SMOOTH, &m_bSmooth);
	AddValidator(ID_POS_ONLY, &m_bPosOnly);
	AddNumValidator(ID_SPEED, &m_fSpeed, 2);
	AddValidator(ID_SPEEDSLIDER, &m_iSpeed);
	AddNumValidator(ID_RECORD_SPACING, &m_fRecordSpacing);
	AddValidator(ID_ANIM_POS, &m_iPos);

	AddValidator(ID_RECORD_LINEAR, &m_bRecordLinear);
	AddValidator(ID_RECORD_INTERVAL, &m_bRecordInterval);

	RefreshButtons();
	UpdateEnabling();
}

LocationDlg::~LocationDlg()
{
}

void LocationDlg::SetLocSaver(vtLocationSaver *saver)
{
	m_pSaver = saver;
	RefreshList();
	RefreshButtons();
}

void LocationDlg::SetAnimContainer(vtAnimContainer *ac)
{
	m_pAnimPaths = ac;
	m_iAnim = -1;
	RefreshAnims();
	UpdateSlider();
	UpdateEnabling();
}

void LocationDlg::Update()
{
	RefreshAnimsText();
	UpdateSlider();
}

void LocationDlg::RefreshList()
{
	m_pLocList->Clear();
	if (!m_pSaver)
		return;

	wxString str;
	int num = m_pSaver->GetNumLocations();
	for (int i = 0; i < num; i++)
	{
		vtLocation *loc = m_pSaver->GetLocation(i);

		str.Printf(_T("%d. %ls"), i+1, loc->m_strName.c_str());
		m_pLocList->Append(str);
	}
}

void LocationDlg::RefreshAnims()
{
	GetAnims()->Clear();
	unsigned int i, num = m_pAnimPaths->GetSize();
	for (i = 0; i < num; i++)
		GetAnims()->Append(_T("anim"));

	RefreshAnimsText();
}

void LocationDlg::RefreshAnimsText()
{
	wxString str;
	unsigned int i, num = m_pAnimPaths->GetSize();
	for (i = 0; i < num; i++)
	{
		vtAnimEntry *entry = m_pAnimPaths->GetAt(i);
		vtAnimPath *anim = GetAnim(i);
		vtAnimPathEngine *eng = GetEngine(i);

		str.Printf(_T("%hs (%.1f/%.1f, %d)"), (const char *) entry->m_Name,
			eng->GetTime(), (float) anim->GetLastTime(), anim->GetNumPoints());
		GetAnims()->SetString(i, str);
	}
}

void LocationDlg::UpdateSlider()
{
	if (m_iAnim != -1)
	{
		// time slider
		float fTotalTime = GetAnim(m_iAnim)->GetLastTime();
		float fTime = GetEngine(m_iAnim)->GetTime();
		m_iPos = (int) (fTime / fTotalTime * 1000);
		GetAnimPos()->SetValue(m_iPos);
	}
}

void LocationDlg::UpdateEnabling()
{
	GetAnimPos()->Enable(m_iAnim != -1);
	GetSaveAnim()->Enable(m_iAnim != -1);
	GetActive()->Enable(m_iAnim != -1);
	GetSpeed()->Enable(m_iAnim != -1);
	GetReset()->Enable(m_iAnim != -1);
	GetPlay()->Enable(m_iAnim != -1);
	GetRecord1()->Enable(m_iAnim != -1);
	GetStop()->Enable(m_iAnim != -1);
	GetLoop()->Enable(m_iAnim != -1);
	GetContinuous()->Enable(m_iAnim != -1);
	GetSmooth()->Enable(m_iAnim != -1 && GetAnim(m_iAnim)->GetNumPoints() > 2);
	GetPosOnly()->Enable(m_iAnim != -1);

	GetRecordInterval()->Enable(m_iAnim != -1);
	GetRecordLinear()->Enable(m_iAnim != -1);

	GetRecordSpacing()->Enable(m_bRecordInterval);
}

void LocationDlg::AppendAnimPath(vtAnimPath *anim, const char *name)
{
	vtAnimPathEngine *engine = new vtAnimPathEngine(anim);
	engine->SetName2("AnimPathEngine");
	engine->SetTarget(m_pSaver->GetTransform());
	engine->SetEnabled(false);

	vtAnimEntry *entry = new vtAnimEntry();
	entry->m_pAnim = anim;
	entry->m_pEngine = engine;
	entry->m_Name = name;
	m_pAnimPaths->AppendEntry(entry);
}

vtAnimPath *LocationDlg::CreateAnimPath()
{
	vtAnimPath *anim = new vtAnimPath();

	// Ensure that anim knows the projection
	const vtProjection &proj = m_pSaver->GetAtProjection();
	anim->SetProjection(proj);

	return anim;
}

#define SPEED_MIN   0.0f
#define SPEED_MAX   4.0f
#define SPEED_RANGE (SPEED_MAX-(SPEED_MIN)) // 1 to 10000 meters/sec

void LocationDlg::SlidersToValues()
{
	m_fSpeed =  powf(10, (SPEED_MIN + m_iSpeed * SPEED_RANGE / 100));
}

void LocationDlg::ValuesToSliders()
{
	m_iSpeed =  (int) ((log10f(m_fSpeed) - SPEED_MIN) / SPEED_RANGE * 100);
}

void LocationDlg::SetValues(int which)
{
	if (m_iAnim == -1)
		return;

	vtAnimPath *anim = GetAnim(m_iAnim);
	if (which & PF_LOOP)
		anim->SetLoop(m_bLoop);
	if (which & PF_INTERP)
		anim->SetInterpMode(m_bSmooth ? vtAnimPath::CUBIC_SPLINE : vtAnimPath::LINEAR);
	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	if (which & PF_CONTIN)
		engine->SetContinuous(m_bContinuous);
	if (which & PF_POSONLY)
		engine->SetPosOnly(m_bPosOnly);
//  engine->SetEnabled(m_bActive);
	if (which & PF_SPEED)
		engine->SetSpeed(m_fSpeed);
}

void LocationDlg::GetValues()
{
	if (m_iAnim == -1)
		return;

	vtAnimPath *anim = GetAnim(m_iAnim);
	m_bSmooth = (anim->GetInterpMode() == vtAnimPath::CUBIC_SPLINE);
	m_bLoop = anim->GetLoop();

	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	m_bContinuous = engine->GetContinuous();
	m_bPosOnly = engine->GetPosOnly();
	m_fSpeed = engine->GetSpeed();
//  m_bActive = engine->GetEnabled();
}

void LocationDlg::TransferToWindow()
{
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}


// WDR: handler implementations for LocationDlg

void LocationDlg::OnAnimPosSlider( wxCommandEvent &event )
{
	if (m_iAnim == -1)
		return;

	TransferDataFromWindow();

	float fTotalTime = GetAnim(m_iAnim)->GetLastTime();
	vtAnimPathEngine *eng = GetEngine(m_iAnim);
	eng->SetTime(m_iPos * fTotalTime / 1000);
	if (m_bActive)
		eng->UpdateTargets();
}

void LocationDlg::OnActive( wxCommandEvent &event )
{
	TransferDataFromWindow();
}

void LocationDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnabling();
}

void LocationDlg::OnText( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	ValuesToSliders();
	SetValues(PF_SPEED);
	TransferToWindow();
}

void LocationDlg::OnSpeedSlider( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	SlidersToValues();
	SetValues(PF_SPEED);
	TransferToWindow();
}

void LocationDlg::OnReset( wxCommandEvent &event )
{
	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	engine->Reset();
	if (m_bActive)
		engine->UpdateTargets();
}

void LocationDlg::OnCheckbox( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	SetValues(PF_ALL);
	RefreshAnimsText();
}

void LocationDlg::OnAnim( wxCommandEvent &event )
{
	m_iAnim = GetAnims()->GetSelection();

	UpdateEnabling();
	GetValues();
	UpdateSlider();
	ValuesToSliders();
	TransferToWindow();
}

void LocationDlg::OnStop( wxCommandEvent &event )
{
	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	engine->SetEnabled(false);
}

void LocationDlg::OnRecord1( wxCommandEvent &event )
{
	TransferDataFromWindow();

	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	vtTransform *xform = (vtTransform *) engine->GetTarget();
	vtAnimPath *path = engine->GetAnimationPath();

	FPoint3 pos = xform->GetTrans();
	FQuat rot = xform->GetOrient();
	ControlPoint cp(pos, rot);

	float fTime;
	if (path->GetNumPoints() == 0)
		fTime = 0;
	else
	{
		if (m_bRecordInterval)
		{
			// Record Interval: Append an element whose time is the desired number
			//  of seconds after the last point.
			fTime = path->GetLastTime() + m_fRecordSpacing;
		}
		else
		{
			// RecordLinear: Append an element whose time is derived from the linear
			//  distance from the last point.
			ControlPoint &prev_cp =
			path->GetTimeControlPointMap().rbegin()->second;
			float dist = (pos - prev_cp.m_Position).Length();

			// convert directly at 1 meter/second
			fTime = path->GetLastTime() + dist;
		}
	}
	path->Insert(fTime, cp);
	path->ProcessPoints();

	RefreshAnimsText();
	UpdateEnabling();	// Smooth might be allowed now
}

void LocationDlg::OnPlay( wxCommandEvent &event )
{
	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	engine->SetEnabled(true);
}

void LocationDlg::OnLoadAnim( wxCommandEvent &event )
{
	wxString2 filter = _("Polyline Data Sources");
	filter += _T(" (*.vtap,*.shp,*.dxf,*.igc)|*.vtap;*.shp;*.dxf;*.igc");
	wxFileDialog loadFile(NULL, _("Load Animation Path"), _T(""), _T(""),
		filter, wxOPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	vtAnimPath *anim;
	bool bSuccess;
	wxString2 str = loadFile.GetPath();
	const char *filename = str.mb_str();
	if (GetExtension(filename) == ".vtap")
	{
		anim = CreateAnimPath();
		bSuccess = anim->Read(filename);
	}
	else
	{
		vtFeatureLoader loader;
		vtFeatureSet *pSet = loader.LoadFrom(filename);
		if (!pSet)
			return;
		anim = CreateAnimPath();
		bSuccess = anim->CreateFromLineString(m_pSaver->GetAtProjection(), pSet);
		delete pSet;
	}
	if (bSuccess)
	{
		AppendAnimPath(anim, filename);
		RefreshAnims();
	}
	else
		delete anim;
}

void LocationDlg::OnNewAnim( wxCommandEvent &event )
{
	vtAnimPath *anim = CreateAnimPath();
	AppendAnimPath(anim, "New Anim");
	RefreshAnims();
}

void LocationDlg::OnSaveAnim( wxCommandEvent &event )
{
	if (m_iAnim == -1)
		return;
	vtAnimPath *path = GetAnim(m_iAnim);

	wxFileDialog saveFile(NULL, _("Save AnimPath"), _T(""), _T(""),
		_("AnimPath Files (*.vtap)|*.vtap"), wxSAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxString2 filepath = saveFile.GetPath();
	path->Write(filepath.mb_str());
}

void LocationDlg::OnRemove( wxCommandEvent &event )
{
	int num = m_pLocList->GetSelection();
	if (num >= 0 && num < m_pSaver->GetNumLocations())
	   m_pSaver->Remove(num);
	RefreshList();
	RefreshButtons();
}

void LocationDlg::OnListDblClick( wxCommandEvent &event )
{
	int num = m_pLocList->GetSelection();
	if (num >= 0 && num < m_pSaver->GetNumLocations())
	{
		bool success = m_pSaver->RecallFrom(num);
		if (!success)
			BlockingMessageBox(_("Couldn't recall point, probably a coordinate system transformation problem."));
	}
}

void LocationDlg::OnLoad( wxCommandEvent &event )
{
	wxFileDialog loadFile(NULL, _("Load Locations"), _T(""), _T(""),
		_("Location Files (*.loc)|*.loc"), wxOPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxString2 path = loadFile.GetPath();
	if (m_pSaver->Read(path.mb_str()))
	{
		RefreshList();
		RefreshButtons();
	}
}

void LocationDlg::OnSave( wxCommandEvent &event )
{
	wxFileDialog saveFile(NULL, _("Save Locations"), _T(""), _T(""),
		_("Location Files (*.loc)|*.loc"), wxSAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxString2 str = saveFile.GetPath();
	if (!m_pSaver->Write(str.mb_str()))
		return;  // couldn't write
}

void LocationDlg::OnStoreAs( wxCommandEvent &event )
{
	int num = m_pSaver->GetNumLocations();

	wxString str;
	str.Printf(_("Location %d"), num+1);
	wxTextEntryDialog dlg(NULL, _("Type a name for the new location:"),
		_("Location Name"), str);
	if (dlg.ShowModal() != wxID_OK)
		return;

	str = dlg.GetValue();
	bool success = m_pSaver->StoreTo(num, str.c_str());
	if (success)
	{
		RefreshList();
		m_pLocList->SetSelection(num);
		RefreshButtons();
	}
	else
		BlockingMessageBox(_("Couldn't store point, probably a coordinate system transformation problem."));
}

void LocationDlg::OnStore( wxCommandEvent &event )
{
	int num = m_pLocList->GetSelection();
	if (num >= 0 && num < m_pSaver->GetNumLocations())
	{
		bool success = m_pSaver->StoreTo(num);
		if (!success)
			BlockingMessageBox(_("Couldn't store point, probably a coordinate system transformation problem."));
	}
}

void LocationDlg::OnRecall( wxCommandEvent &event )
{
	int num = m_pLocList->GetSelection();
	if (num >= 0 && num < m_pSaver->GetNumLocations())
	{
		bool success = m_pSaver->RecallFrom(num);
		if (!success)
			BlockingMessageBox(_("Couldn't recall point, probably a coordinate system transformation problem."));
	}
}

void LocationDlg::OnLocList( wxCommandEvent &event )
{
	RefreshButtons();
}

void LocationDlg::RefreshButtons()
{
	int num = m_pLocList->GetSelection();
	GetStore()->Enable(num != -1);
	GetRecall()->Enable(num != -1);
	GetRemove()->Enable(num != -1);
}

void LocationDlg::RecallFrom(const vtString &locname)
{
	int num = m_pSaver->FindLocation(locname);
	if (num != -1)
	{
		m_pSaver->RecallFrom(num);
		m_pLocList->SetSelection(num);
	}
}

