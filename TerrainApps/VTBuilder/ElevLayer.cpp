//
// ElevLayer.cpp
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/ElevationGrid.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtDIB.h"
#include "vtdata/vtLog.h"
#include "vtdata/DataPath.h"
#include "vtui/Helper.h"	// for FormatCoord

#include "BuilderView.h"	// For grid marks
#include "ElevLayer.h"
#include "ExtentDlg.h"
#include "FileFilters.h"
#include "Frame.h"
#include "Helper.h"
#include "RawDlg.h"
#include "vtBitmap.h"
#include "LocalDatabuf.h"
#include "Options.h"

////////////////////////////////////////////////////////////////////

vtTin2d::vtTin2d()
{
	m_fEdgeLen = NULL;
	m_bConstrain = false;
}

void vtTin2d::DrawTin(wxDC* pDC, vtScaledView *pView)
{
	wxPen TinPen(wxColor(128,0,128), 1, wxSOLID);
	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(TinPen);

	FPoint2 p2;
	unsigned int i, tris = NumTris();
	for (i = 0; i < tris; i++)
	{
		if (m_bConstrain)
		{
			if (m_fEdgeLen[i] > m_fMaxEdge)
				continue;
		}
		int v0 = m_tri[i*3+0];
		int v1 = m_tri[i*3+1];
		int v2 = m_tri[i*3+2];

		pView->screen(m_vert[v0], g_screenbuf[0]);
		pView->screen(m_vert[v1], g_screenbuf[1]);
		pView->screen(m_vert[v2], g_screenbuf[2]);

		g_screenbuf[3] = g_screenbuf[0];
		pDC->DrawLines(4, g_screenbuf);
	}
#if 0
	// For testing purposes, draw the vertices as well
	unsigned int points = NumVerts();
	for (i = 0; i < tris; i++)
	{
		pView->screen(m_vert[i], g_screenbuf[0]);
		pDC->DrawPoint(g_screenbuf[0]);
	}
#endif
}

void vtTin2d::SetConstraint(bool bConstrain, double fMaxEdge)
{
	m_bConstrain = bConstrain;
	m_fMaxEdge = fMaxEdge;
}

void vtTin2d::ComputeEdgeLengths()
{
	int nTris = NumTris();
	m_fEdgeLen = new double[nTris];
	for (int i = 0; i < nTris; i++)
		m_fEdgeLen[i] = GetTriMaxEdgeLength(i);
}

void vtTin2d::CullLongEdgeTris()
{
	if (!m_fEdgeLen)
		return;
	int nTris = NumTris();
	int b1, b2;
	int to = 0;
	int kept = 0;
	for (int i = 0; i < nTris; i++)
	{
		b1 = i * 3;
		if (m_fEdgeLen[i] < m_fMaxEdge)
		{
			// keep
			b2 = to * 3;
			m_tri[b2] = m_tri[b1];
			m_tri[b2+1] = m_tri[b1+1];
			m_tri[b2+2] = m_tri[b1+2];
			to++;
			kept++;
		}
	}
	m_tri.SetSize(kept*3);
}


void vtTin2d::FreeEdgeLengths()
{
	delete m_fEdgeLen;
	m_fEdgeLen = NULL;
}


////////////////////////////////////////////////////////////////////

ElevDrawOptions vtElevLayer::m_draw;


vtElevLayer::vtElevLayer() : vtLayer(LT_ELEVATION)
{
	SetupDefaults();
	m_pGrid = NULL;
	m_pTin = NULL;
}

vtElevLayer::vtElevLayer(const DRECT &area, int iColumns, int iRows,
	bool bFloats, float fScale, const vtProjection &proj) : vtLayer(LT_ELEVATION)
{
	SetupDefaults();

	VTLOG(" Constructing vtElevLayer of size %d x %d, floats %d\n",
		iColumns, iRows, bFloats);

	m_pTin = NULL;
	m_pGrid = new vtElevationGrid(area, iColumns, iRows, bFloats, proj);
	if (!m_pGrid->HasData())
		VTLOG1(" Grid allocation failed.\n");

	m_pGrid->SetScale(fScale);
	m_pGrid->GetDimensions(m_iColumns, m_iRows);
}

vtElevLayer::~vtElevLayer()
{
	if (m_bBitmapRendered)
	{
		delete m_pBitmap;
		m_pBitmap = NULL;
	}
	if (m_pGrid)
		delete m_pGrid;
	if (m_pTin)
		delete m_pTin;
}

bool vtElevLayer::OnSave()
{
	vtString fname = (const char *) GetLayerFilename().mb_str(wxConvUTF8);
	if (m_pGrid)
		return m_pGrid->SaveToBT(fname, NULL, m_bPreferGZip);
	if (m_pTin)
		return m_pTin->Write(fname);
	return false;
}

bool vtElevLayer::OnLoad()
{
	OpenProgressDialog(_("Loading Elevation Layer"));

	bool success = false;

	wxString fname = GetLayerFilename();
	if (fname.Contains(_T(".bt")))
	{
		// remember whether this layer was read from a compressed file
		if (!fname.Right(6).CmpNoCase(_T(".bt.gz")))
			m_bPreferGZip = true;

		m_pGrid = new vtElevationGrid;

		vtElevGridError err;
		success = m_pGrid->LoadFromBT(fname.mb_str(wxConvUTF8), progress_callback, &err);
		if (!success && err == EGE_READ_CRS)
		{
			// Missing prj file
			wxString str = _("CRS file");
			str += _T(" (");
			RemoveFileExtensions(fname);
			str += fname;
			str += _T(".prj) ");
			str += _("is missing or unreadable.\n");
			wxMessageBox(str);
		}
		if (success)
		{
			m_pGrid->GetDimensions(m_iColumns, m_iRows);
			m_pGrid->SetupConversion(1.0f);
		}
	}
	else if (!fname.Right(4).CmpNoCase(_T(".tin")) ||
			 !fname.Right(4).CmpNoCase(_T(".itf")))
	{
		m_pTin = new vtTin2d;
		success = m_pTin->Read(fname.mb_str(wxConvUTF8));
	}

	CloseProgressDialog();
	return success;
}

