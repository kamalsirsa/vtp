//
// Name: ProfileDlg.h
//
// Copyright (c) 2004-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ProfileDlg_H__
#define __ProfileDlg_H__

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "vtui_wdr.h"
#include "AutoDialog.h"
#include "vtdata/HeightField.h"
#include "wxString2.h"

class ProfileCallback
{
public:
	virtual void Begin() {}
	virtual float GetElevation(const DPoint2 &p) = 0;
};

// WDR: class declarations

//----------------------------------------------------------------------------
// ProfileDlg
//----------------------------------------------------------------------------

class ProfileDlg: public AutoDialog
{
public:
	// constructors and destructors
	ProfileDlg( wxWindow *parent, wxWindowID id,
		const wxString& title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for ProfileDlg
	void DrawChart(wxDC& dc);
	void SetPoints(const DPoint2 &p1, const DPoint2 &p2);
	void SetCallback(ProfileCallback *callback);
	void GetValues();

private:
	// WDR: member variable declarations for ProfileDlg
	ProfileCallback *m_callback;
	std::vector<float> m_values;
	DPoint2 m_p1, m_p2;
	wxSize m_clientsize;
	int m_xrange, m_yrange;
	float m_fMin, m_fMax, m_fRange;
	bool m_bHavePoints, m_bHaveValues;

private:
	// WDR: handler declarations for ProfileDlg
	void OnPaint(wxPaintEvent &event);
	void OnDraw(wxDC& dc); // overridden to draw this view
	void OnSize(wxSizeEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __ProfileDlg_H__

