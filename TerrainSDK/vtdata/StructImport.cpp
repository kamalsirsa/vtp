//
// Import methods for the vtStructureArray class.
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "shapelib/shapefil.h"
#include "ogrsf_frmts.h"
#include "StructArray.h"
#include "Building.h"
#include "Fence.h"
#include "PolyChecker.h"
#include "vtLog.h"

//
// Helper: find the index of a field in a DBF file, given the name of the field.
// Returns -1 if not found.
//
int FindDBField(DBFHandle db, const char *field_name)
{
	int count = DBFGetFieldCount(db);
	for (int i = 0; i < count; i++)
	{
		int pnWidth, pnDecimals;
		char pszFieldName[80];
		DBFFieldType fieldtype = DBFGetFieldInfo(db, i,
			pszFieldName, &pnWidth, &pnDecimals );
		if (!stricmp(field_name, pszFieldName))
			return i;
	}
	return -1;
}

#define BCFVERSION_SUPPORTED	1.1f

bool vtStructureArray::ReadBCF(const char* pathname)
{
	FILE* fp;
	if ( (fp = fopen(pathname, "rb")) == NULL )
		return false;

	char buf[4];
	fread(buf, 3, 1, fp);
	if (strncmp(buf, "bcf", 3))
	{
		// not current bcf, try reading old format
		rewind(fp);
		return ReadBCF_Old(fp);
	}

	float version;
	fscanf(fp, "%f\n", &version);

	if (version < BCFVERSION_SUPPORTED)
	{
		// too old, unsupported version
		fclose(fp);
		return false;
	}

	int zone = 1;
	if (version == 1.2f)
	{
		fscanf(fp, "utm_zone %d\n", &zone);
	}
	m_proj.SetUTMZone(zone);

	int i, j, count;
	DPoint2 p;
	int points;
	char key[80];
	RGBi color;
	float fRotation;

	fscanf(fp, "buildings %d\n", &count);
	for (i = 0; i < count; i++)	//for each building
	{
		vtBuilding *bld = NewBuilding();

		int type;
		fscanf(fp, "type %d\n", &type);

		int stories = 1;
		DPoint2 loc;
		fRotation = 0.0f;
		while (1)
		{
			long start = ftell(fp);

			int result = fscanf(fp, "%s ", key);
			if (result == -1)
				break;

			if (!strcmp(key, "type"))
			{
				fseek(fp, start, SEEK_SET);
				break;
			}
			if (!strcmp(key, "loc"))
			{
				fscanf(fp, "%lf %lf\n", &loc.x, &loc.y);
				bld->SetRectangle(loc, 10, 10);
			}
			else if (!strcmp(key, "rot"))
			{
				fscanf(fp, "%f\n", &fRotation);
			}
			else if (!strcmp(key, "stories"))
			{
				fscanf(fp, "%d\n", &stories);

				if (stories < 1 || stories > 10) stories = 1;	// TEMPORARY FIX!
				bld->SetStories(stories);
			}
			else if (!strcmp(key, "color"))
			{
				fscanf(fp, "%hd %hd %hd\n", &color.r, &color.g, &color.b);
				bld->SetColor(BLD_BASIC, color);
			}
			else if (!strcmp(key, "color_roof"))
			{
				fscanf(fp, "%hd %hd %hd\n", &color.r, &color.g, &color.b);
				bld->SetColor(BLD_ROOF, color);
			}
			else if (!strcmp(key, "size"))
			{
				float w, d;
				fscanf(fp, "%f %f\n", &w, &d);
				bld->SetRectangle(loc, w, d, fRotation);
			}
			else if (!strcmp(key, "radius"))
			{
				float rad;
				fscanf(fp, "%f\n", &rad);
				bld->SetCircle(loc, rad);
			}
			else if (!strcmp(key, "footprint"))
			{
				DLine2 dl;
				fscanf(fp, "%d", &points);
				dl.SetSize(points);

				for (j = 0; j < points; j++)
				{
					fscanf(fp, " %lf %lf", &p.x, &p.y);
					dl.SetAt(j, p);
				}
				fscanf(fp, "\n");
				bld->SetFootprint(0, dl);
			}
			else if (!strcmp(key, "trim"))
			{
				int trim;
				fscanf(fp, "%d\n", &trim);
			}
			else if (!strcmp(key, "roof_type"))
			{
				int rt;
				fscanf(fp, "%d\n", &rt);
				bld->SetRoofType((RoofType) rt);
			}
		}
		Append(bld);
	}
	fclose(fp);
	return true;
}