bool vtElevLayer::TransformCoords(vtProjection &proj_new)
{
	vtProjection proj_old;
	GetProjection(proj_old);

	if (proj_old == proj_new)
		return true;		// No conversion necessary

	bool success = false;
	if (m_pGrid)
	{
		// Check to see if the projections differ *only* by datum
		vtProjection test = proj_old;
		test.SetDatum(proj_new.GetDatum());
		if (test == proj_new)
		{
			success = m_pGrid->ReprojectExtents(proj_new);
		}
		else
		{
			bool bUpgradeToFloat = false;

			if (!m_pGrid->IsFloatMode())
			{
				if (g_Options.GetValueBool(TAG_REPRO_TO_FLOAT_NEVER))
					bUpgradeToFloat = false;
				else if (g_Options.GetValueBool(TAG_REPRO_TO_FLOAT_ALWAYS))
					bUpgradeToFloat = true;
				else
				{
					// Ask
					int res = wxMessageBox(_("Input grid is integer.  Use floating-point values in reprojected grid?"),
						_("query"), wxYES_NO);
					if (res == wxYES)
						bUpgradeToFloat = true;
				}
			}

			// actually re-project the grid elements
			vtElevationGrid *grid_new = new vtElevationGrid;

			OpenProgressDialog(_("Converting Elevation Projection"));
			success = grid_new->ConvertProjection(m_pGrid, proj_new,
				bUpgradeToFloat, progress_callback);

			if (success)
			{
				delete m_pGrid;
				m_pGrid = grid_new;
				ReImage();
			}
			else
			{
				wxString msg(grid_new->GetErrorMsg(), wxConvUTF8);
				wxMessageBox(msg, _("Error"));
				delete grid_new;
			}
		}
	}
	if (m_pTin)
	{
		OpenProgressDialog(_("Converting Elevation Projection"));
		success = m_pTin->ConvertProjection(proj_new);
	}
	SetModified(true);

	CloseProgressDialog();
	return success;
}

bool vtElevLayer::NeedsDraw()
{
	if (m_bNeedsDraw)
		return true;
	if (m_pBitmap != NULL && m_draw.m_bShowElevation && !m_bBitmapRendered)
		return true;
	return false;
}

void vtElevLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	if (m_pGrid)
	{
		if (m_draw.m_bShowElevation)
			DrawLayerBitmap(pDC, pView);
		else
			DrawLayerOutline(pDC, pView);
	}
	if (m_pTin)
	{
		m_pTin->DrawTin(pDC, pView);
	}
}

bool vtElevLayer::GetAreaExtent(DRECT &rect)
{
	if (m_pGrid)
	{
		rect = m_pGrid->GetAreaExtents();
		return true;
	}
	else if (m_pTin)
	{
		// for a TIN, area extents are the same as normal extents
		rect = m_pTin->GetEarthExtents();
		return true;
	}
	return false;
}

void vtElevLayer::OnLeftDown(BuilderView *pView, UIContext &ui)
{
	if (ui.mode == LB_TrimTIN)
	{
		pView->DrawInvertedLine(ui.m_DownLocation, ui.m_CurLocation);	//draw
		ui.m_bRubber = true;
	}
}

void vtElevLayer::OnMouseMove(BuilderView *pView, UIContext &ui)
{
	if (ui.mode == LB_TrimTIN && ui.m_bRubber)
	{
		pView->DrawInvertedLine(ui.m_DownLocation, ui.m_PrevLocation);	// erase
		pView->DrawInvertedLine(ui.m_DownLocation, ui.m_CurLocation);	// redraw
	}
}

void vtElevLayer::OnLeftUp(BuilderView *pView, UIContext &ui)
{
	if (ui.mode == LB_TrimTIN && ui.m_bRubber)
	{
		ui.m_bRubber = false;
		pView->DrawInvertedLine(ui.m_DownLocation, ui.m_PrevLocation);	// erase

		int num = m_pTin->RemoveTrianglesBySegment(ui.m_DownLocation, ui.m_CurLocation);
		if (num)
		{
			SetModified(true);
			pView->Refresh();
		}
	}
}

