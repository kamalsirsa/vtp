//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef EXTENTDLG2H
#define EXTENTDLG2H

#include "vtdata/MathTypes.h"
#include "AutoDialog.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExtentDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExtentDlg dialog

class CExtentDlg : public AutoDialog
{
// Construction
public:
	CExtentDlg(DRECT area, bool bUTM);   // standard constructor

	wxString	m_strAll;
	wxString	m_strEast;
	wxString	m_strNorth;
	wxString	m_strSouth;
	wxString	m_strWest;

	DRECT m_area;
	bool m_bUTM;

// Implementation
protected:
	void FormatStrings();

	void OnInitDialog(wxInitDialogEvent& event);
	void OnChangeExtentE(wxCommandEvent&);
	void OnChangeExtentN(wxCommandEvent&);
	void OnChangeExtentS(wxCommandEvent&);
	void OnChangeExtentW(wxCommandEvent&);
	void OnChangeExtentAll(wxCommandEvent&);

    DECLARE_EVENT_TABLE()
};

#endif