bool vtStructureArray::ReadBCF_Old(FILE *fp)
{
	int ncoords;
	int num = fscanf(fp, "%d\n", &ncoords);
	if (num != 1)
		return false;

	DPoint2 point;
	for (int i = 0; i < ncoords; i++)
	{
		fscanf(fp, "%lf %lf\n", &point.x, &point.y);
		vtBuilding *bld = NewBuilding();
		bld->SetRectangle(point, 10, 10);
		Append(bld);
	}

	fclose(fp);
	return true;
}

/**
 * Import structure information from a Shapefile.
 *
 * \param pathname A resolvable filename of a Shapefile (.shp)
 * \param opt	Options which specify how to import structures from the file.
 * \param progress_callback If supplied, this function will be called back
 *				with a value of 0 to 100 as the operation progresses.
 *
 * \return True if successful.
 */
bool vtStructureArray::ReadSHP(const char *pathname, StructImportOptions &opt,
							   void progress_callback(int))
{
	SHPHandle hSHP = SHPOpen(pathname, "rb");
	if (hSHP == NULL)
		return false;

	int		nEntities, nShapeType;
	DPoint2 point;
	DLine2	line;
	int		i, j, k;
	int		field_height = -1;
	int		field_filename = -1;
	int		field_itemname = -1;
	int		field_scale = -1;
	int		field_rotation = -1;

	SHPGetInfo(hSHP, &nEntities, &nShapeType, NULL, NULL);

	// Open DBF File & Get DBF Info:
	DBFHandle db = DBFOpen(pathname, "rb");

	// Make sure that entities are of the expected type
	if (opt.type == ST_BUILDING)
	{
		if (nShapeType != SHPT_POINT && nShapeType != SHPT_POLYGON &&
			nShapeType != SHPT_ARC)
			return false;
		// Check for field with number of stories
		if (db != NULL)
			field_height = FindDBField(db, (const char *)opt.m_strFieldNameHeight);
	}
	if (opt.type == ST_INSTANCE)
	{
		if (nShapeType != SHPT_POINT)
			return false;
		if (db == NULL)
			return false;

		field_filename = FindDBField(db, (const char *)opt.m_strFieldNameFile);
		field_itemname = FindDBField(db, "itemname");
		if (field_filename == -1 && field_itemname == -1)
			return false;

		field_scale = FindDBField(db, "scale");
		field_rotation = FindDBField(db, "rotation");
	}
	if (opt.type == ST_LINEAR)
	{
		if (nShapeType != SHPT_ARC && nShapeType != SHPT_POLYGON)
			return false;
	}

	for (i = 0; i < nEntities; i++)
	{
		if (progress_callback != NULL && (i & 0xff) == 0)
			progress_callback(i * 100 / nEntities);

		SHPObject *psShape = SHPReadObject(hSHP, i);

		if (opt.bInsideOnly)
		{
			// do exclusion of shapes outside the indicated extents
			if (psShape->dfXMax < opt.rect.left ||
				psShape->dfXMin > opt.rect.right ||
				psShape->dfYMax < opt.rect.bottom ||
				psShape->dfYMin > opt.rect.top)
			{
				SHPDestroyObject(psShape);
				continue;
			}
		}

		int num_points = psShape->nVertices;

		if (opt.type == ST_BUILDING)
		{
			vtBuilding *bld = NewBuilding();
			if (nShapeType == SHPT_POINT)
			{
				point.x = psShape->padfX[0];
				point.y = psShape->padfY[0];
				bld->SetRectangle(point, 10, 10);	// default size
			}
			if (nShapeType == SHPT_POLYGON || nShapeType == SHPT_POLYGONZ ||
				nShapeType == SHPT_ARC)
			{
				if (nShapeType == SHPT_POLYGON || nShapeType == SHPT_POLYGONZ)
				{
					// for some reason, for SHPT_POLYGON, Shapelib duplicates
					// the first point, so we need to ignore it
					num_points--;
					// Quick fix for multiple rings
					if (psShape->nParts > 1)
						num_points = psShape->panPartStart[1] - 1;
				}
				if (nShapeType == SHPT_ARC)
				{
					// the ARC type is different; Shapelib doesn't duplicate
					// the first point, but since it is closed, we still need
					// to ignore it the first point
					num_points--;
				}
				// must have at least 3 points in a footprint
				if (num_points < 3)
				{
					SHPDestroyObject(psShape);
					continue;
				}
				DLine2 foot;
				foot.SetSize(num_points);
				for (j = 0; j < num_points; j++)
				{
					if (opt.bFlip)
						k = num_points - 1 - j;
					else
						k = j;
					foot.SetAt(j, DPoint2(psShape->padfX[k], psShape->padfY[k]));
				}
				bld->SetFootprint(0, foot);
				// Give it a flat roof with the same footprint
				bld->SetFootprint(1, foot);
				bld->SetRoofType(ROOF_FLAT);
			}

			// attempt to get height from the DBF
			int stories;
			if (field_height != -1)
			{
				double height = DBFReadDoubleAttribute(db, i, field_height);
				switch (opt.m_HeightType)
				{
				case StructImportOptions::STORIES:
					stories = (int) height;
					if (stories >= 1)
						bld->SetStories(stories);
					break;
				case StructImportOptions::FEET:
					height = height * 0.3048;
				case StructImportOptions::METERS:
					stories = (int) (height / 3.2);
					if (stories < 1)
						stories = 1;
					bld->SetStories((int) stories);
					bld->GetLevel(0)->m_fStoryHeight = (float) (height / stories);
					break;
				}
			}
			Append(bld);
		}
		if (opt.type == ST_INSTANCE)
		{
			vtStructInstance *inst = NewInstance();
			inst->m_p.x = psShape->padfX[0];
			inst->m_p.y = psShape->padfY[0];
			// attempt to get properties from the DBF
			const char *string;
			vtTag tag;
			if (field_filename != -1)
			{
				string = DBFReadStringAttribute(db, i, field_filename);
				tag.name = "filename";
				tag.value = string;
				inst->AddTag(tag);
			}
			if (field_itemname != -1)
			{
				string = DBFReadStringAttribute(db, i, field_itemname);
				tag.name = "itemname";
				tag.value = string;
				inst->AddTag(tag);
			}
			if (field_scale != -1)
			{
				double scale = DBFReadDoubleAttribute(db, i, field_scale);
				if (scale != 1.0)
				{
					tag.name = "scale";
					tag.value.Format("%lf", scale);
					inst->AddTag(tag);
				}
			}
			if (field_rotation != -1)
			{
				double rotation = DBFReadDoubleAttribute(db, i, field_rotation);
				inst->m_fRotation = (float) (rotation / 180.0 * PId);
			}
			Append(inst);
		}
		if (opt.type == ST_LINEAR)
		{
			vtFence *fen = NewFence();
			for (j = 0; j < num_points; j++)
			{
				point.x = psShape->padfX[j];
				point.y = psShape->padfY[j];
				fen->AddPoint(point);
			}
			Append(fen);
		}
		SHPDestroyObject(psShape);
	}
	DBFClose(db);
	SHPClose(hSHP);
	return true;
}

