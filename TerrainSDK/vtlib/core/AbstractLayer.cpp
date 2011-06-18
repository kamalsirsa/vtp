//
// AbstractLayer.cpp
//
// Copyright (c) 2006-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#include "AbstractLayer.h"
#include "Terrain.h"

#include "vtdata/DataPath.h"
#include "vtdata/Features.h"	// for vtFeatureSet
#include "vtdata/vtLog.h"

vtAbstractLayer::vtAbstractLayer(vtTerrain *pTerr)
{
	m_pTerr = pTerr;

	pSet = NULL;
	pContainer = NULL;
	pGeomGroup = NULL;
	pLabelGroup = NULL;
	pMultiTexture = NULL;

	m_bNeedRebuild = false;
}

vtAbstractLayer::~vtAbstractLayer()
{
	ReleaseGeometry();
	if (pContainer)
	{
		pContainer->getParent(0)->removeChild(pContainer);
		pContainer = NULL;
	}
	delete pMultiTexture;
	delete pSet;
}

void vtAbstractLayer::SetLayerName(const vtString &fname)
{
	if (pSet)
		pSet->SetFilename(fname);
}

vtString vtAbstractLayer::GetLayerName()
{
	if (pSet)
		return pSet->GetFilename();
	else
		return "";
}

void vtAbstractLayer::SetVisible(bool bVis)
{
	if (pContainer != NULL)
		pContainer->SetEnabled(bVis);

	if (pMultiTexture)
		EnableMultiTexture(pMultiTexture->m_pNode, pMultiTexture, bVis);
}

bool vtAbstractLayer::GetVisible()
{
	if (pContainer != NULL)
		return pContainer->GetEnabled();

	else if (pMultiTexture)
		return MultiTextureIsEnabled(pMultiTexture->m_pNode, pMultiTexture);

	return false;
}

void vtAbstractLayer::SetFeatureSet(vtFeatureSet *pFeatureSet)
{
	pSet = pFeatureSet;

	// Handy pointers to disambiguate pSet
	pSetP2 = dynamic_cast<vtFeatureSetPoint2D*>(pSet);
	pSetP3 = dynamic_cast<vtFeatureSetPoint3D*>(pSet);
	pSetLS2 = dynamic_cast<vtFeatureSetLineString*>(pSet);
	pSetLS3 = dynamic_cast<vtFeatureSetLineString3D*>(pSet);
	pSetPoly = dynamic_cast<vtFeatureSetPolygon*>(pSet);
}


// Helper for the CreateFeature methods
bool GetColorField(const vtFeatureSet &feat, int iRecord, int iField, RGBAf &rgba)
{
	vtString str;
	float r, g, b;
	feat.GetValueAsString(iRecord, iField, str);
	if (sscanf((const char *)str, "%f %f %f", &r, &g, &b) != 3)
		return false;
	rgba.Set(r, g, b, 1);
	return true;
}

void vtAbstractLayer::CreateContainer()
{
	// first time
	pContainer = new vtGroup;
	pContainer->setName("Abstract Layer");

	// Abstract geometry goes into the scale features group, so it will be
	//  scaled up/down with the vertical exaggeration.
	m_pTerr->GetScaledFeatures()->addChild(pContainer);
}

/**
 * Given a featureset, create the geometry and place it
 * on the terrain.
 */
void vtAbstractLayer::CreateStyledFeatures()
{
	VTLOG1("CreateStyledFeatures\n");
	if (!pContainer)
		CreateContainer();

	unsigned int entities = pSet->GetNumEntities();
	VTLOG("  Creating %d entities.. ", entities);

	for (unsigned int i = 0; i < entities; i++)
	{
		CreateStyledFeature(i);
		m_pTerr->ProgressCallback(i * 100 / entities);
	}

	// A few types of visuals are not strictly per-feature; they must be
	//  created at once from all the features:
	//
	// 1. A line going through a point set.
	// 2. A TextureOverlay which rasterizes all the featues.

	if (m_StyleProps.GetValueBool("LineGeometry") && pSetP3 != NULL)
		CreateLineGeometryForPoints();

	if (m_StyleProps.GetValueBool("TextureOverlay"))
		CreateTextureOverlay();

	VTLOG1("Done.\n");
}

