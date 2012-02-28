//
// VisualImpactCalculatorOSG.cpp
//
// Framework for calculating visual impact of selected geometry in a scene.
// This code calculates the position of all the pixels that a particular set of
// drawing primitives has contributed to the frame buffer at the end of a rendering pass.
// i.e. All the pixels of a particular geometry or geometries that are visible in the current view.
// It then summates the solid angle that these pixels represent using Gauss Bonnett.
// This data can be used to give a quantatative assessment of the visual impact of the geometry.
//
// Contributed by Roger James (roger@beardandsandals.co.uk) and Virtual Outlooks Limited.
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "VisualImpactCalculatorOSG.h"
#include <osg/GLExtensions>
#include "vtdata/HeightField.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/Terrain.h"
#include <gdal_priv.h>
#include "vtdata/vtLog.h"

// !!!!!!!!! I think this may need more work in a multiprocessor environment
// !!!!!!!!! If threading is not CullDrawThreadPerContext then pViewer->frame may return before
// !!!!!!!!! drawing has finished.

static const char VISUAL_IMPACT_BIN_NAME[] = "Visual Impact Bin";
static const int VISUAL_IMPACT_BIN_NUMBER = 9999; // Set this number sufficiently high so that this bin will be rendered after everything else
static const int DEFAULT_VISUAL_IMPACT_RESOLUTION = 256;
// The following are accepted standard values for the field of view of normal binocular (non peripheral) vision
static const float DEFAULT_HUMAN_FOV_ASPECT_RATIO = 1.0;
static const float DEFAULT_HUMAN_FOV_DEGREES = 120;
static const float DEFAULT_HUMAN_FOV_SOLID_ANGLE = PIf; // 2pi(1 - cos(120/2)) = pi using formula for area of spherical cap
// The standard for foveal (high detail) vision is 2 degrees and 1.0 aspect. Which is a bit too small for us.
// The standard for peripheral vision is 180 degrees horizontal 130 degrees vertical. (Some sources go as far as 210
// horizontal but vision is very limited at the extremes.)
static const unsigned int DEFAULT_GRAPHICS_CONTEXT = 0;


class MyRenderBinDrawCallback : public osgUtil::RenderBin::DrawCallback
{
public:
	MyRenderBinDrawCallback(osg::Image *pImage) : m_pImage(pImage) {}
	virtual void drawImplementation(osgUtil::RenderBin *pBin, osg::RenderInfo& renderInfo, osgUtil::RenderLeaf* &Previous);
protected:
	osg::ref_ptr<osg::Image> m_pImage;
};

class MyCullCallback : public osg::NodeCallback
{
public:
	MyCullCallback(osg::Image *pImage) : m_pImage(pImage) {}
	virtual void operator()(osg::Node* pNode, osg::NodeVisitor* pNodeVisitor)
	{
		pNode->traverse(*pNodeVisitor);
		osgUtil::CullVisitor *pCullVisitor = dynamic_cast<osgUtil::CullVisitor*>(pNodeVisitor);
		if ((NULL != pCullVisitor) &&
			(NULL != pCullVisitor->getCurrentRenderBin()) &&
			(NULL == pCullVisitor->getCurrentRenderBin()->getDrawCallback()))
		{
			pCullVisitor->getCurrentRenderBin()->setDrawCallback(new MyRenderBinDrawCallback(m_pImage.get()));
		}
	}
protected:
	osg::ref_ptr<osg::Image> m_pImage;
};

void CVisualImpactCalculatorOSG::Initialise()
{
    osgUtil::RenderBin::addRenderBinPrototype(VISUAL_IMPACT_BIN_NAME, new osgUtil::RenderBin(osgUtil::RenderBin::getDefaultRenderBinSortMode()));
	vtCamera *pCamera = vtGetScene()->GetCamera();

	m_ProjectionMatrix.makePerspective(DEFAULT_HUMAN_FOV_DEGREES, DEFAULT_HUMAN_FOV_ASPECT_RATIO, pCamera->GetHither(), pCamera->GetYon());

	osg::FBOExtensions* fbo_ext = osg::FBOExtensions::instance(DEFAULT_GRAPHICS_CONTEXT, true);
//	if ((fbo_ext && fbo_ext->isSupported()) || osg::isGLExtensionSupported(DEFAULT_GRAPHICS_CONTEXT, "ARB_render_texture"))
	if (fbo_ext && fbo_ext->isSupported())
		m_bUsingLiveFrameBuffer = false;
	else
		m_bUsingLiveFrameBuffer = true;

	m_pIntermediateImage = new osg::Image;

	m_bInitialised = true;
}


