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
	double 	adfMinBound[4], adfMaxBound[4];
	DPoint2 point;
	DLine2	line;
	int		i, j, k;
	int		field_height = -1;
	int		field_filename = -1;
	int		field_itemname = -1;
	int		field_scale = -1;
	int		field_rotation = -1;

	SHPGetInfo(hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound);

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
			vtTag *tag;
			if (field_filename != -1)
			{
				string = DBFReadStringAttribute(db, i, field_filename);
				tag = new vtTag;
				tag->name = "filename";
				tag->value = string;
				inst->AddTag(tag);
			}
			if (field_itemname != -1)
			{
				string = DBFReadStringAttribute(db, i, field_itemname);
				tag = new vtTag;
				tag->name = "itemname";
				tag->value = string;
				inst->AddTag(tag);
			}
			if (field_scale != -1)
			{
				double scale = DBFReadDoubleAttribute(db, i, field_scale);
				if (scale != 1.0)
				{
					tag = new vtTag;
					tag->name = "scale";
					tag->value.Format("%lf", scale);
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
	if (opt.m_strLayerName != "")
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

					for (int i = 0; i < pDefBld->GetNumLevels(); i++)
					{
						if (i != 0)
							pBld->CreateLevel(foot);
						vtLevel *pLevel = pBld->GetLevel(i);
						pLevel->m_iStories = pDefBld->GetLevel(i)->m_iStories * num_stories;
						pLevel->m_fStoryHeight = pDefBld->GetLevel(i)->m_fStoryHeight;
						pLevel->SetEdgeColor(pDefBld->GetLevel(i)->m_Edges[0]->m_Color);
						pLevel->SetEdgeMaterial(pDefBld->GetLevel(i)->m_Edges[0]->m_Material);
						for (int j = 0; j < pLevel->GetNumEdges(); j++)
							pLevel->m_Edges[j]->m_iSlope = pDefBld->GetLevel(i)->m_Edges[0]->m_iSlope;
					}
				}

				Append(pBld);
			}
		}
	}
}

