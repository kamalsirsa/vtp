/////////////////////////////////////////////////////////////////////////////
// Name:        PerformanceMonitor.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __PerformanceMonitor_H__
#define __PerformanceMonitor_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "PerformanceMonitor.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "enviro_wdr.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// CPerformanceMonitorDialog
//----------------------------------------------------------------------------

class CPerformanceMonitorDialog: public wxDialog
{
public:
    // constructors and destructors
    CPerformanceMonitorDialog( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    
    // WDR: method declarations for CPerformanceMonitorDailog
    wxStaticText* GetFps()  { return (wxStaticText*) FindWindow( ID_FPS ); }

	// Static functions
	static void NVPM_init();
	static void NVPM_shutdown();
	static void NVPM_frame();
    
private:
    // WDR: member variable declarations for CPerformanceMonitorDialog
    
private:
    // WDR: handler declarations for CPerformanceMonitorDialog

private:
	static bool m_NVPMInitialised;
    DECLARE_EVENT_TABLE()
};



#define NVIDIA_PERFORMANCE_MONITOR_INIT CPerformanceMonitorDialog::NVPM_init();
#define NVIDIA_PERFORMANCE_MONITOR_SHUTDOWN CPerformanceMonitorDialog::NVPM_shutdown();
#define NVIDIA_PERFORMANCE_MONITOR_FRAME CPerformanceMonitorDialog::NVPM_frame();

#endif
