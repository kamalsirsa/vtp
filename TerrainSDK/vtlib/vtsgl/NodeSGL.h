//
// vtNode - encapsulate behavior for a movable object
//

#ifndef VTSGL_NODEH
#define VTSGL_NODEH

class sglPerspectiveCamera;
class sglOrthographicCamera;


//////////////////////
// wrapper classes

class vtNode : public vtNodeBase, public vtEnabledBase
{
public:
	vtNode();

	// implement vtNodeBase methods
	vtNodeBase *CreateClone();
	void Release();

	void SetEnabled(bool bOn);
	bool GetEnabled();

	void SetName2(const char *str);
	const char *GetName2();

	void GetBoundBox(FBox3 &box);
	void GetBoundSphere(FSphere &sphere);

	int GetTriCount() { return 0; }

	void SetFog(bool bOn, float start = 0, float end = 10000, const RGBf &color = s_white, int iType = GL_LINEAR);

	// implementation data
	void SetSglNode(sglNode *n);
	sglNode *GetSglNode() { return m_pNode; }

protected:
	~vtNode();
	sglNode	*m_pNode;
};

class vtGroup : public vtNode, public vtGroupBase
{
public:
	vtGroup(bool suppress = false);

	// implement vtGroupBase methods
	void AddChild(vtNodeBase *pChild);
	void RemoveChild(vtNodeBase *pChild);
	vtNode *GetChild(int num);
	int GetNumChildren();

	vtNodeBase *FindDescendantByName(const char *name);
	bool ContainsChild(vtNodeBase *pNode);

	// implementation data
	void SetSglGroup(sglGroup *n)
	{
		m_pGroup = n;
		SetSglNode(n);
	}
	sglGroup *GetSglGroup() { return m_pGroup; }

protected:
	~vtGroup();
	sglGroup	*m_pGroup;
};

class vtTransform : public vtGroup, public vtTransformBase
{
public:
	vtTransform();
	~vtTransform();

	// implement vtTransformBase methods
	void Identity();
	void SetTrans(const FPoint3 &pos);
	FPoint3 GetTrans();
	void Translate1(const FPoint3 &pos);
	void TranslateLocal(const FPoint3 &pos);
	void Rotate2(const FPoint3 &axis, float angle);
	void RotateLocal(const FPoint3 &axis, float angle);
	void RotateParent(const FPoint3 &axis, float angle);
	void Scale3(float x, float y, float z);

	void SetTransform1(const FMatrix4 &mat);
	void GetTransform1(FMatrix4 &mat);
	void PointTowards(const FPoint3 &point);

protected:
	sglTransformf	*m_pTrans;
	FPoint3			m_Scale;
};

class vtLight : public vtNode
{
public:
	vtLight();
	void SetColor2(RGBf color);
	void SetAmbient2(RGBf &color);

	sglDirectionalLight	*m_pLight;
};

class vtMovLight : public vtTransform
{
public:
	vtMovLight(vtLight *pContained);
	vtLight *GetLight() { return m_pLight; }
	vtLight	*m_pLight;
};

class vtGeom : public vtGeomBase, public vtNode
{
public:
	vtGeom();

	void AddMesh(vtMesh *pMesh, int iMatIdx);
	void AddTextMesh(vtTextMesh *pMesh, int iMatIdx);
	void RemoveMesh(vtMesh *pMesh);

	int GetNumMeshes();
	vtMesh *GetMesh(int i);
	vtTextMesh *GetTextMesh(int i);

	virtual void SetMaterials(class vtMaterialArray *mats);
	vtMaterialArray	*GetMaterials();

	vtMaterial *GetMaterial(int i);

	void SetMeshMatIndex(vtMesh *pMesh, int iMatIdx);

	// implementation
	vtMaterialArray *m_pMaterialArray;
	sglGeode	*m_pGeode;
};

class vtMovGeom : public vtTransform
{
public:
	vtMovGeom(vtGeom *pContained) : vtTransform()
	{
		m_pGeom = pContained;
		AddChild(m_pGeom);
	}
	vtGeom	*m_pGeom;
};


class vtDynDrawable : public sglDrawable
{
public:
	vtDynDrawable();

	virtual const sglBoxBound &getBound();
	virtual void drawGeometry(sglVec2f *tex_coords) const;
	virtual void addStats(sglStats &stats) const;
	virtual bool computeBounds();
	virtual bool isValid() const;

	class vtDynGeom *m_pDynGeom;
};

class vtDynGeom : public vtGeom
{
public:
	vtDynGeom();

	// culling
	int IsVisible(const FSphere &sphere) const;
	bool IsVisible(const FPoint3 &point) const;
	int IsVisible(const FPoint3 &point0,
					const FPoint3 &point1,
					const FPoint3 &point2,
					const float fTolerance = 0.0f) const;
	int IsVisible(FPoint3 point, float radius);

	// vt methods (must be overriden)
	virtual void DoRender() = 0;
	virtual void DoCalcBoundBox(FBox3 &box) = 0;
	virtual void DoCull(FPoint3 &eyepos_ogl, IPoint2 window_size, float fov) = 0;

	void CalcCullPlanes();

	// for culling
	FPlane		m_cullPlanes[4];

	vtDynDrawable	*m_pDynDrawable;

protected:
};

//////////////////////////////////////////////////

class vtLOD : public vtGroup
{
public:
	vtLOD() : vtGroup()
	{
		m_pLOD = new sglLOD;
		SetSglGroup(m_pLOD);
	}

	void SetRanges(float *ranges, int nranges);
	void SetCenter(FPoint3 &center);

protected:
	sglLOD	*m_pLOD;
};

class vtCamera : public vtTransform
{
public:
	vtCamera();

	void SetHither(float f);
	float GetHither();
	void SetYon(float f);
	float GetYon();

	void SetFOV(float f);
	float GetFOV();

	void ZoomToSphere(const FSphere &sphere);

	void SetOrtho(float fWidth);

	sglViewPlatform			*m_psglViewPlatform;
	sglPerspectiveCamera	*m_psglPerspCamera;
	sglOrthographicCamera	*m_psglOrthoCamera;
	sglCamera				*m_psglCamera;
};


class vtSprite : public vtGroup
{
public:
	vtSprite() : vtGroup() {}
	void SetText(const char *msg);
	void SetWindowRect(float l, float t, float r, float b) {}
};

#endif // VTSGL_NODEH