void vtAbstractLayer::CreateStyledFeature(int iIndex)
{
	if (!pContainer)
		CreateContainer();

	if (m_StyleProps.GetValueBool("ObjectGeometry"))
		CreateObjectGeometry(iIndex);

	if (m_StyleProps.GetValueBool("LineGeometry"))
		CreateLineGeometry(iIndex);

	if (m_StyleProps.GetValueBool("Labels"))
		CreateFeatureLabel(iIndex);
}

void vtAbstractLayer::CreateGeomGroup()
{
	// create geometry group to contain all the meshes
	pGeomGroup = new vtGroup;
	pGeomGroup->setName("Geometry");
	pContainer->addChild(pGeomGroup);

	// Create materials.
	pGeomMats = new vtMaterialArray;

	// common colors
	RGBi color;

	color = m_StyleProps.GetValueRGBi("ObjectGeomColor");
	material_index_object = pGeomMats->AddRGBMaterial1(color, true, true);

	color = m_StyleProps.GetValueRGBi("LineGeomColor");
	material_index_line = pGeomMats->AddRGBMaterial1(color, false, false);

	// There is always a yellow highlight material
	material_index_yellow = pGeomMats->AddRGBMaterial1(RGBf(1,1,0), false, false);

	pGeodeObject = new vtGeode;
	pGeodeObject->setName("Objects");
	pGeodeObject->SetMaterials(pGeomMats);
	pGeomGroup->addChild(pGeodeObject);

	pGeodeLine = new vtGeode;
	pGeodeLine->setName("Lines");
	pGeodeLine->SetMaterials(pGeomMats);
	pGeomGroup->addChild(pGeodeLine);
}

void vtAbstractLayer::CreateLabelGroup()
{
	pLabelGroup = new vtGroup;
	pLabelGroup->setName("Labels");
	pContainer->addChild(pLabelGroup);

	// If they specified a font name, use it
	vtString fontfile;
	if (!m_StyleProps.GetValueString("Font", fontfile))
	{
		// otherwise, use the default
		fontfile = "Arial.ttf";
	}
	// First, let the underlying scenegraph library try to find the font
	m_pFont = osgText::readFontFile((const char *)fontfile);
	if (!m_pFont.valid())
	{
		// look on VTP data paths
		vtString vtname = "Fonts/" + fontfile;
		fontfile = FindFileOnPaths(vtGetDataPath(), vtname);
		if (fontfile != "")
			m_pFont = osgText::readFontFile((const char *)fontfile);
	}
	if (m_pFont.valid())
		VTLOG("Successfully read font from '%s'\n", (const char *) fontfile);
	else
		VTLOG("Couldn't read font from file '%s', not creating labels.\n", (const char *) fontfile);
}

int vtAbstractLayer::GetObjectMaterialIndex(vtTagArray &style, unsigned int iIndex)
{
	int result;
	int color_field_index;
	if (style.GetValueInt("ObjectColorFieldIndex", color_field_index))
	{
		RGBAf rgba;
		if (GetColorField(*pSet, iIndex, color_field_index, rgba))
		{
			result = pGeomMats->FindByDiffuse(rgba);
			if (result == -1)
			{
				RGBf rgb = (RGBf) rgba;
				result = pGeomMats->AddRGBMaterial1(rgb, true, true);
			}
		}
		else
			result = material_index_object;
	}
	else
		result = material_index_object;
	return result;
}


