//
// SkyDome.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_SKYDOMEH
#define VTLIB_SKYDOMEH

#include <time.h>

/// Convert a time (hours, minutes, seconds) to a single value (seconds)
inline const int TIME_TO_INT(const double hr, const double min, const double sec)
{
	return (int)((hr * 60 + min) * 60 + sec);
}


/**
 * A DayDome is a hemisphere which is colored according to the time of day:
 * shades of blue, plus yellow-orange at dawn and dusk.  The intended use is
 * to make it large and far away from the camera, so that it is always behind
 * all the terrain and objects in the world.  It also contains an image of the
 * Sun (as a texture billboard).  The Sun is moved and sky is colored
 * appropriately for the time of day (set with SetTimeOfDay).
 */
class vtDayDome : public vtTransform
{
public:
	vtDayDome();
	~vtDayDome();

	void	Create(int depth, float radius,
					const char *sun_texture = NULL);

	void	ApplyDayColors();
	void 	SetDayColors(const RGBf &horizon, const RGBf &azimuth);
	void	SetTimeOfDay(int time, bool bFullRefresh);
	void	SetDawnTimes(int start_hr, int start_min, int end_hr, int end_min);
	void	SetDuskTimes(int start_hr, int start_min, int end_hr, int end_min);
	void	SetSunsetColor(const RGBf &sunset);
	void	SetInterpCutoff(float cutoff);
	void	SetRadius(float radius);
	bool	SetTexture(const char *filename);

	/// As measured from horizon to max of 90deg at azimuth - default = 30deg
	void	SetMaxSunsetAngle(float sunset_angle);
	void	SetSunModifier(float sunpct);

	float	GetSkyBrightness() { return m_fademod; }
	void	SetSunColor(const RGBf &color);

private:
	RGBf 	DayHorizonCol, DayAzimuthCol, SunsetCol;
	float	Radius;
	float	Cutoff, MaxSunsetAngle, SunTimePctMod;

	int		m_iTimeOfDay, MaxTimeOfDay;
	int		DuskStartTime, DuskEndTime, DawnStartTime, DawnEndTime;
	float	DawnDuration, DuskDuration;

	float	DawnSeqStart, DuskSeqEnd;
	float	DuskMidSeq, DuskMidSeqTime, DawnMidSeq, DawnMidSeqTime;

	int		NumVertices;
	FPoint3	*SphVertices;

	float	m_fademod;
	void	ConvertVertices();

	vtGeom			*m_pDomeGeom;

	vtMaterialArray *m_pMats;
	vtMaterial		*m_pMat;
	vtMesh			*m_pDomeMesh;

	vtMovGeom		*m_pSunShape;
	vtMaterial		*m_pSunMat;
	vtImage			*m_pSunImage;

	bool		m_bHasTexture;
};


/**
 * A StarDome is a sphere of stars, based on real star data, implemented as
 * Points.  The intended use is to make it large and far away from the camera,
 * so that it is always behind all the terrain and objects in the world.
 * It also contains an image of the Moon (as a texture billboard).  The Moon
 * and Stars are moved appropriately for the time of day (set with SetTimeOfDay).
 */
class vtStarDome : public vtTransform
{
public:
	vtStarDome();
	~vtStarDome();

	void	Create(const char *starfile, float radius, float brightness,
					const char *moon_texture = NULL);

	void	SetTimeOfDay(int time);
	void	SetDawnTimes(int start_hr, int start_min, int end_hr, int end_min);
	void	SetDuskTimes(int start_hr, int start_min, int end_hr, int end_min);
	void	SetStarFile(char *starpath);
	void	SetRadius(float radius);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	struct Star
	{
		float 	ra;		// Right Ascension
		float	dec;	// Declination
		float	mag;	// Magnitude
		float	relmag;
		FPoint3 cartpt;	// Cartesian coordinate
	};
#endif // DOXYGEN_SHOULD_SKIP_THIS

private:

	float	Radius;
	float	RelativeBrightness;

	int		m_iTimeOfDay, MaxTimeOfDay;
	int		DuskStartTime, DuskEndTime;
	int		DawnStartTime, DawnEndTime;
	float	DawnPct, DuskPct;
	float	DawnDuration, DuskDuration;

	float	HighMag, LowMag, MagRange;

	/// Star array and the number of stars in the array
	int		NumStars;
	Star	*Starfield;
	char	StarFile[100];

	/// Read star data from file - returns number of stars read into array
	bool	ReadStarData(const char *starfile);
	void	ConvertStarCoord(Star *star);

	void	AddStars(vtMesh *geo);
	void	AddConstellation(vtMesh *geo);
	void	FadeStars();

	vtGeom		*m_pStarGeom;
	vtMesh		*m_pStarMesh;
	vtMovGeom	*m_pMoonGeom;
	vtMaterialArray *m_pMats;
	vtImage		*m_pMoonImage;
};


/**
 * A SkyDome is comprised of a vtDayDome and a vtStarDome.  It contains and
 * controls the two domes appropriately.  It also supplies a real Light (vtLight)
 * which simulates the actual color, direction and intensity of sunlight.
 */
class vtSkyDome : public vtTransform
{
public:
	vtSkyDome();

	void	Create(const char *starfile, int depth, float radius,
					const char *sun_texture = NULL, const char *moon_texture = NULL);

	void	SetRadius(float radius);
	void	SetTimeOfDay(int hr, int min, int sec);
	void	SetTimeOfDay(int time, bool bFullRefresh = false);
	void 	SetDayColors(const RGBf &horizon, const RGBf &azimuth);
	void	SetDawnTimes(int start_hr, int start_min, int end_hr, int end_min);
	void	SetDuskTimes(int start_hr, int start_min, int end_hr, int end_min);
	void 	SetSunsetColor(const RGBf &sunset);
	void	SetMaxSunsetAngle(float sunset_angle);
	void	SetInterpCutoff(float cutoff);
	void	SetSunLight(vtMovLight *light) { m_pSunLight = light; }
	bool	SetTexture(const char *filename);

protected:
	vtDayDome	*m_pDayDome;
	vtStarDome	*m_pStarDome;
	vtMovLight	*m_pSunLight;

private:
	float	Radius;

	int		m_iTimeOfDay, MaxTimeOfDay;
	int		DuskStartTime, DuskEndTime;
	int		DawnStartTime, DawnEndTime;
	float	HighMag, LowMag, MagRange;
};

#endif	// VTLIB_SKYDOMEH