void vtElevLayer::DrawLayerBitmap(wxDC* pDC, vtScaledView *pView)
{
	if (!m_pGrid)
		return;

	if (m_pBitmap == NULL)
		SetupBitmap(pDC);

	if (m_pBitmap == NULL)
	{
		DrawLayerOutline(pDC, pView);
		return;
	}

	if (!m_bBitmapRendered)
		return;

	int iColumns, iRows;
	m_pGrid->GetDimensions(iColumns, iRows);

	wxRect screenrect = pView->WorldToCanvas(m_pGrid->GetAreaExtents());
	wxRect destRect = screenrect;
	wxRect srcRect(0, 0, m_iImageWidth, m_iImageHeight);

	double ratio_x = (float) srcRect.GetWidth() / destRect.GetWidth();
	double ratio_y = (float) srcRect.GetHeight() / destRect.GetHeight();

#if 0
	//clip stuff, so we only blit what we need
	int client_width, client_height;
	pView->GetClientSize(&client_width, &client_height); //get client window size
	if ((destRect.x + destRect.GetWidth() < 0) ||
			(destRect.y + destRect.GetHeight() < 0) ||
			(destRect.x > client_width) ||
			(destRect.y > client_height))
		//image completely off screen, return
		return;
	int diff;

	// clip left
	diff = 0 - destRect.x;
	if (diff > 0)
	{
		destRect.x += diff;
		srcRect.x += (long)(diff * ratio_x);
	}

	// clip top
	diff = 0 - destRect.y;
	if (diff > 0)
	{
		destRect.y += diff;
		srcRect.y += (long)(diff * ratio_y);
	}

	// clip right
	diff = destRect.x + destRect.GetWidth() - client_width;
	if (diff > 0)
	{
		destRect.width -= diff;
		srcRect.width -= (long)(diff * ratio_x);
	}

	// clip bottom
	diff = destRect.y + destRect.GetHeight() - client_height;
	if (diff > 0)
	{
		destRect.height -= diff;
		srcRect.height -= (long)(diff * ratio_y);
	}
#endif

	bool bDrawNormal = true;
#if WIN32
	::SetStretchBltMode((HDC) (pDC->GetHDC()), HALFTONE );

	if (!m_bHasMask)
	{
		// Using StretchBlit is much faster and has less scaling/roundoff
		//  problems than using the wx method DrawBitmap.  However, GDI
		//  won't stretch and mask at the same time!
		wxDC2 *pDC2 = (wxDC2 *) pDC;
		pDC2->StretchBlit(*m_pBitmap->m_pBitmap, destRect.x, destRect.y,
			destRect.width, destRect.height, srcRect.x, srcRect.y,
			srcRect.width, srcRect.height);
		bDrawNormal = false;
	}
#endif
	if (bDrawNormal)
	{
		// scale and draw the bitmap
		// must use SetUserScale since wxWindows doesn't provide StretchBlt
		double scale_x = 1.0/ratio_x;
		double scale_y = 1.0/ratio_y;
		pDC->SetUserScale(scale_x, scale_y);

		//  On Windows, this does a BitBlt (or MaskBlt if there is a mask)
		pDC->DrawBitmap(*m_pBitmap->m_pBitmap, (int) (destRect.x/scale_x),
			(int) (destRect.y/scale_y), m_bHasMask);

		// restore
		pDC->SetUserScale(1.0, 1.0);
	}

#if 0
	// This is how we used to do it, with raw Win32 calls
	t->d_pDIBSection->Draw( pDC, destRect.left, destRect.top);
	m_bitmap.d_pDIBSection->DrawScaled( pDC, destRect, srcRect);
#endif
}

void vtElevLayer::DrawLayerOutline(wxDC* pDC, vtScaledView *pView)
{
	wxRect screenrect = pView->WorldToCanvas(m_pGrid->GetAreaExtents());

	// draw a simple crossed box with green lines
	wxPen Pen1(wxColor(0x00, 0x80, 0x00), 1, wxSOLID);
	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(Pen1);

	screenrect.x++;
	screenrect.y++;
	screenrect.width-=2;
	screenrect.height-=2;

	DrawRectangle(pDC, screenrect);
}

bool vtElevLayer::GetExtent(DRECT &rect)
{
	if (m_pGrid)
	{
		rect = m_pGrid->GetEarthExtents();
		return true;
	}
	if (m_pTin)
	{
		rect = m_pTin->GetEarthExtents();
		return true;
	}
	return false;
}

bool vtElevLayer::AppendDataFrom(vtLayer *pL)
{
	if (pL->GetType() != LT_ELEVATION)
		return false;

	vtElevLayer *pEL = (vtElevLayer *)pL;
	if (m_pGrid)
	{
		// deliberately unimplemented - it does not make sense to do this
		// operation with grids
	}
	if (m_pTin && pEL->m_pTin)
	{
		// TODO (long-term..)
	}
	return false;
}


////////////////////////////////////////////////////////////////////

void vtElevLayer::SetupDefaults()
{
	m_bNeedsDraw = false;
	m_bBitmapRendered = false;
	SetLayerFilename(_("Untitled"));
	m_bPreferGZip = false;

	m_pBitmap = NULL;
	m_pMask = NULL;
	m_iImageWidth = 0;
	m_iImageHeight = 0;
	m_iColumns = 0;
	m_iRows = 0;
	m_fSpacing = 0.0f;
}


void vtElevLayer::SetupBitmap(wxDC* pDC)
{
	m_pGrid->GetDimensions(m_iColumns, m_iRows);

	m_iImageWidth = m_iColumns;
	m_iImageHeight = m_iRows;

	int div = 1;
	while (m_iImageWidth * m_iImageHeight > 4096*4096)
	{
		// really huge bitmap is going to fail, chop it down
		div++;
		m_iImageWidth = m_iColumns / div;
		m_iImageHeight = m_iRows / div;
	}

	m_pBitmap = new vtBitmap;
	if (!m_pBitmap->Allocate(m_iImageWidth, m_iImageHeight))
	{
		DisplayAndLog(_("Couldn't create bitmap, probably too large."));
		delete m_pBitmap;
		m_pBitmap = NULL;
	}
	m_bNeedsDraw = true;
}

void vtElevLayer::SetupDefaultColors(ColorMap &cmap)
{
	cmap.m_bRelative = false;
	cmap.Add(-400*17, RGBi(255, 255, 255));
	cmap.Add(-400*16, RGBi(20, 20, 30	));
	cmap.Add(-400*15, RGBi(60, 60, 70	));
	cmap.Add(-400*14, RGBi(120, 120, 130));
	cmap.Add(-400*13, RGBi(180, 185, 190));
	cmap.Add(-400*12, RGBi(160, 80, 0	));
	cmap.Add(-400*11, RGBi(128, 128, 0	));
	cmap.Add(-400*10, RGBi(160, 0, 160	));
	cmap.Add(-400* 9, RGBi(144, 64, 144	));
	cmap.Add(-400* 8, RGBi(128, 128, 128));
	cmap.Add(-400* 7, RGBi(64, 128, 60	));
	cmap.Add(-400* 6, RGBi(0, 128, 0	));
	cmap.Add(-400* 5, RGBi(0, 128, 128	));
	cmap.Add(-400* 4, RGBi(0, 0, 160	));
	cmap.Add(-400* 3, RGBi(43, 90, 142	));
	cmap.Add(-400* 2, RGBi(81, 121, 172	));
	cmap.Add(-400* 1, RGBi(108, 156, 195));
	cmap.Add(-1.0f,  RGBi(182, 228, 255));
	cmap.Add(0, RGBi(0, 0, 0xee));
	cmap.Add( 0.1f,  RGBi(40, 224, 40	));
	cmap.Add( 450* 1, RGBi(0, 128, 0	));
	cmap.Add( 450* 2, RGBi(100, 144, 76	));
	cmap.Add( 450* 3, RGBi(204, 170, 136));
	cmap.Add( 450* 4, RGBi(136, 100, 70	));
	cmap.Add( 450* 5, RGBi(128, 128, 128));
	cmap.Add( 450* 6, RGBi(180, 128, 64	));
	cmap.Add( 450* 7, RGBi(255, 144, 32	));
	cmap.Add( 450* 8, RGBi(200, 110, 80	));
	cmap.Add( 450* 9, RGBi(160, 80, 160	));
	cmap.Add( 450*10, RGBi(144, 40, 128	));
	cmap.Add( 450*11, RGBi(128, 128, 128));
	cmap.Add( 450*12, RGBi(255, 255, 255));
	cmap.Add( 450*13, RGBi(255, 255, 128));
	cmap.Add( 450*14, RGBi(255, 128, 0));
	cmap.Add( 450*15, RGBi(0, 128, 0));
}