void vtStructureArray::AddElementsFromOGR(OGRDataSource *pDatasource,
		StructImportOptions &opt, void progress_callback(int))
{
	if (opt.m_strLayerName == "")
		AddElementsFromOGR_SDTS(pDatasource, progress_callback);
	else
		AddElementsFromOGR_RAW(pDatasource, opt, progress_callback);
}

void vtStructureArray::AddElementsFromOGR_SDTS(OGRDataSource *pDatasource,
		void progress_callback(int))
{
	int i, j, feature_count, count;
	OGRLayer		*pLayer;
	OGRFeature		*pFeature;
	OGRGeometry		*pGeom;
	OGRPoint		*pPoint;
	OGRPolygon		*pPolygon;
	vtBuilding		*pBld;
	DPoint2			point;
	DLine2 foot;
	OGRLinearRing *ring;
	int num_points;
	OGRFeatureDefn *pLayerDefn;

	int num_layers = pDatasource->GetLayerCount();

	//
	// Iterate through the layers looking
	// Test for known USGS SDTS DLG layer names
	// Treat unknown ones as containing feature polygons
	//
	for (i = 0; i < num_layers; i++)
	{
		pLayer = pDatasource->GetLayer(i);
		if (!pLayer)
			continue;

		feature_count = pLayer->GetFeatureCount();
  		pLayer->ResetReading();

		pLayerDefn = pLayer->GetLayerDefn();
		if (!pLayerDefn)
			return;

		const char *layer_name = pLayerDefn->GetName();

		// Nodes
		if (!strcmp(layer_name, "NO01"))
		{
			// only 1 field: RCID - not enough to do anything useful
		}
		else if (!strcmp(layer_name, "NE01"))
		{
			// get field indices
			int index_entity = pLayerDefn->GetFieldIndex("ENTITY_LABEL");

			count = 0;
			while( (pFeature = pLayer->GetNextFeature()) != NULL )
			{
				pGeom = pFeature->GetGeometryRef();
				if (!pGeom) continue;
				pPoint = (OGRPoint *) pGeom;
				pBld = NewBuilding();

				point.x = pPoint->getX();
				point.y = pPoint->getY();
				pBld->SetRectangle(point, 10, 10);
				pBld->SetStories(1);

				Append(pBld);

				count++;
			}
		}
		// Lines
		else if (!strcmp(layer_name, "LE01"))
		{
			// only 3 field: RCID, SNID, ENID - not enough to do anything useful
		}
		// Areas (buildings, built-up areas, other areas like golf courses)
		else if (!strcmp(layer_name, "PC01"))
		{
			// get field indices
			int index_entity = pLayerDefn->GetFieldIndex("ENTITY_LABEL");

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

				int num_stories = 1; // Use this as a multiplier
				pBld = NULL;
				switch (iMinorAttr)
				{
				case 123:	// golf course
				case 150:	// built-up area
				case 420:	// cemetary
					break;
				case 402:	// church
				case 403:	// school
				case 405:	// courthouse
				case 406:	// post office
				case 407:	// city/town hall
				case 408:	// hospital
				case 409:	// prison
				case 412:	// customs building
				case 413:	// capitol
				case 414:	// community center
				case 415:	// muesum
				case 418:	// library
					num_stories = 2;
				case 400:	// building (general case)
					pBld = NewBuilding();
					break;
				}
				if (!pBld)
					continue;
				pGeom = pFeature->GetGeometryRef();
				if (!pGeom) continue;
				pPolygon = (OGRPolygon *) pGeom;

				ring = pPolygon->getExteriorRing();
				num_points = ring->getNumPoints();

				foot.SetSize(num_points);
				for (j = 0; j < num_points; j++)
					foot.SetAt(j, DPoint2(ring->getX(j),
						ring->getY(j)));

				pBld->SetFootprint(0, foot);

				vtBuilding *pDefBld = GetClosestDefault(pBld);
				if (!pDefBld)
				{
					pBld->SetStories(1);
				}
				else
				{
					pBld->SetElevationOffset(pDefBld->GetElevationOffset());

					for (unsigned int i = 0; i < pDefBld->GetNumLevels(); i++)
					{
						if (i != 0)
							pBld->CreateLevel(foot);
						vtLevel *pLevel = pBld->GetLevel(i);
						pLevel->m_iStories = pDefBld->GetLevel(i)->m_iStories * num_stories;
						pLevel->m_fStoryHeight = pDefBld->GetLevel(i)->m_fStoryHeight;
						pLevel->SetEdgeColor(pDefBld->GetLevel(i)->GetEdge(0)->m_Color);
						pLevel->SetEdgeMaterial(*pDefBld->GetLevel(i)->GetEdge(0)->m_pMaterial);
						for (int j = 0; j < pLevel->NumEdges(); j++)
							pLevel->GetEdge(j)->m_iSlope = pDefBld->GetLevel(i)->GetEdge(0)->m_iSlope;
					}
				}

				Append(pBld);
			}
		}
	}
}


