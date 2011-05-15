//
// Base.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_BASEH
#define VTLIB_BASEH

#include "vtdata/DataPath.h"

class vtCamera;
class vtGroup;
class vtEngine;
struct vtMouseEvent;

/** \defgroup sg Scene Graph
 * These classes are used for scene graph functionality: geometry, cameras,
 * rendering, LOD, and simulation.
 */
/*@{*/

/**
 * This class simply provides the ability to store whether an object is
 * "enabled" or not.  This is generally useful, such as for Nodes or Engines
 * which can be turned on and off.
 */
class vtEnabledBase
{
public:
	vtEnabledBase() { m_bEnabled = true; }

	virtual void SetEnabled(bool bOn) { m_bEnabled = bOn; }
	bool GetEnabled() { return m_bEnabled; }

protected:
	bool m_bEnabled;
};

/** This class is a placeholder parent class for all objects which can
 * be the target of an Engine (vtEngine).  Given a vtTarget point, you
 * can use dynamic_cast<> to downcast safely to any specific child class.
 */
class vtTarget
{
public:
	// need at least one method to make this class polymorphic
	virtual void PlaceHolder() {}
};

// Vertex values
#define VT_Normals		1
#define VT_Colors		2
#define VT_TexCoords	4

// Visibility return codes
#define	VT_Visible	1
#define	VT_AllVisible	2

/**
 * This class provides basic methods for the vtMesh class.
 */
class vtMeshBase
{
public:
	enum PrimType
	{
		POINTS,
		LINES,
		LINE_STRIP,
		TRIANGLES,
		TRIANGLE_STRIP,
		TRIANGLE_FAN,
		QUADS,
		QUAD_STRIP,
		POLYGON
	};
	vtMeshBase(PrimType ePrimType, int VtxType, int NumVertices)
	{
		m_ePrimType = ePrimType;
		m_iVtxType = VtxType;
		m_iMatIdx = -1;
	}

	// Accessors
	virtual unsigned int GetNumVertices() const = 0;
	PrimType GetPrimType() const { return m_ePrimType; }
	int GetVtxType() const { return m_iVtxType; }
	virtual void GetBoundBox(FBox3 &box) const;

	void SetMatIndex(int i) { m_iMatIdx = i; }
	int GetMatIndex() const { return m_iMatIdx; }

	// Adding vertices
	int AddVertex(float x, float y, float z);
	int AddVertexN(float x, float y, float z, float nx, float ny, float nz);
	int AddVertexUV(float x, float y, float z, float u, float v);

	int AddVertex(const FPoint3 &p);
	int AddVertexN(const FPoint3 &p, const FPoint3 &n);
	int AddVertexUV(const FPoint3 &p, float u, float v);
	int AddVertexUV(const FPoint3 &p, const FPoint2 &uv);
	int AddVertexNUV(const FPoint3 &p, const FPoint3 &n, const FPoint2 &uv);

	// Adding primitives
	virtual void AddTri(int p0, int p1, int p2) = 0;
	virtual void AddFan(int p0, int p1, int p2, int p3 = -1, int p4 = -1, int p5 = -1) = 0;
	virtual void AddFan(int *idx, int iNVerts) = 0;
	void AddStrip2(int iNVerts, int iStartIndex);
	virtual void AddStrip(int iNVerts, unsigned short *pIndices) = 0;
	virtual void AddQuad(int p0, int p1, int p2, int p3) = 0;

	// Access vertex properties
	virtual void SetVtxPos(unsigned int, const FPoint3&) = 0;
	virtual FPoint3 GetVtxPos(unsigned int i) const = 0;

	virtual void SetVtxNormal(unsigned int, const FPoint3&) = 0;
	virtual FPoint3 GetVtxNormal(unsigned int i) const = 0;

	virtual void SetVtxColor(unsigned int, const RGBAf&) = 0;
	virtual RGBAf GetVtxColor(unsigned int i) const = 0;

	virtual void SetVtxTexCoord(unsigned int, const FPoint2&) = 0;
	virtual FPoint2 GetVtxTexCoord(unsigned int i) const = 0;

	void SetVtxPUV(unsigned int i, const FPoint3 &pos, float u, float v)
	{
		SetVtxPos(i, pos);
		SetVtxTexCoord(i, FPoint2(u, v));
	}
	void SetVtxPN(unsigned int i, const FPoint3 &pos, const FPoint3 &norm)
	{
		SetVtxPos(i, pos);
		SetVtxNormal(i, norm);
	}

	void TransformVertices(const FMatrix4 &mat);

