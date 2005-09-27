//
// Name: ProfileDlg.cpp
//
// Copyright (c) 2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ProfileDlg.h"

#define MARGIN_LEFT   60
#define MARGIN_BOTTOM 170

// WDR: class implementations

//----------------------------------------------------------------------------
// ProfileDlg
//----------------------------------------------------------------------------

// WDR: event table for ProfileDlg

BEGIN_EVENT_TABLE(ProfileDlg, AutoDialog)
	EVT_PAINT(ProfileDlg::OnPaint)
	EVT_SIZE(ProfileDlg::OnSize)
	EVT_LEFT_DOWN(ProfileDlg::OnLeftDown)
	EVT_LEFT_UP(ProfileDlg::OnLeftUp)
	EVT_MOTION(ProfileDlg::OnMouseMove)
	EVT_CHECKBOX( ID_LINE_OF_SIGHT, ProfileDlg::OnLineOfSight )
	EVT_CHECKBOX( ID_VISIBILITY, ProfileDlg::OnVisibility )
	EVT_TEXT( ID_HEIGHT1, ProfileDlg::OnHeight1 )
	EVT_TEXT( ID_HEIGHT2, ProfileDlg::OnHeight2 )
	EVT_CHECKBOX( ID_FRESNEL, ProfileDlg::OnFresnel )
	EVT_CHECKBOX( ID_USE_EFFECTIVE, ProfileDlg::OnUseEffective )
	EVT_TEXT( ID_RF, ProfileDlg::OnRF )
	EVT_CHOICE( ID_CURVATURE, ProfileDlg::OnCurvature )
END_EVENT_TABLE()


ProfileDlg::ProfileDlg( wxWindow *parent, wxWindowID id,
	const wxString& title, const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style | wxRESIZE_BORDER | wxMAXIMIZE)
{
	m_callback = NULL;
	m_clientsize.Set(0, 0);
	m_bHavePoints = false;
	m_bHaveValues = false;
	m_bMouseOnLine = false;
	m_bLeftButton = false;
	m_fGeodesicDistance=0;
	m_bHaveLOS = false;
	m_bHaveFresnel = false;
	m_bHaveGeoidSurface=false;
	m_bLineOfSight = false;
	m_bVisibility = false;
	m_bUseFresnel = false;
	m_fHeight1 = 1;
	m_fHeight2 = 1;
	m_xrange = 0;
	m_fRadioFrequency = 2400.0;
	m_iCurvature = 0;

	// WDR: dialog function ColorMapDialogFunc for ProfileDlg
	ProfileDialogFunc( this, TRUE );

	AddNumValidator(ID_HEIGHT1, &m_fHeight1);
	AddNumValidator(ID_HEIGHT2, &m_fHeight2);
	AddNumValidator(ID_RF, &m_fRadioFrequency);
	AddValidator(ID_CURVATURE, &m_iCurvature);

	GetHeight1()->Enable(false);
	GetHeight2()->Enable(false);

	SetBackgroundColour(wxColour(255,255,255));

	GetCurvature()->SetSelection(m_iCurvature);
	UpdateEnabling();
}

void ProfileDlg::SetProjection(const vtProjection &proj)
{
	m_proj = proj;
}

void ProfileDlg::SetPoints(const DPoint2 &p1, const DPoint2 &p2)
{
	m_p1 = p1;
	m_p2 = p2;
	m_bHavePoints = true;
	m_bHaveValues = false;
	m_bHaveLOS = false;
	m_bHaveFresnel = false;
	m_bHaveGeoidSurface = false;

	m_geo1 = m_p1;
	m_geo2 = m_p2;
	if (!m_proj.IsGeographic())
	{
		// convert points to geographic CS
		vtProjection geo;
		CreateSimilarGeographicProjection(m_proj, geo);
		OCT *trans = CreateCoordTransform(&m_proj, &geo);
		trans->Transform(1, &m_geo1.x, &m_geo1.y);
		trans->Transform(1, &m_geo2.x, &m_geo2.y);
		m_fGeodesicDistance=vtProjection::GeodesicDistance(m_p1,m_p2);
		delete trans;
	}
	m_fGeodesicDistance=vtProjection::GeodesicDistance(m_geo1,m_geo2);

	Refresh();
	UpdateEnabling();
}