//#pragma message( "Look here for more things to do " __FILE__ )

// 1. Better copy syntax for vtStructure and its descendants
// 2. extra import fields for fences and structures.
// 3. Handle tags

void vtStructureArray::AddElementsFromOGR_RAW(OGRDataSource *pDatasource,
		StructImportOptions &opt, void progress_callback(int))
{
	OGRLayer		*pLayer;

	pLayer = pDatasource->GetLayerByName(opt.m_strLayerName);
	if (!pLayer)
		return;

	// Get the projection (SpatialReference) from this layer
	OGRSpatialReference *pSpatialRef = pLayer->GetSpatialRef();
	if (pSpatialRef)
		m_proj.SetSpatialReference(pSpatialRef);

	switch (opt.type)
	{
		case ST_BUILDING:
			AddBuildingsFromOGR(pLayer, opt, progress_callback);
			break;
		case ST_LINEAR:
			AddLinearsFromOGR(pLayer, opt, progress_callback);
			break;
		case ST_INSTANCE:
			AddInstancesFromOGR(pLayer, opt, progress_callback);
			break;
		default:
			break;
	}
}

void vtStructureArray::AddBuildingsFromOGR(OGRLayer *pLayer,
		StructImportOptions &opt, void progress_callback(int))
{
	unsigned int	i, j;
	int				count;
	OGRFeature		*pFeature;
	OGRPolygon		*pPolygon;
	vtBuilding		*pBld;
	vtLevel         *pLevel, *pNewLevel;
	DPoint2			point;
	DLine2 footprint;
	OGRLinearRing *pRing;
	OGRLineString *pLineString;
	unsigned int num_points;
	OGRwkbGeometryType GeometryType;
	int iHeightIndex = -1;
	int iElevationIndex = -1;
	OGRFeatureDefn *pLayerDefn;
	float fMinZ, fMaxZ, fTotalZ;
	float fAverageZ;
	float fZ;
	float fOriginalElevation;
	float fMin, fMax, fDiff, fElev;
	PolyChecker PolyChecker;
	SchemaType Schema = SCHEMA_UI;
	int iFeatureCode;
	DPoint2 dPoint;

	int feature_count = pLayer->GetFeatureCount();
  	pLayer->ResetReading();

	pLayerDefn = pLayer->GetLayerDefn();
	if (!pLayerDefn)
		return;

	const char *layer_name = pLayerDefn->GetName();

	iHeightIndex = pLayerDefn->GetFieldIndex(opt.m_strFieldNameHeight);
	iElevationIndex = pLayerDefn->GetFieldIndex(opt.m_strFieldNameElevation);

	// Check for layers with known schemas
	if (!strcmp(layer_name, "osgb:TopographicArea"))
		Schema = SCHEMA_OSGB_TOPO_AREA;
	else if (!strcmp(layer_name, "osgb:TopographicPoint"))
		Schema = SCHEMA_OSGB_TOPO_POINT;

	count = 0;
	while( (pFeature = pLayer->GetNextFeature()) != NULL )
	{
		count++;
		progress_callback(count * 100 / feature_count);
		// Preprocess according to schema
		switch(Schema)
		{
			case SCHEMA_OSGB_TOPO_AREA:
				// Skip things that are not buildings
				iFeatureCode = pFeature->GetFieldAsInteger(pLayerDefn->GetFieldIndex("osgb:featureCode"));
				switch(iFeatureCode)
				{
					// Just do polygons for the time being
					case 10021: // Building defined by area
					case 10062: // Glasshouse
					case 10185: // Generic structure
					case 10190: // Archway
					// case 10193: // Pylon
					case 10187: // Upper level of communication
					case 10025: // Buildings or structure
						break;
					default:
						continue;
				}
				break;
			default:
				break;
		}


		OGRGeometry *pGeom = pFeature->GetGeometryRef();
		if (!pGeom)
			continue;
		GeometryType = pGeom->getGeometryType();

		// For the moment ignore multi polygons .. although we could
		// treat them as multiple buildings !!
		switch(wkbFlatten(GeometryType))
		{
			case wkbPolygon:
				pPolygon = (OGRPolygon *) pGeom;
				pRing = pPolygon->getExteriorRing();
				num_points = pRing->getNumPoints();

				// Ignore last point if it is the same as the first
				if (DPoint2(pRing->getX(0), pRing->getY(0)) == DPoint2(pRing->getX(num_points - 1), pRing->getY(num_points - 1)))
					num_points--;

				footprint.SetSize(num_points);
				fMaxZ = -1E9;
				fMinZ = 1E9;
				fTotalZ = 0;
				for (j = 0; j < num_points; j++)
				{
					fZ = (float)pRing->getZ(j);
					if (fZ > fMaxZ)
						fMaxZ = fZ;
					if (fZ < fMinZ)
						fMinZ = fZ;
					fTotalZ += fZ;
					footprint.SetAt(j, DPoint2(pRing->getX(j), pRing->getY(j)));
				}
				fAverageZ = fTotalZ/num_points;
				break;

			case wkbLineString:
				pLineString = (OGRLineString *) pGeom;
				num_points = pLineString->getNumPoints();

				// Ignore last point if it is the same as the first
				if (DPoint2(pLineString->getX(0), pLineString->getY(0)) == DPoint2(pLineString->getX(num_points - 1), pLineString->getY(num_points - 1)))
					num_points--;

				footprint.SetSize(num_points);
				fMaxZ = -1E9;
				fMinZ = 1E9;
				fTotalZ = 0;
				for (j = 0; j < num_points; j++)
				{
					fZ = (float)pLineString->getZ(j);
					if (fZ > fMaxZ)
						fMaxZ = fZ;
					if (fZ < fMinZ)
						fMinZ = fZ;
					fTotalZ += fZ;
					footprint.SetAt(j, DPoint2(pLineString->getX(j), pLineString->getY(j)));
				}
				fAverageZ = fTotalZ/num_points;
				break;

			case wkbPoint:
				dPoint = DPoint2(((OGRPoint *)pGeom)->getX(), ((OGRPoint *)pGeom)->getY());

				footprint.Empty();
				footprint.Append(dPoint + DPoint2(- DEFAULT_BUILDING_SIZE / 2, - DEFAULT_BUILDING_SIZE / 2));
				footprint.Append(dPoint + DPoint2(DEFAULT_BUILDING_SIZE / 2, - DEFAULT_BUILDING_SIZE / 2));
				footprint.Append(dPoint + DPoint2(DEFAULT_BUILDING_SIZE / 2, DEFAULT_BUILDING_SIZE / 2));
				footprint.Append(dPoint + DPoint2(- DEFAULT_BUILDING_SIZE / 2, DEFAULT_BUILDING_SIZE / 2));

				fAverageZ = (float)((OGRPoint *)pGeom)->getZ();

				break;

			default:
				continue;
		}

		// Ensure footprint is simple
		if (!PolyChecker.IsSimplePolygon(footprint))
			continue;

		if (opt.bInsideOnly)
		{
			// Exclude footprints outside the indicated extents
			for (i = 0; i < num_points; i++)
				if (opt.rect.ContainsPoint(footprint.GetAt(i)))
					break;
			if (i != num_points)
				continue;
		}

		pBld = NewBuilding();
		if (!pBld)
			return;

		pBld->SetFootprint(0, footprint);

		// Force footprint anticlockwise
		if (PolyChecker.IsClockwisePolygon(footprint))
		{
			pBld->FlipFootprintDirection();
			footprint  = pBld->GetFootprint(0);
		}

		vtBuilding *pDefBld = GetClosestDefault(pBld);
		if (!pDefBld)
		{
			pBld->SetStories(1);
		}
		else
		{
			pBld->SetElevationOffset(pDefBld->GetElevationOffset());

			for (i = 0; i < pDefBld->GetNumLevels(); i++)
			{
				if (i != 0)
					pBld->CreateLevel(footprint);
				vtLevel *pLevel = pBld->GetLevel(i);
				pLevel->m_iStories = pDefBld->GetLevel(i)->m_iStories;
				pLevel->m_fStoryHeight = pDefBld->GetLevel(i)->m_fStoryHeight;
				pLevel->SetEdgeColor(pDefBld->GetLevel(i)->GetEdge(0)->m_Color);
				pLevel->SetEdgeMaterial(*pDefBld->GetLevel(i)->GetEdge(0)->m_pMaterial);
				for (int j = 0;  j < pLevel->NumEdges(); j++)
					pLevel->GetEdge(j)->m_iSlope = pDefBld->GetLevel(i)->GetEdge(0)->m_iSlope;
			}
		}
		// Set the correct height for the roof level if neccessary
		pLevel = pBld->GetLevel(pBld->GetNumLevels() - 1);
		pBld->SetRoofType(pLevel->GuessRoofType(), pLevel->GetEdge(0)->m_iSlope);

		// Modify the height of the building if neccessary
		if (iHeightIndex != -1)
		{
			float fTotalHeight = 0;
			float fScaleFactor;
			unsigned int iNumLevels = pBld->GetNumLevels();
			RoofType eRoofType = pBld->GetRoofType();
			float fRoofHeight = pBld->GetLevel(iNumLevels - 1)->m_fStoryHeight;

			// If building has a roof I must exclude this from the calculation
			if (ROOF_UNKNOWN != eRoofType)
				iNumLevels--;
			else
				fRoofHeight = 0;

			for (i = 0; i < iNumLevels; i++)
				fTotalHeight += pBld->GetLevel(i)->m_fStoryHeight;

			fScaleFactor = ((float)pFeature->GetFieldAsDouble(iHeightIndex) - fRoofHeight)/fTotalHeight;
			for (i = 0; i < iNumLevels; i++)
				pBld->GetLevel(i)->m_fStoryHeight *= fScaleFactor;
		}

		// Modify elevation of building
		fOriginalElevation = -1E9;
		if ((GeometryType & wkb25DBit) && (opt.bUse25DForElevation))
			fOriginalElevation = fAverageZ;
		else if (iElevationIndex != -1)
			fOriginalElevation = (float) pFeature->GetFieldAsDouble(iElevationIndex);
		if (fOriginalElevation != -1E9)
			pBld->SetOriginalElevation(fOriginalElevation);

		// Add foundation
		if ((opt.bBuildFoundations) && (NULL != opt.pHeightField))
		{
			// Get the footprint of the lowest level
			pLevel = pBld->GetLevel(0);
			footprint = pLevel->GetFootprint();
			unsigned int iVertices = footprint.GetSize();

			fMin = 1E9;
			fMax = -1E9;
			for (j = 0; j < iVertices; j++)
			{
				if (!opt.pHeightField->FindAltitudeAtPoint2(footprint.GetAt(j), fElev))
					continue;

				if (fElev < fMin)
					fMin = fElev;
				if (fElev > fMax)
					fMax = fElev;
			}
			if (fOriginalElevation != -1E9)
				// I have a valid elevation
				fDiff = fOriginalElevation - fMin;
			else
				fDiff = fMax - fMin;

			if (fDiff > MINIMUM_BASEMENT_SIZE)
			{
				// Create and add a foundation level
				pNewLevel = new vtLevel();
				pNewLevel->m_iStories = 1;
				pNewLevel->m_fStoryHeight = fDiff;
				pBld->InsertLevel(0, pNewLevel);
				pBld->SetFootprint(0, footprint);
				pNewLevel->SetEdgeMaterial(BMAT_NAME_PLAIN);
				pNewLevel->SetEdgeColor(RGBi(128, 128, 128));
			}
			else
				pBld->SetElevationOffset(fDiff);

		}
		Append(pBld);
	}
}

