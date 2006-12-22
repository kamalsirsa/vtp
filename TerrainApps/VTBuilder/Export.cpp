//
//  The export functions of the VTBuilder application.
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/progdlg.h>

#include "vtdata/ChunkLOD.h"
#include "vtdata/vtDIB.h"
#include "vtdata/vtLog.h"

#include "vtui/Helper.h"

#include "BuilderView.h"
#include "FileFilters.h"
#include "Frame.h"
#include "Helper.h"
#include "vtBitmap.h"
// Layers
#include "ElevLayer.h"
#include "ImageLayer.h"
// Dialogs
#include "ChunkDlg.h"
#include "RenderDlg.h"
#include "RenderOptionsDlg.h"
#include "TileDlg.h"

#if USE_OPENGL
	#include "wx/glcanvas.h"	// needed for writing pre-compressed textures
#endif


void MainFrame::ExportASC()
{
	// check spacing
	vtElevationGrid *grid = GetActiveElevLayer()->m_pGrid;
	DPoint2 spacing = grid->GetSpacing();
	double ratio = spacing.x / spacing.y;
	if (ratio < 0.999 || ratio > 1.001)
	{
		wxString str, str2;
		str = _("The Arc ASCII format only supports evenly spaced elevation grids.\n");
		str2.Printf(_("The spacing of this grid is %g x %g\n"), spacing.x, spacing.y);
		str += str2;
		str += _("The result my be stretched.  Do you want to continue anyway?");
		int result = wxMessageBox(str, _("Warning"), wxYES_NO | wxICON_QUESTION, this);
		if (result != wxYES)
			return;
	}

	vtString fname = GetActiveLayer()->GetExportFilename(FSTRING_ASC);
	if (fname == "")
		return;
	bool success = grid->SaveToASC(fname);
	if (success)
		DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::ExportTerragen()
{
	vtString fname = GetActiveLayer()->GetExportFilename(FSTRING_TER);
	if (fname == "")
		return;
	bool success = GetActiveElevLayer()->m_pGrid->SaveToTerragen(fname);
	if (success)
		DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::ExportGeoTIFF()
{
	vtString fname = GetActiveLayer()->GetExportFilename(FSTRING_TIF);
	if (fname == "")
		return;
	bool success = GetActiveElevLayer()->m_pGrid->SaveToGeoTIFF(fname);
	if (success)
		DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::ExportBMP()
{
	vtString fname = GetActiveLayer()->GetExportFilename(FSTRING_BMP);
	if (fname == "")
		return;
	bool success = GetActiveElevLayer()->m_pGrid->SaveToBMP(fname);
	if (success)
		DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::ExportSTM()
{
	vtString fname = GetActiveLayer()->GetExportFilename(FSTRING_STM);
	if (fname == "")
		return;
	bool success = GetActiveElevLayer()->m_pGrid->SaveToSTM(fname);
	if (success)
		DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::ExportPlanet()
{
	static wxString default_path = wxEmptyString;

	// ask the user for a directory
	wxDirDialog getDir(this, _("Export Planet Data to Directory"),
		default_path, wxDD_DEFAULT_STYLE);
	getDir.SetWindowStyle(getDir.GetWindowStyle() | wxDD_NEW_DIR_BUTTON);
	bool bResult = (getDir.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxString strDirName = getDir.GetPath();
	default_path = strDirName;	// save it for next time

	if (strDirName == _T(""))
		return;
	bool success = GetActiveElevLayer()->m_pGrid->SaveToPlanet(strDirName.mb_str(wxConvUTF8));
	if (success)
		DisplayAndLog("Successfully wrote Planet dataset to '%s'",
		(const char *) strDirName.mb_str(wxConvUTF8));
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::ExportVRML()
{
	vtString fname = GetActiveLayer()->GetExportFilename(FSTRING_WRL);
	if (fname == "")
		return;
	bool success = GetActiveElevLayer()->m_pGrid->SaveToVRML(fname);
	if (success)
		DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::ExportRAWINF()
{
	vtString fname = GetActiveLayer()->GetExportFilename(FSTRING_RAW);
	if (fname == "")
		return;
	bool success = GetActiveElevLayer()->m_pGrid->SaveToRAWINF(fname);
	if (success)
		DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::ExportChunkLOD()
{
	// Check dimensions; must be 2^n+1 for .chu
	vtElevationGrid *grid = GetActiveElevLayer()->m_pGrid;
	int x, y;
	grid->GetDimensions(x, y);
	bool good = false;
	for (int i = 3; i < 20; i++)
		if (x == ((1<<i)+1) && y == ((1<<i)+1))
			good = true;
	if (!good)
	{
		DisplayAndLog("The elevation grid has dimensions %d x %d.  They must be\n"
					  "a power of 2 plus 1 for .chu, e.g. 1025x1025.", x, y);
		return;
	}

	// Ask for filename
	vtString fname = GetActiveLayer()->GetExportFilename(FSTRING_CHU);
	if (fname == "")
		return;

	ChunkDlg dlg(this, -1, _("Export ChunkLOD"));
	dlg.m_iDepth = 6;
	dlg.m_fMaxError = 1.0f;
	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	FILE *out = vtFileOpen(fname, "wb");
	if (out == 0) {
		DisplayAndLog("Error: can't open %s for output.", (const char *) fname);
		return;
	}

	const int CHUNKLOD_MAX_HEIGHT = 10000;
	float vertical_scale = CHUNKLOD_MAX_HEIGHT / 32767.0f;
	float input_vertical_scale = 1.0f;

	OpenProgressDialog(_T("Writing ChunkLOD"), false, this);

	// Process the data.
	HeightfieldChunker hc;
	bool success = hc.ProcessGrid(grid, out, dlg.m_iDepth, dlg.m_fMaxError,
		vertical_scale, input_vertical_scale, progress_callback);
	fseek(out, 0, SEEK_END);
	g_chunkstats.output_size = ftell(out);
	fclose(out);

	CloseProgressDialog();

	if (success)
	{
		vtString msg, str;
		msg.Format("Successfully wrote file '%s'\n", (const char *) fname);

		float verts_per_chunk = g_chunkstats.output_vertices / (float) g_chunkstats.output_chunks;

		str.Format(" Average verts/chunk: %.0f\n", verts_per_chunk);
		msg += str;
		str.Format(" Output filesize: %dk\n", (int) (g_chunkstats.output_size / 1024.0f));
		msg += str;
		str.Format(" Bytes/input vert: %.2f\n", g_chunkstats.output_size / (float) g_chunkstats.input_vertices);
		msg += str;
		str.Format(" Bytes/output vert: %.2f\n", g_chunkstats.output_size / (float) g_chunkstats.output_vertices);
		msg += str;

		if (verts_per_chunk < 500)
		{
			str.Format("NOTE: verts/chunk is low; for higher poly throughput\nconsider setting depth to %d and reprocessing.\n",
				dlg.m_iDepth - 1);
			msg += "\n";
			msg += str;
		} else if (verts_per_chunk > 5000)
		{
			str.Format("NOTE: verts/chunk is high; for smoother framerate\nconsider setting depth to %d and reprocessing.\n",
				dlg.m_iDepth + 1);
			msg += "\n";
			msg += str;
		}
		DisplayAndLog(msg);
	}
	else
	{
		// TODO: Politely delete the incomplete file?
		if (g_chunkstats.output_most_vertices_per_chunk > (1<<16))
		{
			DisplayAndLog("Error: chunk contains > 64K vertices.  Try processing again, but use\n"\
				"a deeper chunk tree, for fewer vertices per chunk.");
		}
		else
			DisplayAndLog("Error writing file.");
	}
}

void MainFrame::ExportPNG16()
{
	vtString fname = GetActiveLayer()->GetExportFilename(FSTRING_PNG);
	if (fname == "")
		return;
	bool success = GetActiveElevLayer()->m_pGrid->SaveToPNG16(fname);
	if (success)
		DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::Export3TX()
{
	vtElevationGrid *grid = GetActiveElevLayer()->m_pGrid;
	int col, row;
	grid->GetDimensions(col, row);
	if (col != 1201 || row != 1201)
	{
		DisplayAndLog("3TX expects grid dimensions of 1201 x 1201");
		return;
	}
	vtString fname = GetActiveLayer()->GetExportFilename(FSTRING_3TX);
	if (fname == "")
		return;
	bool success = grid->SaveTo3TX(fname);
	if (success)
		DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::ElevExportTiles()
{
	vtElevLayer *pEL = GetActiveElevLayer();
	vtElevationGrid *grid = pEL->m_pGrid;
	bool floatmode = (grid->IsFloatMode() || grid->GetScale() != 1.0f);
	DPoint2 spacing = grid->GetSpacing();
	DRECT area = grid->GetEarthExtents();

	TilingOptions tileopts;
	tileopts.cols = 4;
	tileopts.rows = 4;
	tileopts.lod0size = 256;
	tileopts.numlods = 3;

	TileDlg dlg(this, -1, _("Tiling Options"));
	dlg.m_fEstX = spacing.x;
	dlg.m_fEstY = spacing.y;
	dlg.SetElevation(true);
	dlg.SetArea(area);
	dlg.SetTilingOptions(tileopts);
	dlg.SetView(GetView());

	int ret = dlg.ShowModal();
	GetView()->HideGridMarks();
	if (ret == wxID_CANCEL)
		return;

	dlg.GetTilingOptions(tileopts);

	// Also write derived image tiles?
	int res = wxMessageBox(_("Also write derived image tiles?"), _("Tiled output"), wxYES_NO | wxCANCEL);
	if (res == wxCANCEL)
		return;
	if (res == wxYES)
	{
		// Ask them where to write the image tiles
		tileopts.bCreateDerivedImages = true;
		wxString filter = FSTRING_INI;
		wxFileDialog saveFile(NULL, _T(".Ini file"), _T(""), _T(""), filter, wxFD_SAVE);
		bool bResult = (saveFile.ShowModal() == wxID_OK);
		if (!bResult)
			return;
		wxString str = saveFile.GetPath();
		tileopts.fname_images = str.mb_str(wxConvUTF8);

		// Ask them how to render the image tiles
		RenderOptionsDlg dlg(this, -1, _("Rendering options"));
		dlg.SetOptions(tileopts.draw);
		dlg.m_datapaths = m_datapaths;
		if (dlg.ShowModal() != wxID_OK)
			return;
		dlg.m_opt.m_strColorMapFile = dlg.m_strColorMap.mb_str(wxConvUTF8);
		tileopts.draw = dlg.m_opt;
	}
	else
		tileopts.bCreateDerivedImages = false;

	OpenProgressDialog(_T("Writing tiles"), true);
	bool success = pEL->WriteGridOfTilePyramids(tileopts, GetView());
	GetView()->HideGridMarks();
	CloseProgressDialog();
	if (success)
		DisplayAndLog("Successfully wrote to '%s'", (const char *) tileopts.fname);
	else
		DisplayAndLog("Did not successfully write to '%s'", (const char *) tileopts.fname);
}

void MainFrame::ExportBitmap(RenderDlg &dlg)
{
	int xsize = dlg.m_iSizeX;
	int ysize = dlg.m_iSizeY;

	vtElevLayer *pEL = GetActiveElevLayer();

	ColorMap cmap;
	vtString fname = (const char *) dlg.m_strColorMap.mb_str(wxConvUTF8);
	vtString path = FindFileOnPaths(m_datapaths, "GeoTypical/" + fname);
	if (path == "")
	{
		DisplayAndLog("Couldn't load color map.");
		return;
	}
	if (!cmap.Load(path))
	{
		DisplayAndLog("Couldn't load color map.");
		return;
	}

	// Get attributes of existing layer
	DRECT area;
	vtProjection proj;
	pEL->GetExtent(area);
	pEL->GetProjection(proj);

	vtImageLayer *pOutput = NULL;
	vtBitmapBase *pBitmap = NULL;
	vtDIB dib;

	if (dlg.m_bToFile)
	{
		if (!dib.Create(xsize, ysize, 24))
		{
			DisplayAndLog("Failed to create bitmap.");
			return;
		}
		pBitmap = &dib;
	}
	else
	{
		pOutput = new vtImageLayer(area, xsize, ysize, proj);
		pBitmap = pOutput->GetBitmap();
	}

	pEL->m_pGrid->ColorDibFromElevation(pBitmap, &cmap, 8000, progress_callback);
	if (dlg.m_bShading)
	{
		// Quick and simple sunlight vector
		FPoint3 light_dir = LightDirection(vtElevLayer::m_draw.m_iCastAngle,
			vtElevLayer::m_draw.m_iCastDirection);

		if (vtElevLayer::m_draw.m_bCastShadows)
			pEL->m_pGrid->ShadowCastDib(pBitmap, light_dir, 1.0, progress_callback);
		else
			pEL->m_pGrid->ShadeDibFromElevation(pBitmap, light_dir, 1.0, true, progress_callback);
	}

	if (dlg.m_bToFile)
	{
		UpdateProgressDialog(1, _("Writing file"));
		vtString fname = (const char *) dlg.m_strToFile.mb_str(wxConvUTF8);
		bool success;
		if (dlg.m_bJPEG)
			success = dib.WriteJPEG(fname, 99, progress_callback);
		else
			success = dib.WriteTIF(fname, &area, &proj, progress_callback);
		if (success)
			DisplayAndLog("Successfully wrote to file '%s'", (const char *) fname);
		else
			DisplayAndLog("Couldn't open file for writing.");
	}
	else
	{
		AddLayerWithCheck(pOutput);
	}
#if 0
	// TEST - try coloring from water polygons
	int layers = m_Layers.GetSize();
	for (int l = 0; l < layers; l++)
	{
		vtLayer *lp = m_Layers.GetAt(l);
		if (lp->GetType() == LT_WATER)
			((vtWaterLayer*)lp)->PaintDibWithWater(&dib);
	}
#endif
}

void MainFrame::ImageExportTiles()
{
	vtImageLayer *pIL = GetActiveImageLayer();
	DRECT area;
	pIL->GetExtent(area);
	DPoint2 spacing = pIL->GetSpacing();

	TilingOptions tileopts;
	tileopts.cols = 4;
	tileopts.rows = 4;
	tileopts.lod0size = 256;
	tileopts.numlods = 3;
	tileopts.bOmitFlatTiles = true;
	tileopts.bUseTextureCompression = true;

	TileDlg dlg(this, -1, _("Tiling Options"));
	dlg.m_fEstX = spacing.x;
	dlg.m_fEstY = spacing.y;
	dlg.SetElevation(false);
	dlg.SetArea(area);
	dlg.SetTilingOptions(tileopts);
	dlg.SetView(GetView());

	int ret = dlg.ShowModal();
	GetView()->HideGridMarks();
	if (ret == wxID_CANCEL)
		return;

	dlg.GetTilingOptions(tileopts);

	OpenProgressDialog(_T("Writing tiles"), true);
	bool success = pIL->WriteGridOfTilePyramids(tileopts, GetView());
	CloseProgressDialog();
	if (success)
		DisplayAndLog("Successfully wrote to '%s'", (const char *) tileopts.fname);
	else
		DisplayAndLog("Did not successfully write to '%s'", (const char *) tileopts.fname);
}

void MainFrame::ExportAreaOptimizedElevTileset()
{
	TilingOptions tileopts;
	tileopts.cols = 4;
	tileopts.rows = 4;
	tileopts.lod0size = 256;
	tileopts.numlods = 3;

	int count, floating;
	DPoint2 spacing;
	ScanElevationLayers(count, floating, spacing);

	if (count == 0)
	{
		DisplayAndLog("Sorry, you must have some elevation grid layers\n"
					  "to perform a sampling operation on them.");
		return;
	}

	TileDlg dlg(this, -1, _("Tiling Options"));
	dlg.m_fEstX = spacing.x;
	dlg.m_fEstY = spacing.y;
	dlg.SetElevation(true);
	dlg.SetArea(m_area);
	dlg.SetTilingOptions(tileopts);
	dlg.SetView(GetView());

	if (dlg.ShowModal() != wxID_OK)
	{
		GetView()->HideGridMarks();
		return;
	}
	dlg.GetTilingOptions(tileopts);

	// If some of the input grids have floating-point elevation values, ask
	//  the user if they want their resampled output to be floating-point.
	bool bFloat = false;
	if (floating > 0)
	{
		int result = wxMessageBox(_("Sample floating-point elevation values?"),
				_("Question"), wxYES_NO | wxICON_QUESTION, this);
		if (result == wxYES)
			bFloat = true;
	}
	
	// Also write derived image tiles?
	int res = wxMessageBox(_("Also write derived image tiles?"), _("Tiled output"), wxYES_NO | wxCANCEL);
	if (res == wxCANCEL)
		return;
	if (res == wxYES)
	{
		// Ask them where to write the image tiles
		tileopts.bCreateDerivedImages = true;
		wxString filter = FSTRING_INI;
		wxFileDialog saveFile(NULL, _T(".Ini file"), _T(""), _T(""), filter, wxFD_SAVE);
		bool bResult = (saveFile.ShowModal() == wxID_OK);
		if (!bResult)
			return;
		wxString str = saveFile.GetPath();
		tileopts.fname_images = str.mb_str(wxConvUTF8);

		// Ask them how to render the image tiles
		RenderOptionsDlg dlg(this, -1, _("Rendering options"));
		dlg.SetOptions(tileopts.draw);
		dlg.m_datapaths = m_datapaths;
		if (dlg.ShowModal() != wxID_OK)
			return;
		tileopts.draw = dlg.m_opt;
	}
	else
		tileopts.bCreateDerivedImages = false;

	OpenProgressDialog(_T("Writing tiles"), true);
	bool success = SampleElevationToTilePyramids(tileopts, bFloat);
	GetView()->HideGridMarks();
	CloseProgressDialog();
	if (success)
		DisplayAndLog("Successfully wrote to '%s'", (const char *) tileopts.fname);
	else
		DisplayAndLog("Could not successfully write to '%s'", (const char *) tileopts.fname);
}

void MainFrame::ExportAreaOptimizedImageTileset()
{
	TilingOptions tileopts;
	tileopts.cols = 4;
	tileopts.rows = 4;
	tileopts.lod0size = 256;
	tileopts.numlods = 3;

	DPoint2 spacing(0, 0);
	for (unsigned int i = 0; i < m_Layers.GetSize(); i++)
	{
		vtLayer *l = m_Layers.GetAt(i);
		if (l->GetType() == LT_IMAGE)
		{
			vtImageLayer *im = (vtImageLayer *)l;
			spacing = im->GetSpacing();
		}
	}

	TileDlg dlg(this, -1, _("Tiling Options"));
	dlg.m_fEstX = spacing.x;
	dlg.m_fEstY = spacing.y;
	dlg.SetElevation(true);
	dlg.SetArea(m_area);
	dlg.SetTilingOptions(tileopts);
	dlg.SetView(GetView());

	if (dlg.ShowModal() != wxID_OK)
	{
		GetView()->HideGridMarks();
		return;
	}
	dlg.GetTilingOptions(tileopts);
	tileopts.bCreateDerivedImages = false;

	OpenProgressDialog(_T("Writing tiles"), true);
	bool success = SampleImageryToTilePyramids(tileopts);
	GetView()->HideGridMarks();
	CloseProgressDialog();
	if (success)
		DisplayAndLog("Successfully wrote to '%s'", (const char *) tileopts.fname);
	else
		DisplayAndLog("Could not successfully write to '%s'", (const char *) tileopts.fname);
}

bool MainFrame::SampleElevationToTilePyramids(const TilingOptions &opts, bool bFloat)
{
	VTLOG1("SampleElevationToTilePyramids\n");

	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// Size of each rectangular tile area
	DPoint2 tile_dim(m_area.Width()/opts.cols, m_area.Height()/opts.rows);

	// Try to create directory to hold the tiles
	vtString dirname = opts.fname;
	RemoveFileExtensions(dirname);
	if (!vtCreateDir(dirname))
		return false;

	// Gather height extents as we produce the tiles
	float minheight = 1E9, maxheight = -1E9;

	ColorMap cmap;
	vtElevLayer::SetupDefaultColors(cmap);	// defaults
	vtString dirname_image = opts.fname_images;
	RemoveFileExtensions(dirname_image);
	if (opts.bCreateDerivedImages)
	{
		if (!vtCreateDir(dirname_image))
			return false;

		// Write .ini file
		if (!WriteTilesetHeader(opts.fname_images, opts.cols, opts.rows,
			opts.lod0size, m_area, m_proj))
		{
			vtDestroyDir(dirname_image);
			return false;
		}

		vtString cmap_fname = opts.draw.m_strColorMapFile;
		vtString cmap_path = FindFileOnPaths(GetMainFrame()->m_datapaths, "GeoTypical/" + cmap_fname);
		if (cmap_path == "")
			DisplayAndLog("Couldn't find color map.");
		else
		{
			if (!cmap.Load(cmap_path))
				DisplayAndLog("Couldn't load color map.");
		}
	}

#if USE_OPENGL
	wxFrame *frame = new wxFrame;
	ImageGLCanvas *pCanvas = NULL;
	if (opts.bCreateDerivedImages)
	{
		frame->Create(this, -1, _T("Texture Compression OpenGL Context"),
			wxPoint(100,400), wxSize(280, 300), wxCAPTION | wxCLIP_CHILDREN);
		pCanvas = new ImageGLCanvas(frame);
	}
#endif

	// Form an array of pointers to the existing elevation layers
	std::vector<vtElevLayer*> elevs;
	int elev_layers = ElevLayerArray(elevs);

	int i, j, e;
	int total = opts.rows * opts.cols * opts.numlods, done = 0;
	for (j = 0; j < opts.rows; j++)
	{
		for (i = 0; i < opts.cols; i++)
		{
			// draw our progress in the main view
			GetView()->ShowGridMarks(m_area, opts.cols, opts.rows, i, j);

			DRECT tile_area;
			tile_area.left =	m_area.left + tile_dim.x * i;
			tile_area.right =	m_area.left + tile_dim.x * (i+1);
			tile_area.bottom =	m_area.bottom + tile_dim.y * j;
			tile_area.top =		m_area.bottom + tile_dim.y * (j+1);

			// Look through the elevation layers to find those which this
			//  tile can sample from.  Determine the highest resolution
			//  available for this tile.
			std::vector<vtElevationGrid*> grids;
			DPoint2 best_spacing(1E9, 1E9);
			for (e = 0; e < elev_layers; e++)
			{
				DRECT layer_extent;
				elevs[e]->GetExtent(layer_extent);
				if (tile_area.OverlapsRect(layer_extent))
				{
					// TODO: extend support here to sampling from TINs
					vtElevationGrid *grid = elevs[e]->m_pGrid;
					if (!grid)
						continue;

					grids.push_back(grid);
					DPoint2 spacing = grid->GetSpacing();
					if (spacing.x < best_spacing.x ||
						spacing.y < best_spacing.y)
						best_spacing = spacing;
				}
			}

			// increment progress count whether we omit tile or not
			done++;

			// if there is no data, omit this tile
			if (grids.size() == 0)
				continue;

			// Estimate what tile resolution is appropriate.
			//  If we can produce a lower resolution, then we can produce fewer lods.
			int total_lods = 1;
			int start_lod = opts.numlods-1;
			int base_tilesize = opts.lod0size >> start_lod;
			float width = tile_area.Width(), height = tile_area.Height();
			while (width / base_tilesize > (best_spacing.x * 1.1) &&	// 10% to avoid roundoff
				   height / base_tilesize > (best_spacing.y * 1.1) &&
				   total_lods < opts.numlods)
			{
			   base_tilesize <<= 1;
			   start_lod--;
			   total_lods++;
			}

			int col = i;
			int row = opts.rows-1-j;

			// Now sample the grids we found to the highest LOD we need
			vtElevationGrid base_lod(tile_area, base_tilesize+1, base_tilesize+1,
				bFloat, m_proj);

			bool bAllValid = true;
			bool bAllInvalid = true;
			bool bAllZero = true;
			DPoint2 p;
			int x, y;
			for (y = base_tilesize; y >= 0; y--)
			{
				p.y = m_area.bottom + (j*tile_dim.y) + ((double)y / base_tilesize * tile_dim.y);
				for (x = 0; x <= base_tilesize; x++)
				{
					p.x = m_area.left + (i*tile_dim.x) + ((double)x / base_tilesize * tile_dim.x);

					float value = GridLayerArrayValue(grids, p);
					base_lod.SetFValue(x, y, value);

					if (value == INVALID_ELEVATION)
						bAllValid = false;
					else
					{
						bAllInvalid = false;

						// Gather height extents
						if (value < minheight)
							minheight = value;
						if (value > maxheight)
							maxheight = value;
					}
					if (value != 0)
						bAllZero = false;
				}
			}

			// If there is no real data there, omit this tile
			if (bAllInvalid)
				continue;

			// Omit all-zero tiles (flat sea-level) if desired
			if (opts.bOmitFlatTiles && bAllZero)
				continue;

			if (!bAllValid)
			{
				// We don't want any gaps at all in the output tiles, because
				//  they will cause huge cliffs.
				UpdateProgressDialog(done*99/total, _("Filling gaps"));
				base_lod.FillGaps2();
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
					base_lod.ShadeDibFromElevation(&dib, light_dir, 1.0f, true);
				}

				for (int k = 0; k < 3; k++)
				{
					vtString fname = dirname_image, str;
					fname += '/';
					if (k == 0)
						str.Format("tile.%d-%d.db", col, row);
					else
						str.Format("tile.%d-%d.db%d", col, row, k);
					fname += str;

					int tilesize = base_tilesize >> k;

					MiniDatabuf output_buf;
					output_buf.xsize = tilesize;
					output_buf.ysize = tilesize;
					output_buf.zsize = 1;
					output_buf.tsteps = 1;
					output_buf.set_extents(tile_area.left, tile_area.right, tile_area.top, tile_area.bottom);

					int iUncompressedSize = tilesize * tilesize * 3;
					unsigned char *rgb_bytes = (unsigned char *) malloc(iUncompressedSize);
//					output_buf.alloc(tilesize, tilesize, 1, 1, 3);
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
#if USE_OPENGL
					// Compressed
					DoTextureCompress(rgb_bytes, output_buf, pCanvas->m_iTex);

					output_buf.savedata(fname);
					free(output_buf.data);
					output_buf.data = NULL;

					if (tilesize == 256)
						pCanvas->Refresh(false);
#else
					// Uncompressed
					// Output to a plain RGB .db file
					output_buf.type = 3;	// RGB
					output_buf.bytes = iUncompressedSize;
					output_buf.data = rgb_bytes;
					output_buf.savedata(fname);
					output_buf.data = NULL;
#endif
					// Free the uncompressed image
					free(rgb_bytes);

					// Don't bother making tiny tiles
					if (tilesize == 64)
						break;
				}
			}

			for (int k = 0; k < total_lods; k++)
			{
				int lod = start_lod + k;
				int tilesize = base_tilesize >> k;

				vtString fname = dirname, str;
				fname += '/';
				if (k == 0)
					str.Format("tile.%d-%d.db", col, row);
				else
					str.Format("tile.%d-%d.db%d", col, row, k);
				fname += str;

				// make a message for the progress dialog
				wxString msg;
				msg.Printf(_("Tile '%hs', size %dx%d"),
					(const char *)fname, tilesize, tilesize);
				UpdateProgressDialog(done*99/total, msg);

				MiniDatabuf buf;
				buf.set_extents(tile_area.left, tile_area.right, tile_area.top, tile_area.bottom);
				buf.alloc(tilesize+1, tilesize+1, 1, 1, bFloat ? 2 : 1);
				float *fdata = (float *) buf.data;
				short *sdata = (short *) buf.data;

				for (y = base_tilesize; y >= 0; y -= (1<<k))
				{
					p.y = m_area.bottom + (j*tile_dim.y) + ((double)y / base_tilesize * tile_dim.y);
					for (x = 0; x <= base_tilesize; x += (1<<k))
					{
						p.x = m_area.left + (i*tile_dim.x) + ((double)x / base_tilesize * tile_dim.x);
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
		m_area, m_proj, minheight, maxheight))
	{
		vtDestroyDir(dirname);
		return false;
	}

#if USE_OPENGL
	frame->Close();
	delete frame;
#endif

	return true;
}

bool MainFrame::SampleImageryToTilePyramids(const TilingOptions &opts)
{
	VTLOG1("SampleImageryToTilePyramids\n");

	// Gather array of existing image layers we will sample from
	int l, layers = m_Layers.GetSize(), num_image = 0;
	vtImageLayer **images = new vtImageLayer *[LayersOfType(LT_IMAGE)];
	for (l = 0; l < layers; l++)
	{
		vtLayer *lp = m_Layers.GetAt(l);
		if (lp->GetType() == LT_IMAGE)
			images[num_image++] = (vtImageLayer *)lp;
	}

	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// Size of each rectangular tile area
	DPoint2 tile_dim(m_area.Width()/opts.cols, m_area.Height()/opts.rows);

	// Try to create directory to hold the tiles
	vtString dirname = opts.fname;
	RemoveFileExtensions(dirname);
	if (!vtCreateDir(dirname))
		return false;

	// Write .ini file
	if (!WriteTilesetHeader(opts.fname, opts.cols, opts.rows, opts.lod0size,
		m_area, m_proj))
	{
		vtDestroyDir(dirname);
		return false;
	}

	wxFrame *frame = NULL;
	ImageGLCanvas *pCanvas = NULL;
	bool bCompress = false;
#if USE_OPENGL
	if (opts.bUseTextureCompression)
	{
		bCompress = true;
		wxFrame *frame = new wxFrame;
		frame->Create(this, -1, _T("Texture Compression OpenGL Context"),
			wxPoint(100,400), wxSize(280, 300), wxCAPTION | wxCLIP_CHILDREN);
		pCanvas = new ImageGLCanvas(frame);
	}
#endif

	int i, j, im;
	int total = opts.rows * opts.cols, done = 0;
	for (j = 0; j < opts.rows; j++)
	{
		for (i = 0; i < opts.cols; i++)
		{
			// draw our progress in the main view
			GetView()->ShowGridMarks(m_area, opts.cols, opts.rows, i, j);

			DRECT tile_area;
			tile_area.left =	m_area.left + tile_dim.x * i;
			tile_area.right =	m_area.left + tile_dim.x * (i+1);
			tile_area.bottom =	m_area.bottom + tile_dim.y * j;
			tile_area.top =		m_area.bottom + tile_dim.y * (j+1);

			// Look through the elevation layers to find those which this
			//  tile can sample from.  Determine the highest resolution
			//  available for this tile.
			DPoint2 best_spacing(1E9, 1E9);
			int num_source_images = 0;
			for (im = 0; im < num_image; im++)
			{
				DRECT layer_extent;
				images[im]->GetExtent(layer_extent);
				if (tile_area.OverlapsRect(layer_extent))
				{
					num_source_images++;
					DPoint2 spacing = images[im]->GetSpacing();
					if (spacing.x < best_spacing.x ||
						spacing.y < best_spacing.y)
						best_spacing = spacing;
				}
			}

			// increment progress count whether we omit tile or not
			done++;

			// if there is no data, omit this tile
			if (num_source_images == 0)
				continue;

			// Estimate what tile resolution is appropriate.
			//  If we can produce a lower resolution, then we can produce fewer lods.
			int total_lods = 1;
			int start_lod = opts.numlods-1;
			int base_tilesize = opts.lod0size >> start_lod;
			float width = tile_area.Width(), height = tile_area.Height();
			while (width / base_tilesize > (best_spacing.x * 1.1) &&	// 10% to avoid roundoff
				   height / base_tilesize > (best_spacing.y * 1.1) &&
				   total_lods < opts.numlods)
			{
			   base_tilesize <<= 1;
			   start_lod--;
			   total_lods++;
			}

			int col = i;
			int row = opts.rows-1-j;

			// Now sample the images we found to the highest LOD we need
			vtImageLayer Target(tile_area, base_tilesize, base_tilesize, m_proj);

			DPoint2 p;
			int x, y;
			RGBi pixel, rgb;
			for (y = base_tilesize-1; y >= 0; y--)
			{
				p.y = m_area.bottom + (j*tile_dim.y) + ((double)y / base_tilesize * tile_dim.y);
				for (x = 0; x < base_tilesize; x++)
				{
					p.x = m_area.left + (i*tile_dim.x) + ((double)x / base_tilesize * tile_dim.x);

					// find some data for this point
					rgb.Set(0,0,0);
					for (int im = 0; im < num_image; im++)
						if (images[im]->GetFilteredColor(p, pixel))
							rgb = pixel;

					Target.SetRGB(x, y, rgb);
				}
			}

			for (int k = 0; k < total_lods; k++)
			{
				int lod = start_lod + k;
				int tilesize = base_tilesize >> k;

				vtString fname = dirname, str;
				fname += '/';
				if (k == 0)
					str.Format("tile.%d-%d.db", col, row);
				else
					str.Format("tile.%d-%d.db%d", col, row, k);
				fname += str;

				// make a message for the progress dialog
				wxString msg;
				msg.Printf(_("Tile '%hs', size %dx%d"),
					(const char *)fname, tilesize, tilesize);
				UpdateProgressDialog(done*99/total, msg);

				unsigned char *rgb_bytes = (unsigned char *) malloc(tilesize * tilesize * 3);
				int cb = 0;	// count bytes

				for (y = base_tilesize-1; y >= 0; y -= (1<<k))
				{
					for (x = 0; x < base_tilesize; x += (1<<k))
					{
						Target.GetRGB(x, y, rgb);
						rgb_bytes[cb++] = rgb.r;
						rgb_bytes[cb++] = rgb.g;
						rgb_bytes[cb++] = rgb.b;
					}
				}
				int iUncompressedSize = cb;

				MiniDatabuf output_buf;
				output_buf.xsize = tilesize;
				output_buf.ysize = tilesize;
				output_buf.zsize = 1;
				output_buf.tsteps = 1;
				output_buf.set_extents(tile_area.left, tile_area.right, tile_area.top, tile_area.bottom);

				if (bCompress)
				{
#if USE_OPENGL
					// Compressed
					DoTextureCompress(rgb_bytes, output_buf, pCanvas->m_iTex);

					output_buf.savedata(fname);
					free(output_buf.data);
					output_buf.data = NULL;

					if (tilesize == 256)
						pCanvas->Refresh(false);
#endif
				}
				else
				{
					// Uncompressed
					// Output to a plain RGB .db file
					output_buf.type = 3;	// RGB
					output_buf.bytes = iUncompressedSize;
					output_buf.data = rgb_bytes;
					output_buf.savedata(fname);
					output_buf.data = NULL;
				}
				// Free the uncompressed image
				free(rgb_bytes);
			}
		}
	}

	if (bCompress)
	{
		frame->Close();
		delete frame;
	}
	return true;
}

void MainFrame::ExportDymaxTexture()
{
	int i, x, y, face;
	DPoint3 uvw;
	uvw.z = 0.0f;
	double u, v;
	double lat, lon;

	wxFileDialog dlg(this, _("Choose input file"), _T(""), _T(""), _T("*.bmp;*.png"));
	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	wxString choices[6];
	choices[0] = _T("128");
	choices[1] = _T("256");
	choices[2] = _T("512");
	choices[3] = _T("1024");
	choices[4] = _T("2048");
	choices[5] = _T("4096");
	wxSingleChoiceDialog dlg2(this, _("Size of each output tile?"),
		_("Query"), 6, choices);
	if (dlg2.ShowModal() == wxID_CANCEL)
		return;
	int sel = dlg2.GetSelection();
	int output_size = 1 << (7+sel);

	wxTextEntryDialog dlg3(this, _("Prefix for output filenames?"), _("Query"));
	if (dlg3.ShowModal() == wxID_CANCEL)
		return;
	wxString prefix = dlg3.GetValue();

	// TODO? change this code to use vtBitmap instead of vtDIB?

	wxProgressDialog prog(_("Processing"), _("Loading source bitmap.."), 100);
	prog.Show(TRUE);

	// read texture
	int input_x, input_y;
	vtDIB img;
	wxString path = dlg.GetPath();

	OpenProgressDialog(_T("Reading file"), false, this);
	bool success = img.Read(path.mb_str(wxConvUTF8), progress_callback);
	CloseProgressDialog();
	if (!success)
	{
		DisplayAndLog("File read failed");
		return;
	}
	input_x = img.GetWidth();
	input_y = img.GetHeight();

	DymaxIcosa ico;

	RGBi rgb;
	for (i = 0; i < 10; i++)
	{
		vtDIB out;
		out.Create(output_size, output_size, 24);

		wxString msg;
		msg.Printf(_("Creating tile %d ..."), i+1);
		prog.Update((i+1)*10, msg);

		for (x = 0; x < output_size; x++)
		{
			for (y = 0; y < output_size; y++)
			{
				if (y < output_size-1-x)
				{
					face = icosa_face_pairs[i][0];
					u = (double)x / output_size;
					v = (double)y / output_size;
				}
				else
				{
					face = icosa_face_pairs[i][1];
					u = (double)(output_size-1-x) / output_size;
					v = (double)(output_size-1-y) / output_size;
				}
				uvw.x = u;
				uvw.y = v;
				ico.FaceUVToGeo(face, uvw, lon, lat);

				int source_x = (int) (lon / PI2d * input_x);
				int source_y = (int) (lat / PId * input_y);

				img.GetPixel24(source_x, source_y, rgb);
				out.SetPixel24(x, output_size-1-y, rgb);
			}
		}
		vtString name;
		name.Format("%s_%02d%02d.png", prefix.mb_str(wxConvUTF8),
			icosa_face_pairs[i][0]+1, icosa_face_pairs[i][1]+1);
		success = out.WritePNG(name);
		if (!success)
		{
			DisplayAndLog("Failed to write file %s.", (const char *) name);
			return;
		}
	}

	DisplayAndLog("Successful.");
}