void ProfileDlg::SetCallback(ProfileCallback *callback)
{
	m_callback = callback;
}

void ProfileDlg::GetValues()
{
	if (!m_callback)
		return;

	m_clientsize = GetClientSize();

	// compute how large an area we have to draw the chart
	m_xrange = m_clientsize.x - MARGIN_LEFT - 10;
	if (m_xrange < 2) m_xrange = 2;
	m_yrange = m_clientsize.y - MARGIN_BOTTOM - 10;
	if (m_yrange < 2) m_yrange = 2;

	// fill array with elevation values, collect extents
	m_fMin = 1E9;
	m_fMax = -1E9;
	m_values.resize(m_xrange);
	m_callback->Begin();
	DPoint2 p;
	m_bHaveValidData = false;
	m_bHaveInvalid = false;
	int i;
	for (i = 0; i < m_xrange; i++)
	{
		double ratio = (double)i / (m_xrange-1);
		p = m_p1 + (m_p2 - m_p1) * ratio;

		float f = m_callback->GetElevation(p);
		m_values[i] = f;

		if (f == INVALID_ELEVATION)
			m_bHaveInvalid = true;
		else
		{
			m_bHaveValidData  = true;
			if (f < m_fMin)
			{
				m_fMin = f;
				m_iMin = i;
				m_fMinDist = ratio * m_fGeodesicDistance;
			}
			if (f > m_fMax)
			{
				m_fMax = f;
				m_iMax = i;
				m_fMaxDist = ratio * m_fGeodesicDistance;
			}
		}
	}
	m_fRange = m_fMax - m_fMin;
	m_bHaveValues = true;
	m_bMouseOnLine = false;

	Analyze();
}

void ProfileDlg::Analyze()
{
	m_bValidStart = false;
	m_bValidLine = false;
	m_bIntersectsGround = false;

	if (!m_bHaveValues)
		return;

	// Some analysis is shared between the line of sight and the visibility
	m_fHeightAtStart = m_values[0];
	if (m_fHeightAtStart != INVALID_ELEVATION)
	{
		m_fHeightAtStart += m_fHeight1;
		m_bValidStart = true;
	}

	if (m_bHavePoints)
		ComputeGeoidSurface();

	if (m_bLineOfSight)
		ComputeLineOfSight();

	if (m_bVisibility && m_bValidStart)
		ComputeVisibility();

	if (m_bValidLine && m_bLineOfSight && m_bUseFresnel && m_fGeodesicDistance>0)
		ComputeFirstFresnel(); // depends on ComputeLineOfSight()
	else
		m_bHaveFresnel=false;
}

void ProfileDlg::ComputeLineOfSight()
{
	// compute height at end of line-of-sight, and the height
	//  range to draw
	m_fDrawRange = m_fRange;
	if (m_bValidStart)
	{
		m_fHeightAtEnd = m_values[m_xrange - 1];
		if (m_fHeightAtEnd != INVALID_ELEVATION)
		{
			m_bValidLine = true;
			m_fHeightAtEnd += m_fHeight2;
		}
	}
	if (!m_bValidLine)
		return;

	m_LineOfSight.resize(m_xrange);

	// Cast line from beginning to end
	float diff = m_fHeightAtEnd - m_fHeightAtStart;
	for (int i = 0; i < m_xrange; i++)
	{
		float fLineHeight = m_fHeightAtStart + diff * i / m_xrange;

		if (m_bHaveGeoidSurface && m_iCurvature == 2)
			fLineHeight -= m_GeoidSurface[i];

		float fGroundHeight = m_values[i];

		if (m_bHaveGeoidSurface && m_iCurvature == 1)
			fGroundHeight += m_GeoidSurface[i];

		m_LineOfSight[i]=fLineHeight;
		if (!m_bIntersectsGround && fLineHeight < fGroundHeight)
		{
			// line of sight intersects the ground
			m_bIntersectsGround = true;
			m_fIntersectHeight = fGroundHeight;
			m_fIntersectDistance = (float)i / (m_xrange-1) * m_fGeodesicDistance;
			m_iIntersectIndex = i;
		}
	}
	m_bHaveLOS = true;
}

