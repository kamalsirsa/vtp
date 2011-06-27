///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __wxosg_UI__
#define __wxosg_UI__

#include <wx/intl.h>

#include <wx/treectrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

#define ID_SCENETREE 1000
#define ID_ENABLED 1001
#define ID_ZOOMTO 1002
#define ID_REFRESH 1003
#define ID_LOG 1004

///////////////////////////////////////////////////////////////////////////////
/// Class SceneGraphDlgBase
///////////////////////////////////////////////////////////////////////////////
class SceneGraphDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxTreeCtrl* m_scenetree;
		wxCheckBox* m_enabled;
		wxButton* m_zoomto;
		wxButton* m_refresh;
		wxButton* m_log;
	
	public:
		
		SceneGraphDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxTAB_TRAVERSAL );
		~SceneGraphDlgBase();
	
};

#endif //__wxosg_UI__