// The following pair of functions assume that the existing RenderBinMode is INHERIT_RENDERBIN_DETAILS (default)
void CVisualImpactCalculatorOSG::AddVisualImpactContributor(osg::Node *pOsgNode)
{
	if (!m_bInitialised)
		Initialise();
	if (NULL != pOsgNode)
	{
		pOsgNode->getOrCreateStateSet()->setRenderBinDetails(VISUAL_IMPACT_BIN_NUMBER, VISUAL_IMPACT_BIN_NAME);
		pOsgNode->setCullCallback(new MyCullCallback(m_pIntermediateImage.get()));
	}
}

void CVisualImpactCalculatorOSG::RemoveVisualImpactContributor(osg::Node *pOsgNode)
{
	if (!m_bInitialised)
		Initialise();
	if (NULL != pOsgNode)
	{
		osg::StateSet *pStateSet = pOsgNode->getOrCreateStateSet();
		pStateSet->setRenderBinMode(osg::StateSet::INHERIT_RENDERBIN_DETAILS);
		pStateSet->setRenderingHint(osg::StateSet::DEFAULT_BIN);
		pOsgNode->setCullCallback(NULL);
	}
}

void CVisualImpactCalculatorOSG::SetVisualImpactTarget(const FPoint3 Target)
{
	m_Target = Target;
}

const FPoint3& CVisualImpactCalculatorOSG::GetVisualImpactTarget() const
{
	return m_Target;
}


bool CVisualImpactCalculatorOSG::UsingLiveFrameBuffer()
{
	if (!m_bInitialised)
		Initialise();
	return m_bUsingLiveFrameBuffer;
}

float CVisualImpactCalculatorOSG::Calculate()
{
	m_ViewMatrix = dynamic_cast<osgViewer::Renderer*>(vtGetScene()->getViewer()->getCamera()->getRenderer())->getSceneView(0)->getViewMatrix();
	return Implementation(true);
}

bool CVisualImpactCalculatorOSG::Plot(GDALRasterBand *pRasterBand, float fScaleFactor, double dXSampleInterval, double dYSampleInterval, bool progress_callback(int))
{
	if (0.0f == Implementation(false, pRasterBand, fScaleFactor, dXSampleInterval, dYSampleInterval, progress_callback))
		return true;
	else
		return false;
}