void ProfileDlg::ComputeVisibility()
{
	// prepare visibility array
	int i, j;
	float diff;
	int apply_geoid = (m_bHaveGeoidSurface ? m_iCurvature : 0);
	m_visible.resize(m_xrange);
	m_rvisible.resize(m_xrange);

	// compute visibility at each point on the line
	m_visible[0] = 1;
	for (j = 1; j < m_xrange; j++)
	{
		// forward visibility
		bool vis = true;
		diff = m_values[j] + (apply_geoid>0 ? m_GeoidSurface[j] : 0) - m_fHeightAtStart;
		for (i = 0; i < j; i++)
		{
			float fLineHeight = m_fHeightAtStart + diff * i / j;
			if (fLineHeight <= (m_values[i]+(apply_geoid>0 ? m_GeoidSurface[i] : 0)))
			{
				vis = false;
				break;
			}
		}
		m_visible[j] = vis;

		// reverse visibility
		vis = true;
		diff = m_values[j] + (apply_geoid>0 ? m_GeoidSurface[j] : 0) - m_fHeightAtEnd;
		for (i = m_xrange-1; i > j; i--)
		{
			float fLineHeight = m_fHeightAtEnd + diff * (m_xrange-i) / (m_xrange-j);
			if (fLineHeight <= (m_values[i]+(apply_geoid>0 ? m_GeoidSurface[i] : 0)))
			{
				vis = 0;
				break;
			}
		}
		m_rvisible[j] = vis;
	}
}

// dist = metres from source
// freq = frequency in Hz
// zone = fresnel zone number (0 = object free zone)
float ProfileDlg::ComputeFresnelRadius(float dist, float freq, int zone)
{
	float radius;
	if (zone==1 || zone==0)
	{
		float total_dist=m_fGeodesicDistance/1000;
		dist/=1000;		// need this in Km
		freq/=1000;		// need this in GHz
		radius=17.3 * sqrt( (dist * (total_dist-dist)) / (freq * m_fGeodesicDistance) );
		if (zone==0) radius *= 0.60f;
	}
	else if (zone>1)
	{
		float wavelength=299792458.0/(freq*1000000);
		radius=sqrt(( zone * wavelength * dist * (m_fGeodesicDistance - dist))/m_fGeodesicDistance);
	}
	else radius=0;
	return radius;
}

void ProfileDlg::ComputeFirstFresnel()
{
	float total_dist=m_fGeodesicDistance/1000; // Km
	float freq=m_fRadioFrequency/1000; // in GHz
	m_FirstFresnel.resize(m_xrange);

	for (int i=0; i<m_xrange; i++)
	{
		float range=total_dist * i/m_xrange;
		m_FirstFresnel[i]=17.3 * sqrt( (range * (total_dist-range)) / (freq * total_dist) );
	}
	m_bHaveFresnel=true;
}


