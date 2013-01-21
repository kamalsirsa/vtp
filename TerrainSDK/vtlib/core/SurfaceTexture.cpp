//
// SurfaceTexture.cpp
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "SurfaceTexture.h"
#include "vtdata/DataPath.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtLog.h"


///////////////////

void SurfaceTexture::LoadTexture(const TParams &options, const vtHeightFieldGrid3d *pHFGrid,
	bool progress_callback(int))
{
	const TextureEnum eTex = options.GetTextureEnum();

	VTLOG("LoadTexture(%d)\n", eTex);

	if (eTex == TE_SINGLE)
		LoadSingleTexture(options);
	else if (eTex == TE_DERIVED)
		MakeDerivedTexture(options, pHFGrid, progress_callback);

	if (m_pUnshadedImage.get() == NULL)	// none or failed to find texture
	{
		// no texture: create plain white material
		m_pMaterials->AddRGBMaterial(RGBf(1.0f, 1.0f, 1.0f),
			RGBf(0.2f, 0.2f, 0.2f), true, false);
		return;
	}

	// If the user has asked for 16-bit textures to be sent down to the
	//  card (internal memory format), then tell this Image
	Set16BitInternal(m_pUnshadedImage, options.GetValueBool(STR_REQUEST16BIT));

	CopyFromUnshaded(options);

	const bool bTransp = (GetDepth(m_pTextureImage) == 32);
	const bool bMipmap = options.GetValueBool(STR_MIPMAP);
	const bool bBothSides = options.GetValueBool(STR_SHOW_UNDERSIDE);
	const float ambient = 0.0f, diffuse = 1.0f, emmisive = 0.0f;

	m_pMaterials->AddTextureMaterial(m_pTextureImage,
		!bBothSides,	// culling
		false,			// lighting
		bTransp,		// transparency blending
		false,			// additive
		ambient, diffuse,
		1.0f,			// alpha
		0.0f,			// emmisive,
		false,			// clamp
		bMipmap);
}

void SurfaceTexture::LoadSingleTexture(const TParams &options)
{
	// look for texture
	vtString texture_fname = "GeoSpecific/";
	texture_fname += options.GetValueString(STR_TEXTUREFILE);

	VTLOG("  Looking for single texture: %s\n", (const char *) texture_fname);
	vtString texture_path = FindFileOnPaths(vtGetDataPath(), texture_fname);
	if (texture_path == "")
	{
		// failed to find texture
		VTLOG("  Failed to find texture.\n");
		return;
	}
	VTLOG("  Found texture, path is: %s\n", (const char *) texture_path);

	clock_t r1 = clock();
	m_pUnshadedImage = osgDB::readImageFile((const char *)texture_path);
	if (m_pUnshadedImage.valid())
	{
		VTLOG("  Loaded texture: size %d x %d, depth %d, %.2f seconds.\n",
			m_pUnshadedImage->s(), m_pUnshadedImage->t(),
			m_pUnshadedImage->getPixelSizeInBits(),
			(float)(clock() - r1) / CLOCKS_PER_SEC);
	}
	else
	{
		VTLOG("  Failed to load texture.\n");
		m_pMaterials->AddRGBMaterial(RGBf(1.0f, 1.0f, 1.0f),
			RGBf(0.2f, 0.2f, 0.2f), true, false);
		//_SetErrorMessage("Failed to load texture.");
	}
}
	
void SurfaceTexture::MakeDerivedTexture(const TParams &options, const vtHeightFieldGrid3d *pHFGrid,
	bool progress_callback(int))
{
	// Derive color from elevation.
	// Determine the correct size for the derived texture: ideally as
	// large as the input grid, but not larger than the hardware texture
	// size limit.
	int tmax = vtGetMaxTextureSize();

	int cols, rows;
	pHFGrid->GetDimensions(cols, rows);

	int tsize = cols-1;
	if ((tmax > 0) && (tsize > tmax))
		tsize = tmax;
	VTLOG("\t grid width is %d, texture max is %d, creating artificial texture of dimension %d\n",
		cols, tmax, tsize);

	vtImage *vti = new vtImage;
	vti->Create(tsize, tsize, 24, false);
	m_pUnshadedImage = vti;

	MakeColorMap(options);

	clock_t r1 = clock();
	// The PaintDib method is virtual to allow subclasses to customize
	// the unshaded image.
	PaintDib(pHFGrid, progress_callback);
	VTLOG("  PaintDib: %.2f seconds.\n", (float)(clock() - r1) / CLOCKS_PER_SEC);
}
	
