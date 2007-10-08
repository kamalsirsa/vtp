//
// AbstractLayer.cpp
//
// Copyright (c) 2006-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Terrain.h"

#include "AbstractLayer.h"

#include "vtdata/Features.h"	// for vtFeatureSet
#include "vtdata/vtLog.h"


vtAbstractLayer::vtAbstractLayer()
{
	pSet = NULL;
	pContainer = NULL;
	pGeomGroup = NULL;
	pLabelGroup = NULL;
	pMultiTexture = NULL;

	pLabelMats = NULL;
	pFont = NULL;
}

vtAbstractLayer::~vtAbstractLayer()
{
	delete pSet;
	delete pFont;
	ReleaseGeometry();
	if (pLabelMats)
		pLabelMats->Release();
	if (pContainer)
	{
		pContainer->GetParent()->RemoveChild(pContainer);
		pContainer->Release();
	}
	delete pMultiTexture;
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
 * Release all the 3D stuff created for the layer (including geometry and labels).
 */
void vtAbstractLayer::ReleaseGeometry()
{
	if (pGeomGroup)
	{
		pContainer->RemoveChild(pGeomGroup);
		pGeomGroup->Release();
		pGeomGroup = NULL;
	}
	if (pLabelGroup)
	{
		pContainer->RemoveChild(pLabelGroup);
		pLabelGroup->Release();
		pLabelGroup = NULL;
	}
}

/**
 * Given a featureset and style description, create the geometry and place it
 * on the terrain.
 *
 * \param pTerr The terrain to drape on.
 */
void vtAbstractLayer::CreateStyledFeatures(vtTerrain *pTerr)
{
	VTLOG1("CreateStyledFeatures\n");
	if (!pContainer)
	{
		// first time
		pContainer = new vtGroup;
		pContainer->SetName2("Abstract Layer");

		// Abstract geometry goes into the scale features group, so it will be
		//  scaled up/down with the vertical exaggeration.
		pTerr->GetScaledFeatures()->AddChild(pContainer);
	}

	vtTagArray &style = pSet->GetProperties();

	if (style.GetValueBool("ObjectGeometry"))
		CreateObjectGeometry(pTerr);

	if (style.GetValueBool("LineGeometry"))
		CreateLineGeometry(pTerr);

	if (style.GetValueBool("Labels"))
		CreateFeatureLabels(pTerr);

	if (style.GetValueBool("TextureOverlay"))
		CreateTextureOverlay(pTerr);
}

/**
	Given a featureset and style description, create geometry objects (such as
	spheres) and place them on the terrain.
	If 2D, they will be draped on the terrain.

	\param pTerr The terrain to drape on.
*/
void vtAbstractLayer::CreateObjectGeometry(vtTerrain *pTerr)
{
	VTLOG1("  CreateObjectGeometry\n");
	vtTagArray &style = pSet->GetProperties();

	vtHeightField3d *hf = pTerr->GetHeightField();

	// for GetValueFloat below
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// We support geometry for 2D and 3D polylines
	if (!pSetP2 && !pSetP3 && !pSetLS3)
		return;

	// shorthand ref
	vtFeatureSet &feat = *pSet;

	// create geometry group to contain all the meshes
	if (!pGeomGroup)
	{
		pGeomGroup = new vtGroup;
		pGeomGroup->SetName2("Geometry");
		pContainer->AddChild(pGeomGroup);
	}

	// Create materials.
	vtMaterialArray *pMats = new vtMaterialArray;

	// common color
	RGBi color = style.GetValueRGBi("ObjectGeomColor");
	int common_material_index = pMats->AddRGBMaterial1(color, true, true);

	// If each feature has its own color, we need to create those materials
	int color_field_index;
	RGBAf rgba;
	if (!style.GetValueInt("ObjectColorFieldIndex", color_field_index))
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
					pMats->AddRGBMaterial1(rgb, true, true);
				}
			}
		}
	}

	vtGeom *geom = new vtGeom;
	geom->SetName2("Objects");
	geom->SetMaterials(pMats);
	pMats->Release();	// pass ownership

	float fHeight;
	if (!style.GetValueFloat("ObjectGeomHeight", fHeight))
		fHeight = 1;

	float fRadius;
	if (!style.GetValueFloat("ObjectGeomSize", fRadius))
		fRadius = 1;

	int res = 3;

	// If a large number of entities, make as simple geometry as possible
	bool bTetrahedra = (feat.GetNumEntities() > 10000);

	int material_index;
	FPoint3 f3;
	VTLOG("  Creating %d entities.. ", feat.GetNumEntities());
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

		FPoint3 p3;
		if (pSetP2)
		{
			const DPoint2 &epos = pSetP2->GetPoint(i);
			hf->ConvertEarthToSurfacePoint(epos, p3);
			p3.y += fHeight;

			vtMesh *mesh = new vtMesh(vtMesh::TRIANGLE_STRIP, VT_Normals, res*res*2);
			mesh->CreateEllipsoid(p3, FPoint3(fRadius, fRadius, fRadius), res);

			geom->AddMesh(mesh, material_index);
			mesh->Release();
		}
		else if (pSetP3)
		{
			const DPoint3 &epos = pSetP3->GetPoint(i);
			float original_z = epos.z;
			hf->m_Conversion.ConvertFromEarth(epos, p3);

			bool bShaded = true;
			vtMesh *mesh;
			if (bTetrahedra)
			{
				mesh = new vtMesh(vtMesh::TRIANGLES, bShaded ? VT_Normals : 0, 12);
				mesh->CreateTetrahedron(p3, fRadius);
				if (bShaded)
					mesh->SetNormalsFromPrimitives();
			}
			else
			{
				mesh = new vtMesh(vtMesh::TRIANGLE_STRIP, bShaded ? VT_Normals : 0, res*res*2);
				mesh->CreateEllipsoid(p3, FPoint3(fRadius, fRadius, fRadius), res);
			}

			geom->AddMesh(mesh, material_index);
			mesh->Release();
		}
		else if (pSetLS3)
		{
			const DLine3 &dline = pSetLS3->GetPolyLine(i);
			for (unsigned int j = 0; j < dline.GetSize(); j++)
			{
				// preserve 3D point's elevation: don't drape
				hf->m_Conversion.ConvertFromEarth(dline[j], p3);

				vtMesh *mesh = new vtMesh(vtMesh::TRIANGLE_STRIP, VT_Normals, res*res*2);
				mesh->CreateEllipsoid(p3, FPoint3(fRadius, fRadius, fRadius), res);

				geom->AddMesh(mesh, material_index);
				mesh->Release();
			}
		}
		pTerr->ProgressCallback(i * 100 / feat.GetNumEntities());
	}
	pGeomGroup->AddChild(geom);
	VTLOG1("Done.\n");
}

