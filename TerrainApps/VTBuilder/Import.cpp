//
// methods for importing data
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/DLG.h"
#include "vtdata/LULC.h"

#include "Frame.h"
#include "Helper.h"
// Layers
#include "StructLayer.h"
#include "WaterLayer.h"
#include "ElevLayer.h"
#include "RawLayer.h"
#include "RoadLayer.h"
#include "VegLayer.h"
// Dialogs
#include "ProjectionDlg.h"
#include "ImportVegDlg.h"

#include "ogrsf_frmts.h"

//
// remember a set of directories, one for each layer type
//
wxString ImportDirectory[LAYER_TYPES];


//
// Ask the user for a filename, and import data from it.
//
void MainFrame::ImportData(LayerType ltype)
{
	// make a string which contains filters for the appropriate file types
	wxString filter = GetImportFilterString(ltype);

	// ask the user for a filename
	// default the same directory they used last time for a layer of this type
	wxFileDialog loadFile(NULL, "Import Data", ImportDirectory[ltype], "", filter, wxOPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString strFileName = loadFile.GetPath();

	// remember the directory they used
	ImportDirectory[ltype] = loadFile.GetDirectory();

	ImportDataFromFile(ltype, strFileName, true);
}

//
// Import data of a given type from a file
//
void MainFrame::ImportDataFromFile(LayerType ltype, wxString strFileName, bool bRefresh)
{
	// check to see if the file is readable
	FILE *fp = fopen(strFileName, "rb");
	if (!fp)
		return;	// Cannot Open File
	fclose(fp);

	wxString msg = "Importing Data from ";
	msg += strFileName;
	OpenProgressDialog(msg);

	// check the file extension
	wxString strExt = strFileName.AfterLast('.');

	// call the appropriate reader
	vtLayerPtr pLayer = NULL;
	switch (ltype)
	{
	case LT_ELEVATION:
		pLayer = ImportElevation(strFileName);
		break;
	case LT_ROAD:
	case LT_WATER:
		if (!strExt.CmpNoCase("dlg"))
		{
			pLayer = ImportFromDLG(strFileName, ltype);
		}
		else if (!strExt.CmpNoCase("shp"))
		{
			pLayer = ImportFromSHP(strFileName, ltype);
		}
		else if (!strFileName.Right(8).CmpNoCase("catd.ddf"))
		{
			pLayer = ImportVectorsWithOGR(strFileName, ltype);
		}
		break;
	case LT_STRUCTURE:
		if (!strExt.CmpNoCase("shp"))
		{
			pLayer = ImportFromSHP(strFileName, ltype);
		}
		if (!strExt.CmpNoCase("bcf"))
		{
			pLayer = ImportFromBCF(strFileName);
		}
		break;
	case LT_VEG:
		if (!strExt.CmpNoCase("gir"))
		{
			pLayer = ImportFromLULC(strFileName, ltype);
		}
		if (!strExt.CmpNoCase("shp"))
		{
			pLayer = ImportFromSHP(strFileName, ltype);
		}
		break;
	case LT_UNKNOWN:
		if (!strExt.CmpNoCase("gir"))
		{
			pLayer = ImportFromLULC(strFileName, ltype);
		}
		if (!strExt.CmpNoCase("bcf"))
		{
			pLayer = ImportFromBCF(strFileName);
		}
		else if (!strExt.CmpNoCase("dlg"))
		{
			pLayer = ImportFromDLG(strFileName, ltype);
		}
		else if (!strFileName.Right(8).CmpNoCase("catd.ddf"))
		{
			// SDTS file: might be Elevation or Vector (SDTS-DEM or SDTS-DLG)
			// To try to distinguish, look for a file called xxxxrsdf.ddf
			// which would indicate that it is a raster.
			bool bRaster = false;
			int len = strFileName.Length();
			wxString strFileName2 = strFileName.Left(len - 8);
			FILE *fp;
			fp = fopen(strFileName2 + "rsdf.ddf", "rb");
			if (fp != NULL)
			{
				bRaster = true;
				fclose(fp);
			}
			else
			{
				// also try with upper-case (for Unix)
				fp = fopen(strFileName2 + "RSDF.DDF", "rb");
				if (fp != NULL)
				{
					bRaster = true;
					fclose(fp);
				}
			}
		
			if (bRaster)
				pLayer = ImportElevation(strFileName);
			else
				pLayer = ImportVectorsWithOGR(strFileName, ltype);
		}
		else if (!strExt.CmpNoCase("shp"))
		{
			pLayer = ImportFromSHP(strFileName, ltype);
		}
		else
		{
			// Many other Elevation formats are supported
			pLayer = ImportElevation(strFileName);
		}
		break;
	case LT_RAW:
		if (!strExt.CmpNoCase("shp"))
			pLayer = ImportFromSHP(strFileName, ltype);
		break;
	}

	CloseProgressDialog();

	if (!pLayer)
	{
		// import failed
		return;
	}
	pLayer->SetFilename(strFileName);
	pLayer->SetModified(true);

	bool success = AddLayerWithCheck(pLayer, true);
	if (!success)
		delete pLayer;
}

//
// type to guess layer type from a DLG file
//
LayerType MainFrame::GuessLayerTypeFromDLG(vtDLGFile *pDLG)
{
	LayerType ltype;
	DLGType dtype = pDLG->GuessFileType();

	// convert the DLG type to one of our layer types
	switch (dtype)
	{
	case DLG_HYPSO:		ltype = LT_RAW; break;
	case DLG_HYDRO:		ltype = LT_WATER; break;
	case DLG_VEG:		ltype = LT_RAW; break;
	case DLG_NONVEG:	ltype = LT_RAW; break;
	case DLG_BOUNDARIES:ltype = LT_RAW; break;
	case DLG_MARKERS:	ltype = LT_RAW; break;
	case DLG_ROAD:		ltype = LT_ROAD; break;
	case DLG_RAIL:		ltype = LT_ROAD; break;
	case DLG_MTF:		ltype = LT_RAW; break;
	case DLG_UNKNOWN:
		{
			// if we can't tell from the DLG, ask the user
			ltype = AskLayerType();
		}
		break;
	}
	return ltype;
}

//
// based on the type of layer, choose which file
// types (file extensions) to allow for import
//
wxString GetImportFilterString(LayerType ltype)
{
	wxString filter = "All Known ";
	filter += vtLayer::LayerTypeName[ltype];
	filter += " Formats||";

	switch (ltype)
	{
	case LT_RAW:
		// shp
		AddType(filter, FSTRING_SHP);
		break;
	case LT_ELEVATION:
		// dem, etc.
		AddType(filter, FSTRING_ADF);
		AddType(filter, FSTRING_ASC);
		AddType(filter, FSTRING_BIL);
		AddType(filter, FSTRING_CDF);
		AddType(filter, FSTRING_DEM);
		AddType(filter, FSTRING_DTED);
		AddType(filter, FSTRING_GTOPO);
		AddType(filter, FSTRING_PGM);
		AddType(filter, FSTRING_RAW);
		AddType(filter, FSTRING_SDTS);
		AddType(filter, FSTRING_Surfer);
		AddType(filter, FSTRING_TER);
		AddType(filter, FSTRING_TIF);
		break;
	case LT_IMAGE:
		// doq, tif
		AddType(filter, FSTRING_DOQ);
		AddType(filter, FSTRING_TIF);
		break;
	case LT_ROAD:
		// dlg, shp, sdts-dlg
		AddType(filter, FSTRING_DLG);
		AddType(filter, FSTRING_SHP);
		AddType(filter, FSTRING_SDTS);
		break;
	case LT_STRUCTURE:
		// shp
		AddType(filter, FSTRING_SHP);
		AddType(filter, FSTRING_BCF);
		break;
	case LT_WATER:
		// dlg, shp, sdts
		AddType(filter, FSTRING_DLG);
		AddType(filter, FSTRING_SHP);
		AddType(filter, FSTRING_SDTS);
		break;
	case LT_VEG:
		// lulc, shp, sdts
		AddType(filter, FSTRING_LULC);
		AddType(filter, FSTRING_SHP);
		AddType(filter, FSTRING_SDTS);
		break;
	}
	return filter;
}


vtLayerPtr MainFrame::ImportFromDLG(wxString &strFileName, LayerType ltype)
{
	vtDLGFile *pDLG = new vtDLGFile();
	bool success = pDLG->Read(strFileName, progress_callback);
	if (!success)
	{
		wxString msg = pDLG->GetErrorMessage();
		wxMessageBox(msg);
		delete pDLG;
		return NULL;
	}

	// try to guess what kind of data it is by asking the DLG object
	// to look at its attributes
	if (ltype == LT_UNKNOWN)
		ltype = GuessLayerTypeFromDLG(pDLG);

	// create the new layer
	vtLayerPtr pLayer = vtLayer::CreateNewLayer(ltype);

	// read the DLG data into the layer
	if (ltype == LT_ROAD)
	{
		vtRoadLayer *pRL = (vtRoadLayer *)pLayer;
		pRL->AddElementsFromDLG(pDLG);
		pRL->RemoveUnusedNodes();
	}
	if (ltype == LT_WATER)
	{
		vtWaterLayer *pWL = (vtWaterLayer *)pLayer;
		pWL->AddElementsFromDLG(pDLG);
	}
	// now we no longer need the DLG object
	delete pDLG;

	return pLayer;
}

vtLayerPtr MainFrame::ImportFromSHP(wxString &strFileName, LayerType ltype)
{
	SHPHandle hSHP = SHPOpen(strFileName, "rb");
	if (hSHP == NULL)
	{
		wxMessageBox("Couldn't read that Shape file.  Perhaps it is\n"
			"missing its corresponding .dbf and .shx files.");
		return NULL;
	}
	else
		SHPClose(hSHP);

	//if layer type unknow, ask user input
	if (ltype == LT_UNKNOWN)
	{
		ltype = AskLayerType();
	}

	// create the new layer
	vtLayerPtr pLayer = vtLayer::CreateNewLayer(ltype);

	// ask use for a projection
	ProjectionDlg dlg(NULL, -1, "Projection", wxDefaultPosition);
	dlg.m_strCaption = "Shapefiles do not contain projection information.  "
		"Please indicate the projection of this file:";
	dlg.SetProjection(m_proj);

	if (dlg.ShowModal() == wxID_CANCEL)
		return NULL;
	vtProjection proj;
	dlg.GetProjection(proj);

	// read DLG data into the layer
	if (ltype == LT_ROAD)
	{
		vtRoadLayer *pRL = (vtRoadLayer *)pLayer;
		pRL->AddElementsFromSHP(strFileName, proj);
		pRL->RemoveUnusedNodes();
	}

	// read vegtype SHP data into the layer
	if (ltype == LT_VEG)
	{
		ImportVegDlg dlg(this, -1, "Import Vegetation Information");
		dlg.SetShapefileName(strFileName);
		if (dlg.ShowModal() == wxID_CANCEL)
			return NULL;
		vtVegLayer *pVL = (vtVegLayer *)pLayer;
		pVL->AddElementsFromSHP(strFileName, proj, dlg.m_fieldindex, dlg.m_datatype);
	}

	if (ltype == LT_WATER)
	{
		vtWaterLayer *pWL = (vtWaterLayer *)pLayer;
		pWL->AddElementsFromSHP(strFileName, proj);
	}

	if (ltype == LT_STRUCTURE)
	{
		vtStructureLayer *pSL = (vtStructureLayer *)pLayer;
		pSL->AddElementsFromSHP(strFileName, proj);
	}

	if (ltype == LT_RAW)
	{
		pLayer->SetFilename(strFileName);
		if (!pLayer->OnLoad())
		{
			delete pLayer;
			pLayer = NULL;
		}
		pLayer->SetProjection(proj);
	}
	return pLayer;
}


vtLayerPtr MainFrame::ImportElevation(wxString &strFileName)
{
	bool bFirst = (m_Layers.GetSize() == 0);
	wxString strExt = strFileName.AfterLast('.');

	vtElevLayer *pElev = new vtElevLayer();

	bool success = pElev->ImportFromFile(strFileName, progress_callback);

	if (success)
		return pElev;
	else
	{
		delete pElev;
		return NULL;
	}
};


vtLayerPtr MainFrame::ImportFromLULC(wxString &strFileName, LayerType ltype)
{
	// Read LULC file, check for errors
	vtLULCFile *pLULC = new vtLULCFile(strFileName);
	if (pLULC->m_iError)
	{
		wxString msg = pLULC->GetErrorMessage();
		wxMessageBox(msg);
		delete pLULC;
		return NULL;
	}

	// If layer type unknown, assume it's veg type
	if (ltype = LT_UNKNOWN)
		ltype=LT_VEG;

	pLULC->ProcessLULCPolys();

	// Create New Layer
	vtLayerPtr pLayer = vtLayer::CreateNewLayer(ltype);

	// Read LULC data into the new Veg layer
	vtVegLayer *pVL = (vtVegLayer *)pLayer;

	pVL->AddElementsFromLULC(pLULC);

	// Now we no longer need the LULC object
	delete pLULC;

	return pLayer;
}

vtStructureLayer *MainFrame::ImportFromBCF(wxString &strFileName)
{
	vtStructureLayer *pSL = new vtStructureLayer();
	if (pSL->ReadBCF(strFileName))
		return pSL;
	else
	{
		delete pSL;
		return NULL;
	}
}

vtLayerPtr MainFrame::ImportVectorsWithOGR(wxString &strFileName, LayerType ltype)
{
	OGRRegisterAll();

	OGRDataSource *datasource = OGRSFDriverRegistrar::Open( strFileName );
	if (!datasource)
		return NULL;

	if (ltype == LT_UNKNOWN)
	{
		// TODO: Try to guess the layer type from the file
		// FOr now, just assume it's transportation
		ltype = LT_ROAD;
	}

	// create the new layer
	vtLayerPtr pLayer = vtLayer::CreateNewLayer(ltype);

	// read the OGR data into the layer
	if (ltype == LT_ROAD)
	{
		vtRoadLayer *pRL = (vtRoadLayer *)pLayer;
		pRL->AddElementsFromOGR(datasource, progress_callback);
//		pRL->RemoveUnusedNodes();
	}
	if (ltype == LT_WATER)
	{
		vtWaterLayer *pWL = (vtWaterLayer *)pLayer;
//		pWL->AddElementsFromSDTSSource(datasource, progress_callback);
	}

	delete datasource;

	return pLayer;
}

