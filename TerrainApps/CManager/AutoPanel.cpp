//
// Implements the following classes:
//
// AutoPanel - An improvement to wxDialog which maked validation easier.
//
// wxNumericValidator - A validator capable of transfering numeric values.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"
#include "AutoPanel.h"

/////////////////////////////////////////////////
//

wxNumericValidator::wxNumericValidator(int *val) : wxValidator()
{
    Initialize();
    m_pValInt = val;
}

wxNumericValidator::wxNumericValidator(float *val) : wxValidator()
{
    Initialize();
    m_pValFloat = val;
}

wxNumericValidator::wxNumericValidator(double *val) : wxValidator()
{
    Initialize();
    m_pValDouble = val;
}

wxNumericValidator::wxNumericValidator(const wxNumericValidator& val)
{
    Copy(val);
}

bool wxNumericValidator::Copy(const wxNumericValidator& val)
{
    wxValidator::Copy(val);

    m_pValInt = val.m_pValInt;
    m_pValFloat = val.m_pValFloat;
    m_pValDouble = val.m_pValDouble;

    return TRUE;
}

// Called to transfer data to the window
bool wxNumericValidator::TransferToWindow()
{
    if ( !m_validatorWindow )
        return FALSE;

	wxString str;
	if (m_pValInt)
		str = wxString::Format("%d", *m_pValInt);
	if (m_pValFloat)
		str = wxString::Format("%f", *m_pValFloat);
	if (m_pValDouble)
		str = wxString::Format("%lf", *m_pValDouble);

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxStaticText)) )
    {
        wxStaticText* pControl = (wxStaticText*) m_validatorWindow;
		if (pControl)
		{
			pControl->SetLabel(str) ;
			return TRUE;
		}
    }
	else 
    if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) )
    {
        wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;
		if (pControl)
		{
			pControl->SetValue(str) ;
			return TRUE;
		}
    }
	else
		return FALSE;

	// unrecognized control, or bad pointer
	return FALSE;
}

// Called to transfer data from the window
bool wxNumericValidator::TransferFromWindow()
{
	if ( !m_validatorWindow )
		return FALSE;

	wxString str = "";
	// string controls
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxStaticText)) )
	{
		wxStaticText* pControl = (wxStaticText*) m_validatorWindow;
		if (pControl)
			str = pControl->GetLabel() ;
	} else
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) )
	{
		wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;
		if (pControl)
			str = pControl->GetValue() ;
	}
	else // unrecognized control, or bad pointer
		return FALSE;

	if (str != "")
	{
		if (m_pValInt)
			sscanf(str, "%d", m_pValInt);
		if (m_pValFloat)
			sscanf(str, "%f", m_pValFloat);
		if (m_pValDouble)
			sscanf(str, "%lf", m_pValDouble);
		return TRUE;
	}
	return FALSE;
}

/*
  Called by constructors to initialize ALL data members
*/
void wxNumericValidator::Initialize()
{
    m_pValInt = NULL;
    m_pValFloat = NULL;
    m_pValDouble = NULL;
}

/////////////////////////////////////////////////
//

BEGIN_EVENT_TABLE(AutoPanel, wxPanel)
EVT_INIT_DIALOG (AutoPanel::OnInitDialog)
END_EVENT_TABLE()

void AutoPanel::AddValidator(long id, wxString *sptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return;
	wxGenericValidator *gv = new wxGenericValidator(sptr);
	pWin->SetValidator(*gv);
}

void AutoPanel::AddValidator(long id, bool *bptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return;
	wxGenericValidator *gv = new wxGenericValidator(bptr);
	pWin->SetValidator(*gv);
}

void AutoPanel::AddValidator(long id, int *iptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return;
	wxGenericValidator *gv = new wxGenericValidator(iptr);
	pWin->SetValidator(*gv);
}

void AutoPanel::AddNumValidator(long id, int *iptr)
{
	wxWindow *pWin = FindWindow(id);
	if (pWin)
	{
		wxNumericValidator *gv = new wxNumericValidator(iptr);
		pWin->SetValidator(*gv);
	}
}

void AutoPanel::AddNumValidator(long id, float *fptr)
{
	wxWindow *pWin = FindWindow(id);
	if (pWin)
	{
		wxNumericValidator *gv = new wxNumericValidator(fptr);
		pWin->SetValidator(*gv);
	}
}

void AutoPanel::AddNumValidator(long id, double *dptr)
{
	wxWindow *pWin = FindWindow(id);
	if (pWin)
	{
		wxNumericValidator *gv = new wxNumericValidator(dptr);
		pWin->SetValidator(*gv);
	}
}