/**
	Given a featureset and style description, create a geometry object (such as
	spheres) and place it on the terrain.
	If 2D, they will be draped on the terrain.
*/
void vtAbstractLayer::CreateObjectGeometry(unsigned int iIndex)
{
	if (!pGeomGroup)
		CreateGeomGroup();

	// for GetValueFloat below
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// We support geometry for 2D and 3D points, and 2D and 3D polylines
	if (!pSetP2 && !pSetP3 && !pSetLS2 && !pSetLS3)
		return;

	// Determine color and material index
	int material_index = GetObjectMaterialIndex(m_StyleProps, iIndex);

	// Determine geometry size and placement
	float fHeight = 0.0f;
	if (pSetP2 || pSetLS2)
		m_StyleProps.GetValueFloat("ObjectGeomHeight", fHeight);

	float fRadius;
	if (!m_StyleProps.GetValueFloat("ObjectGeomSize", fRadius))
		fRadius = 1;

	vtHeightField3d *hf = m_pTerr->GetHeightField();
	int res = 3;
	FPoint3 p3;

	// Track what is created
	vtVisual *viz = GetViz(pSet->GetFeature(iIndex));

	if (pSetP2)
	{
		const DPoint2 &epos = pSetP2->GetPoint(iIndex);
		hf->ConvertEarthToSurfacePoint(epos, p3, 0, true);	// use true elev
		p3.y += fHeight;

		vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, VT_Normals, res*res*2);
		mesh->CreateEllipsoid(p3, FPoint3(fRadius, fRadius, fRadius), res);

		pGeodeObject->AddMesh(mesh, material_index);

		// Track
		if (viz) viz->m_meshes.push_back(mesh);
	}
	else if (pSetP3)
	{
		const DPoint3 &epos = pSetP3->GetPoint(iIndex);
		float original_z = (float) epos.z;
		hf->m_Conversion.ConvertFromEarth(epos, p3);

		// If a large number of entities, make as simple geometry as possible
		bool bTetrahedra = (pSet->GetNumEntities() > 10000);

		bool bShaded = true;
		vtMesh *mesh;
		if (bTetrahedra)
		{
			mesh = new vtMesh(osg::PrimitiveSet::TRIANGLES, bShaded ? VT_Normals : 0, 12);
			mesh->CreateTetrahedron(p3, fRadius);
			if (bShaded)
				mesh->SetNormalsFromPrimitives();
		}
		else
		{
			mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, bShaded ? VT_Normals : 0, res*res*2);
			mesh->CreateEllipsoid(p3, FPoint3(fRadius, fRadius, fRadius), res);
		}

		pGeodeObject->AddMesh(mesh, material_index);

		// Track
		if (viz) viz->m_meshes.push_back(mesh);
	}
	else if (pSetLS2)
	{
		const DLine2 &dline = pSetLS2->GetPolyLine(iIndex);
		for (unsigned int j = 0; j < dline.GetSize(); j++)
		{
			// preserve 3D point's elevation: don't drape
			hf->ConvertEarthToSurfacePoint(dline[j], p3);
			p3.y += fHeight;

			vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, VT_Normals, res*res*2);
			mesh->CreateEllipsoid(p3, FPoint3(fRadius, fRadius, fRadius), res);

			pGeodeObject->AddMesh(mesh, material_index);

			// Track
			if (viz) viz->m_meshes.push_back(mesh);
		}
	}
	else if (pSetLS3)
	{
		const DLine3 &dline = pSetLS3->GetPolyLine(iIndex);
		for (unsigned int j = 0; j < dline.GetSize(); j++)
		{
			// preserve 3D point's elevation: don't drape
			hf->m_Conversion.ConvertFromEarth(dline[j], p3);

			vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, VT_Normals, res*res*2);
			mesh->CreateEllipsoid(p3, FPoint3(fRadius, fRadius, fRadius), res);

			pGeodeObject->AddMesh(mesh, material_index);

			// Track
			if (viz) viz->m_meshes.push_back(mesh);
		}
	}
}

