//
// VegLayer.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
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

vtVegLayer::vtVegLayer() : vtRawLayer()
{
	m_type = LT_VEG;
	m_VLType = VLT_None;

	// default dark green
	m_DrawStyle.m_LineColor.Set(0,128,0);
}

vtVegLayer::~vtVegLayer()
{
}

bool vtVegLayer::GetExtent(DRECT &rect)
{
	if (m_VLType == VLT_Density || m_VLType == VLT_BioMap)
		return vtRawLayer::GetExtent(rect);
	else if (m_VLType == VLT_Instances)
		return m_Pia.GetExtent(rect);
	else
		return false;
}

void vtVegLayer::DrawInstances(wxDC *pDC, vtScaledView *pView)
{
	wxPoint origin;

	int m_size = pView->sdx(20);
	if (m_size > 5) m_size = 5;
	if (m_size < 1) m_size = 1;

	unsigned int size = m_Pia.GetSize();
	for (unsigned int i = 0; i < size; i++)
	{
		vtPlantInstance &plant = m_Pia.GetAt(i);

		pView->screen(plant.m_p, origin);

		pDC->DrawLine(origin.x-m_size, origin.y, origin.x+m_size+1, origin.y);
		pDC->DrawLine(origin.x, origin.y-m_size, origin.x, origin.y+m_size+1);
	}
}

void vtVegLayer::DrawLayer(wxDC *pDC, vtScaledView *pView)
{
	if (m_VLType == VLT_Instances)
		DrawInstances(pDC, pView);
	else
		vtRawLayer::DrawLayer(pDC, pView);
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
	else
		return vtRawLayer::OnSave();
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

bool vtVegLayer::AppendDataFrom(vtLayer *pL)
{
	if (pL->GetType() != LT_VEG)
		return false;

	vtVegLayer *pVL = (vtVegLayer *)pL;

	if (m_VLType == VLT_Instances)
	{
		m_Pia.AppendFrom(pVL->m_Pia);
		return true;
	}
	else
	{
		// Must be of compatible types
		if (m_VLType != pVL->m_VLType)
			return false;

		return vtRawLayer::AppendDataFrom(pL);
	}
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

	vtFeatureSetPolygon *pSet = new vtFeatureSetPolygon();
	m_pSet = pSet;

	// Create density field
	m_field_density = pSet->AddField("Density", FT_Float);
	pSet->SetNumEntities(size);

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
			pSet->SetValue(count, m_field_density, density);

			DLine2 dline;
			dline.SetSize(poly->m_iCoords);

			// get Coords of LULCpoly and store as latlon, then save in VPoly
			for (i = 0; i < dline.GetSize(); i++)
				dline.SetAt(i, poly->m_p[i]);

			DPolygon2 dpoly;
			dpoly.push_back(dline);

			pSet->SetPolygon(count, dpoly);
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
bool vtVegLayer::AddElementsFromSHP_Polys(const wxString2 &filename,
										  const vtProjection &proj,
										  int iField, VegImportFieldType datatype)
{
	// Open the SHP File
	SHPHandle hSHP = SHPOpen(filename.mb_str(), "rb");
	if (hSHP == NULL)
		return false;

	// Get number of polys and type of data
	int		nElem;
	int		nShapeType;
	SHPGetInfo(hSHP, &nElem, &nShapeType, NULL, NULL);

	// Check Shape Type, Veg Layer should be Poly data
	if (nShapeType != SHPT_POLYGON)
		return false;

	// Open DBF File
	DBFHandle db = DBFOpen(filename.mb_str(), "rb");
	if (db == NULL)
		return false;

	// Check for field of poly id, current default field in dbf is Id
	int *pnWidth = 0, *pnDecimals = 0;
	char *pszFieldName = NULL;

	DBFFieldType fieldtype = DBFGetFieldInfo(db, iField,
		pszFieldName, pnWidth, pnDecimals );

	if (datatype == VIFT_Density)
	{
		if (fieldtype != FTDouble)
		{
			VTLOG(" Expected the DBF field '%s' to be of type 'Double', but found '%s' instead.\n",
				pszFieldName, DescribeFieldType(fieldtype));
			return false;
		}
	}
	if (datatype == VIFT_BiotypeName)
	{
		if (fieldtype != FTString)
		{
			VTLOG(" Expected the DBF field '%s' to be of type 'String', but found '%s' instead.\n",
				pszFieldName, DescribeFieldType(fieldtype));
			return false;
		}
	}
	if (datatype == VIFT_BiotypeID)
	{
		if (fieldtype != FTInteger)
		{
			VTLOG(" Expected the DBF field '%s' to be of type 'Integer', but found '%s' instead.\n",
				pszFieldName, DescribeFieldType(fieldtype));
			return false;
		}
	}

	// OK, ready to allocate our featureset
	vtFeatureSetPolygon *pSet = new vtFeatureSetPolygon();
	m_pSet = pSet;

	if (datatype == VIFT_Density)
	{
		m_VLType = VLT_Density;
		m_field_density = pSet->AddField("Density", FT_Float);
	}
	if (datatype == VIFT_BiotypeName || datatype == VIFT_BiotypeID)
	{
		m_VLType = VLT_BioMap;
		m_field_biotype = pSet->AddField("Biotype", FT_Integer);
	}

	SetProjection(proj);

	// Read Polys from SHP into Veg Poly
	pSet->LoadGeomFromSHP(hSHP);
	SHPClose(hSHP);

	// Read fields
	for (unsigned int i = 0; i < (unsigned int) nElem; i++)
	{
		int record = pSet->AddRecord();
		// Read DBF Attributes per poly
		if (datatype == VIFT_Density)
		{
			// density
			pSet->SetValue(record, m_field_density, (float) DBFReadDoubleAttribute(db, i, iField));
		}
		if (datatype == VIFT_BiotypeName)
		{
			const char *str = DBFReadStringAttribute(db, i, iField);
			// TODO
//			m_pAttrib[i] = m_BioRegions.FindBiotypeIdByName(str);
			pSet->SetValue(record, m_field_biotype, -1);
		}
		if (datatype == VIFT_BiotypeID)
		{
			pSet->SetValue(record, m_field_biotype, DBFReadIntegerAttribute(db, i, iField));
		}
	}
	DBFClose(db);
	return true;
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
	SHPGetInfo(hSHP, &nElem, &nShapeType, NULL, NULL);

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
	if (m_VLType != VLT_Density)
		return -1;

	int poly = ((vtFeatureSetPolygon*)m_pSet)->FindSimplePolygon(p);
	if (poly != -1)
		return m_pSet->GetFloatValue(poly, m_field_density);
	else
		return -1;
}

int vtVegLayer::FindBiotype(const DPoint2 &p)
{
	if (m_VLType != VLT_BioMap)
		return -1;

	int poly = ((vtFeatureSetPolygon*)m_pSet)->FindSimplePolygon(p);
	if (poly != -1)
		return m_pSet->GetIntegerValue(poly, m_field_biotype);
	else
		return -1;
}

bool vtVegLayer::ExportToSHP(const char *fname)
{
	if (m_VLType != VLT_Instances)
		return false;

	return m_Pia.WriteSHP(fname);
}

