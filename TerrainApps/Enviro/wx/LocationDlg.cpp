//
// Name:		LocationDlg.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
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
#include "canvas.h"
#include "LocationDlg.h"

void BlockingMessageBox(const char *msg)
{
	EnableContinuousRendering(false);
	wxMessageBox(wxString::FromAscii(msg));
	EnableContinuousRendering(true);
}

// WDR: class implementations

//----------------------------------------------------------------------------
// LocationDlg
//----------------------------------------------------------------------------

// WDR: event table for LocationDlg

BEGIN_EVENT_TABLE(LocationDlg,wxDialog)
	EVT_LISTBOX( ID_LOCLIST, LocationDlg::OnLocList )
	EVT_BUTTON( ID_RECALL, LocationDlg::OnRecall )
	EVT_BUTTON( ID_STORE, LocationDlg::OnStore )
	EVT_BUTTON( ID_STOREAS, LocationDlg::OnStoreAs )
	EVT_BUTTON( ID_SAVE, LocationDlg::OnSave )
	EVT_BUTTON( ID_LOAD, LocationDlg::OnLoad )
	EVT_LISTBOX_DCLICK( ID_LOCLIST, LocationDlg::OnListDblClick )
	EVT_BUTTON( ID_REMOVE, LocationDlg::OnRemove )
END_EVENT_TABLE()

LocationDlg::LocationDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxDialog( parent, id, title, position, size, style )
{
	LocationDialogFunc( this, TRUE );
	m_pSaver = new vtLocationSaver();
	m_pStoreAs = GetStoreas();
	m_pStore = GetStore();
	m_pRecall = GetRecall();
	m_pRemove = GetRemove();
	m_pLocList = GetLoclist();
	RefreshButtons();
}

LocationDlg::~LocationDlg()
{
	delete m_pSaver;
}

void LocationDlg::SetTarget(vtTransformBase *pTarget, const vtProjection &proj,
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

// WDR: handler implementations for LocationDlg

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
			BlockingMessageBox("Couldn't recall point, probably a coordinate system transformation problem.");
	}
}

void LocationDlg::OnLoad( wxCommandEvent &event )
{
	wxFileDialog loadFile(NULL, _T("Load Locations"), _T(""), _T(""),
		_T("Location Files (*.loc)|*.loc|"), wxOPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxString path = loadFile.GetPath();
	SetLocFile(path.c_str());
}

void LocationDlg::OnSave( wxCommandEvent &event )
{
	wxFileDialog saveFile(NULL, _T("Save Locations"), _T(""), _T(""),
		_T("Location Files (*.loc)|*.loc|"), wxSAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxString str = saveFile.GetPath();
	if (!m_pSaver->Write(str.c_str()))
		return;  // couldn't write
}

void LocationDlg::OnStoreAs( wxCommandEvent &event )
{
	int num = m_pSaver->GetNumLocations();

	wxString str;
	str.Printf(_T("Location %d"), num+1);
	wxTextEntryDialog dlg(NULL, _T("Type a name for the new location:"),
		_T("Location Name"), str);
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
		BlockingMessageBox("Couldn't store point, probably a coordinate system transformation problem.");
}

void LocationDlg::OnStore( wxCommandEvent &event )
{
	int num = m_pLocList->GetSelection();
	if (num >= 0 && num < m_pSaver->GetNumLocations())
	{
		bool success = m_pSaver->StoreTo(num);
		if (!success)
			BlockingMessageBox("Couldn't store point, probably a coordinate system transformation problem.");
	}
}

void LocationDlg::OnRecall( wxCommandEvent &event )
{
	int num = m_pLocList->GetSelection();
	if (num >= 0 && num < m_pSaver->GetNumLocations())
	{
		bool success = m_pSaver->RecallFrom(num);
		if (!success)
			BlockingMessageBox("Couldn't recall point, probably a coordinate system transformation problem.");
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


