//
// StraightSkeleton.h: interface for the CStraightSkeleton class.
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// Straight skeleton algorithm and original implementation
// courtesy of Petr Felkel and Stepan Obdrzalek (petr.felkel@tiani.com)
// Re-implemented for the Virtual Terrain Project (vterrain.org)
// by Roger James (www.beardandsandals.co.uk)
//

#ifndef STRAIGHTSKELETONH
#define STRAIGHTSKELETONH

#include "FelkelComponents.h"
#include "FelkelIntersection.h"

/**
 * This class implements a Straight skeleton algorithm, used for generating
 * geometry for building roofs when the footprint is complicated (more than
 * just a regular rectangle).
 *
 * See the top of the header file for history and credits.
 */
class CStraightSkeleton
{
public:
	CStraightSkeleton();
	virtual ~CStraightSkeleton();

	CSkeleton& MakeSkeleton(ContourVector &contours);
	CSkeleton& MakeSkeleton(Contour &points);
	CSkeleton CompleteWingedEdgeStructure(ContourVector &contours);
#ifdef FELKELDEBUG
	void Dump();
#endif

	IntersectionQueue m_iq;
	CVertexList m_vl;
	CSkeleton m_skeleton;
	CSkeleton m_boundaryedges;
	int m_NumberOfBoundaryVertices;
	int m_NumberOfBoundaryEdges;
private:
	bool IsClockwise(Contour& points);
	bool FixSkeleton(Contour& points);
	CSkeletonLine* FindNextRightEdge(CSkeletonLine* pEdge, bool *bReversed);
	CNumber CalculateNormal(const CSkeletonLine& Edge, const C3DPoint& Point);
};

#endif // STRAIGHTSKELETONH
