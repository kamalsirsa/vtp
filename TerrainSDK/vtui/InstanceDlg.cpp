//
// InstanceDlg.cpp
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "InstanceDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#include "InstanceDlg.h"
#include "wxString2.h"
#include "vtdata/Content.h"
#include "vtdata/FilePath.h"
#include "vtui/Helper.h"
#include "xmlhelper/exception.hpp"

// WDR: class implementations

//----------------------------------------------------------------------------
// InstanceDlg
//----------------------------------------------------------------------------

// WDR: event table for InstanceDlg

BEGIN_EVENT_TABLE(InstanceDlg,AutoDialog)
	EVT_RADIOBUTTON( ID_RADIO_CONTENT, InstanceDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_MODEL, InstanceDlg::OnRadio )
	EVT_CHOICE( ID_CHOICE_FILE, InstanceDlg::OnChoice )
	EVT_CHOICE( ID_CHOICE_ITEM, InstanceDlg::OnChoiceItem )
	EVT_BUTTON( ID_BROWSE_MODEL_FILE, InstanceDlg::OnBrowseModeFile )
	EVT_TEXT( ID_LOCATION, InstanceDlg::OnLocationText )
END_EVENT_TABLE()

InstanceDlg::InstanceDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function InstanceDialogFunc for InstanceDlg
	InstanceDialogFunc( this, TRUE );
	m_bContent = true;
}

void InstanceDlg::UpdateLoc()
{
	LinearUnits lu = m_proj.GetUnits();

	wxString2 str;
	if (lu == LU_DEGREES)
		str.Printf(_T("%lf, %lf"), m_pos.x, m_pos.y);
	else
		str.Printf(_T("%.2lf, %.2lf"), m_pos.x, m_pos.y);
	GetLocation()->SetValue(str);
}

void InstanceDlg::SetLocation(const DPoint2 &pos)
{
	m_pos = pos;
	UpdateLoc();
}

void InstanceDlg::UpdateEnabling()
{
	GetChoiceFile()->Enable(m_bContent);
	GetChoiceItem()->Enable(m_bContent);

	GetModelFile()->Enable(!m_bContent);
	GetBrowseModelFile()->Enable(!m_bContent);
}

void InstanceDlg::UpdateContentItems()
{
	GetChoiceItem()->Clear();

	wxString2 path = GetChoiceFile()->GetStringSelection();
	if (path == _T(""))
		return;

	vtString fname = FindFileOnPaths(m_datapaths, path.mb_str());
	if (fname == "")
		return;

	vtContentManager mng;
	try
	{
		mng.ReadXML(fname);
	}
	catch (xh_io_exception &ex)
	{
		// could display error message here
		return;
	}

	wxString2 str;
	for (unsigned int i = 0; i < mng.NumItems(); i++)
	{
		vtItem *item = mng.GetItem(i);
		str = item->m_name;
//		str += _T(" (");
//		str += item->GetValue("filename");
//		str += _T(")");
		GetChoiceItem()->Append(str);
	}
	GetChoiceItem()->SetSelection(0);
}


// WDR: handler implementations for InstanceDlg

void InstanceDlg::OnInitDialog(wxInitDialogEvent& event)
{
	GetChoiceFile()->Clear();
	for (unsigned int i = 0; i < m_datapaths.size(); i++)
	{
		AddFilenamesToChoice(GetChoiceFile(), m_datapaths[i], "*.vtco");
	}
	GetChoiceFile()->Select(0);

	AddValidator(ID_RADIO_CONTENT, &m_bContent);
	UpdateLoc();
	UpdateEnabling();
	UpdateContentItems();

	wxDialog::OnInitDialog(event);
}

void InstanceDlg::OnLocationText( wxCommandEvent &event )
{
	// todo? only for edit
}

void InstanceDlg::OnBrowseModeFile( wxCommandEvent &event )
{
	wxFileDialog SelectFile(this, _T("Choose model file"),
							_T(""),
							_T(""),
							_T("3D Model files (*.3ds;*.obj;*.lwo;*.flt)|*.3ds;*.obj;*.lwo;*.flt|All files(*.*)|*.*|"),
							wxOPEN);
	if (SelectFile.ShowModal() != wxID_OK)
		return;
	GetModelFile()->SetValue(SelectFile.GetPath());
}

void InstanceDlg::OnChoice( wxCommandEvent &event )
{
	UpdateContentItems();
}

void InstanceDlg::OnChoiceItem( wxCommandEvent &event )
{
}

void InstanceDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnabling();
}