void vtStructureArray::AddElementsFromOGR_RAW(OGRDataSource *pDatasource,
		StructImportOptions &opt, void progress_callback(int))
{
	int i, j, feature_count, count;
	OGRLayer		*pLayer;
	OGRFeature		*pFeature;
	OGRGeometry		*pGeom;
	OGRPolygon		*pPolygon;
	vtBuilding		*pBld;
	vtLevel         *pLevel, *pNewLevel;
	DPoint2			point;
	DLine2 foot;
	OGRLinearRing *ring;
	OGRLineString *pLineString;
	int num_points;
	OGRwkbGeometryType GeometryType;
	int iHeightIndex;
	int iElevationIndex;
	int iFilenameIndex;
	OGRFeatureDefn *pLayerDefn;
	float fMinZ, fMaxZ, fTotalZ;
	float dAverageZ;
	float fOriginalElevation;
	int iVertices;
	float fMin, fMax, fDiff, fElev;

	pLayer = pDatasource->GetLayerByName(opt.m_strLayerName);
	if (!pLayer)
		return;

	feature_count = pLayer->GetFeatureCount();
  	pLayer->ResetReading();

	pLayerDefn = pLayer->GetLayerDefn();
	if (!pLayerDefn)
		return;

	GeometryType = pLayerDefn->GetGeomType();

	// Get the projection (SpatialReference) from this layer
	OGRSpatialReference *pSpatialRef = pLayer->GetSpatialRef();
	if (pSpatialRef)
		m_proj.SetSpatialReference(pSpatialRef);

	const char *layer_name = pLayerDefn->GetName();

	// Use the schema provided in the UI
	iHeightIndex = pLayerDefn->GetFieldIndex(opt.m_strFieldNameHeight);
	iElevationIndex = pLayerDefn->GetFieldIndex(opt.m_strFieldNameElevation);
	iFilenameIndex = pLayerDefn->GetFieldIndex(opt.m_strFieldNameFile);

	count = 0;
	while( (pFeature = pLayer->GetNextFeature()) != NULL )
	{
		count++;
		progress_callback(count * 100 / feature_count);


		pGeom = pFeature->GetGeometryRef();
		if (!pGeom)
			continue;

		// For the moment ignore multi polygons .. although we could
		// treat them as multiple buildings !!
		switch(wkbFlatten(GeometryType))
		{
			case wkbPolygon:
				pPolygon = (OGRPolygon *) pGeom;
				

				ring = pPolygon->getExteriorRing();
				num_points = ring->getNumPoints();

				foot.SetSize(num_points);
				for (j = 0; j < num_points; j++)
					foot.SetAt(j, DPoint2(ring->getX(j), ring->getY(j)));
				break;

			case wkbLineString:
				pLineString = (OGRLineString *) pGeom;
				
				num_points = pLineString->getNumPoints();

				// Ignore last point if it is the same as the first
				if (DPoint2(pLineString->getX(0), pLineString->getY(0)) == DPoint2(pLineString->getX(num_points - 1), pLineString->getY(num_points - 1)))
					num_points--;

				foot.SetSize(num_points);
				fMaxZ = -1E9;
				fMinZ = 1E9;
				fTotalZ = 0;
				for (j = 0; j < num_points; j++)
				{
#ifdef _DEBUG
					double dx = pLineString->getX(j);
					double dy = pLineString->getY(j);
#endif
					double dz = pLineString->getZ(j);
					if (dz > fMaxZ)
						fMaxZ = (float) dz;
					if (dz < fMinZ)
						fMinZ = (float) dz;
					fTotalZ += (float) dz;
					foot.SetAt(j, DPoint2(pLineString->getX(j), pLineString->getY(j)));
				}
				dAverageZ = fTotalZ/num_points;
				break;

			case wkbPoint:
				break;

			default:
				continue;
		}

		pBld = NewBuilding();
		if (!pBld)
			return;


		pBld->SetFootprint(0, foot);

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
					pBld->CreateLevel(foot);
				vtLevel *pLevel = pBld->GetLevel(i);
				pLevel->m_iStories = pDefBld->GetLevel(i)->m_iStories;
				pLevel->m_fStoryHeight = pDefBld->GetLevel(i)->m_fStoryHeight;
				pLevel->SetEdgeColor(pDefBld->GetLevel(i)->m_Edges[0]->m_Color);
				pLevel->SetEdgeMaterial(pDefBld->GetLevel(i)->m_Edges[0]->m_Material);
				for (int j = 0;  j < pLevel->GetNumEdges(); j++)
					pLevel->m_Edges[j]->m_iSlope = pDefBld->GetLevel(i)->m_Edges[0]->m_iSlope;
			}
		}
		// Modify the height of the building if neccessary
		if (iHeightIndex != -1)
		{
			float fTotalHeight = 0;
			float fScaleFactor;
			for (i = 0; i < pBld->GetNumLevels(); i++)
				fTotalHeight += pBld->GetLevel(i)->m_fStoryHeight;
			fScaleFactor = (float)pFeature->GetFieldAsDouble(iHeightIndex)/fTotalHeight;
			for (i = 0; i < pBld->GetNumLevels(); i++)
				pBld->GetLevel(i)->m_fStoryHeight *= fScaleFactor;
		}
		// Modify elevation of building
		fOriginalElevation = -1E9;
		if ((GeometryType & wkb25DBit) && (opt.bUse25DForElevation))
			fOriginalElevation = dAverageZ;
		else if (iElevationIndex != -1)
			fOriginalElevation = (float) pFeature->GetFieldAsDouble(iElevationIndex);
		// Add foundation
		if ((opt.bBuildFoundations) && (NULL != opt.pHeightField))
		{
			// Get the footprint of the lowest level
			pLevel = pBld->GetLevel(0);
			foot = pLevel->GetFootprint();
			iVertices = foot.GetSize();

			fMin = 1E9;
			fMax = -1E9;
			for (j = 0; j < iVertices; j++)
			{
				if (!opt.pHeightField->FindAltitudeAtPoint2(foot.GetAt(j), fElev))
					continue;

				if (fElev < fMin)
					fMin = fElev;
				if (fElev > fMax)
					fMax = fElev;
			}
			if (fOriginalElevation != 1E9)
			{
				// I have a valid elevation
				if (fOriginalElevation > fMin)
					// Build foundation to elevation level
					fDiff = fOriginalElevation - fMin;
				else
				{
					// Sink building into the ground for the time being
					fDiff = 0.0;
					pBld->SetElevationOffset((float)fOriginalElevation - fMin);
				}
			}
			else
				fDiff = fMax - fMin;

			if (0.0 != fDiff)
			{
				// Create and add a foundation level
				pNewLevel = new vtLevel();
				pNewLevel->m_iStories = 1;
				pNewLevel->m_fStoryHeight = fDiff;
				pBld->InsertLevel(0, pNewLevel);
				pBld->SetFootprint(0, foot);
				pNewLevel->SetEdgeMaterial(BMAT_CEMENT);
				pNewLevel->SetEdgeColor(RGBi(255, 255, 255));
			}
		}
		// Until I agree it with Ben set the offset to zero
		Append(pBld);
	}
}


