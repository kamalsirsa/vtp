//
// VegLayer.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/shapelib/shapefil.h"
#include "vtdata/vtLog.h"
#include "ScaledView.h"
#include "VegLayer.h"
#include "Helper.h"

#include "Frame.h"	// for access to the application's plants and biotypes

//////////////////////////////////////////////////////////////////////////

vtVegLayer::vtVegLayer() : vtLayer(LT_VEG)
{
	m_VLType = VLT_None;
}

vtVegLayer::~vtVegLayer()
{
}

bool vtVegLayer::GetExtent(DRECT &rect)
{
	int i, size;
	rect.SetRect(1E9, -1E9, -1E9, 1E9);

	if (m_VLType == VLT_Density || m_VLType == VLT_BioMap)
	{
		size = m_Poly.size();
		if (size == 0)
			return false;
		for (i = 0; i < size; i++)
			rect.GrowToContainLine(m_Poly[i]);
	}
	else if (m_VLType == VLT_Instances)
	{
		return m_Pia.GetExtent(rect);
	}

	return true;
}

void vtVegLayer::DrawPolysHiddenLines(wxDC* pDC, vtScaledView *pView)
{
	// draw each polygon in m_Poly
	bool pbNoLine[30000];

	int num_polys = m_Poly.size();
	int i, j, k;
	for (i = 0; i < num_polys; i++)
	{
		int vbuflength = 0;
		int num_points = m_Poly[i].GetSize();

		// safety check
		assert (num_points < 30000);

		for (j = 0; j < num_points; j++)
			pbNoLine[j] = false;

		bool cont = true;
		int a = num_points - 1, b = 0;

		while (cont)
		{
			pbNoLine[a] = true;
			for (k = b+1; k; k++)
			{
				if (a == k)
				{
					cont=false;
					break;
				}
				if (m_Poly[i].GetAt(a) == m_Poly[i].GetAt(k))
				{
					pbNoLine[k] = true;
					a -= 1;
					b = k+1;
					break;
				}
			}
		}
		for (int c = 0; c < num_points && c < SCREENBUF_SIZE; c++)
		{
			pView->screen(m_Poly[i].GetAt(c), g_screenbuf[vbuflength]);
			vbuflength += 1;

			if (pbNoLine[c] == true || c == num_points-1)
			{
				if (vbuflength > 1)
					pDC->DrawLines(vbuflength, g_screenbuf);
				vbuflength = 0;
			}
		}
	}
}

void vtVegLayer::DrawInstances(wxDC* pDC, vtScaledView *pView)
{
	int i, size;
	wxPoint origin;

	int m_size = pView->sdx(20);
	if (m_size > 5) m_size = 5;
	if (m_size < 1) m_size = 1;

	size = m_Pia.GetSize();
	for (i = 0; i < size; i++)
	{
		vtPlantInstance &plant = m_Pia.GetAt(i);

		pView->screen(plant.m_p, origin);

		pDC->DrawLine(origin.x-m_size, origin.y, origin.x+m_size+1, origin.y);
		pDC->DrawLine(origin.x, origin.y-m_size, origin.x, origin.y+m_size+1);

	}
}

void vtVegLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	//set the pen options
	wxPen VegPen(wxColor(0,100,0), 1, wxSOLID);  //single pixel solid green pen
	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(VegPen);

	if (m_VLType == VLT_Instances)
		DrawInstances(pDC, pView);

	if (m_VLType == VLT_BioMap || m_VLType == VLT_Density)
		DrawPolysHiddenLines(pDC, pView);
}

void vtVegLayer::GetProjection(vtProjection &proj)
{
	if (m_VLType == VLT_Density || m_VLType == VLT_BioMap)
		proj = m_proj;
	else if (m_VLType == VLT_Instances)
		m_Pia.GetProjection(proj);
}

void vtVegLayer::SetProjection(const vtProjection &proj)
{
	if (m_VLType == VLT_Density || m_VLType == VLT_BioMap)
		m_proj = proj;
	else if (m_VLType == VLT_Instances)
		m_Pia.SetProjection(proj);
}

void vtVegLayer::Offset(const DPoint2 &p)
{
	int i, size;

	if (m_VLType == VLT_Density || m_VLType == VLT_BioMap)
	{
		size = m_Poly.size();
		for (i = 0; i < size; i++)
			m_Poly[i].Add(p);
	}
	else if (m_VLType == VLT_Instances)
	{
		size = m_Pia.GetSize();
		for (i = 0; i < size; i++)
			m_Pia.GetAt(i).m_p += p;
	}
}

