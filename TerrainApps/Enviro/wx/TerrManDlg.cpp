//
// Name: TerrManDlg.cpp
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "TerrManDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "vtdata/TParams.h"
#include "vtdata/FilePath.h"
#include "vtui/wxString2.h"
#include "TerrManDlg.h"
#include "../Options.h"
#include "app.h"

/////////////////////////

class TMTreeItemData : public wxTreeItemData
{
public:
	vtString m_strDir;
	vtString m_strIniFile;
	vtString m_strIniPath;
	vtString m_strName;
};

// WDR: class implementations

//---------------------------------------------------------------------------
// TerrainManagerDlg
//---------------------------------------------------------------------------

// WDR: event table for TerrainManagerDlg

BEGIN_EVENT_TABLE(TerrainManagerDlg,wxDialog)
	EVT_INIT_DIALOG (TerrainManagerDlg::OnInitDialog)
	EVT_TREE_SEL_CHANGED( ID_TREECTRL, TerrainManagerDlg::OnSelChanged )
	EVT_TREE_DELETE_ITEM( ID_TREECTRL, TerrainManagerDlg::OnDeleteItem )
	EVT_BUTTON( ID_ADD_PATH, TerrainManagerDlg::OnAddPath )
	EVT_BUTTON( ID_ADD_TERRAIN, TerrainManagerDlg::OnAddTerrain )
	EVT_BUTTON( ID_DELETE, TerrainManagerDlg::OnDelete )
	EVT_BUTTON( ID_EDIT_PARAMS, TerrainManagerDlg::OnEditParams )
	EVT_BUTTON( ID_COPY, TerrainManagerDlg::OnCopy )
	EVT_BUTTON( wxID_OK, TerrainManagerDlg::OnOK )
END_EVENT_TABLE()

TerrainManagerDlg::TerrainManagerDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxDialog( parent, id, title, position, size, style | wxRESIZE_BORDER )
{
	// WDR: dialog function TerrManFunc for TerrainManagerDlg
	TerrManFunc( this, TRUE );
	m_iSelect = 0;
}

void TerrainManagerDlg::RefreshTreeContents()
{
	m_pTree->DeleteAllItems();

	vtStringArray &paths = g_Options.m_DataPaths;
	int i, num = paths.size();
	wxString2 wstr, wstr2;

	m_Root = m_pTree->AddRoot(_T("Terrain Data Paths"));

	for (i = 0; i < num; i++)
	{
		vtString str = paths[i];

		wstr = str;
		wxTreeItemId hPath = m_pTree->AppendItem(m_Root, wstr);

		vtString directory = str + "Terrains";
		for (dir_iter it((const char *)directory); it != dir_iter(); ++it)
		{
			if (it.is_hidden() || it.is_directory())
				continue;

			std::string name1 = it.filename();
			vtString name = name1.c_str();

			// only look terrain parameters files
			vtString ext = GetExtension(name);
			if (ext.CompareNoCase(".ini") != 0 &&
				ext.CompareNoCase(".xml") != 0)
				continue;

			TParams params;
			bool success = params.LoadFrom(directory + "/" + name);

			wstr = name;
			if (success)
			{
				wstr += _T(" (");
				wstr2.from_utf8(params.GetValueString(STR_NAME));
				wstr += wstr2;
				wstr += _T(")");
			}

			wxTreeItemId hItem = m_pTree->AppendItem(hPath, wstr);
			TMTreeItemData *data = new TMTreeItemData;
			data->m_strDir = directory;
			data->m_strIniFile = name;
			data->m_strName = params.GetValueString(STR_NAME);
			m_pTree->SetItemData(hItem, data);
		}
		m_pTree->Expand(hPath);
	}
	m_pTree->Expand(m_Root);
}

void TerrainManagerDlg::RefreshTreeText()
{
	wxTreeItemId i1, i2;
	long cookie1, cookie2;
	TParams params;
	wxString2 wstr, wstr2;

	for (i1 = m_pTree->GetFirstChild(m_Root, cookie1); i1.IsOk(); i1 = m_pTree->GetNextChild(i1, cookie1))
	{
		for (i2 = m_pTree->GetFirstChild(i1, cookie2); i2.IsOk(); i2 = m_pTree->GetNextChild(i2, cookie2))
		{
			TMTreeItemData *data = (TMTreeItemData *) m_pTree->GetItemData(i2);
			wxString2 path = data->m_strDir + "/" + data->m_strIniFile;
			if (params.LoadFrom(path.mb_str()))
			{
				data->m_strName = params.GetValueString(STR_NAME);
				wstr = data->m_strIniFile;
				wstr += _T(" (");
				wstr2.from_utf8(params.GetValueString(STR_NAME));
				wstr += wstr2;
				wstr += _T(")");
				m_pTree->SetItemText(i2, wstr);
			}
		}
	}
}

wxString TerrainManagerDlg::GetCurrentPath()
{
	wxTreeItemId parent = m_pTree->GetItemParent(m_Selected);
	return m_pTree->GetItemText(parent);
}

wxString TerrainManagerDlg::GetCurrentTerrainPath()
{
	wxString2 path = GetCurrentPath();
	TMTreeItemData *data = (TMTreeItemData *) m_pTree->GetItemData(m_Selected);
	path += "Terrains/";
	path += data->m_strIniFile;
	return path;
}

