//
// ScaledView.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef SCALEDVIEWH
#define SCALEDVIEWH

#include "vtdata/MathTypes.h"

class vtScaledView : public wxScrolledWindow
{
public:
	vtScaledView(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, const wxString& name = _T(""));

	void SetScale(double scale);
	double GetScale();

	void ZoomAll();
	void ZoomToPoint(const FPoint2 &p);
	void ZoomToRect(const DRECT &geo_rect, float margin);
	void ZoomOutToRect(const DRECT &geo_rect);

	wxRect WorldToCanvas(const DRECT &r);
	wxRect WorldToWindow(const DRECT &r);
	DRECT CanvasToWorld(const wxRect &r);

	void GetCanvasPosition(const wxMouseEvent &event, wxPoint &pos);
	DRECT GetWorldRect();
	wxRect PointsToRect(const wxPoint &p1, const wxPoint &p2);

	// transform object space -> screen space
	int	sx(double x) { return (int)(x*m_fScale - m_limits.x); }
	int	sy(double y) { return (int)(-y*m_fScale - m_limits.y); }
	void screen(const DPoint2 &p, wxPoint &sp) const
	{
		sp.x = (int)(p.x*m_fScale - m_limits.x);
		sp.y = (int)(-p.y*m_fScale - m_limits.y);
	}
	// transform object space -> screen space (relative delta)
	int	sdx(double x) { return (int)(x*m_fScale); }
	int	sdy(double y) { return (int)(-y*m_fScale); }

	wxPoint screen_delta(const DPoint2 &p) const
	{
		wxPoint sp;
		sp.x = (int)(p.x*m_fScale);
		sp.y = (int)(-p.y*m_fScale);
		return sp;
	}

	// transform screen space -> object space
	double ox(int x) { return (x + m_limits.x) / m_fScale; }
	double oy(int y) { return -(y + m_limits.y) / m_fScale; }
	void object(const wxPoint &sp, DPoint2 &p) const
	{
		p.x = (sp.x + m_limits.x) / m_fScale;
		p.y = -(sp.y + m_limits.y) / m_fScale;
	}

	// transform screen space -> object space (relative delta)
	double odx(int x) { return x/m_fScale; }
	double ody(int y) { return -y/m_fScale; }

	void DrawDLine(wxDC *pDC, const DLine2 &line, bool bClose);

protected:
	void UpdateRanges();

	double	m_fScale;	// pixels per UTM meter/pixel per degree
	wxRect	m_limits;	// allowed range of m_offset
};

#define SCREENBUF_SIZE 12000
extern wxPoint g_screenbuf[SCREENBUF_SIZE];

#endif
