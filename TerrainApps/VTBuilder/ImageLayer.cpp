//
// ImageLayer.cpp
//
// Copyright (c) 2002-2012 Virtual Terrain Project
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
#include "minidata/LocalDatabuf.h"

vtImageLayer::vtImageLayer() : vtLayer(LT_IMAGE)
{
	m_wsFilename = _("Untitled");
	m_pImage = new vtImage;
}

vtImageLayer::vtImageLayer(const DRECT &area, const IPoint2 &size,
						   const vtProjection &proj) : vtLayer(LT_IMAGE)
{
	m_wsFilename = _("Untitled");
	m_pImage = new vtImage(area, size, proj);
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

void vtImageLayer::DrawLayer(wxDC *pDC, vtScaledView *pView)
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
		// Easy case: we only change the extents
		success = m_pImage->ReprojectExtents(proj_new);
	}
	else
	{
		// Actually re-project the image pixels
		vtImage *img_new = new vtImage;

		int iSampleN = g_Options.GetValueInt(TAG_SAMPLING_N);

		OpenProgressDialog(_("Converting Image CRS"), _T(""));
		success = img_new->ConvertProjection(m_pImage, proj_new, iSampleN,
			progress_callback);

		if (success)
		{
			delete m_pImage;
			m_pImage = img_new;
		}
		else
		{
			wxString msg(_T("Couldn't reproject"));
			wxMessageBox(msg, _("Error"));
			delete img_new;
		}
		CloseProgressDialog();
	}
	SetModified(true);

	return success;
}

bool vtImageLayer::OnSave(bool progress_callback(int))
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
	rect.Offset(delta);
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

	strIn += _("Bitmaps:");
	strIn += _T("\n");
	for (size_t i = 0; i < m_pImage->NumBitmaps(); i++)
	{
		BitmapInfo &bmi = m_pImage->GetBitmapInfo(i);

		IPoint2 size = bmi.m_Size;
		str.Printf(_T("  %d: %d x %d"), i, size.x, size.y);

		//DPoint2 spacing = bmi.m_Spacing;
		//str += wxString(FormatCoord(bGeo, spacing.x), wxConvUTF8);
		//str += _T(" x ");
		//str += wxString(FormatCoord(bGeo, spacing.y), wxConvUTF8);

		if (bmi.m_pBitmap != NULL)
		{
			str += _T(", ");
			str += _("in memory");
		}

		if (bmi.m_bOnDisk)
		{
			str += _T(", ");
			str += _("overview on disk");
		}

		str += _T("\n");
		strIn += str;
	}
}

DPoint2 vtImageLayer::GetSpacing() const
{
	return m_pImage->GetSpacing();
}

bool vtImageLayer::ImportFromFile(const wxString &strFileName, bool progress_callback(int))
{
	VTLOG("ImportFromFile '%s'\n", (const char *) strFileName.mb_str(wxConvUTF8));

	wxString strExt = strFileName.AfterLast('.');

	bool success;
	if (!strExt.Left(2).CmpNoCase(_T("db")))
	{
		success = ImportFromDB(strFileName.mb_str(wxConvUTF8));
	}
	else if (!strExt.Left(3).CmpNoCase(_T("ppm")))
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

/**
 * Loads from a "DB" file, which is the format of libMini tilesets tiles.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtImageLayer::ImportFromDB(const char *szFileName, bool progress_callback(int))
{
#if USE_LIBMINI_DATABUF
	DRECT area;
	bool bAlpha;
	vtProjection proj;	// Projection is always unknown

	vtMiniDatabuf dbuf;
	dbuf.loaddata(szFileName);

	if (dbuf.type == 3)	// must be plain uncompressed RGB
		bAlpha = false;
	else if (dbuf.type == 4)
		bAlpha = true;
	else
		return false;

	area.SetRect(dbuf.nwx, dbuf.nwy, dbuf.sex, dbuf.sey);

	m_wsFilename = _("Untitled");
	m_pImage = new vtImage(area, IPoint2(dbuf.xsize, dbuf.ysize), proj);

	RGBf rgb;
	RGBAf rgba;

	int i, j;
	for (j = 0; j < (int)dbuf.ysize; j++)
	{
		//if (progress_callback != NULL)
		//	progress_callback(j * 100 / dbuf.ysize);

		for (i = 0; i < (int)dbuf.xsize; i++)
		{
			if (bAlpha)
			{
				dbuf.getrgba(i, j, 0, &rgba.r);
				m_pImage->SetRGBA(i, j, RGBAf(rgba.r, rgba.g, rgba.b, 1.0f));
			}
			else
			{
				dbuf.getrgb(i, j, 0, &rgb.r);
				m_pImage->SetRGBA(i, j, RGBi((int) rgb.r, (int) rgb.g, (int) rgb.b));
			}
		}
	}
	return true;
#else
	return false;
#endif
}