void vtElevLayer::RenderBitmap()
{
	if (!m_pGrid)
		return;

	// flag as being rendered
	m_bNeedsDraw = false;

	// only show a progress dialog for large terrain (>300 points tall)
	bool bProg = (m_iRows > 300);

#if WIN32
	// mew 2002-08-17: reuse of wxProgressDialog causes SIGSEGV,
	// so just disable for now. (wxGTK 2.2.9 on Linux Mandrake 8.1)
	if (bProg)
		OpenProgressDialog(_("Rendering Bitmap"), true);
#endif

	// safety check
	if (m_iImageWidth == 0 || m_iImageHeight == 0)
		return;

	UpdateProgressDialog(0, _("Generating colors from elevation..."));
	DetermineMeterSpacing();

	clock_t tm1 = clock();

#if 0
	// TODO: re-enable this friendly cancel behavior
	if (UpdateProgressDialog(j*100/m_iImageHeight))
	{
		wxString msg = _("Turn off displayed elevation for elevation layers?");
		if (wxMessageBox(msg, _T(""), wxYES_NO) == wxYES)
		{
			m_draw.m_bShowElevation = false;
			CloseProgressDialog();
			return;
		}
		else
			ResumeProgressDialog();
	}
#endif
	ColorMap cmap;
	vtString cmap_fname = m_draw.m_strColorMapFile;
	vtString cmap_path = FindFileOnPaths(vtGetDataPath(), "GeoTypical/" + cmap_fname);
	bool bLoaded = false;
	if (cmap_path != "")
	{
		if (cmap.Load(cmap_path))
			bLoaded = true;
	}
	if (!bLoaded)
		SetupDefaultColors(cmap);

	bool has_invalid = m_pGrid->ColorDibFromElevation(m_pBitmap, &cmap,
		8000, progress_callback);

	UpdateProgressDialog(0, _("Shading colors..."));
	if (m_draw.m_bShadingQuick)
		m_pGrid->ShadeQuick(m_pBitmap, SHADING_BIAS, true, progress_callback);
	else if (m_draw.m_bShadingDot)
	{
		// Quick and simple sunlight vector
		FPoint3 light_dir = LightDirection(m_draw.m_iCastAngle, m_draw.m_iCastDirection);

		if (m_draw.m_bCastShadows)
			m_pGrid->ShadowCastDib(m_pBitmap, light_dir, 1.0f,
				m_draw.m_fAmbient, progress_callback);
		else
			m_pGrid->ShadeDibFromElevation(m_pBitmap, light_dir, 1.0f,
				m_draw.m_fAmbient, m_draw.m_fGamma, true, progress_callback);
	}

	if (has_invalid && m_draw.m_bDoMask)
	{
		UpdateProgressDialog(90, _("Hiding unknown areas..."));
		m_pMask = new wxMask(*m_pBitmap->m_pBitmap, wxColour(255, 0, 0));
		m_pBitmap->m_pBitmap->SetMask(m_pMask);
		m_bHasMask = true;
	}
	else
		m_bHasMask = false;

	clock_t tm2 = clock();
	float time = ((float)tm2 - tm1)/CLOCKS_PER_SEC;
	VTLOG("RenderBitmap: %.3f seconds.\n", time);

	m_pBitmap->ContentsChanged();
	m_bBitmapRendered = true;

	if (bProg)
		CloseProgressDialog();
}

void vtElevLayer::ReRender()
{
	if (IsGrid())
	{
		m_bBitmapRendered = false;
		m_bNeedsDraw = true;
	}
}

void vtElevLayer::ReImage()
{
	if (m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap = NULL;
	}
	m_bBitmapRendered = false;
}


/**
 * Determine the approximate spacing, in meters, between each grid cell, in the X
 * direction.  The result is placed in the m_fSpacing member for use in shading.
 */
void vtElevLayer::DetermineMeterSpacing()
{
	vtProjection &proj = m_pGrid->GetProjection();
	if (proj.IsGeographic())
	{
		DRECT area = m_pGrid->GetEarthExtents();

		double fToMeters = EstimateDegreesToMeters((area.bottom + area.top)/2);
		m_fSpacing = (float) (area.Width()) * fToMeters / (m_iColumns - 1);
	}
	else
	{
		// Linear units-based projections are much simpler
		DPoint2 spacing = m_pGrid->GetSpacing();
		m_fSpacing = spacing.x * GetMetersPerUnit(proj.GetUnits());
	}
}

void vtElevLayer::Offset(const DPoint2 &p)
{
	if (m_pGrid)
	{
		m_pGrid->Offset(p);
	}
	if (m_pTin)
	{
		m_pTin->Offset(p);
	}
}

vtHeightField *vtElevLayer::GetHeightField()
{
	if (m_pGrid)
		return m_pGrid;
	if (m_pTin)
		return m_pTin;
	return NULL;
}

