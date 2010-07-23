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
    wxStaticText* GetPmCulledPrimitiveCount()  { return (wxStaticText*) FindWindow( ID_PM_culled_primitive_count ); }
    wxStaticText* GetPmPrimitiveCount()  { return (wxStaticText*) FindWindow( ID_PM_primitive_count ); }
    wxStaticText* GetPmTriangleCount()  { return (wxStaticText*) FindWindow( ID_PM_triangle_count ); }
    wxStaticText* GetPmVertexCount()  { return (wxStaticText*) FindWindow( ID_PM_vertex_count ); }
    wxStaticText* GetPmShadedPixelCount()  { return (wxStaticText*) FindWindow( ID_PM_shaded_pixel_count ); }
    wxStaticText* GetPmVertexShaderBusy()  { return (wxStaticText*) FindWindow( ID_PM_vertex_shader_busy ); }
    wxStaticText* GetPmVertexAttributeCount()  { return (wxStaticText*) FindWindow( ID_PM_vertex_attribute_count ); }
    wxStaticText* GetPmFastZCount()  { return (wxStaticText*) FindWindow( ID_PM_fast_z_count ); }
    wxStaticText* GetPmRopBusy()  { return (wxStaticText*) FindWindow( ID_PM_rop_busy ); }
    wxStaticText* GetPmShaderWaitsForRop()  { return (wxStaticText*) FindWindow( ID_PM_shader_waits_for_rop ); }
    wxStaticText* GetPmShaderWaitsForTexture()  { return (wxStaticText*) FindWindow( ID_PM_shader_waits_for_texture ); }
    wxStaticText* GetPmPixelShaderBusy()  { return (wxStaticText*) FindWindow( ID_PM_pixel_shader_busy ); }
    wxStaticText* GetPmGpuIdlePercent()  { return (wxStaticText*) FindWindow( ID_PM_GPU_IDLE_PERCENT ); }
    wxStaticText* GetPmOglDriverWaitPercent()  { return (wxStaticText*) FindWindow( ID_PM_OGL_DRIVER_WAIT_PERCENT ); }
    wxStaticText* GetFps()  { return (wxStaticText*) FindWindow( ID_FPS ); }

    // Static functions
    static void NVPM_init();
    static void NVPM_shutdown();
    static void NVPM_frame();

private:
    // WDR: member variable declarations for CPerformanceMonitorDialog
    wxString m_FPS;

private:
    // WDR: handler declarations for CPerformanceMonitorDialog

private:
    void UpdateCounters();
    static bool m_NVPMInitialised;
    DECLARE_EVENT_TABLE()
};



#define NVIDIA_PERFORMANCE_MONITOR_INIT CPerformanceMonitorDialog::NVPM_init();
#define NVIDIA_PERFORMANCE_MONITOR_SHUTDOWN CPerformanceMonitorDialog::NVPM_shutdown();
#define NVIDIA_PERFORMANCE_MONITOR_FRAME CPerformanceMonitorDialog::NVPM_frame();

#endif