// WDR: handler implementations for TerrainManagerDlg

void TerrainManagerDlg::OnCopy( wxCommandEvent &event )
{
	if (m_iSelect != 2)
		return;

	TMTreeItemData *data = (TMTreeItemData *) m_pTree->GetItemData(m_Selected);
	wxString2 file = data->m_strIniFile;

	wxString2 msg = "Please enter the name for the terrain copy.";
	wxString2 str = wxGetTextFromUser(msg, _T("Add Copy of Terrain"), file);
	if (str == _T(""))
		return;

	TParams params;
	params.LoadFrom(GetCurrentTerrainPath().mb_str());

	wxString2 newpath = GetCurrentPath();
	newpath += "Terrains/";
	newpath += str;
	params.WriteToXML(newpath.mb_str(), STR_TPARAMS_FORMAT_NAME);
	RefreshTreeContents();
}

void TerrainManagerDlg::OnEditParams( wxCommandEvent &event )
{
	if (m_iSelect != 2)
		return;

	int res = EditTerrainParameters(this, GetCurrentTerrainPath().mb_str());
	if (res == wxID_OK)
	{
		// They might have changed the terrain name
		RefreshTreeText();
	}
}

void TerrainManagerDlg::OnDelete( wxCommandEvent &event )
{
	vtStringArray &paths = g_Options.m_DataPaths;
	if (m_iSelect == 1)
	{
		// remove path
		wxString2 path = m_pTree->GetItemText(m_Selected);
		for (vtStringArray::iterator it = paths.begin(); it != paths.end(); it++)
		{
			if (*it == vtString(path.mb_str()))
			{
				paths.erase(it);
				break;
			}
		}
	}
	if (m_iSelect == 2)
	{
		// delete terrain .ini file
		wxTreeItemId parent = m_pTree->GetItemParent(m_Selected);
		wxString2 path = m_pTree->GetItemText(parent);
		path += "Terrains/";

		TMTreeItemData *data = (TMTreeItemData *) m_pTree->GetItemData(m_Selected);
		path += data->m_strIniFile;
		vtDeleteFile(path.mb_str());
	}
	RefreshTreeContents();
}

void TerrainManagerDlg::OnAddTerrain( wxCommandEvent &event )
{
	wxString2 msg = "Please enter the name for a new terrain .xml file.";
	wxString2 str = wxGetTextFromUser(msg, _T("Add Terrain"));
	if (str == _T(""))
		return;

	if (str.Right(4).CmpNoCase(_T(".xml")))
		str += ".xml";

	wxString2 path = m_pTree->GetItemText(m_Selected);
	path += "Terrains";

	// Make sure Terrains directory exists
	vtCreateDir(path.mb_str());

	path += "/";
	path += str;

	TParams params;
	params.SetValueString(STR_NAME, "Untitled");
	params.WriteToXML(path.mb_str(), STR_TPARAMS_FORMAT_NAME);

	RefreshTreeContents();
}

void TerrainManagerDlg::OnAddPath( wxCommandEvent &event )
{
	wxString2 msg = "Please enter an absolute or relative path.";
	wxString2 str = wxGetTextFromUser(msg, _T("Add Path"));
	if (str == _T(""))
		return;

	// Make sure there is a trailing slash
	if (str.Length() > 1)
	{
		char ch = str.GetChar(str.Length()-1);
		if (ch != '/' && ch != '\\')
			str += _T("/");
	}

	g_Options.m_DataPaths.push_back(vtString(str.mb_str()));
	RefreshTreeContents();
}

void TerrainManagerDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_pTree = GetTree();

	RefreshTreeContents();
	UpdateEnabling();
	wxWindow::OnInitDialog(event);
}

void TerrainManagerDlg::OnOK( wxCommandEvent &event )
{
	wxDialog::OnOK(event);
}


//
// Tree events
//
void TerrainManagerDlg::OnDeleteItem( wxTreeEvent &event )
{
}

void TerrainManagerDlg::OnSelChanged( wxTreeEvent &event )
{
	wxTreeItemId item = event.GetItem();
	m_Selected = item;

//  MyTreeItemData *data = (MyTreeItemData *)m_pTree->GetItemData(item);

	if (item == m_Root)
		m_iSelect = 0;
	else
	{
		item = m_pTree->GetItemParent(item);
		if (item == m_Root)
			m_iSelect = 1;
		else
		{
			item = m_pTree->GetItemParent(item);
			if (item == m_Root)
				m_iSelect = 2;
		}
	}
	UpdateEnabling();
}

void TerrainManagerDlg::UpdateEnabling()
{
	GetAddTerrain()->Enable(m_iSelect == 1);
	GetCopy()->Enable(m_iSelect == 2);
	GetDelete()->Enable(m_iSelect == 1 || m_iSelect == 2);
	if (m_iSelect == 1)
		GetDelete()->SetLabel(_T("Remove"));
	else if (m_iSelect == 2)
		GetDelete()->SetLabel(_T("Delete"));
	GetEditParams()->Enable(m_iSelect == 2);
}

