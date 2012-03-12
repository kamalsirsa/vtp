//
// Name:		VIADlg.cpp
//
// Copyright (c) Beard and Sandals Ltd.
//

#ifdef __GNUG__
	#pragma implementation "VIADlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "VIADlg.h"
#include "../Enviro.h"
#include "vtdata/LocalConversion.h"

//----------------------------------------------------------------------------
// VIADlg
//----------------------------------------------------------------------------

void VIADlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_VIF = wxString::Format(_("%0.2f"), vtGetScene()->GetVisualImpactCalculator().Calculate());
	wxDialog::OnInitDialog(event);
}

void VIADlg::OnRecalculate( wxCommandEvent &event)
{
	m_VIF = wxString::Format(_("%0.2f"), vtGetScene()->GetVisualImpactCalculator().Calculate());
	TransferDataToWindow();
}

