//
// WaterLayer.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "WaterLayer.h"
#include "ScaledView.h"
#include "Helper.h"
#include "ogrsf_frmts.h"

bool vtWaterLayer::m_bFill = false;

//////////////////////////////////////////////////////////////////////////

vtWaterLayer::vtWaterLayer() : vtLayer(LT_WATER)
{
	m_strFilename = "Untitled.hyd";
}

vtWaterLayer::~vtWaterLayer()
{
}


bool vtWaterLayer::OnSave()
{
	// unimplemented
	return true;
}

bool vtWaterLayer::OnLoad()
{
	// unimplemented
	return true;
}

bool vtWaterLayer::ConvertProjection(vtProjection &proj_new)
{
	// Create conversion object
	OCT *trans = OGRCreateCoordinateTransformation(&m_proj, &proj_new);
	if (!trans)
		return false;		// inconvertible projections

	int i, c, size, num_lines = m_Lines.GetSize();

	DPoint2 p;
	for (i = 0; i < num_lines; i++)
	{
		size = m_Lines[i]->GetSize();
		for (c = 0; c < size; c++)
		{
			p = m_Lines[i]->GetAt(c);
			trans->Transform(1, &p.x, &p.y);
			m_Lines[i]->SetAt(c, p);
		}
	}
	delete trans;
	return true;
}

void vtWaterLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	wxPen WaterPen(wxColor(0,40,160), 1, wxSOLID);
	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(WaterPen);
	wxBrush WaterBrush(wxColor(0,200,200), wxSOLID);
	pDC->SetBrush(WaterBrush);

	vtWaterFeature *feat;
	int num_lines = m_Lines.GetSize();
	for (int i = 0; i < num_lines; i++)
	{
		feat = GetFeature(i);
		int c;
		int size = feat->GetSize();
		for (c = 0; c < size && c < SCREENBUF_SIZE; c++)
			pView->screen(feat->GetAt(c), g_screenbuf[c]);

		if (m_bFill && feat->m_bIsBody)
			pDC->DrawPolygon(c, g_screenbuf);
		else
			pDC->DrawLines(c, g_screenbuf);
	}
}

bool vtWaterLayer::GetExtent(DRECT &rect)
{
	int size = m_Lines.GetSize();
	if (size == 0)
		return false;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);
	for (int i = 0; i < size; i++)
		rect.GrowToContainLine(*m_Lines[i]);
	return true;
}

void vtWaterLayer::AddFeature(vtWaterFeature *pFeat)
{
	m_Lines.Append(pFeat);
}

bool vtWaterLayer::AppendDataFrom(vtLayer *pL)
{
	// safety check
	if (pL->GetType() != LT_WATER)
		return false;

	vtWaterLayer *pFrom = (vtWaterLayer *)pL;

	int from_size = pFrom->m_Lines.GetSize();
	for (int i = 0; i < from_size; i++)
		m_Lines.Append(pFrom->m_Lines[i]);

	pFrom->m_Lines.SetSize(0);
	return true;
}

void vtWaterLayer::GetProjection(vtProjection &proj)
{
	proj = m_proj;
}

void vtWaterLayer::SetProjection(vtProjection &proj)
{
	m_proj = proj;
}

void vtWaterLayer::Offset(const DPoint2 &p)
{
	int size = m_Lines.GetSize();
	for (int i = 0; i < size; i++)
	{
		for (int c = 0; c < m_Lines[i]->GetSize(); c++)
			m_Lines[i]->GetAt(c) += p;
	}
}

void vtWaterLayer::PaintDibWithWater(vtDIB *dib)
{
	// TODO - need extents of DIB, create DIB subclass?
}

void vtWaterLayer::AddElementsFromDLG(vtDLGFile *pDlg)
{
	// set projection
	m_proj = pDlg->GetProjection();

	m_Lines.SetSize(pDlg->m_iLines);
	for (int i = 0; i < pDlg->m_iLines; i++)
	{
		DLGLine *pDLine = pDlg->m_lines + i;
		DLine2 *new_line = new DLine2();
		new_line->SetSize(pDLine->m_iCoords);

		for (int j = 0; j < pDLine->m_iCoords; j++)
		{
			new_line->SetAt(j, pDLine->m_p[j]);
		}
		m_Lines.SetAt(i, new_line);
	}
}

