//
// WaterLayer.cpp
//
// Copyright (c) 2001-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <memory>	// for auto_ptr

#include "WaterLayer.h"
#include "ScaledView.h"
#include "Helper.h"
#include "ogrsf_frmts.h"


//////////////////////////////////////////////////////////////////////////

vtWaterLayer::vtWaterLayer() : vtLayer(LT_WATER)
{
	SetLayerFilename(_("Untitled"));
}

vtWaterLayer::~vtWaterLayer()
{
}


bool vtWaterLayer::OnSave(bool progress_callback(int))
{
	// unimplemented
	return true;
}

bool vtWaterLayer::OnLoad()
{
	// unimplemented
	return true;
}

bool vtWaterLayer::TransformCoords(vtProjection &proj_new)
{
	// Create conversion object
	OCT *trans = CreateCoordTransform(&m_proj, &proj_new);
	if (!trans)
		return false;		// inconvertible projections

	int i, c, size, num_lines = (int)m_Lines.size();

	DPoint2 p;
	for (i = 0; i < num_lines; i++)
	{
		size = m_Lines[i].GetSize();
		for (c = 0; c < size; c++)
		{
			p = m_Lines[i].GetAt(c);
			trans->Transform(1, &p.x, &p.y);
			m_Lines[i].SetAt(c, p);
		}
	}
	delete trans;
	return true;
}

void vtWaterLayer::DrawLayer(wxDC *pDC, vtScaledView *pView)
{
	wxPen WaterPen(wxColor(0,40,160), 1, wxSOLID);
	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(WaterPen);
	wxBrush WaterBrush(wxColor(0,200,200), wxSOLID);
	pDC->SetBrush(WaterBrush);

	int num_lines = (int)m_Lines.size();
	for (int i = 0; i < num_lines; i++)
	{
		const vtWaterFeature &feat = GetFeature(i);

		pView->DrawPolyLine(pDC, feat, false);
	}
}

bool vtWaterLayer::GetExtent(DRECT &rect)
{
	int size = (int)m_Lines.size();
	if (size == 0)
		return false;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);
	for (int i = 0; i < size; i++)
		rect.GrowToContainLine(m_Lines[i]);
	return true;
}

void vtWaterLayer::AddFeature(const DLine2 &dline, bool bIsBody)
{
	m_Lines.push_back(dline);
	m_IsBody.push_back(bIsBody);
}

bool vtWaterLayer::AppendDataFrom(vtLayer *pL)
{
	// safety check
	if (pL->GetType() != LT_WATER)
		return false;

	vtWaterLayer *pFrom = (vtWaterLayer *)pL;

	int from_size = (int)pFrom->m_Lines.size();
	for (int i = 0; i < from_size; i++)
	{
		m_Lines.push_back(pFrom->m_Lines[i]);
		m_IsBody.push_back(pFrom->m_IsBody[i]);
	}

	pFrom->m_Lines.resize(0);
	pFrom->m_IsBody.resize(0);
	return true;
}

void vtWaterLayer::GetProjection(vtProjection &proj)
{
	proj = m_proj;
}

void vtWaterLayer::SetProjection(const vtProjection &proj)
{
	m_proj = proj;
}

void vtWaterLayer::Offset(const DPoint2 &p)
{
	unsigned int size = (unsigned int)m_Lines.size();
	for (unsigned int i = 0; i < size; i++)
	{
		for (unsigned int c = 0; c < m_Lines[i].GetSize(); c++)
			m_Lines[i].GetAt(c) += p;
	}
}

void vtWaterLayer::GetPropertyText(wxString &strIn)
{
	wxString str;
	str.Printf(_("Features: %d\n"), m_Lines.size());
	strIn += str;

	unsigned int i;
	int count = 0;
	for (i = 0; i < m_Lines.size(); i++)
		if (m_IsBody[i]) count++;
	str.Printf(_("Water bodies: %d\n"), count);
	strIn += str;
	str.Printf(_("Water vectors: %d\n"), m_Lines.size()-count);
	strIn += str;
}

