//
// Main header for VT library, for all platforms
//

#ifdef _MSC_VER
#pragma warning(disable: 4786)	// prevent common warning about templates
#endif

// frequently used standard headers
#include <stdio.h>

// vtdata headers
#include "vtdata/MathTypes.h"

// OpenGL headers
#if WIN32
  #if VTLIB_PLIB
	#include <windows.h>	// PLIB requires the full windows.h
  #else
	// these definitions let us include gl.h without the entire Windows headers
	#include "core/winglue.h"
  #endif
#endif

#include <GL/gl.h>

//
// Base classes from which the API-specific implementation
// classes are derived.
//
#include "core/Base.h"

//
// API-specific headers
//

////////////////////////////// DSM ///////////////////////////////
#if VTLIB_DSM
	#define SUPPORT_XFROG 0
	#define ISM_OGL
	#ifdef _DEBUG
	#define ISM_NOTHREAD 1
	#else
	#define ISM_NOTHREAD 0
	#endif

	#include "ismcore.h"
	#include "ismplugins.h"
	#include "uiplugins.h"
	#include "utils/VCursor.h"
	#include "utils/flyer.h"

	#include "vtdsm/ImageDSM.h"
	#include "vtdsm/MeshMat.h"
	#include "vtdsm/Movable.h"
	#include "vtdsm/SceneDSM.h"
#endif	// DSM

////////////////////////////// PLIB/SSG ////////////////////////////
#if VTLIB_PLIB
	#include "ssg.h"

	#include "vtplib/ImageSSG.h"
	#include "vtplib/MeshMat.h"
	#include "vtplib/NodeSSG.h"
	#include "vtplib/SceneSSG.h"
  #ifdef _MSC_VER
	#pragma comment(lib, "sg.lib")
	#pragma comment(lib, "ssg.lib")
	#pragma comment(lib, "ul.lib")
  #endif
#endif	// PLIB

///////////////////////////////// OSG //////////////////////////////
#if VTLIB_OSG
	#include <osgUtil/SceneView>
	#include <osg/Billboard>
	#include <osg/GeoSet>
	#include <osg/Image>
	#include <osg/LOD>
	#include <osg/Material>
	#include <osg/Texture>
	#include <osg/Transparency>
	#include <osg/Transform>
	#include <osg/Vec2>

	#include "vtosg/ImageOSG.h"
	#include "vtosg/MathOSG.h"
	#include "vtosg/MeshMat.h"
	#include "vtosg/NodeOSG.h"
	#include "vtosg/SceneOSG.h"
#endif // OSG

/////////////////////////////// SGL ////////////////////////////
#if VTLIB_SGL
	#include <sgl/sgl.h>
	#include <sglu/sglu.h>
	#include <sgl/sglVector.hpp>
	#include <sgl/sglMatrix.hpp>
	#include <sgl/sglPlane.hpp>

	#include <sgl/sglBillboard.hpp>
	#include <sgl/sglScene.hpp>
	#include <sgl/sglGeode.hpp>
	#include <sgl/sglMonoIndexedLineSet.hpp>
	#include <sgl/sglMonoIndexedLineStripSet.hpp>
	#include <sgl/sglMonoIndexedPointSet.hpp>
	#include <sgl/sglMonoIndexedQuadSet.hpp>
	#include <sgl/sglMonoIndexedQuadStripSet.hpp>
	#include <sgl/sglMonoIndexedTriangleFanSet.hpp>
	#include <sgl/sglMonoIndexedTriangleSet.hpp>
	#include <sgl/sglMonoIndexedTriangleStripSet.hpp>
	#include <sgl/sglTexture2D.hpp>
	#include <sgl/sglTexEnv.hpp>
	#include <sgl/sglTexGen.hpp>
	#include <sgl/sglTransform.hpp>
	#include <sgl/sglLineSet.hpp>
	#include <sgl/sglLOD.hpp>
	#include <sgl/sglMaterial.hpp>
	#include <sgl/sglMat4.hpp>
	#include <sgl/sglViewPlatform.hpp>
	#include <sgl/sglDirectionalLight.hpp>
	#include <sgl/sglView.hpp>

	#include "vtsgl/ImageSGL.h"
	#include "vtsgl/MathSGL.h"
	#include "vtsgl/MeshMat.h"
	#include "vtsgl/NodeSGL.h"
	#include "vtsgl/SceneSGL.h"
#endif	// SGL