/**
	Given a featureset and style description, create line geometry.
	If 2D, it will be draped on the terrain. Polygon features
	(vtFeatureSetPolygon) will also be created as line geometry
	(unfilled polygons) and draped on the ground.
*/
void vtAbstractLayer::CreateLineGeometry(unsigned int iIndex)
{
	// for GetValueFloat below
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// We support geometry for 2D and 3D polylines, and 2D polygons
	if (!pSetLS2 && !pSetLS3 && !pSetPoly)
		return;

	// geometry group to contain all the meshes
	if (!pGeomGroup)
		CreateGeomGroup();

	// We may need to convert from the CRS of the featureset to the CRS of the
	//  terrain (before converting from terrain to world coordinates)
	vtProjection &proj_feat = pSet->GetAtProjection();
	vtProjection &proj_terr = m_pTerr->GetProjection();
	auto_ptr<OCT> octransform;
	if (proj_feat.GetRoot() && proj_terr.GetRoot() && !proj_feat.IsSame(&proj_terr))
	{
		// If we have two valid CRSs, and they are not the same, then we need a transform
		octransform.reset(CreateCoordTransform(&proj_feat, &proj_terr, true));
	}

	// Determine color and material index
	int color_field_index;
	int material_index;
	if (m_StyleProps.GetValueInt("LineColorFieldIndex", color_field_index))
	{
		RGBAf rgba;
		if (GetColorField(*pSet, iIndex, color_field_index, rgba))
		{
			material_index = pGeomMats->FindByDiffuse(rgba);
			if (material_index == -1)
			{
				RGBf rgb = (RGBf) rgba;
				material_index = pGeomMats->AddRGBMaterial1(rgb, false, false, true);
			}
		}
		else
			material_index = material_index_line;
	}
	else
		material_index = material_index_line;

	// Estimate number of mesh vertices we'll have
	int iEstimatedVerts = 0;
	if (pSetLS2)
	{
		const DLine2 &dline = pSetLS2->GetPolyLine(iIndex);
		iEstimatedVerts = dline.GetSize();
	}
	else if (pSetLS3)
	{
		const DLine3 &dline = pSetLS3->GetPolyLine(iIndex);
		iEstimatedVerts = dline.GetSize();
	}
	else if (pSetPoly)
	{
		const DPolygon2 &dpoly = pSetPoly->GetPolygon(iIndex);
		for (unsigned int k = 0; k < dpoly.size(); k++)
		{
			const DLine2 &dline = dpoly[k];
			iEstimatedVerts += dline.GetSize();
			iEstimatedVerts ++;		// close polygon
		}
	}

	vtGeomFactory mf(pGeodeLine, osg::PrimitiveSet::LINE_STRIP, 0, 3000, material_index,
		iEstimatedVerts);

	float fHeight = 0.0f;
	if (pSetLS2 || pSetPoly)
	{
		if (!m_StyleProps.GetValueFloat("LineGeomHeight", fHeight))
			fHeight = 1.0f;
	}
	bool bTessellate = m_StyleProps.GetValueBool("Tessellate");
	bool bCurve = false;

	FPoint3 f3;
	unsigned int size;
	if (pSetLS2)
	{
		const DLine2 &dline = pSetLS2->GetPolyLine(iIndex);

		m_pTerr->AddSurfaceLineToMesh(&mf, dline, fHeight, bTessellate, bCurve);
	}
	else if (pSetLS3)
	{
		mf.PrimStart();
		const DLine3 &dline = pSetLS3->GetPolyLine(iIndex);
		size = dline.GetSize();
		for (unsigned int j = 0; j < size; j++)
		{
			// preserve 3D point's elevation: don't drape
			DPoint3 p = dline[j];
			if (octransform.get())
				octransform->Transform(1, &p.x, &p.y);
			m_pTerr->GetHeightField()->m_Conversion.ConvertFromEarth(p, f3);
			mf.AddVertex(f3);
		}
		mf.PrimEnd();
	}
	else if (pSetPoly)
	{
		const DPolygon2 &dpoly = pSetPoly->GetPolygon(iIndex);
		for (unsigned int k = 0; k < dpoly.size(); k++)
		{
			// This would be the efficient way
//				const DLine2 &dline = dpoly[k];

			// but we must copy each polyline in order to close it
			DLine2 dline = dpoly[k];
			dline.Append(dline[0]);

			m_pTerr->AddSurfaceLineToMesh(&mf, dline, fHeight, bTessellate, bCurve, true);
		}
	}

	// If the user specified a line width, apply it now
	bool bWidth = false;
	float fWidth;
	if (m_StyleProps.GetValueFloat("LineWidth", fWidth) && fWidth != 1.0f)
		bWidth = true;

	// Track what was created
	vtVisual *viz = GetViz(pSet->GetFeature(iIndex));
	for (unsigned int i = 0; i < mf.m_Meshes.size(); i++)
	{
		vtMesh *mesh = mf.m_Meshes[i];

		if (bWidth)
			mesh->SetLineWidth(fWidth);

		// Track
		if (viz) viz->m_meshes.push_back(mesh);
	}
}