float vtElevLayer::GetElevation(const DPoint2 &p)
{
	if (m_pGrid)
		return m_pGrid->GetFilteredValue(p);
	if (m_pTin)
	{
		float fAltitude;
		if (m_pTin->FindAltitudeOnEarth(p, fAltitude))
			return fAltitude;
	}
	return INVALID_ELEVATION;
}

bool vtElevLayer::GetHeightExtents(float &fMinHeight, float &fMaxHeight) const
{
	if (m_pGrid)
		m_pGrid->GetHeightExtents(fMinHeight, fMaxHeight);
	else if (m_pTin)
		m_pTin->GetHeightExtents(fMinHeight, fMaxHeight);
	else
		return false;
	return true;
}

void vtElevLayer::GetProjection(vtProjection &proj)
{
	if (m_pGrid)
		proj = m_pGrid->GetProjection();
	else if (m_pTin)
		proj = m_pTin->m_proj;
}

void vtElevLayer::SetProjection(const vtProjection &proj)
{
	if (m_pGrid)
	{
		const vtProjection &current = m_pGrid->GetProjection();
		if (proj != current)
			SetModified(true);

		// if units change, meter extents of grid (and the shading which is
		//  derived from them) need to be recomputed
		LinearUnits oldunits = current.GetUnits();
		m_pGrid->SetProjection(proj);
		if (proj.GetUnits() != oldunits)
			ReRender();
	}
	if (m_pTin)
		m_pTin->m_proj = proj;
}