void vtVegLayer::GetPropertyText(wxString &str)
{
	wxString s;
	str = _T("Vegetion layer type: ");
	switch (m_VLType)
	{
	case VLT_None: str += _T("None\n"); break;
	case VLT_Density: str += _T("Density\n"); break;
	case VLT_BioMap: str += _T("BioMap\n"); break;
	case VLT_Instances:
		str += _T("Plant Instances\n");
		s.Printf(_T("Number of Instances: %d\n"), m_Pia.GetSize());
		str += s;
		break;
	}
}

bool vtVegLayer::CanBeSaved()
{
	if (m_VLType == VLT_Instances)
		return true;
	else
		return false;
}

bool vtVegLayer::OnSave()
{
	// currently we can load and save VF files (Plant Instances)
	if (m_VLType == VLT_Instances)
		return m_Pia.WriteVF(GetLayerFilename().mb_str());
	return false;
}

bool vtVegLayer::OnLoad()
{
	vtPlantList *plants = GetMainFrame()->GetPlantList();
	if (plants->NumSpecies() == 0)
	{
		wxMessageBox(_T("You must specify a species file (plant list) to use\n")
			_T("before working with vegetation files.\n"));
		return false;
	}
	// currently we can load and save VF files (Plant Instances), so if they
	//  are loading, we can assume it is from a plant instance file
	m_Pia.SetPlantList(plants);
	if (m_Pia.ReadVF(GetLayerFilename().mb_str()))
	{
		m_VLType = VLT_Instances;
		return true;
	}
	else
		return false;
}

bool vtVegLayer::ConvertProjection(vtProjection &proj_new)
{
	vtProjection proj_old;
	GetProjection(proj_old);

	// Create conversion object
	OCT *trans = OGRCreateCoordinateTransformation(&proj_old, &proj_new);
	if (!trans)
		return false;		// inconvertible projections

	unsigned int i, j;
	for (i = 0; i < m_Poly.size(); i++)
	{
		DLine2 &poly = m_Poly[i];
		unsigned int size = poly.GetSize();
		for (j = 0; j < size; j++)
		{
			DPoint2 &p = poly.GetAt(j);
			trans->Transform(1, &(p.x), &(p.y));
		}
	}
	delete trans;

	SetProjection(proj_new);
	return true;
}

bool vtVegLayer::AppendDataFrom(vtLayer *pL)
{
	vtVegLayer *pVL = (vtVegLayer *)(pL);
	if (!pVL)
		return false;

	// Must be of compatible types
	if (m_VLType != pVL->m_VLType)
		return false;

	int i, count;

	count = pVL->m_Poly.size();
	for (i = 0; i < count; i++)
		m_Poly.push_back(pVL->m_Poly[i]);

	count = pVL->m_Biotype.GetSize();
	for (i = 0; i < count; i++)
		m_Biotype.Append(pVL->m_Biotype.GetAt(i));

	count = pVL->m_Density.GetSize();
	for (i = 0; i < count; i++)
		m_Density.Append(pVL->m_Density.GetAt(i));

	// We've stolen all the polygons from the old layer, so empty it
	pVL->m_Poly.resize(0);

	return true;
}


void vtVegLayer::AddElementsFromLULC(vtLULCFile *pLULC)
{
	LULCSection *section;
	LULCPoly *poly;

	//set projections
	vtProjection proj_new;
	proj_new.SetProjectionSimple(0, -1, EPSG_DATUM_WGS84);
	SetProjection(proj_new);

	m_VLType = VLT_Density;

	// figure out the number of polygons in file 
	unsigned int size = 0;
	for (unsigned int sec = 0; sec < pLULC->NumSections(); sec++)
	{
		section = pLULC->GetSection(sec);
		size = size + section->m_iNumPolys;
	}
	m_Poly.resize(size);
	m_Density.SetSize(size);

	// get each poly from LULC file
	unsigned int i, s, p, count = 0;
	float density;
	for (s = 0; s < pLULC->NumSections(); s++)
	{
		section = pLULC->GetSection(s);
		for (p = 0; p < section->m_iNumPolys; p++)
		{
			poly = section->m_pPoly + p;

			bool wild = false;
			switch (poly->Attribute)
			{
				case 42:	// forest
					wild = true;
					density = 1.0f;
					break;
				case 32:
				case 33:
					wild = true;
					density = 0.5;
					break;
				case 22:	// orchards
					wild = false;
					// no crops for now
					break;
				default:
					density = 0.0f;
					break;
			}
			m_Density.SetAt(count, density);

			DLine2 dline;
			dline.SetSize(poly->m_iCoords);

			// get Coords of LULCpoly and store as latlon, then save in VPoly
			for (i = 0; i < dline.GetSize(); i++)
				dline.SetAt(i, poly->m_p[i]);

			m_Poly[count] = dline;
			count++;
		}
	}
}