/**
	Given a featureset and style description, create line geometry that
	goes through all the points.
*/
void vtAbstractLayer::CreateLineGeometryForPoints()
{
	// for GetValueFloat below
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// We support geometry for 3D point sets (line through the points),
	if (!pSetP3)
		return;

	// geometry group to contain all the meshes
	if (!pGeomGroup)
		CreateGeomGroup();

	int material_index = material_index_line;

	// Estimate number of mesh vertices we'll have
	const DLine3 &dline = pSetP3->GetAllPoints();
	unsigned int size = dline.GetSize();
	int iEstimatedVerts = size;

	vtGeomFactory mf(pGeodeLine, osg::PrimitiveSet::LINE_STRIP, 0, 30000, material_index,
		iEstimatedVerts);

	FPoint3 f3;
	mf.PrimStart();
	for (unsigned int j = 0; j < size; j++)
	{
		// preserve 3D point's elevation: don't drape
		m_pTerr->GetHeightField()->m_Conversion.ConvertFromEarth(dline[j], f3);
		mf.AddVertex(f3);
	}
	mf.PrimEnd();

	// If the user specified a line width, apply it now
	float fWidth;
	if (m_StyleProps.GetValueFloat("LineWidth", fWidth) && fWidth != 1.0f)
	{
		for (unsigned int i = 0; i < mf.m_Meshes.size(); i++)
		{
			vtMesh *mesh = mf.m_Meshes[i];
			mesh->SetLineWidth(fWidth);
		}
	}
}

/**
 * Given a featureset and style description, create a labels and place it
 * on the terrain.
 *
 * If the features are 2D or 3D points (vtFeatureSetPoint2D or
 * vtFeatureSetPoint3D) then the labels will be placed at those points.  If
 * the features are 2D polygons (vtFeatureSetPolygon) then the point used is
 * the centroid of the polygon.
 */
void vtAbstractLayer::CreateFeatureLabel(unsigned int iIndex)
{
	// We support text labels for 2D and 3D points, and 2D polygons
	if (!pSetP2 && !pSetP3 && !pSetPoly)
		return;

	// create group
	if (!pLabelGroup)
		CreateLabelGroup();

	// Must have a font to make a label
	if (!m_pFont.valid())
		return;

	// for GetValueFloat below
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// Get the earth location of the label
	DPoint2 p2;
	DPoint3 p3;
	if (pSetP2)
		p2 = pSetP2->GetPoint(iIndex);
	else if (pSetP3)
	{
		p3 = pSetP3->GetPoint(iIndex);
		p2.Set(p3.x, p3.y);
	}
	else if (pSetPoly)
	{
		const DPolygon2 &dp = pSetPoly->GetPolygon(iIndex);
		p2 = dp[0].Centroid();
	}

	// Don't drape on culture, but do use true elevation
	FPoint3 fp3;
	if (!m_pTerr->GetHeightField()->ConvertEarthToSurfacePoint(p2, fp3, 0, true))
		return;

	float label_elevation;
	if (!m_StyleProps.GetValueFloat("LabelHeight", label_elevation))
		label_elevation = 0.0f;

	// Elevate the location by the desired vertical offset
	fp3.y += label_elevation;

	// If we have a 3D point, we can use the Z component of the point
	//  to further affect the elevation.
	if (pSetP3)
		fp3.y += label_elevation;

	float label_size;
	if (!m_StyleProps.GetValueFloat("LabelSize", label_size))
		label_size = 18;

	// Create the vtTextMesh
	vtTextMesh *text = new vtTextMesh(m_pFont, label_size, true);	// center

	// Get the label text
	int text_field_index;
	if (!m_StyleProps.GetValueInt("TextFieldIndex", text_field_index))
		text_field_index = -1;
	vtString str;
	pSet->GetValueAsString(iIndex, text_field_index, str);

#if SUPPORT_WSTRING
	// Text will be UTF-8
	wstring2 wide_string;
	wide_string.from_utf8(str);
	text->SetText(wide_string);
#else
	// Hope that it isn't
	text->SetText(str);
#endif

	// Create the vtGeode object to contain the vtTextMesh
	vtGeode *geode = new vtGeode;
	geode->setName(str);

	// Determine feature color
	bool bGotColor = false;
	int color_field_index;
	if (m_StyleProps.GetValueInt("ColorFieldIndex", color_field_index))
	{
		RGBAf rgba;
		if (GetColorField(*pSet, iIndex, color_field_index, rgba))
		{
			text->SetColor(rgba);
			bGotColor = true;
		}
	}
	if (!bGotColor)
	{
		RGBf rgb = m_StyleProps.GetValueRGBi("LabelColor");
		text->SetColor(rgb);
	}

	geode->AddTextMesh(text, -1);

	// Add to a billboarding transform so that the labels turn
	// toward the viewer
	vtTransform *bb = new vtTransform;
	bb->addChild(geode);
	m_pTerr->GetBillboardEngine()->AddTarget(bb);

	bb->SetTrans(fp3);
	pLabelGroup->addChild(bb);

	// Track what was created
	vtVisual *viz = GetViz(pSet->GetFeature(iIndex));
	if (viz) viz->m_xform = bb;
}