bool vtElevLayer::ImportFromFile(const wxString &strFileName,
	bool progress_callback(int am))
{
	// Avoid trouble with '.' and ',' in Europe - all the file readers assume
	//  the default "C" locale.
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	wxString strExt = strFileName.AfterLast('.');
	vtString fname = (const char *) strFileName.mb_str(wxConvUTF8);

	VTLOG("ImportFromFile '%s'\n", (const char *) fname);

	if (!strExt.CmpNoCase(_T("gz")))
	{
		// ignore .gz, look at extension under it
		wxString dropped = strFileName.Left(strFileName.Len()-3);
		strExt = dropped.AfterLast('.');
	}

	if (!strExt.CmpNoCase(_T("bz2")))
	{
		// ignore .bz2, look at extension under it
		wxString dropped = strFileName.Left(strFileName.Len()-4);
		strExt = dropped.AfterLast('.');
	}

	// The first character in the file is useful for telling which format
	// the file really is.
	FILE *fp = vtFileOpen(fname, "rb");
	char first = fgetc(fp);
	fclose(fp);

	bool success = false;

	if (!strExt.CmpNoCase(_T("dxf")))
	{
		m_pTin = new vtTin2d;
		success = m_pTin->ReadDXF(fname, progress_callback);
	}
	else
	if (!strFileName.Right(6).CmpNoCase(_T("xy.adf")))
	{
		m_pTin = new vtTin2d;
		success = m_pTin->ReadADF(fname, progress_callback);
	}
	else
	{
		if (m_pGrid == NULL)
			m_pGrid = new vtElevationGrid;
	}

	if (!strExt.CmpNoCase(_T("3tx")))
	{
		success = m_pGrid->LoadFrom3TX(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("dem")))
	{
		// If there is a .hdr file in the same place, it is most likely
		//  a GTOPO30/SRTM30 file
		vtString hdr_fname = ChangeFileExtension(fname, ".hdr");
		if (FileExists(hdr_fname))
			success = m_pGrid->LoadFromGTOPO30(hdr_fname, progress_callback);
		else
		{
			if (first == '*')
				success = m_pGrid->LoadFromMicroDEM(fname, progress_callback);
			else
				success = m_pGrid->LoadFromDEM(fname, progress_callback);
		}
	}
	else if (!strExt.CmpNoCase(_T("asc")))
	{
		success = m_pGrid->LoadFromASC(fname, progress_callback);
		// vtElevationGrid does have its own ASC reader, but use GDAL instead
//		success = m_pGrid->LoadWithGDAL(strFileName.mb_str(wxConvUTF8), progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("bil")))
	{
		success = m_pGrid->LoadWithGDAL(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("mem")))
	{
		success = m_pGrid->LoadWithGDAL(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("ter")))
	{
		success = m_pGrid->LoadFromTerragen(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("cdf")))
	{
		success = m_pGrid->LoadFromCDF(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("hdr")))
	{
		success = m_pGrid->LoadFromGTOPO30(fname, progress_callback);
		if (!success)
			success = m_pGrid->LoadFromGLOBE(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("dte")) ||
			!strExt.CmpNoCase(_T("dt0")) ||
			!strExt.CmpNoCase(_T("dt1")) ||
			!strExt.CmpNoCase(_T("dt2")))
	{
		success = m_pGrid->LoadFromDTED(fname, progress_callback);
	}
	else if (!strExt.Left(3).CmpNoCase(_T("pgm")))
	{
		success = m_pGrid->LoadFromPGM(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("grd")))
	{
		// might by CDF, might be Surfer GRD
		if (first == 'D')
		{
			VTLOG("First character is 'D', attempting load as a Surfer Grid file.\n");
			success = m_pGrid->LoadFromGRD(fname, progress_callback);
		}
		else
		{
			VTLOG("First character is not 'D', attempting load as a netCDF file.\n");
			success = m_pGrid->LoadFromCDF(fname, progress_callback);
		}
		if (!success)
		{
			VTLOG("Didn't load successfully, attempting load with GDAL.\n");
			// Might be 'Arc Binary Grid', try GDAL
			success = m_pGrid->LoadWithGDAL(fname, progress_callback);
		}
	}
	else if (!strFileName.Right(8).CmpNoCase(_T("catd.ddf")) ||
			!strExt.Left(3).CmpNoCase(_T("tif")) ||
			!strExt.Left(3).CmpNoCase(_T("png")) ||
			!strExt.Left(3).CmpNoCase(_T("img")) ||
			!strExt.CmpNoCase(_T("adf")))
	{
		if (m_pGrid)
			success = m_pGrid->LoadWithGDAL(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("raw")))
	{
		RawDlg dlg(NULL, -1, _("Raw Elevation File"));

		dlg.m_iBytes = 2;
		dlg.m_iWidth = 100;
		dlg.m_iHeight = 100;
		dlg.m_fVUnits = 1.0f;
		dlg.m_fSpacing = 30.0f;
		dlg.m_bBigEndian = false;
		dlg.m_extents.Empty();
		GetMainFrame()->GetProjection(dlg.m_original);

		if (dlg.ShowModal() == wxID_OK)
		{
			success = m_pGrid->LoadFromRAW(fname, dlg.m_iWidth,
					dlg.m_iHeight, dlg.m_iBytes, dlg.m_fVUnits, dlg.m_bBigEndian,
					progress_callback);
		}
		if (success)
		{
			m_pGrid->SetEarthExtents(dlg.m_extents);
			m_pGrid->SetProjection(dlg.m_proj);
		}
	}
	else if (!strExt.CmpNoCase(_T("ntf")))
	{
		success = m_pGrid->LoadFromNTF5(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("txt")) ||
		!strExt.CmpNoCase(_T("xyz")))
	{
		success = m_pGrid->LoadFromXYZ(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("hgt")))
	{
		success = m_pGrid->LoadFromHGT(fname, progress_callback);
	}
	if (!success)
		return false;

	vtProjection *pProj;
	if (m_pGrid)
		pProj = &m_pGrid->GetProjection();
	else
		pProj = &m_pTin->m_proj;

	// We should ask for a CRS before asking for extents
	if (!GetMainFrame()->ConfirmValidCRS(pProj))
		return false;

	if (m_pGrid != NULL)
	{
		if (m_pGrid->GetEarthExtents().IsEmpty())
		{
			// No extents.
			wxString msg = _("File lacks geographic location (extents). Would you like to specify extents?\n Yes - specify extents\n No - use some default values\n");
			int res = wxMessageBox(msg, _("Elevation Import"), wxYES_NO | wxCANCEL);
			if (res == wxYES)
			{
				DRECT ext;
				ext.Empty();
				ExtentDlg dlg(NULL, -1, _("Elevation Grid Extents"));
				dlg.SetArea(ext, (pProj->IsGeographic() != 0));
				if (dlg.ShowModal() == wxID_OK)
					m_pGrid->SetEarthExtents(dlg.m_area);
				else
					return false;
			}
			if (res == wxNO)
			{
				// Just make up some fake extents, assuming a regular even grid
				int xsize, ysize;
				m_pGrid->GetDimensions(xsize, ysize);

				DRECT ext;
				ext.left = ext.bottom = 0;
				if (pProj->IsGeographic())
				{
					ext.right = xsize * (1.0/3600);	// arc second
					ext.top = ysize * (1.0/3600);
				}
				else
				{
					ext.right = xsize * 10;	// 10 linear units (meters, feet..)
					ext.top = ysize * 10;
				}
				m_pGrid->SetEarthExtents(ext);
			}
			if (res == wxCANCEL)
				return false;
		}
		m_pGrid->SetupConversion(1.0f);
	}
	return true;
}

void vtElevLayer::SetTin(vtTin2d *pTin)
{
	m_pTin = pTin;
	m_bNative = true;
}

void vtElevLayer::MergeSharedVerts(bool bSilent)
{
	if (!m_pTin)
		return;

	OpenProgressDialog(_("Merging shared vertices"));

	int before = m_pTin->NumVerts();
	m_pTin->MergeSharedVerts(progress_callback);
	int after = m_pTin->NumVerts();

	CloseProgressDialog();

	if (!bSilent)
	{
		if (after < before)
			DisplayAndLog(_("Reduced vertices from %d to %d"), before, after);
		else
			DisplayAndLog(_("There are %d vertices, unable to merge any."), before);
	}
}

bool vtElevLayer::SetExtent(const DRECT &rect)
{
	if (m_pGrid)
	{
		m_pGrid->SetEarthExtents(rect);
		return true;
	}
	return false;
}

void vtElevLayer::GetPropertyText(wxString &strIn)
{
	wxString result = strIn, str;

	if (m_pGrid)
	{
		int cols, rows;
		m_pGrid->GetDimensions(cols, rows);
		str.Printf(_("Grid size: %d x %d\n"), cols, rows);
		result += str;

		bool bGeo = (m_pGrid->GetProjection().IsGeographic() != 0);
		result += _("Grid spacing: ");
		DPoint2 spacing = m_pGrid->GetSpacing();
		result += wxString(FormatCoord(bGeo, spacing.x), wxConvUTF8);
		result += _T(" x ");
		result += wxString(FormatCoord(bGeo, spacing.y), wxConvUTF8);
		result += _T("\n");

		if (m_pGrid->IsFloatMode())
			str.Printf(_("Floating point: Yes\n"));
		else
			str.Printf(_("Floating point: No\n"));
		result += str;

		m_pGrid->ComputeHeightExtents();
		float fMin, fMax;
		m_pGrid->GetHeightExtents(fMin, fMax);
		str.Printf(_("Minimum elevation: %.2f\n"), fMin);
		result += str;
		str.Printf(_("Maximum elevation: %.2f\n"), fMax);
		result += str;

		str.Printf(_("Height scale (meters per vertical unit): %f\n"), m_pGrid->GetScale());
		result += str;

		const char *dem_name = m_pGrid->GetDEMName();
		if (*dem_name)
		{
			str.Printf(_("Original DEM name: \"%hs\"\n"), dem_name);
			result += str;
		}
	}
	if (m_pTin)
	{
		int verts = m_pTin->NumVerts();
		int tris = m_pTin->NumTris();
		str.Printf(_("TIN\nVertices: %d\nTriangles: %d\n"), verts, tris);
		result += str;

		result += _("Min/max elevation: ");
		float minh, maxh;
		m_pTin->GetHeightExtents(minh, maxh);
		if (minh == INVALID_ELEVATION)
			str = _("None\n");
		else
			str.Printf(_T("%.2f, %.2f\n"), minh, maxh);
		result += str;
	}
	strIn = result;
}

wxString vtElevLayer::GetFileExtension()
{
	if (m_pTin)
		return _T(".itf");
	else
	{
		if (m_bPreferGZip)
			return 	_T(".bt.gz");
		else
			return 	_T(".bt");
	}
}

//
// Elevations are slightly more complicated than other layers, because there
// are two formats allowed for saving.  This gets a bit messy, especially since
// wxWindows does not support our double extension (.bt.gz) syntax.
//
bool vtElevLayer::AskForSaveFilename()
{
	wxString filter;

	if (m_pTin)
		filter = FSTRING_TIN;
	else
		filter = _("BT File (.bt)|*.bt|GZipped BT File (.bt.gz)|*.bt.gz");

	wxFileDialog saveFile(NULL, _("Save Layer"), _T(""), GetLayerFilename(),
		filter, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	saveFile.SetFilterIndex(m_pGrid && m_bPreferGZip ? 1 : 0);

	VTLOG("Asking user for elevation file name\n");
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return false;

	vtString fname = (const char *) saveFile.GetPath().mb_str(wxConvUTF8);
	VTLOG("Got filename: '%s'\n", (const char *) fname);

	if (m_pGrid)
	{
		m_bPreferGZip = (saveFile.GetFilterIndex() == 1);

		// work around incorrect extension(s) that wxFileDialog added
		RemoveFileExtensions(fname);
		if (m_bPreferGZip)
			fname += ".bt.gz";
		else
			fname += ".bt";
	}

	SetLayerFilename(wxString(fname, wxConvUTF8));
	m_bNative = true;
	return true;
}

// Helper

FPoint3 LightDirection(float angle, float direction)
{
	float phi = angle / 180.0f * PIf;
	float theta = direction / 180.0f * PIf;
	FPoint3 light_dir;
	light_dir.x = (-sin(theta)*cos(phi));
	light_dir.z = (-cos(theta)*cos(phi));
	light_dir.y = -sin(phi);
	return light_dir;
}

bool vtElevLayer::WriteGridOfElevTilePyramids(const TilingOptions &opts,
											  BuilderView *pView)
{
	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// grid size
	int base_tilesize = opts.lod0size;

	int gridcols, gridrows;
	m_pGrid->GetDimensions(gridcols, gridrows);

	DRECT area = m_pGrid->GetEarthExtents();
	DPoint2 tile_dim(area.Width()/opts.cols, area.Height()/opts.rows);
	DPoint2 cell_size = tile_dim / base_tilesize;

	const vtProjection &proj = m_pGrid->GetProjection();
	vtString units = GetLinearUnitName(proj.GetUnits());
	units.MakeLower();
	int zone = proj.GetUTMZone();
	vtString crs;
	if (proj.IsGeographic())
		crs = "LL";
	else if (zone != 0)
		crs = "UTM";
	else
		crs = "Other";

	// Try to create directory to hold the tiles
	vtString dirname = opts.fname;
	RemoveFileExtensions(dirname);
	if (!vtCreateDir(dirname))
		return false;

	// We won't know the exact height extents until the tiles have generated,
	//  so gather extents as we produce the tiles and write the INI later.
	float minheight = 1E9, maxheight = -1E9;

	ColorMap cmap;
	vtElevLayer::SetupDefaultColors(cmap);	// defaults
	vtString dirname_image = opts.fname_images;
	RemoveFileExtensions(dirname_image);
	if (opts.bCreateDerivedImages)
	{
		if (!vtCreateDir(dirname_image))
			return false;

		vtString cmap_fname = opts.draw.m_strColorMapFile;
		vtString cmap_path = FindFileOnPaths(vtGetDataPath(), "GeoTypical/" + cmap_fname);
		if (cmap_path == "")
			DisplayAndLog("Couldn't find color map.");
		else
		{
			if (!cmap.Load(cmap_path))
				DisplayAndLog("Couldn't load color map.");
		}
	}

	ImageGLCanvas *pCanvas = NULL;
#if USE_OPENGL
	wxFrame *frame = new wxFrame;
	if (opts.bCreateDerivedImages && opts.bUseTextureCompression && opts.eCompressionType == TC_OPENGL)
	{
		frame->Create(GetMainFrame(), -1, _T("Texture Compression OpenGL Context"),
			wxPoint(100,400), wxSize(280, 300), wxCAPTION | wxCLIP_CHILDREN);
		pCanvas = new ImageGLCanvas(frame);
	}
#endif

	// make a note of which lods exist
	LODMap lod_existence_map(opts.cols, opts.rows);

	bool bFloat = m_pGrid->IsFloatMode();

	int i, j, lod;
	int total = opts.rows * opts.cols, done = 0;
	for (j = 0; j < opts.rows; j++)
	{
		for (i = 0; i < opts.cols; i++)
		{
			DRECT tile_area;
			tile_area.left = area.left + tile_dim.x * i;
			tile_area.right = area.left + tile_dim.x * (i+1);
			tile_area.bottom = area.bottom + tile_dim.y * j;
			tile_area.top = area.bottom + tile_dim.y * (j+1);

			int col = i;
			int row = opts.rows-1-j;

			// draw our progress in the main view
			if (pView)
				pView->ShowGridMarks(area, opts.cols, opts.rows, col, opts.rows-1-row);

			// Extract the highest LOD we need
			vtElevationGrid base_lod(tile_area, base_tilesize+1, base_tilesize+1,
				bFloat, proj);

			bool bAllValid = true;
			bool bAllInvalid = true;
			bool bAllZero = true;
			DPoint2 p;
			int x, y;
			for (y = base_tilesize; y >= 0; y--)
			{
				p.y = area.bottom + (j*tile_dim.y) + ((double)y / base_tilesize * tile_dim.y);
				for (x = 0; x <= base_tilesize; x++)
				{
					p.x = area.left + (i*tile_dim.x) + ((double)x / base_tilesize * tile_dim.x);

					float fvalue = m_pGrid->GetFilteredValue(p);
					base_lod.SetFValue(x, y, fvalue);

					if (fvalue == INVALID_ELEVATION)
						bAllValid = false;
					else
					{
						bAllInvalid = false;

						// Gather height extents
						if (fvalue < minheight)
							minheight = fvalue;
						if (fvalue > maxheight)
							maxheight = fvalue;
					}
					if (fvalue != 0)
						bAllZero = false;
				}
			}
			// Increment whether we omit or not
			done++;

			// If there is no real data there, omit this tile
			if (bAllInvalid)
				continue;

			// Omit all-zero tiles (flat sea-level) if desired
			if (opts.bOmitFlatTiles && bAllZero)
				continue;

			// Now we know this tile will be included, so note the LODs present
			int base_tile_exponent = vt_log2(base_tilesize);
			lod_existence_map.set(i, j, base_tile_exponent, base_tile_exponent-(opts.numlods-1));

			if (!bAllValid)
			{
				UpdateProgressDialog2(done*99/total, 0, _("Filling gaps"));

				bool bGood;
				if (g_Options.GetValueBool(TAG_SLOW_FILL_GAPS))
					bGood = base_lod.FillGapsSmooth(progress_callback_minor);
				else
					bGood = base_lod.FillGaps(progress_callback_minor);
				if (!bGood)
					return false;
			}

			// Create a matching derived texture tileset
			if (opts.bCreateDerivedImages)
			{
				vtDIB dib;
				dib.Create(base_tilesize, base_tilesize, 24);
				base_lod.ComputeHeightExtents();
				base_lod.ColorDibFromElevation(&dib, &cmap, 4000);

				if (opts.draw.m_bShadingQuick)
					base_lod.ShadeQuick(&dib, SHADING_BIAS, true);
				else if (opts.draw.m_bShadingDot)
				{
					FPoint3 light_dir = LightDirection(opts.draw.m_iCastAngle,
						opts.draw.m_iCastDirection);

					// Don't cast shadows for tileset; they won't cast
					//  correctly from one tile to the next.
					base_lod.ShadeDibFromElevation(&dib, light_dir, 1.0f,
						opts.draw.m_fAmbient, opts.draw.m_fGamma, true);
				}

				for (int k = 0; k < opts.numlods; k++)
				{
					vtString fname = MakeFilenameDB(dirname_image, col, row, k);

					int tilesize = base_tilesize >> k;

					vtMiniDatabuf output_buf;
					output_buf.xsize = tilesize;
					output_buf.ysize = tilesize;
					output_buf.zsize = 1;
					output_buf.tsteps = 1;
					output_buf.set_extents(tile_area.left, tile_area.right, tile_area.top, tile_area.bottom);

					int iUncompressedSize = tilesize * tilesize * 3;
					unsigned char *rgb_bytes = (unsigned char *) malloc(iUncompressedSize);

					unsigned char *dst = rgb_bytes;
					RGBi rgb;
					for (int ro = 0; ro < base_tilesize; ro += (1<<k))
						for (int co = 0; co < base_tilesize; co += (1<<k))
						{
							dib.GetPixel24(co, ro, rgb);
							*dst++ = rgb.r;
							*dst++ = rgb.g;
							*dst++ = rgb.b;
						}

					// Write and optionally compress the image
					WriteMiniImage(fname, opts, rgb_bytes, output_buf,
						iUncompressedSize, pCanvas);

					// Free the uncompressed image
					free(rgb_bytes);
				}
			}

			for (lod = 0; lod < opts.numlods; lod++)
			{
				int tilesize = base_tilesize >> lod;

				vtString fname = MakeFilenameDB(dirname, col, row, lod);

				// make a message for the progress dialog
				wxString msg;
				msg.Printf(_("Writing tile '%hs', size %dx%d"),
					(const char *)fname, tilesize, tilesize);
				UpdateProgressDialog2(done*99/total, 0, msg);

				vtMiniDatabuf buf;
				buf.set_extents(tile_area.left, tile_area.right, tile_area.top, tile_area.bottom);
				buf.alloc(tilesize+1, tilesize+1, 1, 1, bFloat ? 2 : 1);
				float *fdata = (float *) buf.data;
				short *sdata = (short *) buf.data;

				DPoint2 p;
				int x, y;
				for (y = base_tilesize; y >= 0; y -= (1<<lod))
				{
					p.y = area.bottom + (j*tile_dim.y) + ((double)y / base_tilesize * tile_dim.y);
					for (x = 0; x <= base_tilesize; x += (1<<lod))
					{
						p.x = area.left + (i*tile_dim.x) + ((double)x / base_tilesize * tile_dim.x);

						if (bFloat)
						{
							*fdata = base_lod.GetFilteredValue(p);
							fdata++;
						}
						else
						{
							*sdata = (short) base_lod.GetFilteredValue(p);
							sdata++;
						}
					}
				}
				buf.savedata(fname);
			}
		}
	}

	// Write .ini file
	if (!WriteTilesetHeader(opts.fname, opts.cols, opts.rows, opts.lod0size,
		area, proj, minheight, maxheight, &lod_existence_map))
	{
		vtDestroyDir(dirname);
		return false;
	}

	if (opts.bCreateDerivedImages)
	{
		// Write .ini file for images
		WriteTilesetHeader(opts.fname_images, opts.cols, opts.rows,
			opts.lod0size, area, proj, INVALID_ELEVATION, INVALID_ELEVATION, &lod_existence_map);
	}

#if USE_OPENGL
	if (frame)
	{
		frame->Close();
		delete frame;
	}
#endif

	return true;
}
