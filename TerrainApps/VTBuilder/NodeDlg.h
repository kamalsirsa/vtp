/////////////////////////////////////////////////////////////////////////////
// Name:        NodeDlg.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __NodeDlg_H__
#define __NodeDlg_H__

#ifdef __GNUG__
    #pragma interface "NodeDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "VTBuilder_wdr.h"
#include "AutoDialog.h"

class NodeEdit;
class vtRoadLayer;
enum VisualIntersectionType;

// WDR: class declarations

//----------------------------------------------------------------------------
// NodeDlg
//----------------------------------------------------------------------------

class NodeDlg: public AutoDialog
{
public:
    // constructors and destructors
    NodeDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    
    void SetNode(NodeEdit *pSingleRoad, vtRoadLayer *pLayer);
	void ApplyVisualToNode(NodeEdit *pNode, VisualIntersectionType vitype);

    // WDR: method declarations for NodeDlg
    wxListBox* GetBehavior()  { return (wxListBox*) FindWindow( ID_BEHAVIOR ); }
    wxListBox* GetRoadNum()  { return (wxListBox*) FindWindow( ID_ROADNUM ); }
    wxListBox* GetIntType()  { return (wxListBox*) FindWindow( ID_INTTYPE ); }
    
private:
    // WDR: member variable declarations for NodeDlg
    NodeEdit *m_pNode;
    vtRoadLayer *m_pLayer;

private:
    // WDR: handler declarations for NodeDlg
    void OnBehavior( wxCommandEvent &event );
    void OnRoadNum( wxCommandEvent &event );
    void OnIntType( wxCommandEvent &event );
    void OnOK( wxCommandEvent &event );
    void OnInitDialog(wxInitDialogEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnDraw(wxDC &dc);

private:
    DECLARE_EVENT_TABLE()
};




#endif