/**
	Given a featureset and style description, create line geometry.
	If 2D, it will be draped on the terrain. Polygon features
	(vtFeatureSetPolygon) will also be created as line geometry
	(unfilled polygons) and draped on the ground.

	\param pTerr The terrain to drape on.
*/
void vtAbstractLayer::CreateLineGeometry(vtTerrain *pTerr)
{
	VTLOG1("  CreateLineGeometry\n");
	vtTagArray &style = pSet->GetProperties();

	// for GetValueFloat below
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// We support geometry for 2D and 3D polylines
	if (!pSetP3 && !pSetLS2 && !pSetLS3 && !pSetPoly)
		return;

	// shorthand ref
	vtFeatureSet &feat = *pSet;

	// create geometry group to contain all the meshes
	if (!pGeomGroup)
	{
		pGeomGroup = new vtGroup;
		pGeomGroup->SetName2("Geometry");
		pContainer->AddChild(pGeomGroup);
	}

	// Create materials.
	vtMaterialArray *pMats = new vtMaterialArray;

	// common color
	RGBi color = style.GetValueRGBi("LineGeomColor");
	int common_material_index = pMats->AddRGBMaterial1(color, false, false);

	// If each feature has its own color, we need to create those materials
	int color_field_index;
	RGBAf rgba;
	if (!style.GetValueInt("LineColorFieldIndex", color_field_index))
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
	geom->SetName2("Lines");
	pMats->Release();	// pass ownership

	vtMeshFactory mf(geom, vtMesh::LINE_STRIP, 0, 30000, 0);

	float fHeight;
	if (!style.GetValueFloat("LineGeomHeight", fHeight))
		fHeight = 1;
	bool bTessellate = style.GetValueBool("Tessellate");
	bool bCurve = false;

	int material_index;
	FPoint3 f3;
	VTLOG("  Creating %d entities.. ", feat.GetNumEntities());

	unsigned int size;
	if (pSetP3)
	{
		mf.SetMatIndex(common_material_index);
		mf.PrimStart();
		const DLine3 &dline = pSetP3->GetAllPoints();
		size = dline.GetSize();
		for (unsigned int j = 0; j < size; j++)
		{
			// preserve 3D point's elevation: don't drape
			pTerr->GetHeightField()->m_Conversion.ConvertFromEarth(dline[j], f3);
			mf.AddVertex(f3);
		}
		mf.PrimEnd();
	}
	else
	{
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
					pTerr->GetHeightField()->m_Conversion.ConvertFromEarth(dline[j], f3);
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
			pTerr->ProgressCallback(i * 100 / feat.GetNumEntities());
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
	VTLOG1("Done.\n");
}

/**
 * Given a featureset and style description, create geometry objects (such as
 * lines or spheres) and place them on the terrain.
 *
 * If the features are 2D or 3D points (vtFeatureSetPoint2D or vtFeatureSetPoint3D)
 * then the labels will be placed at those points.  If the features are 2D polygons
 * (vtFeatureSetPolygon) then the point used is the centroid of the polygon.
 *
 * \param pTerr The terrain to drape on.
 */
void vtAbstractLayer::CreateFeatureLabels(vtTerrain *pTerr)
{
	VTLOG1("  CreateFeatureLabels\n");
	vtTagArray &style = pSet->GetProperties();

	// create container group
	if (!pLabelGroup)
	{
		pLabelGroup = new vtGroup;
		pLabelGroup->SetName2("Labels");
		pContainer->AddChild(pLabelGroup);

		// Create materials.
		pLabelMats = new vtMaterialArray;

		// If they specified a font name, use it
		vtString fontfile;
		if (!style.GetValueString("Font", fontfile))
		{
			// otherwise, use the default
	#if VTLIB_OPENSG
			fontfile = "Arial.txf";
	#else
			fontfile = "Arial.ttf";
	#endif
		}
		// First, let the underlying scenegraph library try to find the font
		pFont = new vtFont;
		bool success = pFont->LoadFont(fontfile);
		if (!success)
		{
			// look on VTP data paths
			vtString vtname = "Fonts/" + fontfile;
			fontfile = FindFileOnPaths(vtGetDataPath(), vtname);
			if (fontfile != "")
				success = pFont->LoadFont(fontfile);
		}
		if (success)
			VTLOG("Successfully read font from '%s'\n", (const char *) fontfile);
		else
		{
			VTLOG("Couldn't read font from file '%s', not creating labels.\n", (const char *) fontfile);
			delete pFont;
			pFont = NULL;
			return;
		}
	}

	unsigned int features = pSet->GetNumEntities();
	VTLOG("Creating %d text labels\n", features);
	for (unsigned int i = 0; i < features; i++)
	{
		CreateFeatureLabel(pTerr, style, i);
		pTerr->ProgressCallback(i * 100 / features);
	}
}

void vtAbstractLayer::CreateFeatureLabel(vtTerrain *pTerr, vtTagArray &style,
										 unsigned int iIndex)
{
	// We support text labels for 2D and 3D points, and 2D polygons
	if (!pSetP2 && !pSetP3 && !pSetPoly)
		return;

	// Must have a font to make a label
	if (!pFont)
		return;

	// for GetValueFloat below
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// Get the earth location of the label
	DPoint2 p2;
	DPoint3 p3;
	FPoint3 fp3;
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
	if (!pTerr->GetHeightField()->ConvertEarthToSurfacePoint(p2, fp3, 0, true))
		return;

	float label_elevation;
	if (!style.GetValueFloat("LabelHeight", label_elevation))
		label_elevation = 0.0f;

	// Elevate the location by the desired vertical offset
	fp3.y += label_elevation;

	// If we have a 3D point, we can use the Z component of the point
	//  to further affect the elevation.
	if (pSetP3)
		fp3.y += label_elevation;

	float label_size;
	if (!style.GetValueFloat("LabelSize", label_size))
		label_size = 18;

	// Create the vtTextMesh
	vtTextMesh *text = new vtTextMesh(pFont, label_size, true);	// center

	// Get the label text
	int text_field_index;
	if (!style.GetValueInt("TextFieldIndex", text_field_index))
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

	// Create the vtGeom object to contain the vtTextMesh
	vtGeom *geom = new vtGeom;
	geom->SetName2(str);
	geom->SetMaterials(pLabelMats);

	// Determine feature color
	bool bGotColor = false;
	int color_field_index;
	if (style.GetValueInt("ColorFieldIndex", color_field_index))
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
		RGBf rgb = style.GetValueRGBi("LabelColor");
		text->SetColor(rgb);
	}

	geom->AddTextMesh(text, -1);
	text->Release();	// pass ownership to geometry

	// Add to a billboarding transform so that the labels turn
	// toward the viewer
	vtTransform *bb = new vtTransform;
	bb->AddChild(geom);
	pTerr->GetBillboardEngine()->AddTarget(bb);

	bb->SetTrans(fp3);
	pLabelGroup->AddChild(bb);
}

bool vtAbstractLayer::CreateTextureOverlay(vtTerrain *pTerr)
{
	VTLOG1("  CreateTextureOverlay\n");

	vtTagArray &style = pSet->GetProperties();

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
	RGBAi LayerColour = style.GetValueRGBi("GeomColor");
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
	vtString mode = style.GetValueString("TextureMode");
	if (mode == "ADD") iTextureMode = GL_ADD;
	if (mode == "MODULATE") iTextureMode = GL_MODULATE;
	if (mode == "DECAL") iTextureMode = GL_DECAL;
	pMultiTexture = pTerr->AddMultiTextureOverlay(image, DataExtents, iTextureMode);
	return true;
}

void vtAbstractLayer::SetVisible(bool bVis)
{
	if (pContainer != NULL)
		pContainer->SetEnabled(bVis);

	if (pMultiTexture)
		pMultiTexture->m_pNode->EnableMultiTexture(pMultiTexture, bVis);
}

bool vtAbstractLayer::GetVisible()
{
	if (pContainer != NULL)
		return pContainer->GetEnabled();

	else if (pMultiTexture)
		return pMultiTexture->m_pNode->MultiTextureIsEnabled(pMultiTexture);

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