void vtStructureArray::AddLinearsFromOGR(OGRLayer *pLayer,
		StructImportOptions &opt, void progress_callback(int))
{
	int iFeatureCount;
	OGRFeatureDefn *pLayerDefn;
	const char *pLayerName;
	SchemaType eSchema = SCHEMA_UI;
	int iCount;
	int iFeatureCode;
	OGRFeature		*pFeature;
	OGRGeometry		*pGeom;
	OGRwkbGeometryType GeometryType;
	OGRLineString *pLineString;
	DLine2 FencePoints;
	int iNumPoints;
	int i;
	float fMinZ;
	float fMaxZ;
	float fTotalZ;
	float fAverageZ;
	float fZ;
	vtFence *pFence;
	vtFence *pDefaultFence;
	float fOriginalElevation;
	int iHeightIndex = -1;
	int iElevationIndex = -1;

	iFeatureCount = pLayer->GetFeatureCount();
  	pLayer->ResetReading();

	pLayerDefn = pLayer->GetLayerDefn();
	if (!pLayerDefn)
		return;

	pLayerName = pLayerDefn->GetName();

	iHeightIndex = pLayerDefn->GetFieldIndex(opt.m_strFieldNameHeight);
	iElevationIndex = pLayerDefn->GetFieldIndex(opt.m_strFieldNameElevation);

	// Check for layers with known schemas
	if (!strcmp(pLayerName, "osgb:TopographicLine"))
		eSchema = SCHEMA_OSGB_TOPO_LINE;
	else if (!strcmp(pLayerName, "osgb:TopographicPoint"))
		eSchema = SCHEMA_OSGB_TOPO_POINT;

	iCount = 0;
	while((pFeature = pLayer->GetNextFeature()) != NULL )
	{
		iCount++;
		progress_callback(iCount * 100 / iFeatureCount);
		// Preprocess according to schema
		switch(eSchema)
		{
			case SCHEMA_OSGB_TOPO_LINE:
			case SCHEMA_OSGB_TOPO_POINT:
				// Skip things that are not linears
				iFeatureCode = pFeature->GetFieldAsInteger(pLayerDefn->GetFieldIndex("osgb:featureCode"));
				switch(iFeatureCode)
				{
					case 10045: // General feature - point
					case 10046: // General feature - line
						break;
					default:
						continue;
				}
				break;
			default:
				break;
		}

		pGeom = pFeature->GetGeometryRef();
		if (!pGeom)
			continue;
		GeometryType = pGeom->getGeometryType();

		if (wkbLineString != wkbFlatten(GeometryType))
			continue;

		pLineString = (OGRLineString *) pGeom;

		iNumPoints = pLineString->getNumPoints();

		FencePoints.SetSize(iNumPoints);
		fMaxZ = -1E9;
		fMinZ = 1E9;
		fTotalZ = 0;
		for (i = 0; i < iNumPoints; i++)
		{
			fZ = (float)pLineString->getZ(i);
			if (fZ > fMaxZ)
				fMaxZ = fZ;
			if (fZ < fMinZ)
				fMinZ = fZ;
			fTotalZ += fZ;
			FencePoints.SetAt(i, DPoint2(pLineString->getX(i), pLineString->getY(i)));
		}
		fAverageZ = fTotalZ/iNumPoints;

		if (opt.bInsideOnly)
		{
			// Exclude fences outside the indicated extents
			for (i = 0; i < iNumPoints; i++)
				if (opt.rect.ContainsPoint(FencePoints.GetAt(i)))
					break;
			if (i != iNumPoints)
				continue;
		}

		pFence = NewFence();
		if (!pFence)
			return;

		pDefaultFence = GetClosestDefault(pFence);
		if (NULL != pDefaultFence)
			*pFence = *pDefaultFence;

		for (i = 0; i < iNumPoints; i++)
			pFence->AddPoint(FencePoints[i]);

		// Modify height of fence
		if (iHeightIndex != -1)
			pFence->SetHeight((float)pFeature->GetFieldAsDouble(iHeightIndex));

		// Modify elevation of fence
		fOriginalElevation = -1E9;
		if ((GeometryType & wkb25DBit) && (opt.bUse25DForElevation))
			fOriginalElevation = fAverageZ;
		else if (iElevationIndex != -1)
			fOriginalElevation = (float) pFeature->GetFieldAsDouble(iElevationIndex);
		if (fOriginalElevation != -1E9)
			pFence->SetOriginalElevation(fOriginalElevation);

		Append(pFence);
	}
}