/**
 * Extract data from a SHP/DBF file and intepret it as a vegetation layer.
 * This produces a single-valued polygonal coverage.
 *
 * 'iField' is the index of the field from which to pull the single value.
 * 'datatype' is either 0, 1, or 2 for whether the indicated field should be
 *		intepreted as a density value (double), the name of a biotype
 *		(string), or the ID of a biotype (int).
 */
void vtVegLayer::AddElementsFromSHP_Polys(const wxString2 &filename,
										  const vtProjection &proj,
										  int iField, int datatype)
{
	// Open the SHP File
	SHPHandle hSHP = SHPOpen(filename.mb_str(), "rb");
	if (hSHP == NULL)
		return;

	// Get number of polys and type of data
	int		nElem;
	int		nShapeType;
	double	adfMinBound[4], adfMaxBound[4];
	FPoint2 point;
	SHPGetInfo(hSHP, &nElem, &nShapeType, adfMinBound, adfMaxBound);

	// Check Shape Type, Veg Layer should be Poly data
	if (nShapeType != SHPT_POLYGON)
		return;

	// Open DBF File
	DBFHandle db = DBFOpen(filename.mb_str(), "rb");
	if (db == NULL)
		return;

	// Check for field of poly id, current default field in dbf is Id
	int *pnWidth = 0, *pnDecimals = 0;
	char *pszFieldName = NULL;

	DBFFieldType fieldtype = DBFGetFieldInfo(db, iField,
		pszFieldName, pnWidth, pnDecimals );

	SetProjection(proj);

	if (datatype == 0)
	{
		if (fieldtype != FTDouble)
			return;
		m_VLType = VLT_Density;
		m_Density.SetSize(nElem);
	}
	if (datatype == 1)
	{
		if (fieldtype != FTString)
			return;
	}
	if (datatype == 2)
	{
		if (fieldtype != FTInteger)
			return;
	}
	if (datatype == 1 || datatype == 2)
	{
		m_VLType = VLT_BioMap;
		m_Biotype.SetSize(nElem);
	}

	// Initialize arrays
	m_Poly.resize(nElem);

	// Read Polys from SHP into Veg Poly
	for (unsigned int i = 0; i < (unsigned int) nElem; i++)
	{
		// Read DBF Attributes per poly
		if (datatype == 0)
		{
			// density
			m_Density.SetAt(i, (float) DBFReadDoubleAttribute(db, i, iField));
		}
		if (datatype == 1)
		{
			const char *str = DBFReadStringAttribute(db, i, iField);
			// TODO
//			m_pAttrib[i] = m_BioRegions.FindBiotypeIdByName(str);
			m_Biotype.SetAt(i, -1);
		}
		if (datatype == 2)
		{
			m_Biotype.SetAt(i, DBFReadIntegerAttribute(db, i, iField));
		}

		// Get the i-th Poly in the SHP file
		SHPObject	*psShape;
		psShape = SHPReadObject(hSHP, i);

		DLine2 dline;
		dline.SetSize(psShape->nVertices);

		// Store each SHP Poly Coord in Veg Poly
		for (unsigned int j = 0; j < m_Poly[i].GetSize(); j++)
		{
			dline.GetAt(j).x = psShape->padfX[j];
			dline.GetAt(j).y = psShape->padfY[j];
		}
		// Store the number of coordinate point in the i-th poly
		m_Poly[i] = dline;

		SHPDestroyObject(psShape);
	}

	DBFClose(db);
	SHPClose(hSHP);
}

/**
 * Extract point data from a SHP/DBF file and intepret it as a vegetation
 * layer.  This produces a set of vegetation instances.
 *
 * The 'opt' parameter contains a description of how the fields in the
 * imported file are to be interpreted.
 */
