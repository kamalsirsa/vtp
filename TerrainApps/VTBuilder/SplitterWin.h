//
// SplitterWindow.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/splitter.h"

class MySplitterWindow : public wxSplitterWindow
{
public:
  MySplitterWindow(wxFrame *parent, wxWindowID id) 
    : wxSplitterWindow(parent, id, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE)
  {
    m_frame = parent;
  }

  virtual bool OnSashPositionChange(int newSashPosition)
  {
    if ( !wxSplitterWindow::OnSashPositionChange(newSashPosition) )
      return FALSE;
    
    wxString str;
    str.Printf( _T("Sash position = %d"), newSashPosition);
    m_frame->SetStatusText(str);

    return TRUE;
  }
  
private:
  wxFrame *m_frame;
};

