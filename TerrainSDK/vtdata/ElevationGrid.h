//
// vtElevationGrid.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifndef ELEVATIONGRIDH
#define ELEVATIONGRIDH

#include "MathTypes.h"
#include "Projections.h"
#include "LocalConversion.h"
#include "HeightField.h"
#include "vtString.h"

class vtDIB;

/**
 * The vtElevationGrid class represents a generic grid of elevation data.
 * It supports reading and writing the data from several file formats.
 * \par
 * Height elements ("heixels") can be either integer (2 bytes) or floating
 * point (4 bytes).  Heixel values are always in meters.  Areas of unknown
 * elevation are represented by the value INVALID_ELEVATION.
 * \par
 * To load a grid from a file, first create an empty grid, then call the
 * appropriated Load method.
 */
class vtElevationGrid : public vtHeightFieldGrid3d
{
public:
	vtElevationGrid();
	vtElevationGrid(const vtElevationGrid &rhs);
	vtElevationGrid(const DRECT &area, int iColumns, int iRows, bool bFloat,
		const vtProjection &proj);
	virtual ~vtElevationGrid();

	vtElevationGrid &operator=(const vtElevationGrid &rhs);

	bool ConvertProjection(vtElevationGrid *pOld, const vtProjection &NewProj, bool progress_callback(int) = NULL);
	bool ReprojectExtents(const vtProjection &proj_new);
	void Scale(float fScale, bool bDirect, bool bRecomputeExtents = true);
	void ComputeHeightExtents();
	void Offset(const DPoint2 &delta);
	void ReplaceValue(float value1, float value2);

	// Load from unknown file format
	bool LoadFromFile( const char *szFileName, bool progress_callback(int) = NULL );

	// Load from a specific kind of file
	bool LoadFromDEM(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromASC(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromTerragen(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromCDF(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromDTED(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromGTOPO30(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromGLOBE(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromGRD(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromPGM(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromRAW(const char *szFileName, int width,	int height,
		int bytes_per_element, float vertical_units, bool bBigEndian,
		bool progress_callback(int));
	bool LoadFromMicroDEM(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromXYZ(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromHGT(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromBT(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadBTHeader(const char *szFileName);

	// Use GDAL to read a file
	bool LoadWithGDAL(const char *szFileName, bool progress_callback(int) = NULL);

	// Use OGR to read a file
	bool LoadFromNTF5(const char *szFileName, bool progress_callback(int) = NULL);

	// Save
	bool SaveToGeoTIFF(const char *szFileName);
	bool SaveToBMP(const char *szFileName);
	bool SaveToTerragen(const char *szFileName);
	bool SaveToBT(const char *szFileName, bool progress_callback(int) = NULL, bool bGZip = false);
	bool SaveToSTM(const char *szFileName, bool progress_callback(int) = NULL);
	bool SaveToPlanet(const char *szDirName, bool progress_callback(int) = NULL);
	bool SaveToASC(const char *szFileName, bool progress_callback(int) = NULL);
	bool SaveToVRML(const char *szFileName, bool progress_callback(int) = NULL);

	// Set/Get height values
	void  SetFValue(int i, int j, float value);
	void  SetValue(int i, int j, short value);
	short GetValue(int i, int j) const;	// returns height value as a integer
	float GetFValue(int i, int j) const;	// returns height value as a float
	float GetFValueSafe(int i, int j) const;

	float GetClosestValue(const DPoint2 &p) const;
	float GetFilteredValue(const DPoint2 &p) const;
	float GetFilteredValue2(const DPoint2 &p) const;

	// Accessors
	/** Return the embedded name of the DEM is it has one */
	const char *GetDEMName()	{ return m_strOriginalDEMName; }

	/** Returns the geographic extents of the *area* covered by grid. */
	DRECT GetAreaExtents() const;

	/** Get the data size of the grid: \c true if floating point (4-byte),
	 * \c false if integer (2-byte).
	 */
	bool  IsFloatMode()	const { return m_bFloatMode; }

	void FillWithSingleValue(float fValue);
	void GetEarthLocation(int i, int j, DPoint3 &loc) const;

	vtProjection &GetProjection() { return m_proj; }
	const vtProjection &GetProjection() const { return m_proj; }
	void SetProjection(const vtProjection &proj) { m_proj = proj; }

	bool GetCorners(DLine2 &line, bool bGeo) const;
	void SetCorners(const DLine2 &line);

	short *GetData() { return m_pData; }
	float *GetFloatData() { return m_pFData; }

	const short *GetData()	  const { return m_pData;  }
	const float *GetFloatData() const { return m_pFData; }

	void SetScale(float sc) { m_fVMeters = sc; }
	float GetScale() const { return m_fVMeters; }

	// Implement vtHeightField methods
	void GetChecksum(unsigned char **ppChecksum) const;
	bool FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude, bool bTrue = false) const;

	// Implement vtHeightField3d methods
	virtual float GetElevation(int iX, int iZ, bool bTrue = false) const;
	virtual void GetWorldLocation(int i, int j, FPoint3 &loc, bool bTrue = false) const;

	// methods that deal with world coordinates
	void SetupConversion(float fVerticalExag);
	float GetWorldValue(int i, int j, bool bTrue = false) const;
	bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude,
		bool bTrue = false, bool bIncludeCulture = false,
		FPoint3 *vNormal = NULL) const;

protected:
	bool	m_bFloatMode;
	short	*m_pData;
	float	*m_pFData;
	float	m_fVMeters;	// scale factor to convert stored heights to meters
	float	m_fVerticalScale;

	void ComputeExtentsFromCorners();
	void ComputeCornersFromExtents();

	DPoint2		m_Corners[4];	// data corners, in the projection of this terrain
	vtProjection	m_proj;		// a grid always has some projection

	void	_AllocateArray();
	vtString	m_strOriginalDEMName;
};

#endif	// ELEVATIONGRIDH

