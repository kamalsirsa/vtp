//
// Trees.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINTREESH
#define TERRAINTREESH

#include "vtdata/Selectable.h"
#include "vtdata/Plants.h"
#include "vtdata/LULC.h"
#include "vtdata/FilePath.h"
#include "vtdata/HeightField.h"
#include "vtlib/core/LodGrid.h"

#if SUPPORT_XFROG
// Greenworks Xfrog:  See http://www.xfrog.com for details.  Not currently
//  supported as we don't currently have a way to interface to their engine.
#include "\dism\xfrog2dism\xfrog2dism.h"
#endif

class vtPlantAppearance3d : public vtPlantAppearance
{
public:
	vtPlantAppearance3d(AppearType type, const char *filename,
		float width, float height,
		float shadow_radius, float shadow_darkness);
	~vtPlantAppearance3d();
	vtPlantAppearance3d(const vtPlantAppearance &v);

	void LoadAndCreate(const vtStringArray &paths, float fTreeScale,
					   bool bShadows, bool bBillboards);
	bool GenerateGeom(vtTransform *container);

protected:
	vtMesh *CreateTreeMesh(float fTreeScale, bool bShadows,
						   bool bBillboards);
	void _Defaults();

	vtMaterialArray	*m_pMats;
	vtMesh			*m_pMesh;
	int				m_iMatIdx;
	vtNodeBase		*m_pExternal;
#if SUPPORT_XFROG
	CFrogModel		*m_pFrogModel;
#endif
};

class vtPlantSpecies3d : public vtPlantSpecies
{
public:
	vtPlantSpecies3d();

	// copy
	vtPlantSpecies3d &operator=(const vtPlantSpecies &v);

	vtPlantAppearance3d *GetAppearanceByHeight(float fHeight);
	vtPlantAppearance3d *GetRandomAppearance();
	vtPlantAppearance3d *GetAppearance(int i) const { return (vtPlantAppearance3d *)m_Apps[i]; };

	virtual void AddAppearance(AppearType type, const char *filename, float width, float height,
		float shadow_radius, float shadow_darkness);
};


class AttribMap;

class vtPlantList3d : public vtPlantList
{
public:
	vtPlantList3d();

	// copy
	vtPlantList3d &operator=(const vtPlantList &v);

	void CreatePlantSurfaces(const vtStringArray &paths, float fTreeScale,
		bool bShadows, bool bBillboards);

	// override / replace a few methods of vtPlantList
	vtPlantSpecies3d *GetSpecies(int i) const;
	vtPlantAppearance3d *GetAppearanceByName(const char *szName, float fHeight);
	void AddSpecies(const char *common_name, float max_height);

//	vtGeom *plant_nursery(vtHeightField *pHeightField, float lat, float lon);
};

class vtPlantInstance3d : public Selectable
{
public:
	vtPlantInstance3d();
	~vtPlantInstance3d();

	void ShowBounds(bool bShow);
	void ReleaseContents();

	vtTransform *m_pContainer;
	vtGeom		*m_pHighlight;	// The wireframe highlight
};

/**
 * This class extends vtPlantInstanceArray with the ability to construct and
 * manage 3d representations of the plants.
 */
class vtPlantInstanceArray3d : public vtPlantInstanceArray
{
public:
	vtPlantInstanceArray3d();
	~vtPlantInstanceArray3d();

	int CreatePlantNodes();
	bool CreatePlantNode(int i);

	vtTransform *GetPlantNode(int i) const;
	vtPlantInstance3d *GetInstance3d(int i) const;

	/// Indicate the heightfield which will be used for the structures in this array
	void SetHeightField(vtHeightField3d *hf) { m_pHeightField = hf; }

	/// Indicate the Plant List to use
	vtPlantList3d *GetPlantList() { return (vtPlantList3d *) m_pPlantList; }

	/// Deselect all plants including turning off their visual highlights
	void VisualDeselectAll();

	/// Select a single plant, and visually highlight it
	void VisualSelect(int i);

	// return the number of selected plants
	int NumSelected() const;

	void OffsetSelectedPlants(const DPoint2 &offset);

	void ReleasePlantGeometry(int i);
	void DeletePlant(int i);

	void UpdateTransform(int i);

protected:
	Array<vtPlantInstance3d*>	m_Instances3d;
	vtHeightField3d		*m_pHeightField;
};

#endif	// TERRAINTREESH