bool vtVegLayer::AddElementsFromSHP_Points(const wxString2 &filename,
										   const vtProjection &proj,
										   VegPointOptions &opt)
{
	// We will be creating plant instances
	m_VLType = VLT_Instances;

	vtPlantList *pPlantList = GetMainFrame()->GetPlantList();
	vtBioRegion *pBioRegion = GetMainFrame()->GetBioRegion();
	m_Pia.SetPlantList(pPlantList);

	// Open the SHP File
	SHPHandle hSHP = SHPOpen(filename.mb_str(), "rb");
	if (hSHP == NULL)
		return false;

	// Get number of points and type of data
	int		nElem;
	int		nShapeType;
	double	adfMinBound[4], adfMaxBound[4];
	FPoint2 point;
	SHPGetInfo(hSHP, &nElem, &nShapeType, adfMinBound, adfMaxBound);

	// Check Shape Type, Veg Layer should be Point data
	if (nShapeType != SHPT_POINT)
		return false;

	// Open DBF File
	DBFHandle db = DBFOpen(filename.mb_str(), "rb");
	if (db == NULL)
		return false;

	// Confirm that the field types are correct
	int *pnWidth = 0, *pnDecimals = 0;
	char pszFieldName[80];
	DBFFieldType fieldtype;

	if (!opt.bFixedSpecies)
	{
		// we're going to get species info from a field
		fieldtype = DBFGetFieldInfo(db, opt.iSpeciesFieldIndex, pszFieldName,
			pnWidth, pnDecimals);
		if (opt.iInterpretSpeciesField == 0 || opt.iInterpretSpeciesField == 3)
		{
			if (fieldtype != FTInteger)
			{
				DisplayAndLog("Can't import field '%hs' as an integer, it is type %d.",
					pszFieldName, fieldtype);
				return false;
			}
		}
		else
		{
			if (fieldtype != FTString)
			{
				DisplayAndLog("Can't import field '%hs' as a string, it is type %d.",
					pszFieldName, fieldtype);
				return false;
			}
		}
	}

	// Set projection
	SetProjection(proj);

	// Initialize arrays
	m_Pia.SetMaxSize(nElem);

	// Read Points from SHP and intepret fields
	SHPObject *psShape;
	vtPlantInstance pi;
	const char *str;
	int biotype;
	vtBioType *pBioType;

	int unfound = 0;

	for (int i = 0; i < nElem; i++)
	{
		// Get the i-th Point in the SHP file
		psShape = SHPReadObject(hSHP, i);
		pi.m_p.x = psShape->padfX[0];
		pi.m_p.y = psShape->padfY[0];
		SHPDestroyObject(psShape);

		// Read DBF Attributes per point
		pi.species_id = -1;
		if (opt.bFixedSpecies)
			pi.species_id = pPlantList->GetSpeciesIdByName(opt.strFixedSpeciesName.mb_str());
		else
		{
			switch (opt.iInterpretSpeciesField)
			{
			case 0:
				pi.species_id = DBFReadIntegerAttribute(db, i, opt.iSpeciesFieldIndex);
				break;
			case 1:
				str = DBFReadStringAttribute(db, i, opt.iSpeciesFieldIndex);
				pi.species_id = pPlantList->GetSpeciesIdByName(str);
				if (pi.species_id == -1)
					unfound++;
				break;
			case 2:
				str = DBFReadStringAttribute(db, i, opt.iSpeciesFieldIndex);
				pi.species_id = pPlantList->GetSpeciesIdByCommonName(str);
				if (pi.species_id == -1)
					unfound++;
				break;
			case 3:
				biotype = DBFReadIntegerAttribute(db, i, opt.iSpeciesFieldIndex);
				pBioType = pBioRegion->GetBioType(biotype);
				if (pBioType)
					pi.species_id = pBioType->GetWeightedRandomPlant();
				break;
			case 4:
				str = DBFReadStringAttribute(db, i, opt.iSpeciesFieldIndex);
				biotype = pBioRegion->FindBiotypeIdByName(str);
				pBioType = pBioRegion->GetBioType(biotype);
				if (pBioType)
					pi.species_id = pBioType->GetWeightedRandomPlant();
				break;
			}
		}

		// Make sure we have a valid species
		if (pi.species_id == -1)
			continue;
		vtPlantSpecies *pSpecies = pPlantList->GetSpecies(pi.species_id);
		if (!pSpecies)
			continue;

		// Set height
		if (opt.bHeightRandom)
			pi.size = random(pSpecies->GetMaxHeight());
		else
			pi.size = (float) DBFReadDoubleAttribute(db, i, opt.iHeightFieldIndex);

		// If we get here, there is a valid plant to append
		m_Pia.Append(pi);
	}
	if (unfound)
		DisplayAndLog("Couldn't find species for %d out of %d instances.", unfound, nElem);
	else
		DisplayAndLog("Imported %d plant instances.", nElem);

	DBFClose(db);
	SHPClose(hSHP);
	return true;
}

float vtVegLayer::FindDensity(const DPoint2 &p)
{
	int poly = m_Poly.FindPoly(p);
	if (poly != -1)
		return m_Density.GetAt(poly);
	else
		return -1;
}

int vtVegLayer::FindBiotype(const DPoint2 &p)
{
	int poly = m_Poly.FindPoly(p);
	if (poly != -1)
		return m_Biotype.GetAt(poly);
	else
		return -1;
}

bool vtVegLayer::ExportToSHP(const char *fname)
{
	if (m_VLType != VLT_Instances)
		return false;

	return m_Pia.WriteSHP(fname);
}