void vtWaterLayer::PaintDibWithWater(vtDIB *dib)
{
	// TODO - need extents of DIB, create DIB subclass?
}

void vtWaterLayer::AddElementsFromDLG(vtDLGFile *pDlg)
{
	// set projection
	m_proj = pDlg->GetProjection();

	m_Lines.resize(pDlg->m_iLines);
	m_IsBody.resize(pDlg->m_iLines);

	int i, j;
	for (i = 0; i < pDlg->m_iLines; i++)
	{
		bool bSkip = true;
		int attribs = (int)pDlg->m_lines[i].m_attr.size();
		for (j = 0; j < attribs; j++)
		{
			int iMinorAttr = pDlg->m_lines[i].m_attr[j].m_iMinorAttr;
			switch (iMinorAttr)
			{
			case 200:	// major 50, coastline

			case 101:	// reservoir
			case 111:	// marsh, wetland, swamp, or bog
			case 116:	// bay, estuary, gulf, ocean, or sea
			case 412:	// stream
			case 413:	// braided stream
			case 421:	// lake or pond
				bSkip = false;
				break;
			}
		}
		if (bSkip)
			continue;

		DLGLine &input = pDlg->m_lines[i];
		DLine2 dline;
		dline.SetSize(input.m_iCoords);

		for (j = 0; j < input.m_iCoords; j++)
		{
			dline.SetAt(j, input.m_p[j]);
		}
		m_Lines[i] = dline;
		m_IsBody[i] = false;	// for now
	}
}

void vtWaterLayer::AddElementsFromSHP(const wxString &filename,
									  const vtProjection &proj)
{
	// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(filename.mb_str(wxConvUTF8));

	//Open the SHP File & Get Info from SHP:
	SHPHandle hSHP = SHPOpen(fname_local, "rb");
	if (hSHP == NULL)
		return;

	//  Get number of polys (m_iNumPolys) and type of data (nShapeType)
	int		nElem;
	int		nShapeType;
	SHPGetInfo(hSHP, &nElem, &nShapeType, NULL, NULL);

	//  Check Shape Type, Water Layer should be Poly or Line data
	if (nShapeType != SHPT_ARC && nShapeType != SHPT_POLYGON)
		return;

	m_proj = proj;	// Set projection

	// Initialize arrays
	m_Lines.resize(nElem);
	m_IsBody.resize(nElem);

	// Read Polys from SHP
	for (int i = 0; i < nElem; i++)
	{
		// Get the i-th Poly in the SHP file
		SHPObject	*psShape;
		psShape = SHPReadObject(hSHP, i);

		DLine2 dline;
		dline.SetSize(psShape->nVertices);

		// Copy each SHP Poly Coord
		for (int j = 0; j < psShape->nVertices; j++)
		{
			dline.GetAt(j).x = psShape->padfX[j];
			dline.GetAt(j).y = psShape->padfY[j];
		}
		m_Lines[i] = dline;
		m_IsBody[i] = false;

		SHPDestroyObject(psShape);
	}
	SHPClose(hSHP);
}