void vtWaterLayer::AddElementsFromSHP(const char *filename, vtProjection &proj)
{
	//Open the SHP File & Get Info from SHP:
	SHPHandle hSHP = SHPOpen(filename, "rb");
	if (hSHP == NULL)
		return;

	//  Get number of polys (m_iNumPolys) and type of data (nShapeType)
	int		nElem;
	int		nShapeType;
	double	adfMinBound[4], adfMaxBound[4];
	FPoint2 point;
	SHPGetInfo(hSHP, &nElem, &nShapeType, adfMinBound, adfMaxBound);

	//  Check Shape Type, Water Layer should be Poly or Line data
	if (nShapeType != SHPT_ARC && nShapeType != SHPT_POLYGON)
		return;

	m_proj = proj;	// Set projection

	// Initialize arrays
	m_Lines.SetSize(nElem);

	// Read Polys from SHP
	for (int i = 0; i < nElem; i++)
	{
		// Get the i-th Poly in the SHP file
		SHPObject	*psShape;
		psShape = SHPReadObject(hSHP, i);

		DLine2 *new_poly = new DLine2();
		new_poly->SetSize(psShape->nVertices);

		// Copy each SHP Poly Coord
		for (int j = 0; j < psShape->nVertices; j++)
		{
			new_poly->GetAt(j).x = psShape->padfX[j];
			new_poly->GetAt(j).y = psShape->padfY[j];
		}
		m_Lines.SetAt(i, new_poly);

		SHPDestroyObject(psShape);
	}
	SHPClose(hSHP);
}

void vtWaterLayer::AddElementsFromOGR(OGRDataSource *pDatasource,
									 void progress_callback(int))
{
	int i, j, feature_count, count;
	OGRLayer		*pLayer;
	OGRFeature		*pFeature;
	OGRGeometry		*pGeom;
//	OGRPoint		*pPoint;
	OGRLineString   *pLineString;
	OGRPolygon		*pPolygon;
	vtWaterFeature	*pFeat;

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
				int iMajorAttr = numEntity / 10000;
				int iMinorAttr = numEntity % 10000;

				pFeat = NULL;
				switch (iMinorAttr)
				{
				case 412:	// stream
				case 413:	// braided stream
					pFeat = new vtWaterFeature;
					pFeat->m_bIsBody = false;
					break;
				}
				if (!pFeat)
					continue;
				pGeom = pFeature->GetGeometryRef();
				if (!pGeom) continue;
				pLineString = (OGRLineString *) pGeom;

				int num_points = pLineString->getNumPoints();
				pFeat->SetSize(num_points);
				for (j = 0; j < num_points; j++)
					pFeat->SetAt(j, DPoint2(pLineString->getX(j),
						pLineString->getY(j)));

				AddFeature(pFeat);
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
				int iMajorAttr = numEntity / 10000;
				int iMinorAttr = numEntity % 10000;

				pFeat = NULL;
				switch (iMinorAttr)
				{
				case 101:	// reservoir
				case 111:	// marsh, wetland, swamp, or bog
				case 116:	// bay, estuary, gulf, ocean, or sea
				case 412:	// stream
				case 413:	// braided stream
				case 421:	// lake or pond
					pFeat = new vtWaterFeature;
					pFeat->m_bIsBody = true;
					break;
				}
				if (!pFeat)
					continue;
				pGeom = pFeature->GetGeometryRef();
				if (!pGeom) continue;
				pPolygon = (OGRPolygon *) pGeom;

				OGRLinearRing *ring = pPolygon->getExteriorRing();
				int num_points = ring->getNumPoints();
				pFeat->SetSize(num_points);
				for (j = 0; j < num_points; j++)
					pFeat->SetAt(j, DPoint2(ring->getX(j),
						ring->getY(j)));

				AddFeature(pFeat);
			}
		}
	}
}
