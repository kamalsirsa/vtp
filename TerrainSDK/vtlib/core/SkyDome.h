//
// SkyDome.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_SKYDOMEH
#define VTLIB_SKYDOMEH

#include <time.h>

struct Star
{
	// Right Ascension
	float 	ra;

	// Declination
	float dec;

	// Magnitude
	float	mag;
	float	relmag;

	// Cartesian coordinate
	FPoint3 cartpt;
};


class vtDayDome : public vtTransform
{
public:
	vtDayDome();
	~vtDayDome() { if (SphVertices) delete[] SphVertices; }

	void	Create(int depth, float radius,
					const char *sun_texture = NULL);

	void	ApplyDayColors();
	void 	SetDayColors(RGBf& horizon, RGBf& azimuth);
	void	SetTimeOfDay(int time, bool bFullRefresh);
	void	SetDawnTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec);
	void	SetDuskTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec);
	void	SetSunsetColor(RGBf& sunset);
	void	SetInterpCutoff(float cutoff);
	void	SetRadius(float radius);

	// As measured from horizon to max of 90deg at azimuth - default = 30deg
	void	SetMaxSunsetAngle(float sunset_angle);
	void	SetSunModifier(float sunpct);

	float	GetSkyBrightness() { return m_fademod; }
	void	SetSunColor(RGBf &color);

private:
	RGBf 	DayHorizonCol, DayAzimuthCol, SunsetCol;
	float	Radius;
	float	Cutoff, MaxSunsetAngle, SunTimePctMod;

	int     TimeOfDay, MaxTimeOfDay;
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
	vtMaterial		*m_SunApp;
};

inline vtDayDome::vtDayDome() {
	m_pMats = NULL;
	m_pMat = NULL;
	m_pDomeGeom = NULL;
	m_pDomeMesh = NULL;
	m_pSunShape = NULL;
	m_SunApp = NULL;
	SphVertices = NULL;
}

inline void vtDayDome::SetDayColors(RGBf& horizon, RGBf& azimuth)
{ DayHorizonCol = horizon; DayAzimuthCol = azimuth; }

inline void vtDayDome::SetSunsetColor(RGBf& sunset)
{ SunsetCol = sunset; }

inline void vtDayDome::SetMaxSunsetAngle(float sunset_angle)
{ MaxSunsetAngle = sunset_angle; }

inline void vtDayDome::SetInterpCutoff(float cutoff)
{ Cutoff = cutoff; }

inline void vtDayDome::SetSunModifier(float sunpct)
{ SunTimePctMod = sunpct; }



class vtStarDome : public vtTransform
{
public:
	vtStarDome();
	~vtStarDome() { if (Starfield) delete[] Starfield; }
	void	Create(const char *starfile, float radius, float brightness,
					const char *moon_texture = NULL);

	// Overridden functions
	void	SetTimeOfDay(int time);
	void	SetDawnTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec);
	void	SetDuskTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec);
	void	SetStarFile(char *starpath);
	void	SetRadius(float radius);

private:
	float	Radius;
	float	RelativeBrightness;

	int     TimeOfDay, MaxTimeOfDay;
	int		DuskStartTime, DuskEndTime;
	int		DawnStartTime, DawnEndTime;
	float	DawnPct, DuskPct;
	float	DawnDuration, DuskDuration;

	float	HighMag, LowMag, MagRange;

	// Star array and the number of stars in the array
	int		NumStars;
	Star	*Starfield;
	char	StarFile[100];

	// Read star data from file - returns number of stars read into array
	void	ReadStarData(const char *starfile);
	void	ConvertStarCoord(Star *star);

	void	AddStars(vtMesh *geo);
	void	AddConstellation(vtMesh *geo);
	void	FadeStars();

	vtGeom		*m_pStarGeom;
	vtMesh		*m_pStarMesh;
	vtMovGeom	*m_pMoonGeom;
};

inline vtStarDome::vtStarDome() {
	Starfield = NULL;
}


class vtSkyDome : public vtTransform
{
public:
	vtSkyDome();
	~vtSkyDome() { delete m_pDayDome; delete m_pStarDome; }

	void	Create(const char *starfile, int depth, float radius,
					const char *sun_texture = NULL, const char *moon_texture = NULL);
	void	SetRadius(float radius);
	void	SetTimeOfDay(int hrs, int mins, int secs);
	void	SetTimeOfDay(int time, bool bFullRefresh = false);
	void 	SetDayColors(RGBf& horizon, RGBf& azimuth);
	void	SetDawnTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec);
	void	SetDuskTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec);
	void 	SetSunsetColor(RGBf& sunset);
	void	SetMaxSunsetAngle(float sunset_angle);
	void	SetInterpCutoff(float cutoff);
	vtDayDome *GetDayDome() { return m_pDayDome; }
	void	SetSunLight(vtMovLight *light) { m_pSunLight = light; }

protected:
	vtDayDome	*m_pDayDome;
	vtStarDome	*m_pStarDome;
	vtMovLight	*m_pSunLight;

private:
	float	Radius;

	int     TimeOfDay, MaxTimeOfDay;
	int		DuskStartTime, DuskEndTime;
	int		DawnStartTime, DawnEndTime;
	float	HighMag, LowMag, MagRange;
};

#endif