void vtWaterLayer::AddElementsFromOGR(OGRDataSource *pDatasource,
									 bool progress_callback(int))
{
	int i, j, feature_count, count;
	OGRLayer		*pLayer;
	OGRFeature		*pFeature;
	OGRGeometry		*pGeom;
	OGRLineString   *pLineString;
	OGRPolygon		*pPolygon;
	vtWaterFeature	feat;

	// Assume that this data source is a USGS SDTS DLG
	//
	// Iterate through the layers looking for the ones we care about
	//
	int num_layers = pDatasource->GetLayerCount();
	for (i = 0; i < num_layers; i++)
	{
		pLayer = pDatasource->GetLayer(i);
		if (!pLayer)
			continue;

		feature_count = pLayer->GetFeatureCount();
		pLayer->ResetReading();
		OGRFeatureDefn *defn = pLayer->GetLayerDefn();
		if (!defn)
			continue;

		const char *layer_name = defn->GetName();

		// Lines (streams, shoreline)
		if (!strcmp(layer_name, "LE01"))
		{
			// Get the projection (SpatialReference) from this layer
			OGRSpatialReference *pSpatialRef = pLayer->GetSpatialRef();
			if (pSpatialRef)
				m_proj.SetSpatialReference(pSpatialRef);

			// get field indices
			int index_entity = defn->GetFieldIndex("ENTITY_LABEL");

			count = 0;
			while( (pFeature = pLayer->GetNextFeature()) != NULL )
			{
				// make sure we delete the feature no matter how the loop exits
				std::auto_ptr<OGRFeature> ensure_deletion(pFeature);

				count++;
				progress_callback(count * 100 / feature_count);

				// Ignore non-entities
				if (!pFeature->IsFieldSet(index_entity))
					continue;

				// The "ENTITY_LABEL" contains the same information as the old
				// DLG classification.  First, try to use this field to guess
				// values such as number of lanes, etc.
				const char *str_entity = pFeature->GetFieldAsString(index_entity);
				int numEntity = atoi(str_entity);
				//int iMajorAttr = numEntity / 10000;
				int iMinorAttr = numEntity % 10000;

				bool bSkip = true;
				switch (iMinorAttr)
				{
				case 412:	// stream
				case 413:	// braided stream
					feat.m_bIsBody = false;
					bSkip = false;
					break;
				}
				if (bSkip)
					continue;
				pGeom = pFeature->GetGeometryRef();
				if (!pGeom) continue;
				pLineString = (OGRLineString *) pGeom;

				int num_points = pLineString->getNumPoints();
				feat.SetSize(num_points);
				for (j = 0; j < num_points; j++)
					feat.SetAt(j, DPoint2(pLineString->getX(j),
						pLineString->getY(j)));

				AddFeature(feat);
			}
		}
		// Areas (water bodies)
		if (!strcmp(layer_name, "PC01"))
		{
			// get field indices
			int index_entity = defn->GetFieldIndex("ENTITY_LABEL");

			count = 0;
			while( (pFeature = pLayer->GetNextFeature()) != NULL )
			{
				// make sure we delete the feature no matter how the loop exits
				std::auto_ptr<OGRFeature> ensure_deletion(pFeature);

				count++;
				progress_callback(count * 100 / feature_count);

				// Ignore non-entities
				if (!pFeature->IsFieldSet(index_entity))
					continue;

				// The "ENTITY_LABEL" contains the same information as the old
				// DLG classification.  First, try to use this field to guess
				// values such as number of lanes, etc.
				const char *str_entity = pFeature->GetFieldAsString(index_entity);
				int numEntity = atoi(str_entity);
				//int iMajorAttr = numEntity / 10000;
				int iMinorAttr = numEntity % 10000;

				bool bSkip = true;
				switch (iMinorAttr)
				{
				case 101:	// reservoir
				case 111:	// marsh, wetland, swamp, or bog
				case 116:	// bay, estuary, gulf, ocean, or sea
				case 412:	// stream
				case 413:	// braided stream
				case 421:	// lake or pond
					feat.m_bIsBody = false;		// for now
					bSkip = false;
					break;
				}
				if (bSkip)
					continue;
				pGeom = pFeature->GetGeometryRef();
				if (!pGeom) continue;
				pPolygon = (OGRPolygon *) pGeom;

				OGRLinearRing *ring = pPolygon->getExteriorRing();
				int num_points = ring->getNumPoints();
				feat.SetSize(num_points);
				for (j = 0; j < num_points; j++)
					feat.SetAt(j, DPoint2(ring->getX(j),
						ring->getY(j)));

				AddFeature(feat);
			}
		}
	}
}
