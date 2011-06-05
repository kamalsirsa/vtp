//
// Trees.h
//
// Copyright (c) 2001-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINTREESH
#define TERRAINTREESH

/** \defgroup veg Vegetation
 * These classes are used for vegetation.  Vegetation in the vtlib library
 * consists of subclassing the vtdata vegetation classes to extend them with
 * the ability to create and operate on 3D geometry of the plants.
 */
/*@{*/

#include "vtdata/Plants.h"

class vtHeightField3d;

#if SUPPORT_XFROG
// Greenworks Xfrog:  See http://www.xfrog.com for details.  Not currently
//  supported as we don't currently have a way to interface to their engine.
#include "\dism\xfrog2dism\xfrog2dism.h"
#endif

/**
 * This class extends vtPlantAppearance with the ability to construct 3D
 * geometry for an appearance.
 */
class vtPlantAppearance3d : public vtPlantAppearance
{
public:
	vtPlantAppearance3d(AppearType type, const char *filename,
		float width, float height,
		float shadow_radius, float shadow_darkness);
	~vtPlantAppearance3d();
	vtPlantAppearance3d(const vtPlantAppearance &v);

	void CheckAvailability();
	void LoadAndCreate();
	bool GenerateGeom(vtTransform *container);
	bool IsAvailable() { return m_bAvailable; }

	static float s_fPlantScale;
	static bool  s_bPlantShadows;

protected:
	vtMesh *CreateTreeMesh(float fTreeScale, bool bShadows);
	void _Defaults();

	osg::ref_ptr<vtMaterialArray> m_pMats;
	vtGeodePtr		m_pGeode;
	osg::ref_ptr<vtMesh> m_pMesh;
	int				m_iMatIdx;
	osg::ref_ptr<osg::Node>	m_pExternal;
#if SUPPORT_XFROG
	CFrogModel	*m_pFrogModel;
#endif
	bool m_bAvailable;
	bool m_bCreated;
};

/**
 * This class extends vtPlantSpecies with the ability to manage 3D
 * appearances of the species.
 */
class vtPlantSpecies3d : public vtPlantSpecies
{
public:
	vtPlantSpecies3d();

	// copy
	vtPlantSpecies3d &operator=(const vtPlantSpecies &v);

	vtPlantAppearance3d *GetAppearanceByHeight(float fHeight);
	vtPlantAppearance3d *GetRandomAppearance();
	vtPlantAppearance3d *GetAppearance(int i) const { return (vtPlantAppearance3d *)m_Apps[i]; };

	void CheckAvailability();
	int NumAvailableInstances();

	virtual void AddAppearance(AppearType type, const char *filename, float width, float height,
		float shadow_radius, float shadow_darkness);
};


/**
 * This class extends vtSpeciesList with the ability to construct and
 * manage 3D representations of the plants.
 */
class vtSpeciesList3d : public vtSpeciesList
{
public:
	vtSpeciesList3d();

	// copy operator
	vtSpeciesList3d &operator=(const vtSpeciesList &v);

	int CheckAvailability();
	void CreatePlantSurfaces();

	// override / replace a few methods of vtSpeciesList
	vtPlantSpecies3d *GetSpecies(unsigned int i) const;

//	vtGeode *plant_nursery(vtHeightField *pHeightField, float lat, float lon);
};

/**
 * This class works in parallel with vtPlantInstance to contain the 3D
 * geometry for a plant instance.  An array of these objects are
 * maintained by vtPlantInstanceArray3d.
 */
class vtPlantInstance3d
{
public:
	vtPlantInstance3d();
	~vtPlantInstance3d();

	void ShowBounds(bool bShow);
	void ReleaseContents();

	vtTransform *m_pContainer;
	vtGeode		*m_pHighlight;	// The wireframe highlight
};

/**
 * This class extends vtPlantInstanceArray with the ability to construct and
 * manage 3D representations of the plants.
 *
 * \ingroup veg
 */
class vtPlantInstanceArray3d : public vtPlantInstanceArray
{
public:
	vtPlantInstanceArray3d();
	~vtPlantInstanceArray3d();

	int CreatePlantNodes(bool progress_dialog(int) = NULL);
	bool CreatePlantNode(unsigned int i);
	int NumOffTerrain() { return m_iOffTerrain; }

	vtTransform *GetPlantNode(unsigned int i) const;
	vtPlantInstance3d *GetInstance3d(unsigned int i) const;

	/// Indicate the heightfield which will be used for the structures in this array
	void SetHeightField(vtHeightField3d *hf) { m_pHeightField = hf; }

	/// Indicate the Plant List to use
	vtSpeciesList3d *GetPlantList() { return (vtSpeciesList3d *) m_pPlantList; }

	/// Deselect all plants including turning off their visual highlights
	void VisualDeselectAll();

	/// Select a single plant, and visually highlight it
	void VisualSelect(unsigned int i);

	void OffsetSelectedPlants(const DPoint2 &offset);

	void ReleasePlantGeometry(unsigned int i);
	void DeletePlant(unsigned int i);

	void UpdateTransform(unsigned int i);

	bool FindPlantFromNode(osg::Node *pNode, int &iOffset);

protected:
	vtArray<vtPlantInstance3d*>	m_Instances3d;
	vtHeightField3d		*m_pHeightField;
	int					m_iOffTerrain;
};

/*@}*/	// Group veg

#endif	// TERRAINTREESH

