//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ELEVDLGH
#define ELEVDLGH

#include "vtdata/MathTypes.h"
#include "AutoDialog.h"

class ResampleDlg : public AutoDialog
{
public:
	ResampleDlg(bool bmeters, float fSampleFactor);

	void OnInitDialog(wxInitDialogEvent& event);
	void OnRadio(wxCommandEvent &event);
	void OnChangeSamplefactor(wxCommandEvent& event);
    void OnButtonLess( wxCommandEvent &event );
    void OnButtonMore( wxCommandEvent &event );

	wxString	m_strCurrentSize;
	int			m_iXSamples;
	int			m_iYSamples;
	float		m_fSampleFactor;
	wxString	m_strSaveArea;
	wxString	m_strSFLabel;
	bool		m_bConstraint;

	DRECT		m_area;
	int			m_power;
	bool		m_bMeters;

private:
	wxWindow	*m_pLess, *m_pMore;

	void RecomputeSize();
	void RecomputeSamples();

    DECLARE_EVENT_TABLE()
};

class ScaleDlg : public AutoDialog
{
public:
	void OnInitDialog(wxInitDialogEvent& event);

	bool		m_bScaleUp;
	wxString	m_strUpBy;
	wxString	m_strDownBy;

private:
    DECLARE_EVENT_TABLE()
};

#endif
