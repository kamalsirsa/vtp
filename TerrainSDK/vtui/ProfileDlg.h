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
	wxTextCtrl* GetHeight2()  { return (wxTextCtrl*) FindWindow( ID_HEIGHT2 ); }
	wxTextCtrl* GetHeight1()  { return (wxTextCtrl*) FindWindow( ID_HEIGHT1 ); }
	wxTextCtrl* GetText()  { return (wxTextCtrl*) FindWindow( ID_TEXTCTRL ); }
	void MakePoint(wxPoint &p, int i, float value);
	void DrawChart(wxDC& dc);
	void UpdateMessageText();
	void SetPoints(const DPoint2 &p1, const DPoint2 &p2);
	void SetCallback(ProfileCallback *callback);
	void GetValues();
	void Analyze();
	void ComputeLineOfSight();
	void ComputeVisibility();

private:
	// WDR: member variable declarations for ProfileDlg
	ProfileCallback *m_callback;
	std::vector<float> m_values;
	std::vector<bool> m_visible;
	DPoint2 m_p1, m_p2;
	wxSize m_clientsize;
	int m_xrange, m_yrange;
	float m_fMin, m_fMax, m_fRange, m_fDrawRange;
	float m_fMinDist, m_fMaxDist, m_fTotalDist;
	bool m_bHavePoints, m_bHaveValues;
	bool m_bHaveValidData, m_bHaveInvalid;
	bool m_bMouseOnLine;
	float m_fMouse, m_fMouseDist;
	int m_iMin, m_iMax, m_iMouse;
	wxPoint m_base;
	bool m_bHaveSlope;
	float m_fSlope;
	bool m_bLeftButton;
	bool m_bLineOfSight, m_bVisibility;
	bool m_bValidStart, m_bValidLine;
	float m_fHeight1, m_fHeight2;
	float m_fHeightAtStart, m_fHeightAtEnd;
	bool m_bIntersectsGround;
	float m_fIntersectHeight;
	float m_fIntersectDistance;
	int m_iIntersectIndex;

private:
	// WDR: handler declarations for ProfileDlg
	void OnHeight2( wxCommandEvent &event );
	void OnHeight1( wxCommandEvent &event );
	void OnLineOfSight( wxCommandEvent &event );
	void OnVisibility( wxCommandEvent &event );
	void OnPaint(wxPaintEvent &event);
	void OnDraw(wxDC& dc); // overridden to draw this view
	void OnSize(wxSizeEvent& event);
	void OnLeftDown(wxMouseEvent& event);
	void OnLeftUp(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __ProfileDlg_H__

