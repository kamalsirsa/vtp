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
  #if VTLIB_PLIB || VTLIB_PSM
	#include <windows.h>	// PLIB and PSM require the full windows.h
  #else
	// these definitions let us include gl.h without the entire Windows headers
	#include "core/winglue.h"
  #endif
#endif

#include <GL/gl.h>

#ifdef VTLIB_PSM
  #include "vtpsm/BasePSM.h"
#else
  //
  // Base classes from which the API-specific implementation
  // classes are derived.
  //
  #include "core/Base.h"
#endif

//
// API-specific headers
//

////////////////////////////// PSM ///////////////////////////////
#if VTLIB_PSM

	// the following symbols must be allowed to have PSM definitions
	#undef DECLARE_CLASS
	#undef IMPLEMENT_CLASS
	#undef Ref

	#define SUPPORT_XFROG 0
	#define PV_OGL

	#include "psm.h"
	#include "psutil.h"

	#include "vtpsm/MathPSM.h"
	#include "vtpsm/ImagePSM.h"
	#include "vtpsm/MeshMat.h"
	#include "vtpsm/NodePSM.h"
	#include "vtpsm/ScenePSM.h"

	// the following symbols must be allowed to have non-PSM meanings later
	#undef DECLARE_CLASS
	#undef IMPLEMENT_CLASS
	#undef Ref

#endif	// PSM

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
	#include <osg/Texture2D>
	#include <osg/MatrixTransform>
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