bool vtAbstractLayer::CreateTextureOverlay()
{
	VTLOG1("  CreateTextureOverlay\n");

	// for GetValueFloat below
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// We support texture overlay for only 2D polygons (so far)
	vtFeatureSet &feat = *(pSet);
	if (!pSetPoly)
		return false;

	const int ALPD_RESOLUTION = 1024;

	// Set up the image
	vtImage *image = new vtImage;
	if (!image->Create(ALPD_RESOLUTION, ALPD_RESOLUTION, 32))
		return false;

	// Get data extents
	DRECT DataExtents;
	pSet->ComputeExtent(DataExtents);

	double DeltaX = DataExtents.Width() / (double)ALPD_RESOLUTION;
	double DeltaY = DataExtents.Height() / (double)ALPD_RESOLUTION;

	int iNumFeatures = pSetPoly->GetNumEntities();
	RGBAi LayerColour = m_StyleProps.GetValueRGBi("GeomColor");
	LayerColour.a = 255;

	for (int ImageX = 0; ImageX < ALPD_RESOLUTION; ImageX++)
	{
		for (int ImageY = 0; ImageY < ALPD_RESOLUTION; ImageY++)
		{
			image->SetPixel32(ImageX, ImageY, RGBAi(0,0,0,0));
			for (int feat = 0; feat < iNumFeatures; feat++)
			{
				DPoint2 Point(DataExtents.left + DeltaX / 2 + DeltaX * ImageX,
								DataExtents.top - DeltaY / 2 - DeltaY * ImageY);
				if (pSetPoly->GetPolygon(feat).ContainsPoint(Point))
				{
					image->SetPixel32(ImageX, ImageY, LayerColour);
				}
			}
		}
	}

	int iTextureMode;
	vtString mode = m_StyleProps.GetValueString("TextureMode");
	if (mode == "ADD") iTextureMode = GL_ADD;
	if (mode == "MODULATE") iTextureMode = GL_MODULATE;
	if (mode == "DECAL") iTextureMode = GL_DECAL;
	pMultiTexture = m_pTerr->AddMultiTextureOverlay(image, DataExtents, iTextureMode);
	return true;
}

/**
 * Release all the 3D stuff created for the layer (including geometry and labels).
 */
void vtAbstractLayer::ReleaseGeometry()
{
	for (int i = pSet->GetNumEntities()-1; i >= 0; i--)
	{
		vtFeature *f = pSet->GetFeature(i);
		ReleaseFeatureGeometry(f);
	}
	if (pGeomGroup)
	{
		pContainer->removeChild(pGeomGroup);
		pGeomGroup = NULL;
	}
	if (pLabelGroup)
	{
		pContainer->removeChild(pLabelGroup);
		pLabelGroup = NULL;
	}
}

