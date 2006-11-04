//
// AbstractLayer.cpp
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Terrain.h"
#include "TerrainScene.h"		// for vtGetDataPath()

#include "AbstractLayer.h"

#include "vtdata/Features.h"	// for vtFeatureSet
#include "vtdata/vtLog.h"


vtAbstractLayer::vtAbstractLayer()
{
	pSet = NULL;
	pContainer = NULL;
	pGeomGroup = NULL;
	pLabelGroup = NULL;
}
vtAbstractLayer::~vtAbstractLayer()
{
	delete pSet;
	if (pGeomGroup) pGeomGroup->Release();
	if (pLabelGroup) pLabelGroup->Release();
	if (pContainer) pContainer->Release();
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

/**
 * Given a featureset and style description, create the geometry and place it
 * on the terrain.
 *
 * \param layer The set of features to create.
 * \param style The style (color, size, etc.) to use in creating the features.
		See CreateFeatureGeometry() and CreateFeatureLabels() for descriptions
		of the supported style tags.  Control tags to this method are:
		- Geometry: Boolean, true indicates that geometry (such as lines or
			spheres) should be created.
		- Labels: Boolean, true indicates that text labels should be created.
 */
void vtAbstractLayer::CreateStyledFeatures(vtTerrain *pTerr)
{
	if (pContainer)
	{
		// re-creating, so remove any previous geometry
		if (pGeomGroup)
		{
			pGeomGroup->Release();
			pGeomGroup = NULL;
		}
		if (pLabelGroup)
		{
			pLabelGroup->Release();
			pLabelGroup = NULL;
		}
	}
	else
	{
		// first time
		pContainer = new vtGroup;
		pContainer->SetName2("Abstract Layer");
	}

	// Abstract geometry goes into the scale features group, so it will be
	//  scaled up/down with the vertical exaggeration.
	pTerr->GetScaledFeatures()->AddChild(pContainer);

	vtTagArray &style = pSet->GetProperties();

	if (style.GetValueBool("Geometry"))
		CreateFeatureGeometry(pTerr);

	if (style.GetValueBool("Labels"))
		CreateFeatureLabels(pTerr);
}

/**
 * Given a featureset and style description, create geometry objects (such as
 * lines or spheres) and place them on the terrain.
 *
 * \param layer The set of features to create.  If the features are 2D or 3D
		polylines (vtFeatureSetLineString or vtFeatureSetLineString3D) then
		line geometry will be created.  If 2D, it will be draped on the
		terrain. Polygon features (vtFeatureSetPolygon) will also be created
		as line geometry (unfilled polygons) and draped on the ground.
 * \param style The style (color, size, etc.) to use in creating the features.
		Supported tags include:
		- GeomColor: RGB value to be used for all features.  Default is white.
		- GeomHeight: Height above ground, for draped geometry.
		- LineWidth: Width of line geometry, in pixels.
		- Tessellate: Boolean, indicates whether to tesselate each line segment
			to allow for smoother draping on uneven terrain.  Default is false.
 */
void vtAbstractLayer::CreateFeatureGeometry(vtTerrain *pTerr)
{
	vtTagArray &style = pSet->GetProperties();

	// for GetValueFloat below
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// We support geometry for 2D and 3D polylines
	const vtFeatureSetLineString   *pSetLS2 = dynamic_cast<const vtFeatureSetLineString*>(pSet);
	const vtFeatureSetLineString3D *pSetLS3 = dynamic_cast<const vtFeatureSetLineString3D*>(pSet);
	const vtFeatureSetPolygon *pSetPoly = dynamic_cast<const vtFeatureSetPolygon*>(pSet);
	if (!pSetLS2 && !pSetLS3 && !pSetPoly)
		return;

	// shorthand ref
	vtFeatureSet &feat = *pSet;

	// create geometry group to contain all the meshes
	pGeomGroup = new vtGroup;
	pGeomGroup->SetName2("Geometry");
	pContainer->AddChild(pGeomGroup);

	// Create materials.
	vtMaterialArray *pMats = new vtMaterialArray;

	// common color
	RGBi color = style.GetValueRGBi("GeomColor");
	int common_material_index = pMats->AddRGBMaterial1(color, false, false);

	// If each feature has its own color, we need to create those materials
	int color_field_index;
	RGBAf rgba;
	if (!style.GetValueInt("ColorFieldIndex", color_field_index))
		color_field_index = -1;
	if (color_field_index != -1)
	{
		// go through all the features collecting unique colors
		for (unsigned int i = 0; i < feat.GetNumEntities(); i++)
		{
			// if we have a unique color, add it
			if (GetColorField(feat, i, color_field_index, rgba))
			{
				if (pMats->FindByDiffuse(rgba) == -1)
				{
					RGBf rgb = (RGBf) rgba;
					pMats->AddRGBMaterial1(rgb, false, false);
				}
			}
		}
	}

	vtGeom *geom = new vtGeom;
	geom->SetMaterials(pMats);
	pMats->Release();

	vtMeshFactory mf(geom, vtMesh::LINE_STRIP, 0, 30000, 0);

	float fHeight;
	if (!style.GetValueFloat("GeomHeight", fHeight))
		fHeight = 1;
	bool bTessellate = style.GetValueBool("Tessellate");
	bool bCurve = false;

	int material_index;
	FPoint3 f3;
	for (unsigned int i = 0; i < feat.GetNumEntities(); i++)
	{
		if (color_field_index == -1)
			material_index = common_material_index;
		else
		{
			if (GetColorField(feat, i, color_field_index, rgba))
				material_index = pMats->FindByDiffuse(rgba);
			else
				material_index = common_material_index;
		}

		unsigned int size;
		if (pSetLS2)
		{
			const DLine2 &dline = pSetLS2->GetPolyLine(i);

			mf.SetMatIndex(material_index);
			pTerr->AddSurfaceLineToMesh(&mf, dline, fHeight, bTessellate, bCurve);
		}
		else if (pSetLS3)
		{
			mf.PrimStart();
			const DLine3 &dline = pSetLS3->GetPolyLine(i);
			size = dline.GetSize();
			for (unsigned int j = 0; j < size; j++)
			{
				// preserve 3D point's elevation: don't drape
				pTerr->GetHeightFieldGrid3d()->m_Conversion.ConvertFromEarth(dline[j], f3);
				mf.AddVertex(f3);
			}
			mf.PrimEnd();
		}
		else if (pSetPoly)
		{
			const DPolygon2 &dpoly = pSetPoly->GetPolygon(i);
			for (unsigned int k = 0; k < dpoly.size(); k++)
			{
				// This would be the efficient way
//				const DLine2 &dline = dpoly[k];

				// but we must copy each polyline in order to close it
				DLine2 dline = dpoly[k];
				dline.Append(dline[0]);

				pTerr->AddSurfaceLineToMesh(&mf, dline, fHeight, bTessellate, bCurve, true);
			}
		}
	}

	// If the user specified a line width, apply it now
	float fWidth;
	if (style.GetValueFloat("LineWidth", fWidth) && fWidth != 1.0f)
	{
		for (unsigned int j = 0; j < geom->GetNumMeshes(); j++)
		{
			vtMesh *mesh = geom->GetMesh(j);
			mesh->SetLineWidth(fWidth);
		}
	}

	pGeomGroup->AddChild(geom);
}

/**
 * Given a featureset and style description, create geometry objects (such as
 * lines or spheres) and place them on the terrain.
 *
 * \param layer The set of features to create.  If the features are 2D or 3D
		points (vtFeatureSetPoint2D or vtFeatureSetPoint3D) then the labels
		will be placed at those points.  If the features are 2D polygons
		(vtFeatureSetPolygon) then the point used is the centroid of the
		polygon.
 * \param style The style (color, size, etc.) to use in creating the features.
		Supported tags include:
		- LabelColor: RGB value to be used for all features.  Default is white.
		- TextFieldIndex: The 0-based index of the field to use for the label text.
		- ColorFieldIndex: The 0-based index of the field to use for the label color.
		- Elevation: Float value to elevate the label.  For example, if the
			input is 2D points, and this value is 30, then the labels are
			created 30 meters above the ground.  Default is 0.
		- LabelSize: Size (height) of the label in meters.  Default is 18.
 */
void vtAbstractLayer::CreateFeatureLabels(vtTerrain *pTerr)
{
	vtTagArray &style = pSet->GetProperties();

	// for GetValueFloat below
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	VTLOG1("CreateFeatureLabels\n");

	// We support text labels for both 2D and 3D points, and 2D polygons
	vtFeatureSet &feat = *(pSet);
	const vtFeatureSetPoint2D *pSetP2 = dynamic_cast<const vtFeatureSetPoint2D*>(&feat);
	const vtFeatureSetPoint3D *pSetP3 = dynamic_cast<const vtFeatureSetPoint3D*>(&feat);
	const vtFeatureSetPolygon *pSetPG = dynamic_cast<const vtFeatureSetPolygon*>(&feat);
	if (!pSetP2 && !pSetP3 && !pSetPG)
		return;

	// create container group
	pLabelGroup = new vtGroup;
	pLabelGroup->SetName2("Labels");
	pContainer->AddChild(pLabelGroup);

	// Create materials.
	vtMaterialArray *pLabelMats = new vtMaterialArray;

	unsigned int features = feat.GetNumEntities();
	if (features == 0)
		return;

	// default case: common label color
	RGBi label_color = style.GetValueRGBi("LabelColor");
	int common_material_index =
		pLabelMats->AddRGBMaterial1(label_color, false, true);

#if 0
	// It turns out that we don't have to do this, because OSG lets us
	//  specify text color directly, rather than using materials.
	if (field_index_color != -1)
	{
		// go through all the features collecting unique colors
		for (i = 0; i < features; i++)
		{
			// if we have a unique color, add it
			if (GetColorField(feat, i, field_index_color, rgba))
			{
				if (pLabelMats->FindByDiffuse(rgba) == -1)
				{
					RGBi rgb = (RGBi) (RGBf) rgba;
					pLabelMats->AddRGBMaterial1(rgb, false, false);
				}
			}
		}
	}
#endif

	// Find and load the font.
#if VTLIB_OPENSG
	const char *fontname = "Fonts/Arial.txf";
#else
	const char *fontname = "Fonts/Arial.ttf";
#endif
	vtString font_path = FindFileOnPaths(vtGetDataPath(), fontname);
	if (font_path == "")
	{
		VTLOG("Couldn't find font file '%s'\n", fontname);
		return;
	}
	vtFont *font = new vtFont;
	bool success = font->LoadFont(font_path);
	if (success)
		VTLOG("Successfully read font from '%s'\n", (const char *)font_path);
	else
	{
		VTLOG("Couldn't read font from file '%s', not creating labels.\n", fontname);
		return;
	}

	int text_field_index, color_field_index;

	if (!style.GetValueInt("TextFieldIndex", text_field_index))
		text_field_index = -1;
	if (!style.GetValueInt("ColorFieldIndex", color_field_index))
		color_field_index = -1;

	float label_elevation, label_size;
	if (!style.GetValueFloat("Elevation", label_elevation))
		label_elevation = 0.0f;
	if (!style.GetValueFloat("LabelSize", label_size))
		label_size = 18;

	unsigned int i;
	DPoint2 p2;
	DPoint3 p3;
	FPoint3 fp3;
	vtString str;
	RGBAf rgba;

	VTLOG("Creating %d text labels\n", features);
	for (i = 0; i < features; i++)
	{
		// Get the earth location of the label
		if (pSetP2)
			p2 = pSetP2->GetPoint(i);
		else if (pSetP3)
		{
			p3 = pSetP3->GetPoint(i);
			p2.Set(p3.x, p3.y);
		}
		else if (pSetPG)
		{
			const DPolygon2 &dp = pSetPG->GetPolygon(i);
			p2 = dp[0].Centroid();
		}

		// Don't drape on culture, but do use true elevation
		if (!pTerr->GetHeightField()->ConvertEarthToSurfacePoint(p2, fp3, 0, true))
			continue;

		// Elevate the location by the desired vertical offset
		fp3.y += label_elevation;

		// If we have a 3D point, we can use the Z component of the point
		//  to further affect the elevation.
		if (pSetP3)
			fp3.y += label_elevation;

		// Create the vtTextMesh
		if (features < 40)
			VTLOG(" Constructing TextMesh %d\n", i);
		vtTextMesh *text = new vtTextMesh(font, label_size, true);	// center

		feat.GetValueAsString(i, text_field_index, str);
#if SUPPORT_WSTRING
		// Text might be UTF-8
		wstring2 wide_string;
		wide_string.from_utf8(str);
		text->SetText(wide_string);
#else
		// Hope that it isn't
		text->SetText(str);
#endif

		// Create the vtGeom object to contain the vtTextMesh
		vtGeom *geom = new vtGeom;
		geom->SetName2(str);
		geom->SetMaterials(pLabelMats);

#if 0
		// This is the material code that we don't (apparently) need.
		int material_index;
		if (field_index_color == -1)
			material_index = common_material_index;
		else
		{
			if (GetColorField(feat, i, color_field_index, rgba))
				material_index = pLabelMats->FindByDiffuse(rgba);
			else
				material_index = common_material_index;
		}
		geom->AddTextMesh(text, material_index);
#else
		bool bColorSet = false;
		if (color_field_index != -1)
		{
			if (GetColorField(feat, i, color_field_index, rgba))
			{
				text->SetColor(rgba);
				bColorSet = true;
			}
		}
		if (!bColorSet)
			text->SetColor(RGBf(label_color));
		geom->AddTextMesh(text, common_material_index);
		text->Release();	// pass ownership to geometry
#endif

		// Add to a billboarding transform so that the labels turn
		// toward the viewer
		vtTransform *bb = new vtTransform;
		bb->AddChild(geom);
		pTerr->GetBillboardEngine()->AddTarget(bb);

		bb->SetTrans(fp3);
		pLabelGroup->AddChild(bb);
	}
	delete font;
	pLabelMats->Release();

	VTLOG("Created %d text labels\n", features);
}

void vtAbstractLayer::SetVisible(bool bVis)
{
	if (pContainer != NULL)
		pContainer->SetEnabled(bVis);
}

bool vtAbstractLayer::GetVisible()
{
	if (pContainer != NULL)
		return pContainer->GetEnabled();
	return false;
}

