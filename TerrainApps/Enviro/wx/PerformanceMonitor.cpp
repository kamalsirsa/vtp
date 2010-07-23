/////////////////////////////////////////////////////////////////////////////
// Name:        PerformanceMonitor.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "PerformanceMonitor.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "PerformanceMonitor.h"
#include "NVPerfSDK.h"
#include "vtlib/vtlib.h"
class vtStructInstance;
#include "EnviroFrame.h"
#include "wx/valgen.h"

static NVPMSampleValue PMSamples[20];


// WDR: class implementations

//----------------------------------------------------------------------------
// CPerformanceMonitorDialog
//----------------------------------------------------------------------------

bool CPerformanceMonitorDialog::m_NVPMInitialised = false;


// WDR: event table for CPerformanceMonitorDialog

BEGIN_EVENT_TABLE(CPerformanceMonitorDialog,wxDialog)
END_EVENT_TABLE()

CPerformanceMonitorDialog::CPerformanceMonitorDialog( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( parent, id, title, position, size, style )
{
    // WDR: dialog function PerformanceMonitorDialogFunc for CPerformanceMonitorDialog
    PerformanceMonitorDialogFunc( this, TRUE );
    GetFps()->SetValidator(wxGenericValidator(&m_FPS));
}

void CPerformanceMonitorDialog::NVPM_init()
{
	NVPMRESULT Result;

	if (NVPM_OK != NVPMInit())
		return;
	m_NVPMInitialised = true;
	Result = NVPMAddCounterByName("OGL FPS");
}

void CPerformanceMonitorDialog::NVPM_shutdown()
{
	m_NVPMInitialised = false;
	NVPMShutdown();
}

void CPerformanceMonitorDialog::NVPM_frame()
{
	NVPMRESULT Result;
	UINT Count = 20;

	if (m_NVPMInitialised)
	{
	    CPerformanceMonitorDialog *pPM = GetFrame()->m_pPerformanceMonitorDlg;

		Result = NVPMSample(PMSamples, &Count);
		if (NULL != pPM)
		{
		    pPM->UpdateCounters();
		}
	}
}

void CPerformanceMonitorDialog::UpdateCounters()
{
    m_FPS.Empty();
    m_FPS << PMSamples[0].ulValue;
    TransferDataToWindow();
}

// WDR: handler implementations for CPerformanceMonitorDialog