/**
 * Release all the 3D stuff created for a given feature.
 */
void vtAbstractLayer::ReleaseFeatureGeometry(vtFeature *f)
{
	vtVisual *v = GetViz(f);

	for (unsigned int m = 0; m < v->m_meshes.size(); m++)
	{
		vtMesh *mesh = v->m_meshes[m];

		pGeodeObject->RemoveMesh(mesh);
		pGeodeLine->RemoveMesh(mesh);
	}
	if (v->m_xform)
	{
		pLabelGroup->removeChild(v->m_xform);

		// labels might be targets of the billboard engine
		vtEngine *bbe = m_pTerr->GetBillboardEngine();
		if (bbe)
			bbe->RemoveTarget(v->m_xform);
	}
	delete v;
	m_Map.erase(f);
}

void vtAbstractLayer::DeleteFeature(vtFeature *f)
{
	// Check if we need to rebuild the whole thing
	if (CreateAtOnce())
		m_bNeedRebuild = true;
	else
		ReleaseFeatureGeometry(f);
}

// When the underlying feature changes (in memory), we need to rebuild the visual
void vtAbstractLayer::Rebuild()
{
	ReleaseGeometry();
	CreateStyledFeatures();
}

// When the feature set changes (on disk), we can reload it and rebuild the visual
void vtAbstractLayer::Reload()
{
	// We must release the geometry before changing the featureset
	ReleaseGeometry();

	vtString fname = pSet->GetFilename();

	// Now we can remove the previous featureset
	delete pSet;

	vtFeatureLoader loader;
	vtFeatureSet *newset = loader.LoadFrom(fname);
	if (!newset)
	{
		VTLOG("Couldn't read features from file '%s'\n", (const char *)fname);
		return;
	}
	VTLOG("Successfully read features from file '%s'\n", (const char *)fname);
	SetFeatureSet(newset);

	CreateStyledFeatures();
}

// When the underlying feature changes, we need to rebuild the visual
void vtAbstractLayer::RebuildFeature(unsigned int iIndex)
{
	// If we're not doing a full rebuild, we can create individual items
	if (!m_bNeedRebuild)
	{
		vtFeature *f = pSet->GetFeature(iIndex);
		ReleaseFeatureGeometry(f);
		CreateStyledFeature(iIndex);
	}
}

void vtAbstractLayer::UpdateVisualSelection()
{
	// use SetMeshMatIndex to make the meshes of selected features yellow
	for (unsigned int j = 0; j < pSet->GetNumEntities(); j++)
	{
		vtFeature *feat = pSet->GetFeature(j);
		vtVisual *viz = GetViz(feat);
		if (viz)
		{
			int material_index;
			if (pSet->IsSelected(j))
				material_index = material_index_yellow;
			else
				material_index = GetObjectMaterialIndex(m_StyleProps, j);

			for (unsigned int k = 0; k < viz->m_meshes.size(); k++)
			{
				vtMesh *mesh = viz->m_meshes[k];
				pGeodeObject->SetMeshMatIndex(mesh, material_index);
			}
		}
	}
}

// To make sure all edits are fully reflected in the visual, call these
//  methods around any editing of style or geometry.
void vtAbstractLayer::EditBegin()
{
}

void vtAbstractLayer::EditEnd()
{
	if (m_bNeedRebuild)
	{
		m_bNeedRebuild = false;
		Rebuild();
	}
}

vtVisual *vtAbstractLayer::GetViz(vtFeature *feat)
{
#if 0
	return NULL;
#else
	vtVisual *v = m_Map[feat];
	if (!v)
	{
		v = new vtVisual;
		m_Map[feat] = v;
	}
	return v;
#endif
}

// A few types of visuals are not strictly per-feature; they must be
//  created at once from all the features.
bool vtAbstractLayer::CreateAtOnce()
{
	if (m_StyleProps.GetValueBool("LineGeometry") && pSetP3 != NULL)
		return true;
	if (m_StyleProps.GetValueBool("TextureOverlay"))
		return true;
	return false;
}

