//
// Name:	PropDlg.cpp
//
// Copyright (c) 2001-2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "PropDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "vtlib/vtlib.h"

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include "PropDlg.h"
#include "vtdata/Content.h"
#include "Frame.h"
#include "TagDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// PropDlg
//----------------------------------------------------------------------------

// WDR: event table for PropDlg

BEGIN_EVENT_TABLE(PropDlg,AutoPanel)
	EVT_INIT_DIALOG(PropDlg::OnInitDialog)
	EVT_TEXT( ID_ITEM, PropDlg::OnTextItem )
	EVT_CHOICE( ID_TYPECHOICE, PropDlg::OnChoiceType )
	EVT_BUTTON( ID_ADDTAG, PropDlg::OnAddTag )
	EVT_BUTTON( ID_REMOVETAG, PropDlg::OnRemoveTag )
	EVT_BUTTON( ID_EDITTAG, PropDlg::OnTagEdit )
END_EVENT_TABLE()

PropDlg::PropDlg( wxWindow *parent, wxWindowID id,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoPanel( parent, id, position, size, style )
{
	m_bUpdating = false;
	m_pCurrentItem = NULL;

	PropDialogFunc( this, TRUE );

	m_pTypeChoice = GetTypeChoice();
	m_pTagList = GetTaglist();

	AddValidator(ID_ITEM, &m_strItem);
	AddValidator(ID_TYPECHOICE, &m_strType);
}

// WDR: handler implementations for PropDlg

void PropDlg::OnAddTag( wxCommandEvent &event )
{
	GetMainFrame()->RenderingPause();
	TagDlg dlg(GetMainFrame(), -1, _T("Add New Tag"));
	if (dlg.ShowModal() == wxID_OK)
	{
		vtTag tag;
		tag.name = dlg.m_strName.mb_str();
		tag.value = dlg.m_strValue.mb_str();
		m_pCurrentItem->AddTag(tag);
		UpdateTagList();
	}
	GetMainFrame()->RenderingResume();
}

void PropDlg::OnRemoveTag( wxCommandEvent &event )
{
	int sel = m_pTagList->GetNextItem(-1, wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
		
	if (sel != -1)
	{
		int tagnum = m_pTagList->GetItemData(sel);
		m_pCurrentItem->RemoveTag(tagnum);
		UpdateTagList();
	}
}

void PropDlg::OnTagEdit( wxCommandEvent &event )
{
	GetMainFrame()->RenderingPause();

	int sel = m_pTagList->GetNextItem(-1, wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (sel != -1)
	{
		int tagnum = m_pTagList->GetItemData(sel);
		vtTag *tag = m_pCurrentItem->GetTag(tagnum);

		TagDlg dlg(GetMainFrame(), -1, _T("Edit Tag"));
		dlg.m_strName = tag->name;
		dlg.m_strValue = tag->value;
		if (dlg.ShowModal() == wxID_OK)
		{
			tag->name = dlg.m_strName.mb_str();
			tag->value = dlg.m_strValue.mb_str();
			UpdateTagList();
		}
	}
	GetMainFrame()->RenderingResume();
}

void PropDlg::OnInitDialog(wxInitDialogEvent& event)
{
	char buf[80];
	FILE *fp = fopen("itemtypes.txt", "rb");
	if (fp)
	{
		while (fgets(buf, 80, fp))
		{
			if (buf[strlen(buf)-1] == 10) buf[strlen(buf)-1] = 0;
			if (buf[strlen(buf)-1] == 13) buf[strlen(buf)-1] = 0;
			wxString2 str = buf;
			m_pTypeChoice->Append(str);
		}
		fclose(fp);
	}
	m_pTypeChoice->SetSelection(0);

	m_pTagList->ClearAll();
	m_pTagList->SetSingleStyle(wxLC_REPORT);
	m_pTagList->InsertColumn(0, _T("Tag"));
	m_pTagList->SetColumnWidth(0, 100);
	m_pTagList->InsertColumn(1, _T("Value"));
	m_pTagList->SetColumnWidth(1, 120);
}

void PropDlg::OnChoiceType( wxCommandEvent &event )
{
	UpdateFromControls();
}

void PropDlg::OnTextItem( wxCommandEvent &event )
{
	UpdateFromControls();
	if (!m_bUpdating)
		GetMainFrame()->RefreshTreeItems();
}


//////////////////////////////////////////////////////////////////////////

void PropDlg::UpdateTagList()
{
	m_pTagList->DeleteAllItems();
	if (!m_pCurrentItem)
		return;

	int item;
	vtTag *tag;
	for (unsigned int i = 0; i < m_pCurrentItem->NumTags(); i++)
	{
		tag = m_pCurrentItem->GetTag(i);
		if (!tag->name.Compare("type"))
			continue;
		item = m_pTagList->InsertItem(i, (wxString2) tag->name);
		m_pTagList->SetItem(item, 1, (wxString2) tag->value);
		m_pTagList->SetItemData(item, i);
	}
}

void PropDlg::SetCurrentItem(vtItem *item)
{
	if (item)
	{
		m_strItem = item->m_name;
		const char *type = item->GetValueString("type");
		if (type)
			m_strType = type;
		else
			m_strType = _T("unknown");
	}
	else
	{
		m_strItem = "";
		m_strType = "unspecified";
	}
	m_pCurrentItem = item;

	m_bUpdating = true;
	TransferDataToWindow();
	int sel = m_pTypeChoice->FindString(m_strType);
	if (sel != -1)
		m_pTypeChoice->SetSelection(sel);
	else
		m_pTypeChoice->SetSelection(0);
	UpdateTagList();
	m_bUpdating = false;
}

void PropDlg::UpdateFromControls()
{
	if (m_bUpdating)
		return;

	TransferDataFromWindow();
	if (m_pCurrentItem)
	{
		m_pCurrentItem->m_name = m_strItem.mb_str();
		m_pCurrentItem->SetValueString("type", m_strType.mb_str());
	}
}

