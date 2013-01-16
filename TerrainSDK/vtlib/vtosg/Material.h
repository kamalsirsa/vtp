//
// Material.h for OSG
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_MATERIAL
#define VTOSG_MATERIAL

#include <osg/BlendFunc>
#include <osg/Material>
#include <osg/AlphaFunc>

// Shorthand
#define FAB		osg::Material::FRONT_AND_BACK

/** \addtogroup sg */
/*@{*/

typedef osg::ref_ptr<osg::Image> ImagePtr;
typedef osg::ref_ptr<osg::Texture2D> TexturePtr;

/**
 A material is a description of how geometry (typically, a surface) should be
 rendered.  For a good description of how Materials work, see the opengl.org
 website or the OpenGL Red Book.

 vtlib's concept of a material is slightly more broad than OpenGL's, because
 it includes texture and transparency (alpha).  It maps closely to OSG's "state
 set", though that is even broader in scope.
 */
class vtMaterial : public osg::StateSet
{
public:
	vtMaterial();

	void CopyFrom(vtMaterial *pFrom);

	void SetDiffuse(float r, float g, float b, float a = 1.0f);
	RGBAf GetDiffuse() const;

	void SetSpecular(float r, float g, float b);
	RGBf GetSpecular() const;

	void SetAmbient(float r, float g, float b);
	RGBf GetAmbient() const;

	void SetEmission(float r, float g, float b);
	RGBf GetEmission() const;

	void SetCulling(bool bCulling);
	bool GetCulling() const;

	void SetLighting(bool bLighting);
	bool GetLighting() const;

	void SetTransparent(bool bOn, bool bAdd = false);
	bool GetTransparent() const;

	void SetWireframe(bool bOn);
	bool GetWireframe() const;

	void SetTexture(osg::Image *pImage, int unit = 0);
	osg::Image	*GetTexture(int unit = 0) const;
	void ModifiedTexture(int unit = 0);

	void SetTexGen(const FPoint2 &scale, const FPoint2 &offset,
		int iTextureMode, int unit = 0);

	void SetClamp(bool bClamp, int unit = 0);
	bool GetClamp(int unit = 0) const;

	void SetMipMap(bool bMipMap, int unit = 0);
	bool GetMipMap(int unit = 0) const;

	void SetDiffuse(const RGBAf &c) { SetDiffuse(c.r, c.g, c.b, c.a); }
	void SetDiffuse(float f) { SetDiffuse(f, f, f); }

	void SetSpecular(const RGBf &c) { SetSpecular(c.r, c.g, c.b); }
	void SetSpecular(float f) { SetSpecular(f, f, f); }

	void SetAmbient(const RGBf &c) { SetAmbient(c.r, c.g, c.b); }
	void SetAmbient(float f) { SetAmbient(f, f, f); }

	void SetEmission(const RGBf &c) { SetEmission(c.r, c.g, c.b); }
	void SetEmission(float f) { SetEmission(f, f, f); }

	// global option
	static bool s_bTextureCompression;

	// remember any texture images, by unit, for convenience and referencing
	std::vector<ImagePtr> m_Images;

	std::vector<TexturePtr> m_Textures;

	// the VT material object includes texture
	osg::ref_ptr<osg::Material>		m_pMaterial;
	osg::ref_ptr<osg::BlendFunc>	m_pBlendFunc;
	osg::ref_ptr<osg::AlphaFunc>	m_pAlphaFunc;
};
typedef osg::ref_ptr<vtMaterial> vtMaterialPtr;

/**
 * Contains an array of materials.  Provides useful methods for creating material easily.
 */
class vtMaterialArray : public std::vector<vtMaterialPtr>, public osg::Referenced
{
public:
	int Find(vtMaterial *mat);
	int AddTextureMaterial(osg::Image *pImage,
						   bool bCulling, bool bLighting,
						   bool bTransp = false, bool bAdditive = false,
						   float fAmbient = 0.0f, float fDiffuse = 1.0f,
						   float fAlpha = 1.0f, float fEmissive = 0.0f,
						   bool bClamp = false, bool bMipMap = false);
	int AddTextureMaterial(const char *fname,
						   bool bCulling, bool bLighting,
						   bool bTransp = false, bool bAdditive = false,
						   float fAmbient = 0.0f, float fDiffuse = 1.0f,
						   float fAlpha = 1.0f, float fEmissive = 0.0f,
						   bool bClamp = false, bool bMipMap = false);
	int AddRGBMaterial(const RGBf &diffuse, const RGBf &ambient,
					   bool bCulling = true, bool bLighting= true,
					   bool bWireframe = false, float fAlpha = 1.0f,
					   float fEmissive = 0.0f);
	int AddRGBMaterial(const RGBf &diffuse,
					   bool bCulling = true, bool bLighting= true,
					   bool bWireframe = false, float fAlpha = 1.0f,
					   float fEmissive = 0.0f);
	int FindByDiffuse(const RGBAf &rgba) const;
	int FindByImage(const osg::Image *image) const;

	void CopyFrom(vtMaterialArray *pFromMats);

	void RemoveMaterial(vtMaterial *pMat);

	/// Adds a material to this material array.
	int AppendMaterial(vtMaterial *pMat);
};
typedef osg::ref_ptr<vtMaterialArray> vtMaterialArrayPtr;

/*@}*/	// Group sg

#endif // VTOSG_MATERIAL

