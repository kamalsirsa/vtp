//
// Name:		LocationDlg.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
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
	EVT_CHECKBOX( ID_SMOOTH, LocationDlg::OnCheckbox )
	EVT_CHECKBOX( ID_POS_ONLY, LocationDlg::OnCheckbox )
	EVT_BUTTON( ID_RESET, LocationDlg::OnReset )
	EVT_SLIDER( ID_SPEEDSLIDER, LocationDlg::OnSpeedSlider )
	EVT_TEXT( ID_SPEED, LocationDlg::OnText )
END_EVENT_TABLE()

LocationDlg::LocationDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_bLoop = true;
	m_bSmooth = true;
	m_bPosOnly = false;
	m_iAnim = -1;
	m_fSpeed = 1.0f;
	m_bSetting = false;
	m_fRecordSpacing = 1.0f;

	LocationDialogFunc( this, TRUE );
	m_pSaver = new vtLocationSaver();
	m_pStoreAs = GetStoreas();
	m_pStore = GetStore();
	m_pRecall = GetRecall();
	m_pRemove = GetRemove();
	m_pLocList = GetLoclist();

	AddValidator(ID_LOOP, &m_bLoop);
	AddValidator(ID_SMOOTH, &m_bSmooth);
	AddValidator(ID_POS_ONLY, &m_bPosOnly);
	AddNumValidator(ID_SPEED, &m_fSpeed);
	AddValidator(ID_SPEEDSLIDER, &m_iSpeed);
	AddNumValidator(ID_RECORD_SPACING, &m_fRecordSpacing);

	RefreshButtons();
	UpdateEnabling();
}

LocationDlg::~LocationDlg()
{
	delete m_pSaver;

	unsigned int i;
	for (i = 0; i < m_Entries.GetSize(); i++)
		delete m_Entries[i];
}

void LocationDlg::SetTarget(vtTransform *pTarget, const vtProjection &proj,
							const vtLocalConversion &conv)
{
	m_pSaver->SetTransform(pTarget);
	m_pSaver->SetConversion(conv);
	m_pSaver->SetProjection(proj);
}

void LocationDlg::SetLocFile(const vtString &fname)
{
	m_pSaver->Empty();
	if (!m_pSaver->Read(fname))
		return;  // couldn't read

	RefreshList();
	RefreshButtons();
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
	unsigned int i, num = m_Entries.GetSize();
	for (i = 0; i < num; i++)
		GetAnims()->Append(_T("anim"));

	RefreshAnimsText();
}

void LocationDlg::RefreshAnimsText()
{
	wxString str;
	unsigned int i, num = m_Entries.GetSize();
	for (i = 0; i < num; i++)
	{
		AnimEntry *entry = m_Entries[i];
		vtAnimPath *anim = GetAnim(i);
		vtAnimPathEngine *eng = GetEngine(i);

		str.Printf(_T("%hs (%.1f/%.1f)"), (const char *) entry->m_Name,
			eng->GetTime(), (float) anim->GetLastTime());
		GetAnims()->SetString(i, str);
	}
}
void LocationDlg::UpdateEnabling()
{
	GetReset()->Enable(m_iAnim != -1);
	GetPlay()->Enable(m_iAnim != -1);
	GetRecord1()->Enable(m_iAnim != -1);
	GetStop()->Enable(m_iAnim != -1);
	GetSmooth()->Enable(m_iAnim != -1);
}

void LocationDlg::AppendAnimPath(vtAnimPath *anim, const char *name)
{
	vtAnimPathEngine *engine = new vtAnimPathEngine(anim);
	engine->SetTarget(m_pSaver->GetTransform());
	engine->SetEnabled(false);
	vtGetScene()->AddEngine(engine);

	AnimEntry *entry = new AnimEntry();
	entry->m_pAnim = anim;
	entry->m_pEngine = engine;
	entry->m_Name = name;
	m_Entries.Append(entry);
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

void LocationDlg::SetValues()
{
	if (m_iAnim == -1)
		return;

	vtAnimPath *anim = GetAnim(m_iAnim);
	anim->SetLoop(m_bLoop);
	anim->SetInterpMode(m_bSmooth ? vtAnimPath::CUBIC_SPLINE : vtAnimPath::LINEAR);
	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	engine->SetPosOnly(m_bPosOnly);
	engine->SetSpeed(m_fSpeed);
}

void LocationDlg::GetValues()
{
	if (m_iAnim == -1)
		return;

	vtAnimPath *anim = GetAnim(m_iAnim);
	m_bLoop = anim->GetLoop();
	m_bSmooth = (anim->GetInterpMode() == vtAnimPath::CUBIC_SPLINE);

	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	m_bPosOnly = engine->GetPosOnly();
	m_fSpeed = engine->GetSpeed();
}

void LocationDlg::TransferToWindow()
{
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}


// WDR: handler implementations for LocationDlg

void LocationDlg::OnText( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	ValuesToSliders();
	SetValues();
	TransferToWindow();
}

void LocationDlg::OnSpeedSlider( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	SlidersToValues();
	SetValues();
	TransferToWindow();
}

void LocationDlg::OnReset( wxCommandEvent &event )
{
	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	engine->Reset();
}

void LocationDlg::OnCheckbox( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
 	SetValues();
}

void LocationDlg::OnAnim( wxCommandEvent &event )
{
	m_iAnim = GetAnims()->GetSelection();

	UpdateEnabling();
	GetValues();
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
	path->Insert(path->GetLastTime() + m_fRecordSpacing, cp);
	path->ProcessPoints();

	RefreshAnimsText();
}

void LocationDlg::OnPlay( wxCommandEvent &event )
{
	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	engine->SetEnabled(true);
}

void LocationDlg::OnLoadAnim( wxCommandEvent &event )
{
	wxFileDialog loadFile(NULL, _("Load Animation Path"), _T(""), _T(""),
		_("Polyline Data Sources (*.shp)|*.shp|"), wxOPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	vtFeatureSetLineString3D lines;

	const char *filename = loadFile.GetPath().mb_str();
	if (!lines.LoadFromSHP(filename))
		return;

	vtAnimPath *anim = new vtAnimPath;
	anim->CreateFromLineString(m_pSaver->GetAtProjection(), lines);

	AppendAnimPath(anim, filename);
	RefreshAnims();
}

void LocationDlg::OnNewAnim( wxCommandEvent &event )
{
	vtAnimPath *anim = new vtAnimPath;
	AppendAnimPath(anim, "New Anim");
	RefreshAnims();
}

void LocationDlg::OnSaveAnim( wxCommandEvent &event )
{
	
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
		_("Location Files (*.loc)|*.loc|"), wxOPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxString2 path = loadFile.GetPath();
	SetLocFile(path.mb_str());
}

void LocationDlg::OnSave( wxCommandEvent &event )
{
	wxFileDialog saveFile(NULL, _("Save Locations"), _T(""), _T(""),
		_("Location Files (*.loc)|*.loc|"), wxSAVE);
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
	m_pStore->Enable(num != -1);
	m_pRecall->Enable(num != -1);
	m_pRemove->Enable(num != -1);
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