void SurfaceTexture::ShadeTexture(const TParams &options, const vtHeightFieldGrid3d *pHFGrid,
	const FPoint3 &light_dir, bool progress_callback(int))
{	
	if (!options.GetValueBool(STR_PRELIGHT) || !pHFGrid)
		return;

	// Apply shading (a.k.a. pre-lighting). We only have an osg::Image, so
	//  we wrap it so we can treat it like a vtBitmap.
	vtImageWrapper wrapper(m_pTextureImage);

	// for GetValueFloat below
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	VTLOG("  Prelighting texture: ");

	clock_t c1 = clock();

	float shade_factor = options.GetValueFloat(STR_PRELIGHTFACTOR);
	float ambient = 0.25f;
	float gamma = 0.80f;
	if (options.GetValueBool(STR_CAST_SHADOWS))
	{
		// A more accurate shading, still a little experimental
		pHFGrid->ShadowCastDib(&wrapper, light_dir, shade_factor, ambient, progress_callback);
	}
	//else if (bQuick)
	//	pElevGrid->ShadeQuick(bitmap, shade_factor, bTrue, progress_callback);
	else
		pHFGrid->ShadeDibFromElevation(&wrapper, light_dir, shade_factor,
			ambient, gamma, true, progress_callback);

	//clock_t c3 = c2 - c1;
	//VTLOG("%.3f seconds.\n", (float)c3 / CLOCKS_PER_SEC);	}
	//VTLOG("  Total CreateTextures: %.2f seconds.\n", (float)(clock() - c1) / CLOCKS_PER_SEC);
}

/**
  Color the texture from the elevation using the colormap.
 */
void SurfaceTexture::MakeColorMap(const TParams &options)
{
	m_pColorMap.reset(new ColorMap);

	// Use the info from the terrain parameters
	vtString name = options.GetValueString(STR_COLOR_MAP);
	if (name != "")
	{
		if (!m_pColorMap->Load(name))
		{
			// Look on data paths
			vtString name2 = "GeoTypical/";
			name2 += name;
			name2 = FindFileOnPaths(vtGetDataPath(), name2);
			if (name2 != "")
				m_pColorMap->Load(name2);
		}
	}
	// If the colors couldn't be loaded, then make up some default colors.
	if (m_pColorMap->Num() == 0)
	{
		m_pColorMap->m_bRelative = true;
		m_pColorMap->Add(0, RGBi(0x20, 0x90, 0x20));	// medium green
		m_pColorMap->Add(1, RGBi(0x40, 0xE0, 0x40));	// light green
		m_pColorMap->Add(2, RGBi(0xE0, 0xD0, 0xC0));	// tan
		m_pColorMap->Add(3, RGBi(0xE0, 0x80, 0x10));	// orange
		m_pColorMap->Add(4, RGBi(0xE0, 0xE0, 0xE0));	// light grey
	}
}

/**
  Color the texture from the elevation using the colormap.
 */
void SurfaceTexture::PaintDib(const vtHeightFieldGrid3d *pHFGrid,
	bool progress_callback(int))
{
	vtImageWrapper wrap(m_pUnshadedImage);
	pHFGrid->ColorDibFromElevation(&wrap, m_pColorMap.get(), 4000,
		RGBi(255,0,0), progress_callback);
}

void SurfaceTexture::CopyFromUnshaded(const TParams &options)
{
	if (options.GetValueBool(STR_PRELIGHT))
	{
		// We need to copy from the retained image to a second image which will
		//  be shaded and displayed.
		m_pTextureImage = new osg::Image(*m_pUnshadedImage);
	}
	else
	{
		// We won't shade, so we don't need to make a copy, we can use the original.
		m_pTextureImage = m_pUnshadedImage;
	}
}

