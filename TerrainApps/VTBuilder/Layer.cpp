//
// Layer.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/vtLog.h"
#include "Frame.h"
#include "WaterLayer.h"
#include "ElevLayer.h"
#include "ImageLayer.h"
#include "RoadLayer.h"
#include "StructLayer.h"
#include "VegLayer.h"
#include "TransitLayer.h"
#include "UtilityLayer.h"
#include "RawLayer.h"

wxArrayString vtLayer::LayerTypeNames;
wxChar *vtLayer::LayerFileExtension[LAYER_TYPES] =
{
	_T(".shp"),
	_T(".bt"),
	_T(".tif"),
	_T(".rmf"),
	_T(".vtst"),
	_T(".hyd"),
	_T(".vf"),
	_T(".xml"),
	_T(".utl")
};


void AddType(wxString &str, const wxString &filter)
{
	// Chop up the input string.  Expected form is "str1|str2|str3"
	wxString str1 = str.BeforeFirst('|');

	wxString str2 = str.AfterFirst('|');
	str2 = str2.BeforeFirst('|');

	wxString str3 = str.AfterFirst('|');
	str3 = str3.AfterFirst('|');

	// Chop up the filter string.  str3 is the wildcard part.
	wxString str4 = filter.AfterFirst('|');
	str4 = str4.BeforeFirst('|');

	// Now rebuild the string, with the filter added
	wxString output = str1 + _T("|");
	output += str2;
	if (str2.Len() > 1)
		output += _T(";");
	output += str4;
	output += _T("|");
	output += str3;
	output += filter;

	str = output;
}

wxString2 GetLayerTypeName(const LayerType &ltype)
{
	if (ltype == LT_UNKNOWN)
		return wxString2(_("Unknown"));
	else
		return vtLayer::LayerTypeNames[ltype];
}

//////////////////////////////////////////////////////////

vtLayer::vtLayer(LayerType type)
{
	m_type = type;
	m_bVisible = true;
	m_bModified = false;
	m_bNative = false;
}

vtLayer::~vtLayer()
{
	int foo = 1;
}

bool vtLayer::Save(const wxString &filename)
{
	if (!m_bNative)
	{
		if (!AskForSaveFilename())
			return false;
	}
	VTLOG("Saving data...");
	bool success = OnSave();
	if (success)
	{
		VTLOG("OK.\n");
		m_bNative = true;
		SetModified(false);
	}
	else
		VTLOG("Failed.\n");

	return success;
}

bool vtLayer::Load(const wxString &filename)
{
	if (filename != _T(""))
		SetLayerFilename(filename);
	bool success = OnLoad();
	if (success)
		m_bNative = true;
	return success;
}

bool vtLayer::SetVisible(bool bVisible)
{
	bool prev = m_bVisible;
	m_bVisible = bVisible;
	return prev;
}

void vtLayer::SetModified(bool bModified)
{
	bool bNeedRefresh = (m_bModified != bModified);
	m_bModified = bModified;
	if (bNeedRefresh)
		GetMainFrame()->RefreshTreeStatus();
}

void vtLayer::SetLayerFilename(const wxString2 &fname)
{
	wxString2 fname_in = fname;
	VTLOG("Setting layer filename to '%s'\n", fname_in.mb_str());
	bool bNeedRefresh = (m_wsFilename.Cmp(fname_in) != 0);
	m_wsFilename = fname_in;
	if (bNeedRefresh)
		GetMainFrame()->RefreshTreeStatus();
}

wxString vtLayer::GetSaveFileDialogFilter()
{
	wxString str;

	str = LayerTypeNames[m_type];
	str += _(" Files");
	str += _T(" (*");

	wxString ext = GetFileExtension();
	str += ext;
	str += _T(")|*");
	str += ext;
	str += _("|All Files|*.*|");

	return str;
}

bool vtLayer::AskForSaveFilename()
{
	wxString filter = GetSaveFileDialogFilter();
	wxString2 name = GetLayerFilename();
#if WIN32
	// The dumb Microsoft file dialog can have trouble with forward slashes.
	name = name.ToBackslash();
#endif
	wxFileDialog saveFile(NULL, _("Save Layer"), _T(""), name,
		filter, wxSAVE | wxOVERWRITE_PROMPT);

	VTLOG("Asking user for file name\n");
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return false;

	name = saveFile.GetPath();
	VTLOG("Got filename: '%s'\n", name.mb_str());

	// Add file extension if user didn't specify it
	wxString2 ext = GetFileExtension();
	if (name.Len() < ext.Len() || name.Right(ext.Len()) != ext)
	{
		name += ext;
	}
	SetLayerFilename(name);
	m_bNative = true;
	return true;
}

wxString vtLayer::GetFileExtension()
{
	return (wxString) LayerFileExtension[GetType()];
}

void vtLayer::SetMessageText(const wxString &msg)
{
	GetMainFrame()->SetStatusText(msg);
}

void vtLayer::GetProjection(vtProjection &proj)
{
	// shouldn't get here.
}

void vtLayer::Offset(const DPoint2 &p)
{
	// shouldn't get here unless a layer is incapable of offset
}

/////////////////////////////////////////////////

// static
//
vtLayer *vtLayer::CreateNewLayer(LayerType ltype)
{
	vtLayer *pLayer = NULL;
	switch (ltype)
	{
	case LT_RAW:
		pLayer = new vtRawLayer();
		break;
	case LT_ELEVATION:
		pLayer = new vtElevLayer();
		break;
	case LT_IMAGE:
		pLayer = new vtImageLayer();
		break;
	case LT_ROAD:
		pLayer = new vtRoadLayer();
		break;
	case LT_STRUCTURE:
		pLayer = new vtStructureLayer();
		break;
	case LT_UTILITY:
		pLayer = new vtUtilityLayer();
		break;
	case LT_WATER:
		pLayer = new vtWaterLayer();
		break;
	case LT_VEG:
		pLayer = new vtVegLayer();
		break;
#if SUPPORT_TRANSIT
	case LT_TRANSIT:
		pLayer = new vtTransitLayer();
		break;
#endif
	}
	return pLayer;
}

////////////////////////////////////////////////


void LayerArray::DestructItems(unsigned int first, unsigned int last)
{
	for (unsigned int i = first; i <= last; ++i)
	{
		vtLayerPtr lp = GetAt(i);

		// safety check
		assert(lp->GetType() >= LT_UNKNOWN && lp->GetType() < LAYER_TYPES);

		delete lp;
	}
}


///////////////////////////////////////////////////////////////////////

DrawStyle::DrawStyle()
{
	m_LineColor.Set(0,0,0);
	m_FillColor.Set(255,255,255);
	m_bFill = false;
}