void vtStructureArray::AddInstancesFromOGR(OGRLayer *pLayer,
		StructImportOptions &opt, void progress_callback(int))
{
	int			iCount, iFeatureCount;
	SchemaType	eSchema = SCHEMA_UI;
	OGRFeature	*pFeature;
	OGRGeometry	*pGeom;
	float		fAverageZ;
	int			iFilenameIndex = -1;

	iFeatureCount = pLayer->GetFeatureCount();
  	pLayer->ResetReading();

	OGRFeatureDefn *pLayerDefn = pLayer->GetLayerDefn();
	if (!pLayerDefn)
		return;

	const char *pLayerName = pLayerDefn->GetName();

	// Check for layers with known schemas
	if (!strcmp(pLayerName, "osgb:TopographicArea"))
		eSchema = SCHEMA_OSGB_TOPO_AREA;
	else if (!strcmp(pLayerName, "osgb:TopographicPoint"))
		eSchema = SCHEMA_OSGB_TOPO_POINT;
	else
		iFilenameIndex = pLayerDefn->GetFieldIndex(opt.m_strFieldNameFile);

	if (-1 == iFilenameIndex)
		return;

	int iFeatureCode;
	iCount = 0;
	while((pFeature = pLayer->GetNextFeature()) != NULL )
	{
		iCount++;
		progress_callback(iCount * 100 / iFeatureCount);
		// Preprocess according to schema
		switch(eSchema)
		{
			case SCHEMA_OSGB_TOPO_AREA:
				// Skip things that are not buildings
				iFeatureCode = pFeature->GetFieldAsInteger(pLayerDefn->GetFieldIndex("osgb:featureCode"));
				switch(iFeatureCode)
				{
					case 1:
					default:
						continue;
				}
				break;
			default:
				break;
		}

		pGeom = pFeature->GetGeometryRef();
		if (!pGeom)
			continue;
		OGRwkbGeometryType GeometryType = pGeom->getGeometryType();

		if (wkbPoint != wkbFlatten(GeometryType))
			continue;

		DPoint2 p2(((OGRPoint *)pGeom)->getX(), ((OGRPoint *)pGeom)->getY());
		fAverageZ = (float)((OGRPoint *)pGeom)->getZ();

		if (opt.bInsideOnly && !opt.rect.ContainsPoint(p2))
			// Exclude instances outside the indicated extents
			continue;

		vtStructInstance *pInstance = NewInstance();
		if (!pInstance)
			return;

		vtStructInstance *pDefaultInstance = GetClosestDefault(pInstance);
		if (NULL != pDefaultInstance)
		{
			pInstance->m_fRotation = pDefaultInstance->m_fRotation;
			pInstance->m_fScale = pDefaultInstance->m_fScale;
		}
		pInstance->m_p = p2;

		Append(pInstance);
	}
}