float CVisualImpactCalculatorOSG::Implementation(bool bOneOffMode, GDALRasterBand *pRasterBand, float fScaleFactor, double dXSampleInterval, double dYSampleInterval, bool progress_callback(int))
{
	if (!m_bInitialised)
		Initialise();

	osgViewer::Viewer *pViewer = vtGetScene()->getViewer();

	// Create a new image to capture the current scene
	osg::ref_ptr<osg::Camera> pCamera = new osg::Camera;
	osg::ref_ptr<osg::Image> pImage = new osg::Image;
	if (!pCamera.valid() || !pImage.valid())
	{
		VTLOG("CVisualImpactCalculatorOSG::Implementation - Cannot create camera node or image\n");
		return -1.0f;
	}

	// Even though the camera node knows that you have attached the image to the depth buffer
	// it does not set this up correctly for you. There is no way to set the dataType, so
	// preallocation of the data is easiest
	pImage->allocateImage(DEFAULT_VISUAL_IMPACT_RESOLUTION,
							DEFAULT_VISUAL_IMPACT_RESOLUTION,
							1,
							GL_DEPTH_COMPONENT,
							GL_FLOAT);

	pCamera->setName("Visual impact calculator camera");
	pCamera->setClearColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	pCamera->setViewport(0, 0, DEFAULT_VISUAL_IMPACT_RESOLUTION, DEFAULT_VISUAL_IMPACT_RESOLUTION);
	pCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER);
	pCamera->setRenderOrder(osg::Camera::PRE_RENDER);
	pCamera->attach(osg::Camera::DEPTH_BUFFER, pImage.get());
	pCamera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
	pCamera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
	pCamera->setProjectionMatrix(m_ProjectionMatrix);
	pCamera->setViewMatrix(m_ViewMatrix);
	pCamera->setGraphicsContext(pViewer->getCamera()->getGraphicsContext());

	if (!m_pIntermediateImage.valid())
	{
		VTLOG("CVisualImpactCalculatorOSG::Implementation - Cannot create intermediate image\n");
		return -1.0f;
	}

	pViewer->addSlave(pCamera.get());

	osgViewer::Viewer::ThreadingModel ThreadingModel = pViewer->getThreadingModel();
	if (ThreadingModel > osgViewer::Viewer::CullDrawThreadPerContext)
	{
		pViewer->stopThreading();
		pViewer->setThreadingModel(osgViewer::Viewer::CullDrawThreadPerContext);
		pViewer->startThreading();
	}

	if (bOneOffMode)
	{
		pViewer->frame();

		pViewer->removeSlave(pViewer->findSlaveIndexForCamera(pCamera.get()));

		if (ThreadingModel != pViewer->getThreadingModel())
		{
			pViewer->stopThreading();
			pViewer->setThreadingModel(ThreadingModel);
			pViewer->startThreading();
		}

		return InnerImplementation(pCamera.get(), pImage.get());
	}
	else
	{
		DPoint2 CameraOrigin;
		DPoint2 CurrentCamera;
		vtHeightField3d *pHeightField = vtGetTS()->GetCurrentTerrain()->GetHeightField();
		DRECT EarthExtents = pHeightField->GetEarthExtents();

		CameraOrigin.x = EarthExtents.left;
		CameraOrigin.y = EarthExtents.bottom;

		int iCurrentY = 0;
		int iXsize = (int)((EarthExtents.right - EarthExtents.left)/dXSampleInterval);
		int iYsize = (int)((EarthExtents.top - EarthExtents.bottom)/dYSampleInterval);
		int iTotalProgress = iXsize * iYsize;

#ifdef _DEBUG
		int iBlockSizeX, iBlockSizeY;
		pRasterBand->GetBlockSize(&iBlockSizeX, &iBlockSizeY);
#endif

		for (CurrentCamera.y = CameraOrigin.y; CurrentCamera.y < EarthExtents.top; CurrentCamera.y += dYSampleInterval)
		{
			// Process horizontal scanline
			int iCurrentX = 0;
			for (CurrentCamera.x = CameraOrigin.x; CurrentCamera.x < EarthExtents.right; CurrentCamera.x += dXSampleInterval)
			{
				FPoint3 CameraTranslate;

				pHeightField->ConvertEarthToSurfacePoint(CurrentCamera, CameraTranslate);
				pCamera->setViewMatrixAsLookAt(v2s(CameraTranslate), v2s(m_Target), osg::Vec3(0.0, 1.0, 0.0));

				pViewer->frame();

				float fFactor = InnerImplementation(pCamera.get(), pImage.get());

				pRasterBand->RasterIO(GF_Write, iCurrentX, iYsize - iCurrentY - 1, 1, 1, &fFactor, 1, 1, GDT_Float32, 0, 0);

				iCurrentX++;
				if ((*progress_callback)(100 * (iCurrentY * iXsize + iCurrentX) / iTotalProgress))
				{
					pViewer->removeSlave(pViewer->findSlaveIndexForCamera(pCamera.get()));
					VTLOG("CVisualImpactCalculatorOSG::Implementation - Cancelled by user\n");
					return -1.0f;
				}
			}
			iCurrentY++;
		}
		pViewer->removeSlave(pViewer->findSlaveIndexForCamera(pCamera.get()));

		if (ThreadingModel != pViewer->getThreadingModel())
		{
			pViewer->stopThreading();
			pViewer->setThreadingModel(ThreadingModel);
			pViewer->startThreading();
		}

		return 0.0f;
	}
}