	void CreateEllipsoid(const FPoint3 &center, const FPoint3 &size,
		int res, bool hemi = false, bool bNormalsIn = false);
	void CreateBlock(const FPoint3& size);
	void CreateOptimizedBlock(const FPoint3& size);
	void CreatePrism(const FPoint3 &base, const FPoint3 &vector_up,
					 const FPoint2 &size1, const FPoint2 &size2);
	void CreateRectangularMesh(int xsize, int ysize, bool bReverseNormals = false);
	void CreateCylinder(float height, float radius, int res,
		bool bTop = true, bool bBottom = true, bool bCentered = true,
		int direction = 1);
	void CreateTetrahedron(const FPoint3 &center, float fRadius);

	void AddRectangleXZ(float xsize, float zsize);
	void AddRectangleXY(float x, float y, float xsize, float ysize,
		float z=0.0f, bool bCentered=false);

	void CreateConicalSurface(const FPoint3 &tip, double radial_angle,
							  double theta1, double theta2,
							  double r1, double r2, int res = 40);
	void CreateRectangle(int iQuads1, int iQuads2,
		int Axis1, int Axis2, int Axis3,
		const FPoint2 &min1, const FPoint2 &max1, float fLevel, float fTiling);

protected:
	enum PrimType m_ePrimType;
	int m_iVtxType;
	int m_iMatIdx;
};

/**
 * This class represents a window, an area of the display which corresponds
 * to an OpenGL drawing surface and has the potential to receive UI events.
 */
class vtWindow
{
public:
	vtWindow();

	/// Set the background color for the window.
	void SetBgColor(const RGBf &color) { m_BgColor = color; }
	RGBf GetBgColor() { return m_BgColor; }

	void SetSize(int w, int h);
	IPoint2 GetSize() { return m_Size; }

protected:
	RGBf m_BgColor;
	IPoint2 m_Size;
};

/**
 * vtSceneBase provides the Scene methods that are common across all
 * 3D libraries supported by vtlib.
 * See vtScene for an overview of the Scene class.
 */
class vtSceneBase
{
public:
	vtSceneBase();
	virtual ~vtSceneBase();

	/// Set the top engine in the Engine graph
	void SetRootEngine(vtEngine *ptr) { m_pRootEngine = ptr; }

	/// Get the top engine in the Engine graph
	vtEngine *GetRootEngine() { return m_pRootEngine; }

	/// Set the top engine in the Engine graph
	void SetPostDrawEngine(vtEngine *ptr) { m_pRootEnginePostDraw = ptr; }

	/// Get the top engine in the Engine graph
	vtEngine *GetPostDrawEngine() { return m_pRootEnginePostDraw; }

	/// Add an Engine to the scene. (for backward compatibility only)
	void AddEngine(vtEngine *ptr);

	/// Inform all engines in the scene that a target no longer exists
	void TargetRemoved(vtTarget *tar);

	/** Set the camera associated with this scene.  The scene has
	 * a default camera already supplied; you can use GetCamera()
	 * to simply use it instead of making your own.
	 */
	void SetCamera(vtCamera *cam) { m_pCamera = cam; }
	/// Get the camera associated with the scene.
	vtCamera *GetCamera() { return m_pCamera; }

	void OnMouse(vtMouseEvent &event, vtWindow *pWindow = NULL);
	void OnKey(int key, int flags, vtWindow *pWindow = NULL);
	void SetKeyStates(bool *piKeyState) { m_piKeyState = piKeyState; }
	bool GetKeyState(int key);

	virtual void SetWindowSize(int w, int h, vtWindow *pWindow = NULL);
	IPoint2 GetWindowSize(vtWindow *pWindow = NULL);

	virtual void SetRoot(vtGroup *pRoot) = 0;
	/// Get the root node, which is the top of the scene graph.
	vtGroup *GetRoot() { return m_pRoot; }

	// Windows
	void AddWindow(vtWindow *pWindow) {
		m_Windows.Append(pWindow);
	}
	vtWindow *GetWindow(unsigned int i) {
		if (m_Windows.GetSize() > i)
			return m_Windows[i];
		else
			return NULL;
	}

	// all data will be looked for on the global data path
	void SetDataPath(const vtStringArray &paths) { vtSetDataPath(paths); }

protected:
	void DoEngines(vtEngine *eng);

	vtArray<vtWindow*> m_Windows;
	vtCamera	*m_pCamera;
	vtGroup		*m_pRoot;
	vtEngine	*m_pRootEngine;
	vtEngine	*m_pRootEnginePostDraw;
	bool		*m_piKeyState;

	vtCamera	*m_pDefaultCamera;
	vtWindow	*m_pDefaultWindow;
};

/*@}*/	// Group sg

#endif	// VTLIB_BASEH