void ProfileDlg::ComputeGeoidSurface()
{
  /*
  neato trig lies beneath...

                  S  (arc P1 -> P2)
              ***
          ****   **** P3
       ***           ***   dist (arc P1 -> P3)
     **            H/   **
 P2 *              /      * P1
   *--------------X--------*
  *              /      .   *
  *             /  .  R     *
  *            C            *

  Problem:  Find H (height of chord at angle) given length of arc S and radius R,
	    and the partial arc distance between P1 and P3

  Formulas from: http://mathforum.org/dr.math/faq/faq.circle.segment.html

  The angle at centre C between P1 and P2 is given by  S/R
  The partial arc angle at C between P1 and P3 is given by  dist/R
  We have an iscoceles triangle - P1, P2, C
  The angle at P1 must equal the angle at P2, and is given by (PI-angle_at_C)/2
  
  We now have a triangle X, C, P1 in which we know the angles at C and P1, plus
  the length of R.

  The angle at X is PI - angle_at_P1 - angle_at_C.

  The length CX is (R*sin(P1)) / sin(X)

  H is fairly obviously R - CX

  In one step, this is written:

  H = R - ( (R * sin( (PI-(S/R))/2 )) / sin(PI - angle_at_P1 - (dist/R)) )

  NOTE: It may be more appropriate to compute the length of the normal from the chord
	through P3 (ie a line perpendicular to the chord P1, P2, passing through P3
	but as the heights are drawn as perpendicular to the surface, I think the surface 
	should be drawn in the same manner.

  The code optimises by calculating some values that do not change once only.
*/

#define ER_EQUATORIAL  6378.137		// Equatorial Radius
#define ER_POLAR       6356.752		// Polar Radius
#define ER             6366.000		// accepted average

#define EER (ER * 4.0/3.0)		// Effective earth radius (radio signals bend)
#define PI  3.141592654

	// TODO: Obtain R from the projection, if possible

	float S, dist, h, P1angle, RsinP1angle, PIminusP1angle;
	float R = (m_bUseEffectiveRadius ? EER : ER) * 1000; // everything in metres.
	m_GeoidSurface.resize(m_xrange);

	S = m_fGeodesicDistance;

	P1angle			= (PI-(S/R))/2;
	RsinP1angle		= R*sin( P1angle );
	PIminusP1angle	= PI - P1angle;

	if (S<10 || S >= (PI*R))
	{
		// more than half way around the world, this will break, so don't bother...
		// less than 10m or so, and it breaks due to rounding error.
		m_bHaveGeoidSurface=false;
		return;
	}

	for (int i = 0; i < m_xrange; i++)
	{
		dist = m_fGeodesicDistance * i/m_xrange;
		h = (dist<10 ? 0 : R-(RsinP1angle/sin(PIminusP1angle - (dist/R))) );
		m_GeoidSurface[i]=h;
	}

	// maximum - We assume it's in the middle, like all good chords should be.
	m_fGeoidCurvature = m_GeoidSurface[m_xrange/2];
	m_bHaveGeoidSurface = true;
}


float ProfileDlg::ApplyGeoid(float h, int i, char t)
{
	if (m_bHaveGeoidSurface && m_iCurvature == t)
	{
		switch (t)
		{
		case 0: break;			// don't apply
		case 1: h+=m_GeoidSurface[i];	// apply to heights relative to surface
		case 2: h-=m_GeoidSurface[i];	// apply to heights relative to line of sight
		}
	}
	return h;
}


// Free space loss only (we aren't that fancy yet)
void ProfileDlg::ComputeSignalLoss(float dist, float freq)
{
	// #define log10(x) (log(x)/log(10))
	dist *= 0.621f;		// need it in miles
	freq /= 1000000.0;	// and in MHz
	float free_space_loss = 36.56 + (20*log10(freq)) + (20*log10(dist));
}


void ProfileDlg::MakePoint(wxPoint &p, int i, float value)
{
	p.x = m_base.x + i;
	p.y = (int)(m_base.y - (value - m_fMin) / m_fDrawRange * m_yrange);
}