float CVisualImpactCalculatorOSG::InnerImplementation(osg::Camera *pCameraNode, osg::Image *pImage) const
{
	float fSolidAngle = 0.0f;
	// Compute the PW matrix
	osg::Matrixd PWmatrix(osg::Matrixd(pCameraNode->getProjectionMatrix()));
	PWmatrix.postMult(osg::Matrixd(pCameraNode->getViewport()->computeWindowMatrix()));

	// Compute the inverse PW matrix
	osg::Matrixd InversePWmatrix;
	InversePWmatrix.invert(PWmatrix);

	// Eye coordinates of pixel for solid angle calculation
	osg::Vec3d BottomLeft, BottomRight, TopLeft, TopRight;

	// Dihedral angles for solid angle calculation
	double d1, d2, d3, d4, d5, d6;
	// Normals for solid angle computation
	osg::Vec3d BLBR, BLTR, BRTR, BRBL, TRBL, TRBR, BLTL, TRTL, TLBL, TLTR;

	// Every pixel that has been written to by a contributing geometry should have a different depth value to
	// the one in the intermediate buffer.
	// For each one I find compute the solid angle of that patch using Gauss Bonnett and add to the sum
	float* pFinalBuffer = (float*)pImage->data();
	float* pIntermediateBuffer = (float*)m_pIntermediateImage->data();
	int x, y;
#ifdef _DEBUG
	unsigned int Hits = 0;
#endif
	if (NULL != pIntermediateBuffer) // Buffer will be NULL if our bin has not been rendered
	{
		for (x = 0; x < DEFAULT_VISUAL_IMPACT_RESOLUTION; x++)
			for (y = 0; y < DEFAULT_VISUAL_IMPACT_RESOLUTION; y++)
				if (*(pFinalBuffer + (y * DEFAULT_VISUAL_IMPACT_RESOLUTION) + x) != *(pIntermediateBuffer + (y * DEFAULT_VISUAL_IMPACT_RESOLUTION) + x))
			{
				// Get patch corners in eye coordinates
				BottomLeft = osg::Vec3d(x, y, 0.0f) * InversePWmatrix;
				BottomRight = osg::Vec3d(x + 1, y, 0.0f) * InversePWmatrix;
				TopLeft = osg::Vec3d(x, y + 1, 0.0f) * InversePWmatrix;
				TopRight = osg::Vec3d(x + 1, y + 1, 0.0f) * InversePWmatrix;
				// Split the rectangle into two triangles calculate the dihedral angles
				// First get the normals to the planes
				BLBR = BottomLeft ^ BottomRight; // Normal to plane containing BL BR Origin
				BLTR = BottomLeft ^ TopRight; // Normal to plane containing BL TR Origin
				BRTR = BottomRight ^ TopRight; // Normal to plane containing BR TR Origin
				BRBL = BottomRight ^ BottomLeft; // Normal to plane containing BR BL Origin
				TRBL = TopRight ^ BottomLeft; // Normal to plane containing TR BL Origin
				TRBR = TopRight ^ BottomRight; // Normal to plane containing TR BR Origin
				BLTL = BottomLeft ^ TopLeft; // Normal to plane containing BL TL Origin
				TRTL = TopRight ^ TopLeft; // Normal to plane containing TR TL Origin
				TLBL = TopLeft ^ BottomLeft; // Normal to plane containing TL BL Origin
				TLTR = TopLeft ^ TopRight; // Normal to plane containing TL TR Origin
				BLBR.normalize();
				BLTR.normalize();
				BRTR.normalize();
				BRBL.normalize();
				TRBL.normalize();
				TRBR.normalize();
				BLTL.normalize();
				TRTL.normalize();
				TLBL.normalize();
				TLTR.normalize();
				// Dihedral angles (angles between planes)
				d1 = acos(BLBR * BLTR);
				d2 = acos(BRTR * BRBL);
				d3 = acos(TRBL * TRBR);
				d4 = acos(BLTR * BLTL);
				d5 = acos(TRTL * TRBL);
				d6 = acos(TLBL * TLTR);
				// Gauss Bonnet gives spherical excess which is the solid angle of the patch
				fSolidAngle += d1 + d2 + d3 + d4 + d5 + d6 - PI2d;
#ifdef _DEBUG
				Hits++;
#endif
			}
	}
#ifdef _DEBUG
	float HitPercentage;
	HitPercentage = (float)Hits * 100.0f / (float)(DEFAULT_VISUAL_IMPACT_RESOLUTION * DEFAULT_VISUAL_IMPACT_RESOLUTION);
#endif
	return 100 * fSolidAngle / DEFAULT_HUMAN_FOV_SOLID_ANGLE;
}

void MyRenderBinDrawCallback::drawImplementation(osgUtil::RenderBin *pBin, osg::RenderInfo& renderInfo, osgUtil::RenderLeaf* &Previous)
{
	// Read the current depth buffer
	m_pImage->readPixels(0, 0, DEFAULT_VISUAL_IMPACT_RESOLUTION, DEFAULT_VISUAL_IMPACT_RESOLUTION, GL_DEPTH_COMPONENT, GL_FLOAT);
	// Draw all the visual impact contributors
	pBin->drawImplementation(renderInfo, Previous);
}


