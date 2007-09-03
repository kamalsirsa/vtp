//
// ImageLayer.cpp
//
// Copyright (c) 2002-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/vtLog.h"
#include "vtui/Helper.h"	// For ProgressDialog
#include "ImageLayer.h"
#include "vtImage.h"
#include "Options.h"

vtImageLayer::vtImageLayer() : vtLayer(LT_IMAGE)
{
	m_wsFilename = _("Untitled");
	m_pImage = new vtImage;
}

vtImageLayer::vtImageLayer(const DRECT &area, int xsize, int ysize,
						   const vtProjection &proj) : vtLayer(LT_IMAGE)
{
	m_wsFilename = _("Untitled");
	m_pImage = new vtImage(area, xsize, ysize, proj);
}

vtImageLayer::~vtImageLayer()
{
	delete m_pImage;
	m_pImage = NULL;
}

bool vtImageLayer::GetExtent(DRECT &rect)
{
	return m_pImage->GetExtent(rect);
}

void vtImageLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	m_pImage->DrawToView(pDC, pView);
}

bool vtImageLayer::TransformCoords(vtProjection &proj_new)
{
	vtProjection proj_old;
	GetProjection(proj_old);

	if (proj_old == proj_new)
		return true;		// No conversion necessary

	bool success = false;

	// Check to see if the projections differ *only* by datum
	vtProjection test = proj_old;
	test.SetDatum(proj_new.GetDatum());
	if (test == proj_new)
	{
		success = m_pImage->ReprojectExtents(proj_new);
		SetModified(true);
	}
	else
	{
		// wxMessageBox(_("Transformation of Image Layers is not supported."), _("Warning"));
		// Actually re-project the image pixels
		vtImage *img_new = new vtImage;

		int iSampleN = g_Options.GetValueInt(TAG_SAMPLING_N);

		OpenProgressDialog(_("Converting Image CRS"));
		success = img_new->ConvertProjection(m_pImage, proj_new, iSampleN,
			progress_callback);

		if (success)
		{
			delete m_pImage;
			m_pImage = img_new;
			//  TODO ReImage();
		}
		else
		{
			wxString msg(_T("Couldn't reproject"));
			wxMessageBox(msg, _("Error"));
			delete img_new;
		}
		CloseProgressDialog();
	}

	return success;
}

bool vtImageLayer::OnSave()
{
	return m_pImage->SaveToFile(GetLayerFilename().mb_str(wxConvUTF8));
}

bool vtImageLayer::OnLoad()
{
	vtString fname = (const char *) GetLayerFilename().mb_str(wxConvUTF8);
	return m_pImage->LoadFromGDAL(fname);
}

bool vtImageLayer::AppendDataFrom(vtLayer *pL)
{
	return false;
}

void vtImageLayer::GetProjection(vtProjection &proj)
{
	m_pImage->GetProjection(proj);
}

void vtImageLayer::SetProjection(const vtProjection &proj)
{
	m_pImage->SetProjection(proj);
}

void vtImageLayer::Offset(const DPoint2 &delta)
{
	// Shifting an image is as easy as shifting its extents
	DRECT rect;
	m_pImage->GetExtent(rect);
	rect.left += delta.x;
	rect.right += delta.x;
	rect.top += delta.y;
	rect.bottom += delta.y;
	m_pImage->SetExtent(rect);
}

bool vtImageLayer::SetExtent(const DRECT &rect)
{
	m_pImage->SetExtent(rect);
	return true;
}

void vtImageLayer::GetPropertyText(wxString &strIn)
{
	DRECT extents;
	m_pImage->GetExtent(extents);
	IPoint2 size = m_pImage->GetDimensions();
	vtProjection proj;
	m_pImage->GetProjection(proj);

	strIn.Printf(_("Dimensions %d by %d pixels"), size.x, size.y);
	strIn += _T("\n");

	strIn += _("Spacing: ");
	DPoint2 spacing(extents.Width() / size.x, extents.Height() / size.y);

	bool bGeo = (proj.IsGeographic() != 0);
	wxString str;
	str += wxString(FormatCoord(bGeo, spacing.x), wxConvUTF8);
	str += _T(" x ");
	str += wxString(FormatCoord(bGeo, spacing.y), wxConvUTF8);
	str += _T("\n");
	strIn += str;
}

DPoint2 vtImageLayer::GetSpacing() const
{
	return m_pImage->GetSpacing();
}

bool vtImageLayer::ImportFromFile(const wxString &strFileName, bool progress_callback(int am))
{
	VTLOG("ImportFromFile '%s'\n", (const char *) strFileName.mb_str(wxConvUTF8));

	wxString strExt = strFileName.AfterLast('.');

	bool success;
	if (!strExt.Left(3).CmpNoCase(_T("ppm")))
	{
		m_pImage = new vtImage;
		success = m_pImage->ReadPPM(strFileName.mb_str(wxConvUTF8));
		if (!success)
		{
			delete m_pImage;
			m_pImage = NULL;
		}
	}
	else
	{
		SetLayerFilename(strFileName);
		success = OnLoad();
	}
	return success;
}

void vtImageLayer::ReplaceColor(const RGBi &rgb1, const RGBi &rgb2)
{
	m_pImage->ReplaceColor(rgb1, rgb2);
	SetModified(true);
}

