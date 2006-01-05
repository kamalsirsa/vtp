//
// StructureShadowsOSG.h
//
// Copyright (c) 2005-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef STRUCTURESHADOWSOSG_H
#define STRUCTURESHADOWSOSG_H

#include <osg/Vec3>
#include <osg/TexGenNode>
#include <osg/PolygonOffset>

class CStructureShadowsOSG : public osg::Referenced
{
public:
	CStructureShadowsOSG(bool bDepthShadow = false, bool bStructureOnStructureShadows = false);
	~CStructureShadowsOSG();

	void SetSunPosition(const osg::Vec3 &sunPosition, bool bForceRecompute = false);
	bool Initialise(osgUtil::SceneView *pSceneView, osg::Node *pStructures, osg::Node *pShadowed, const int iResolution);
	void SetShadowDarkness(float fDarkness);
	void SetPolygonOffset(float fFactor, float fUnits);
	void ExcludeFromShadower(osg::Node *pNode, bool bExclude);

	// Strip out any modelling transforms from the model view matrix
	class EyeLinearTexGen : public osg::TexGen
	{
	public:
		void setMatrix(const osg::Matrix& Matrix)
		{
			m_Matrix = Matrix;
		}

		virtual void apply(osg::State& state) const
		{
			glPushMatrix();
			glLoadMatrix(m_Matrix.ptr());
			TexGen::apply(state);
			glPopMatrix();
		}
	protected:
		osg::Matrix m_Matrix;
	};

protected:
	osg::ref_ptr<osg::CameraNode> m_pCameraNode;
	osg::ref_ptr<osg::Texture2D> m_pTexture;
	osg::ref_ptr<osg::Material> m_pMaterial;
	osg::ref_ptr<osgUtil::SceneView> m_pSceneView;
	osg::ref_ptr<osg::Node> m_pShadowed;
	osg::ref_ptr<osg::TexGenNode> m_pTerrainTexGenNode;
	osg::ref_ptr<osg::Group> m_pTop;
	osg::ref_ptr<osg::Uniform> m_pAmbientBias;
	osg::ref_ptr<osg::PolygonOffset> m_pPolygonOffset;
	float m_fShadowDarkness;
	bool m_bDepthShadow;
	bool m_bStructureOnStructureShadows;
	osg::Vec3 m_SunPosition;
	static osg::TexGenNode *m_pStructureTexGenNode;
	int m_iTargetResolution;
	int m_iCurrentResolution;
	bool m_bUsingLiveFrameBuffer;
	std::list<osg::Node*> m_ExcludeFromShadower;
	float m_fPolygonOffsetFactor;
	float m_fPolygonOffsetUnits;
};

#endif // STRUCTURESHADOWSOSG_H
