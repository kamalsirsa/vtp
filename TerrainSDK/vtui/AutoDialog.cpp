//
// Implements the following classes:
//
// AutoDialog - An improvement to wxDialog which makes validation easier.
//
// AutoPanel - An improvement to wxPanel which makes validation easier.
//
// wxNumericValidator - A validator capable of transfering numeric values.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "AutoDialog.h"
#include "wxString2.h"

/////////////////////////////////////////////////
//

wxNumericValidator::wxNumericValidator(int *val) : wxValidator()
{
	Initialize();
	m_pValInt = val;
}

wxNumericValidator::wxNumericValidator(float *val, int digits) : wxValidator()
{
	Initialize();
	m_pValFloat = val;
	m_iDigits = digits;
}

wxNumericValidator::wxNumericValidator(double *val, int digits) : wxValidator()
{
	Initialize();
	m_pValDouble = val;
	m_iDigits = digits;
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
	m_iDigits = val.m_iDigits;

	return TRUE;
}

// Called to transfer data to the window
bool wxNumericValidator::TransferToWindow()
{
	if ( !m_validatorWindow )
		return FALSE;

	wxString str, format;
	if (m_pValInt)
		str.Printf(_T("%d"), *m_pValInt);
	if (m_pValFloat)
	{
		if (m_iDigits != -1)
		{
			format.Printf(_T("%%.%df"), m_iDigits);
			str.Printf(format, *m_pValFloat);
		}
		else
			str.Printf(_T("%.8g"), *m_pValFloat);	// 8 significant digits
	}
	if (m_pValDouble)
	{
		if (m_iDigits != -1)
		{
			format.Printf(_T("%%.%dlf"), m_iDigits);
			str.Printf(format, *m_pValDouble);
		}
		else
			str.Printf(_T("%.16lg"), *m_pValDouble);	// 16 significant digits
	}

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

	wxString2 str = _T("");
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

	if (str != _T(""))
	{
		const char *ccs = str.mb_str();
		if (m_pValInt)
			sscanf(ccs, "%d", m_pValInt);
		if (m_pValFloat)
			sscanf(ccs, "%f", m_pValFloat);
		if (m_pValDouble)
			sscanf(ccs, "%lf", m_pValDouble);
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

BEGIN_EVENT_TABLE(AutoDialog, wxDialog)
EVT_INIT_DIALOG (AutoDialog::OnInitDialog)
END_EVENT_TABLE()

void AutoDialog::AddValidator(long id, wxString *sptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return;
	wxGenericValidator *gv = new wxGenericValidator(sptr);
	pWin->SetValidator(*gv);	// actually clones the one we pass in
	delete gv;
}

void AutoDialog::AddValidator(long id, bool *bptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return;
	wxGenericValidator *gv = new wxGenericValidator(bptr);
	pWin->SetValidator(*gv);	// actually clones the one we pass in
	delete gv;
}

void AutoDialog::AddValidator(long id, int *iptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return;
	wxGenericValidator *gv = new wxGenericValidator(iptr);
	pWin->SetValidator(*gv);	// actually clones the one we pass in
	delete gv;
}

void AutoDialog::AddNumValidator(long id, int *iptr)
{
	wxWindow *pWin = FindWindow(id);
	if (pWin)
	{
		wxNumericValidator *gv = new wxNumericValidator(iptr);
		pWin->SetValidator(*gv);	// actually clones the one we pass in
		delete gv;
	}
}

void AutoDialog::AddNumValidator(long id, float *fptr, int digits)
{
	wxWindow *pWin = FindWindow(id);
	if (pWin)
	{
		wxNumericValidator *gv = new wxNumericValidator(fptr, digits);
		pWin->SetValidator(*gv);	// actually clones the one we pass in
		delete gv;
	}
}

void AutoDialog::AddNumValidator(long id, double *dptr, int digits)
{
	wxWindow *pWin = FindWindow(id);
	if (pWin)
	{
		wxNumericValidator *gv = new wxNumericValidator(dptr, digits);
		pWin->SetValidator(*gv);	// actually clones the one we pass in
		delete gv;
	}
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
	pWin->SetValidator(*gv);	// actually clones the one we pass in
	delete gv;
}

void AutoPanel::AddValidator(long id, bool *bptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return;
	wxGenericValidator *gv = new wxGenericValidator(bptr);
	pWin->SetValidator(*gv);	// actually clones the one we pass in
	delete gv;
}

void AutoPanel::AddValidator(long id, int *iptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return;
	wxGenericValidator *gv = new wxGenericValidator(iptr);
	pWin->SetValidator(*gv);	// actually clones the one we pass in
	delete gv;
}

void AutoPanel::AddNumValidator(long id, int *iptr)
{
	wxWindow *pWin = FindWindow(id);
	if (pWin)
	{
		wxNumericValidator *gv = new wxNumericValidator(iptr);
		pWin->SetValidator(*gv);	// actually clones the one we pass in
		delete gv;
	}
}

void AutoPanel::AddNumValidator(long id, float *fptr, int digits)
{
	wxWindow *pWin = FindWindow(id);
	if (pWin)
	{
		wxNumericValidator *gv = new wxNumericValidator(fptr, digits);
		pWin->SetValidator(*gv);	// actually clones the one we pass in
		delete gv;
	}
}

void AutoPanel::AddNumValidator(long id, double *dptr, int digits)
{
	wxWindow *pWin = FindWindow(id);
	if (pWin)
	{
		wxNumericValidator *gv = new wxNumericValidator(dptr, digits);
		pWin->SetValidator(*gv);	// actually clones the one we pass in
		delete gv;
	}
}


