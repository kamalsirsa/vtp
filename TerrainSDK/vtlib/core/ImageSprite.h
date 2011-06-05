//
// ImageSprite.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_IMAGESPRITE
#define VTLIB_IMAGESPRITE

/**
 * This class which contains a geometry with a single textured rectangle
 *  mesh.  It is particularly useful in conjunction with vtHUD, for
 *  superimposing a single image on the window.
 */
class vtImageSprite : public osg::Referenced
{
public:
	vtImageSprite();
	~vtImageSprite();
	bool Create(const char *szTextureName, bool bBlending = false);
	bool Create(vtImage *pImage, bool bBlending = false);
	IPoint2 GetSize() const { return m_Size; }
	void SetPosition(float l, float t, float r, float b, float rot = 0.0f);
	void SetImage(vtImage *image);
	vtGeode *GetGeode() const { return m_pGeom; }

protected:
	vtMaterialArrayPtr m_pMats;
	vtGeode *m_pGeom;
	vtMesh *m_pMesh;
	IPoint2 m_Size;
};

#endif // VTLIB_IMAGESPRITE
