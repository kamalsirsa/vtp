//
// Trees.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINTREESH
#define TERRAINTREESH

#include "vtdata/Plants.h"
#include "vtdata/LULC.h"
#include "vtdata/FilePath.h"
#include "vtlib/core/HeightField.h"
#include "vtlib/core/LodGrid.h"

#if SUPPORT_XFROG
+/*  Greenworks Xfrog:  See http://www.xfrog.com for details,  */
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

	void LoadAndCreate(const StringArray &paths, float fTreeScale,
					   bool bShadows, bool bBillboards);
	vtTransform *GenerateGeom();

protected:
	vtMesh *CreateTreeMesh(float fTreeScale, bool bShadows,
						   bool bBillboards);

	vtMaterialArray	*m_pMats;
	vtMesh			*m_pMesh;
	int				m_iMatIdx;
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

	void CreatePlantSurfaces(const StringArray &paths, float fTreeScale,
		bool bShadows, bool bBillboards);

	// override / replace a few methods of vtPlantList
	vtPlantSpecies3d *GetSpecies(int i) const;
	vtPlantAppearance3d *GetAppearanceByName(const char *szName, float fHeight);
	void AddSpecies(const char *common_name, float max_height);

//	vtGeom *plant_nursery(vtHeightField *pHeightField, float lat, float lon);
};

#endif
