//
// Layer.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

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

char *vtLayer::LayerTypeName[LAYER_TYPES] =
{
	"Raw",
	"Elevation",
	"Image",
	"Road",
	"Structure",
	"Water",
	"Vegetation",
	"Transit",
	"Utility"
};

char *vtLayer::LayerFileExtension[LAYER_TYPES] =
{
	".shp",
	".bt",
	".bmp",
	".rmf",
	".vtst",
	".hyd",
	".vf",
	".xml",
	".utl"
};


void AddType(wxString &str, wxString filter)
{
	wxString str1 = str.BeforeFirst('|');

	wxString str2 = str.AfterFirst('|');
	str2 = str2.BeforeFirst('|');

	wxString str3 = str.AfterFirst('|');
	str3 = str3.AfterFirst('|');

	wxString str4 = filter.AfterFirst('|');
	str4 = str4.BeforeFirst('|');

//	wxString output = "All Known Formats|";
	wxString output = str1 + "|";
	output += str2;
	if (str2.Len() > 1)
		output += ";";
	output += str4;
	output += "|";
	output += str3;
	output += filter;

	str = output;
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

bool vtLayer::Save(const char *filename)
{
	if (!m_bNative)
	{
		if (!AskForSaveFilename())
			return false;
	}
	bool success = OnSave();
	if (success)
	{
		m_bNative = true;
		SetModified(false);
	}
	return success;
}

bool vtLayer::Load(const char *filename)
{
	if (filename)
		SetFilename(filename);
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

void vtLayer::SetFilename(wxString str)
{
	bool bNeedRefresh = (m_strFilename.Cmp(str) != 0);
	m_strFilename = str;
	GetMainFrame()->RefreshTreeStatus();
}

char *vtLayer::GetFileDialogFilter()
{
	char *str2 = new char[80], *str = str2;
	int len;

	len = strlen(LayerTypeName[m_type]);
	memcpy(str, LayerTypeName[m_type], len);
	str += len;

	memcpy(str, " Files (*", 9);
	str += 9;

	char *ext = GetFileExtension();
	len = strlen(ext);
	memcpy(str, ext, len);
	str += len;

	memcpy(str, ")|*", 3);
	str += 3;

	len = strlen(ext);
	memcpy(str, ext, len);
	str += len;

	memcpy(str, "|All Files|*.*|", 16);

	return str2;
}

bool vtLayer::AskForSaveFilename()
{
	char *filter = GetFileDialogFilter();
	wxFileDialog saveFile(NULL, "Save Layer", "", "", filter, wxSAVE);

	bool bResult = (saveFile.ShowModal() == wxID_OK);
	delete filter;
	if (!bResult)
		return false;

	wxString name = saveFile.GetPath();
	wxString ext = GetFileExtension();
	if (name.Len() < ext.Len() ||
		name.Right(ext.Len()) != ext)
	{
		name += ext;
	}
	SetFilename(name);
	m_bNative = true;
	return true;
}

char *vtLayer::GetFileExtension()
{
	return LayerFileExtension[GetType()];
}

void vtLayer::SetMessageText(const char *msg)
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


void LayerArray::DestructItems(int first, int last)
{
	for (int i = first; i <= last; ++i)
	{
		vtLayerPtr lp = GetAt(i);

		// safety check
		assert(lp->GetType() >= LT_UNKNOWN && lp->GetType() < LAYER_TYPES);

		delete lp;
	}
}


