//
// GeometryUtils.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_GEOMETRYUTILSH
#define VTOSG_GEOMETRYUTILSH

#include "vtlib/core/Structure3d.h"

class vtBuilding3d;
class vtLevel;

namespace OSGGeometryUtils
{
	osg::PrimitiveSet* MakeMeAPrimitiveSet(const int VertexType, const osg::PrimitiveSet::Type Type, const osg::PrimitiveSet::Mode Mode);

	class Arrays
	{
	public:
		int m_VertexType;
		osg::ref_ptr<osg::Vec3Array> m_Vertices;
		osg::ref_ptr<osg::Vec3Array> m_Normals;
		osg::ref_ptr<osg::Vec2Array> m_TexCoords;
	};

	class DrawArraysTriangles : public osg::DrawArrays, public Arrays
	{
	public:
		DrawArraysTriangles(const int VertexType, const osg::PrimitiveSet::Mode Mode);
	};

	class DrawArrayLengthsLineStrip : public osg::DrawArrayLengths, public Arrays
	{
	public:
		DrawArrayLengthsLineStrip(const int VertexType, const osg::PrimitiveSet::Mode Mode);
	};

	class DrawArrayLengthsTriangleFan : public osg::DrawArrayLengths, public Arrays
	{
	public:
		DrawArrayLengthsTriangleFan(const int VertexType, const osg::PrimitiveSet::Mode Mode);
	};

	class PrimitiveSetCacheEntry : public osg::Referenced
	{
	public:
		PrimitiveSetCacheEntry(osg::PrimitiveSet* PrimitiveSet) : m_pPrimitiveSet(PrimitiveSet) {}
		osg::ref_ptr<osg::PrimitiveSet> m_pPrimitiveSet;
	protected:
		~PrimitiveSetCacheEntry() {}
	};

	class PrimitiveSetCache : public std::vector<osg::ref_ptr<PrimitiveSetCacheEntry> >, public osg::Referenced
	{
	public:
		osg::PrimitiveSet* FindOrCreatePrimitive(const int VertexType, const vtMaterial* pMaterial, const osg::PrimitiveSet::Type Type,
												const osg::PrimitiveSet::Mode Mode);
	protected:
		~PrimitiveSetCache() {}
	};

	class StateSetCacheEntry : public osg::Referenced
	{
	public:
		StateSetCacheEntry(osg::StateSet& StateSet) : m_StateSet(StateSet)
		{
			m_pPrimitiveSetCache = new PrimitiveSetCache;
		}
		osg::StateSet& m_StateSet;
		osg::ref_ptr<PrimitiveSetCache> m_pPrimitiveSetCache;
	protected:
		~StateSetCacheEntry() {}
	};

	class StateSetCache : public std::vector<osg::ref_ptr<StateSetCacheEntry> >, public osg::Referenced
	{
	public:
		osg::PrimitiveSet* FindOrCreatePrimitive(const int VertexType, vtMaterial* pMaterial, const osg::PrimitiveSet::Type Type,
												const osg::PrimitiveSet::Mode Mode);
	protected:
		~StateSetCache() {}
	};

	class VertexTypeCacheEntry : public osg::Referenced
	{
	public:
		VertexTypeCacheEntry(const int VertexType) : m_VertexType(VertexType)
		{
			m_pStateSetCache = new StateSetCache;
		}
		int m_VertexType;
		osg::ref_ptr<StateSetCache> m_pStateSetCache;
	protected:
		~VertexTypeCacheEntry() {}
	};

	class PrimitiveCache : public std::vector<osg::ref_ptr<VertexTypeCacheEntry> >, public osg::Referenced
	{
	public:
		DrawArraysTriangles* FindOrCreateDrawArraysTriangles(const int VertexType, vtMaterial* pMaterial);
		DrawArrayLengthsLineStrip* FindOrCreateDrawArrayLengthsLineStrip(const int VertexType, vtMaterial* pMaterial);
		DrawArrayLengthsTriangleFan* FindOrCreateDrawArrayLengthsTriangleFan(const int VertexType, vtMaterial* pMaterial);
		osg::PrimitiveSet* FindOrCreatePrimitive(const int VertexType, vtMaterial* pMaterial, const osg::PrimitiveSet::Type Type,
												const osg::PrimitiveSet::Mode Mode);
		vtGeode* Realise(bool bUseVertexBufferObjects = false) const;
	protected:
		~PrimitiveCache() {}
	};

	class GenerateBuildingGeometry : public osg::Referenced, public vtStructure3d // subclass vtStructure3d to get hold of shared material functions 
	{
	public:
		GenerateBuildingGeometry(const vtBuilding3d& Building) : m_Building(Building) {}
		vtGeode* Generate();
	protected: 
		void AddFlatRoof(const FPolygon3 &pp, const vtLevel *pLev);
		void CreateUniformLevel(int iLevel, float fHeight, int iHighlightEdge);
		bool MakeFacade(vtEdge *pEdge, FLine3 &quad, int stories);
		osg::Vec3 Normal(const vtVec3 &p0, const vtVec3 &p1, const vtVec3 &p2);
		void AddWallSection(vtEdge *pEdge, bool bUniform,
			const FLine3 &quad, float vf1, float vf2, float hf1 = -1.0f);
		void AddHighlightSection(vtEdge *pEdge, const FLine3 &quad);
		float MakeFelkelRoof(const FPolygon3 &EavePolygons, const vtLevel *pLev);
		bool Collinear2d(const FPoint3& Previous, const FPoint3& Current, const FPoint3& Next);
		void CreateUpperPolygon(const vtLevel *lev, FPolygon3 &polygon, FPolygon3 &polygon2);
		void CreateEdgeGeometry(const vtLevel *pLev, const FPolygon3 &polygon1,
									  const FPolygon3 &polygon2, int iEdge, bool bShowEdge);
		void AddWallNormal(vtEdge *pEdge, vtEdgeFeature *pFeat, const FLine3 &quad);
		void AddWindowSection(vtEdge *pEdge, vtEdgeFeature *pFeat, const FLine3 &quad);
		void AddDoorSection(vtEdge *pEdge, vtEdgeFeature *pFeat, const FLine3 &quad);
		int FindVertex(FPoint3 Point, FLine3 &RoofSection3D, vtArray<int> &iaVertices);
		~GenerateBuildingGeometry() {}
		const vtBuilding3d& m_Building;
		osg::ref_ptr<PrimitiveCache> m_pPrimitiveCache;

		// abstract memebers
		osg::Node *GetContained(void) { return NULL; }
		bool CreateNode(vtTerrain *) { return false; }
		bool IsCreated(void) { return false; }
		void DeleteNode(void) {}
	};

	class osg::Geometry* FindOrCreateGeometryObject(osg::Geode *pGeode, vtMaterial& Material, const int ArraysRequired);
	class osg::PrimitiveSet* FindOrCreatePrimitiveSet(osg::Geometry* pGeometry, const osg::PrimitiveSet::Mode Mode, const osg::PrimitiveSet::Type Type);

};
#endif	// VTOSG_GEOMETRYUTILSH