void ProfileDlg::DrawChart(wxDC& dc)
{
	if (!m_bHavePoints)
	{
		GetText()->SetValue(_("No Endpoints"));
		return;
	}

	if (!m_bHaveValues)
		GetValues();

	float DrawMax = m_fMax;
	if (m_bValidStart) DrawMax = std::max(m_fMax, m_fHeightAtStart);
	if (m_bValidLine) DrawMax = std::max(DrawMax, m_fHeightAtEnd);
	if (m_bHaveGeoidSurface && m_iCurvature!=0) DrawMax += m_fGeoidCurvature;
	m_fDrawRange = DrawMax - m_fMin;

	wxPen pen1(*wxMEDIUM_GREY_PEN);
	pen1.SetWidth(2);
	wxPen pen2(*wxLIGHT_GREY_PEN);
	pen2.SetStyle(wxDOT);
	wxPen pen3(wxColour(0,128,0), 1, wxSOLID);  // dark green
	wxPen pen4(wxColour(128,0,0), 1, wxSOLID);  // dark red
	wxPen pen5(wxColour(0,160,0), 1, wxSOLID);  // light green
	wxPen pen6(wxColour(170,0,0), 1, wxSOLID);  // light red

	wxFont font(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	dc.SetFont(font);

	m_base.x = MARGIN_LEFT;
	m_base.y = m_clientsize.y - MARGIN_BOTTOM;

	dc.SetPen(pen1);
	dc.DrawLine(m_base.x, m_base.y, m_base.x + m_xrange, m_base.y);
	dc.DrawLine(m_base.x, m_base.y, m_base.x, m_base.y - m_yrange);
//  dc.DrawLine(m_base.x + m_xrange, m_base.y, m_base.x + m_xrange, m_base.y - m_yrange);

	if (!m_bHaveValidData)
	{
		GetText()->SetValue(_("No Data"));
		return;
	}

	// Draw tick marks
	int numticks, x, y, tick_spacing = 32;
	wxString str;
	int w, h;
	int i;

	// Vertical ticks
	numticks = (m_yrange / tick_spacing)+2;
	for (i = 0; i < numticks; i++)
	{
		y = m_base.y - (i * m_yrange / (numticks-1));

		if (i > 0)
		{
			dc.SetPen(pen2);
			dc.DrawLine(m_base.x, y, m_base.x + m_xrange, y);
		}

		dc.SetPen(pen1);
		dc.DrawLine(m_base.x - 5, y, m_base.x + 5, y);

		str.Printf(_T("%5.1f"), m_fMin + (m_fDrawRange / (numticks-1) * i));
		dc.GetTextExtent(str, &w, &h);
		dc.DrawText(str, MARGIN_LEFT - w - 8, y-(h/2));
	}
	// Horizontal ticks
	numticks = (m_xrange / tick_spacing)+2;
	for (i = 0; i < numticks; i++)
	{
		x = m_base.x + (i * m_xrange / (numticks-1));

		dc.SetPen(pen1);
		dc.DrawLine(x, m_base.y - 5, x, m_base.y + 5);

		if (m_fGeodesicDistance >= 50000)
			str.Printf(_T("%5.0fkm"), m_fGeodesicDistance / (numticks-1) * i / 1000);
		else
			str.Printf(_T("%5.1f"), m_fGeodesicDistance / (numticks-1) * i);
		dc.GetTextExtent(str, &w, &h);
		dc.DrawRotatedText(str, x-(h/2), m_base.y + w + 8, 90);
	}

	// Draw surface line
	wxPoint p1, p2;
	int apply_geoid = (m_bHaveGeoidSurface ? m_iCurvature : 0);

	if (m_bVisibility && m_bValidStart)
	{
		bool vis = true;
		bool visr= true;
		dc.SetPen(pen3);
		for (i = 0; i < m_xrange; i++)
		{
			if (m_visible[i] != vis || m_rvisible[i] != visr)
			{
				vis = m_visible[i];
				visr= m_rvisible[i];
				if (vis && visr) dc.SetPen(pen5);
				else if (visr) dc.SetPen(pen6);
				else if (vis) dc.SetPen(pen3);
				else dc.SetPen(pen4);
			}
			float v1 = m_values[i];
			if (v1 == INVALID_ELEVATION)
				continue;

			if (apply_geoid==1)
				v1 += m_GeoidSurface[i];

			MakePoint(p1, i, v1);
			p2 = p1;
			p2.y = m_base.y;
			dc.DrawLine(p1, p2);
		}
	}
	else
	{
		dc.SetPen(pen3);
		if (m_bHaveInvalid)
		{
			// slow way, one datapoint at a time
			for (i = 0; i < m_xrange-1; i++)
			{
				float v1 = m_values[i];
				float v2 = m_values[i+1];
				if (v1 == INVALID_ELEVATION || v2 == INVALID_ELEVATION)
					continue;
				if (apply_geoid == 1)
				{
					v1+=m_GeoidSurface[i];
					v2+=m_GeoidSurface[i+1];
				}
				MakePoint(p1, i, v1);
				MakePoint(p2, i+1, v2);
				dc.DrawLine(p1, p2);
			}
		}
		else
		{
			// faster way, pass an array
			wxPoint *pts = new wxPoint[m_xrange];
			for (i = 0; i < m_xrange; i++)
			{
				MakePoint(pts[i], i, m_values[i] + (apply_geoid==1 ? m_GeoidSurface[i] : 0));
			}
			dc.DrawLines(m_xrange, pts);
			delete [] pts;
		}
	}

	// Draw the fresnel zones
	if (m_bUseFresnel && m_bHaveFresnel && m_bHaveLOS)
	{
		wxPoint *pts0 = new wxPoint[m_xrange];
		wxPoint *pts1 = new wxPoint[m_xrange];
		wxPoint *pts2 = new wxPoint[m_xrange];

		for (i=0; i<m_xrange; i++)
		{
			float base=(apply_geoid==2 ? m_GeoidSurface[i] : 0);
			MakePoint(pts1[i], i, m_LineOfSight[i] - m_FirstFresnel[i] - base);
			MakePoint(pts0[i], i, m_LineOfSight[i] - (m_FirstFresnel[i] * 0.60) - base);

			float r=ComputeFresnelRadius(m_fGeodesicDistance * i/m_xrange,m_fRadioFrequency,2);
			MakePoint(pts2[i], i, m_LineOfSight[i] - r - base);
		}

		// object free zone (60% of first zone)
		wxPen fresnelColourOF(wxColour(255,200,180));
		dc.SetPen(fresnelColourOF);
		dc.DrawLines(m_xrange, pts0);

		// first zone
		wxPen fresnelColour1(wxColour(255,210,190));
		dc.SetPen(fresnelColour1);
		dc.DrawLines(m_xrange, pts1);

		// second zone
		wxPen fresnelColour2(wxColour(255,220,200));
		dc.SetPen(fresnelColour2);
		dc.DrawLines(m_xrange, pts2);

		delete [] pts0;
		delete [] pts1;
		delete [] pts2;
	}

	// Draw Line of Sight
	if (m_bValidLine)
	{
		wxPen orange(wxColour(255,128,0), 1, wxSOLID);
		dc.SetPen(orange);
		if (apply_geoid == 2)
		{
			wxPoint *pts = new wxPoint[m_xrange];
			for (i=0; i<m_xrange; i++)
				MakePoint(pts[i], i, m_LineOfSight[i]);
			dc.DrawLines(m_xrange, pts);
			delete [] pts;
		}
		else
		{
			MakePoint(p1, 0, m_fHeightAtStart);
			MakePoint(p2, m_xrange - 1, m_fHeightAtEnd);
			dc.DrawLine(p1, p2);
		}
	}

	// Draw origin of line of sight
	if ((m_bLineOfSight || m_bVisibility) && m_bValidStart)
	{
		// it's hard to see a yellow dot without a bit of outline
		wxPen lightgrey(*wxLIGHT_GREY_PEN);
		dc.SetPen(lightgrey);

		wxBrush yellow(wxColour(255,255,0), wxSOLID);
		dc.SetBrush(yellow);
		MakePoint(p1, 0, m_fHeightAtStart);
		dc.DrawCircle(p1, 5);
	}

	// Draw min/max/mouse markers
	wxPen nopen;
	nopen.SetStyle(wxTRANSPARENT);
	dc.SetPen(nopen);

	wxBrush brush1(wxColour(0,0,255), wxSOLID); // blue: minimum
	dc.SetBrush(brush1);
	MakePoint(p1, m_iMin, m_fMin + (apply_geoid==1 ? m_GeoidSurface[m_iMin] : 0));
	dc.DrawCircle(p1, 5);

	wxBrush brush2(wxColour(255,0,0), wxSOLID); // red: maximum
	dc.SetBrush(brush2);
	MakePoint(p1, m_iMax, m_fMax + (apply_geoid==1 ? m_GeoidSurface[m_iMax] : 0));
	dc.DrawCircle(p1, 5);

	if (m_bMouseOnLine)
	{
		wxBrush brush3(wxColour(0,255,0), wxSOLID); // green: mouse
		dc.SetBrush(brush3);
		MakePoint(p1, m_iMouse, m_fMouse + (apply_geoid==1 ? m_GeoidSurface[m_iMouse] : 0));
		dc.DrawCircle(p1, 5);
	}

	if (m_bIntersectsGround)
	{
		wxBrush brush3(wxColour(255,128,0), wxSOLID);	// orange: intersection
		dc.SetBrush(brush3);
		MakePoint(p1, m_iIntersectIndex, m_fIntersectHeight);
		dc.DrawCircle(p1, 5);
	}

	// Also update message text
	UpdateMessageText();
}

void ProfileDlg::UpdateMessageText()
{
	wxString str, str2;

	str2.Printf(_("Minimum: %.2f m at distance %.1f\n"),
		ApplyGeoid(m_fMin,m_iMin,1), m_fMinDist);
	str += str2;
	str2.Printf(_("Maximum: %.2f m at distance %.1f"),
		ApplyGeoid(m_fMax,m_iMax,1), m_fMaxDist);
	str += str2;

	if (m_bMouseOnLine)
	{
		str += _T("\n");
		str2.Printf(_("Mouse: %.2f m at distance %.1f"),
			ApplyGeoid(m_fMouse,m_iMouse,1), m_fMouseDist);
		str += str2;
		if (m_bHaveSlope)
		{
			str += _T(", ");
			str2.Printf(_("Slope %.3f (%.1f degrees)"), m_fSlope,
				atan(m_fSlope) * 180 / PId);
			str += str2;
		}

		if (m_bHaveFresnel && m_bHaveLOS)
		{
			str += _T(", ");
			str2.Printf(_("Fresnel Zone 1: Radius: %.1f, Clearance %.1f"),
				m_fMouseFresnel,
				(ApplyGeoid(m_fMouseLOS,m_iMouse,2)-m_fMouseFresnel) -
				 ApplyGeoid(m_fMouse,m_iMouse,1));
			str+=str2;
			str2.Printf(_("  OF Zone: Radius: %.1f, Clearance %.1f"),
				m_fMouseFresnel*0.6,
				(ApplyGeoid(m_fMouseLOS,m_iMouse,2)-(m_fMouseFresnel*0.6)) -
				 ApplyGeoid(m_fMouse,m_iMouse,1));
			str+=str2;
		}
	}
	else if (m_bHaveFresnel)
	{
		str += _T("\n");
		str2.Printf(_("Fresnel zone radius at midpoint %.2fm (Object free zone %.2fm) Geoid Height %.1fm"),
			m_FirstFresnel[m_xrange/2], m_FirstFresnel[m_xrange/2]*0.60,
			m_fGeoidCurvature);
		str += str2;
	}

	if (m_bValidLine && m_bIntersectsGround)
	{
		str += _T("\n");
		str2.Printf(_("Intersects ground at height %.2f, distance %.1f"),
			m_fIntersectHeight, m_fIntersectDistance);
		str += str2;
	}
	GetText()->SetValue(str);
}

void ProfileDlg::UpdateEnabling()
{
	GetLineOfSight()->Enable(m_bHavePoints);
	GetVisibility()->Enable(m_bHavePoints);

	GetHeight1()->Enable(m_bLineOfSight || m_bVisibility);
	GetHeight2()->Enable(m_bLineOfSight);

	GetFresnel()->Enable(m_bLineOfSight);

	GetRF()->Enable(m_bUseFresnel);
//	GetCurvature()->Enable(m_bUseFresnel);
//	GetEffective()->Enable(m_iCurvature != 0);
}

// WDR: handler implementations for ProfileDlg

void ProfileDlg::OnCurvature( wxCommandEvent &event )
{
	TransferDataFromWindow();
	Analyze();
	Refresh();
	UpdateEnabling();
}

void ProfileDlg::OnRF( wxCommandEvent &event )
{
	TransferDataFromWindow();
	Analyze();
	Refresh();
}

void ProfileDlg::OnUseEffective( wxCommandEvent &event )
{
	m_bUseEffectiveRadius = event.IsChecked();
	Analyze();
	Refresh();
}

void ProfileDlg::OnFresnel( wxCommandEvent &event )
{
	m_bUseFresnel = event.IsChecked();
	Analyze();
	Refresh();
	UpdateEnabling();
}

void ProfileDlg::OnHeight2( wxCommandEvent &event )
{
	TransferDataFromWindow();
	Analyze();
	Refresh();
}

void ProfileDlg::OnHeight1( wxCommandEvent &event )
{
	TransferDataFromWindow();
	Analyze();
	Refresh();
}

// This calls OnDraw, having adjusted the origin according to the current
// scroll position
void ProfileDlg::OnPaint(wxPaintEvent &event)
{
	// don't use m_targetWindow here, this is always called for ourselves
	wxPaintDC dc(this);
	OnDraw(dc);
}

void ProfileDlg::OnDraw(wxDC& dc)  // overridden to draw this view
{
	DrawChart(dc);
}

void ProfileDlg::OnSize(wxSizeEvent& event)
{
	wxSize size = GetClientSize();
	if (size != m_clientsize)
	{
		m_clientsize = size;
		m_bHaveValues = false;
		Refresh();
	}
	event.Skip();
}

void ProfileDlg::OnLeftDown(wxMouseEvent& event)
{
	wxPoint point = event.GetPosition();
	if (point.x > m_base.x && point.x < m_base.x + m_xrange - 1 &&
		point.y < m_base.y && point.y > m_base.x - m_yrange)
	{
		m_bLeftButton = true;
		OnMouseMove(event);
	}
}
void ProfileDlg::OnLeftUp(wxMouseEvent& event)
{
	m_bLeftButton = false;
}

void ProfileDlg::OnMouseMove(wxMouseEvent& event)
{
	if (!m_bHaveValidData || !m_bHaveValues || !m_bLeftButton)
	{
		m_bMouseOnLine = false;
		return;
	}
	wxPoint point = event.GetPosition();
	if (point.x > m_base.x && point.x < m_base.x + m_xrange - 1 &&
		point.y < m_base.y && point.y > m_base.x - m_yrange)
	{
		int offset = point.x - m_base.x;
		m_fMouse = m_values[offset];
		if (m_bHaveFresnel) m_fMouseFresnel = m_FirstFresnel[offset];
			else    m_fMouseFresnel = 0;
		if (m_bHaveLOS) m_fMouseLOS = m_LineOfSight[offset];
			else    m_fMouseLOS = 0;

		if (m_fMouse != INVALID_ELEVATION)
		{
			m_fMouseDist = (float)offset / m_xrange * m_fGeodesicDistance;
			m_iMouse = offset;
			m_bMouseOnLine = true;
			Refresh();

			// calculate slope
			float v2 = m_values[offset+1];
			if (v2 != INVALID_ELEVATION)
			{
				m_bHaveSlope = true;
				m_fSlope = (v2 - m_fMouse) / (m_fGeodesicDistance / m_xrange);
			}
		}
	}
	else
	{
		// mouse out
		if (m_bMouseOnLine)
			Refresh();
		m_bMouseOnLine = false;
	}
}

void ProfileDlg::OnLineOfSight( wxCommandEvent &event )
{
	m_bLineOfSight = event.IsChecked();
	Analyze();
	Refresh();
	UpdateEnabling();
}

void ProfileDlg::OnVisibility( wxCommandEvent &event )
{
	m_bVisibility = event.IsChecked();
	Analyze();
	Refresh();
	UpdateEnabling();
}

