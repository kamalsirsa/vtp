//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VEGDLGH
#define VEGDLGH

#include "wx/splitter.h"
#include "wx/treectrl.h"
#include "wx/listctrl.h"
#include "wx/dialog.h"
#include "AutoDialog.h"

#define PLANTS_FILTER "Plant List Files (*.txt)|*.txt|"
#define BIOREGIONS_FILTER "Plant List Files (*.txt)|*.txt|"
#define WID_PLANTS 200
#define WID_PSPLITTER 201
#define WID_PSTABLE 202
#define WID_PATABLE 203
#define WID_BIOREGIONS 300
#define WID_BTREE 301

class PlantListCtrl : public wxListCtrl
{
public:
	PlantListCtrl(wxWindow *parent,
		wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxDefaultSize, long style = wxLC_ICON, 
		const wxValidator& validator = wxDefaultValidator, 
		const wxString& name = _T("wxListCtrl")) 
		: wxListCtrl (parent, id, pos, size, style, validator, name)
	{
	};

//	virtual ~PlantListCtrl();
	
	void OnSelect(wxListEvent &event);

	DECLARE_EVENT_TABLE()
};

class PlantListDlg : public AutoDialog
{
public:
	PlantListDlg(wxWindow *parent, wxWindowID id, const wxString& title,
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE)
		: AutoDialog(parent, id, title, pos, size, style) {}

	void OnInitDialog(wxInitDialogEvent& event);

	wxDialog *m_PDialog;
	wxSplitterWindow *m_PSplitter;
	PlantListCtrl *m_PSTable;
	wxListCtrl *m_PATable;

};

class BioRegionDlg : public AutoDialog
{
public:
	BioRegionDlg(wxWindow *parent, wxWindowID id, const wxString& title,
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE)
		: AutoDialog(parent, id, title, pos, size, style) {};
	
	void OnInitDialog(wxInitDialogEvent& event);

	wxTreeCtrl *m_BTree;
};

#endif
