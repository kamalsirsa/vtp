//
// SkyDome - a simple day/night skydome, should be replaced with a
// more realistic version at some point.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "SkyDome.h"

// minimum and maximum ambient light values
const float MIN_AMB = 0.0f;
const float MAX_AMB = 1.0f;
const float AMB_RANGE = MAX_AMB-MIN_AMB;

// maximum directional light intensity
const float MAX_INT = 1.0f;

// only show stars up to this magnitude
const float MAX_MAGNITUDE = 5.8f;

// Radians <-> Degrees Macros, single angle
inline float RAD_TO_DEG(float x){return (x * (180.0f/PIf));}
inline float DEG_TO_RAD(float x){return (x * (PIf/180.0f));}

// Coordinate change macros for a point class
// Here x = rho, y = theta, z = phi
#define rho		x
#define theta	y
#define phi		z

inline void PT_CART_TO_SPHERE(const FPoint3& A, FPoint3& B) {
	B.rho = A.Length();
	B.theta = atan2f(-A.z, A.x);
	B.phi = atan2f(A.y, sqrtf(A.x*A.x + A.z*A.z));
}

inline void PT_SPHERE_TO_CART(const FPoint3& A, FPoint3& B) {
	B.x = A.rho * cosf(A.phi) * sinf(A.theta);
	B.y = A.rho * cosf(A.phi) * cosf(A.theta);
	B.z = -A.rho * sinf(A.phi);
}

///////////////////////////////////////////////////////////////////////////
// SkyDome
//

vtSkyDome::vtSkyDome()
{
	m_pDayDome = NULL;
	m_pStarDome = NULL;
	m_pSunLight = NULL;
}

/**
 * Creates a complete SkyDome, which includes a DayDome and a StarDome.
 */
void vtSkyDome::Create(const char *starfile, int depth, float radius,
					 const char *sun_texture, const char *moon_texture)
{
	Radius = radius;

	// Create the vtDayDome
	m_pDayDome = new vtDayDome();
	m_pDayDome->Create(depth, Radius, sun_texture);
	m_pDayDome->ApplyDayColors();
	m_pDayDome->SetName2("DayDome");
	AddChild(m_pDayDome);

	// Create the vtStarDome
	if (starfile && *starfile)
	{
		m_pStarDome = new vtStarDome();
		m_pStarDome->Create(starfile, Radius, 2.0f, moon_texture);
		m_pStarDome->SetName2("StarDome");
		AddChild(m_pStarDome);
	}

	// Set Maximum rotational change dependent only on hours, minutes and secs
	MaxTimeOfDay = TIME_TO_INT(24, 0, 0);
}

int dawn_start = TIME_TO_INT(4, 30, 0);
int dawn_middle = TIME_TO_INT(5, 30, 0);
int dawn_end = TIME_TO_INT(6, 30, 0);
int dusk_start = TIME_TO_INT(17, 30, 0);
int dusk_middle = TIME_TO_INT(18, 30, 0);
int dusk_end = TIME_TO_INT(19, 30, 0);

int sunrise = TIME_TO_INT(6, 0, 0);
int sunset = TIME_TO_INT(18, 0, 0);
int sunsize = TIME_TO_INT(0, 16, 0);

/**
 * Sets the time of day (or night).
 * \param hr,min,sec	Time of day
 */
void vtSkyDome::SetTimeOfDay(int hr, int min, int sec)
{
	SetTimeOfDay(TIME_TO_INT(hr, min, sec));
}

/**
 * Sets the time of day (or night).
 * \param time			Time in seconds since midnight.
 * \param bFullRefresh	Pass true to force the sky colors to be updated;
 *	otherwise, they will only be updated when absolutely necessary, which
 * is during dawn and dusk.
 */
void vtSkyDome::SetTimeOfDay(int time, bool bFullRefresh)
{
	TimeOfDay = time;

	// Pass along time of day information to the vtStarDome and vtDayDome
	if (m_pStarDome) m_pStarDome->SetTimeOfDay(time);
	if (m_pDayDome) m_pDayDome->SetTimeOfDay(time, bFullRefresh);

	// Determine which of the two domes are active according to time of day
	if (m_pStarDome)
	{
		if (TimeOfDay >= DuskStartTime || TimeOfDay <= DawnEndTime)
			m_pStarDome->SetEnabled(true);
		else
			m_pStarDome->SetEnabled(false);
	}

	// set the direction and intensity of the sunlight
	if (m_pSunLight != NULL)
	{
		float angle = 2.0f * PIf * ((float)TimeOfDay / (24 * 60 * 60));
		m_pSunLight->Identity();
		m_pSunLight->Rotate2(FPoint3(0,1,0), -PIf/2.0f);
		m_pSunLight->Rotate2(FPoint3(0,0,1), angle + PIf/2.0f);

		float ambient = 0.0f;

		// set intensity of ambient light based on time of day
		if (TimeOfDay > dusk_end || TimeOfDay < dawn_start)
		{
			// night
			ambient = MIN_AMB;
		}
		else if (TimeOfDay >= dawn_start && TimeOfDay <= dawn_end)
		{
			// dawn
			ambient = MIN_AMB + AMB_RANGE * (float)(TimeOfDay - dawn_start) / (dawn_end - dawn_start);
		}
		else if (TimeOfDay >= dawn_end && TimeOfDay <= dusk_start)
		{
			// day
			ambient = MAX_AMB;
		}
		else if (TimeOfDay >= dusk_start && TimeOfDay <= dusk_end)
		{
			// dusk
			ambient = MIN_AMB + AMB_RANGE * (float)(dusk_end - TimeOfDay) / (dawn_end - dawn_start);
		}

		vtGetScene()->SetAmbient(RGBf(ambient, ambient, ambient));

		float intensity = 0.0f;

		RGBf white(1.0f, 1.0f, 1.0f);
		RGBf yellow(1.0f, 0.6f, 0.4f);
		RGBf color;
		float fraction;

		// set intensity of sunlight based on time of day
		if (TimeOfDay > sunset+sunsize || TimeOfDay < sunrise-sunsize)
		{
			// night
			color = yellow;
			intensity = 0.0f;
		}
		else if (TimeOfDay >= sunrise-sunsize && TimeOfDay <= sunrise+sunsize)
		{
			// dawn
			fraction = (float)(TimeOfDay - (sunrise-sunsize)) / (sunsize*2.0f);
			color = yellow + ((white - yellow) * fraction);
			intensity = MAX_INT * fraction;
		}
		else if (TimeOfDay >= sunrise+sunsize && TimeOfDay <= sunset-sunsize)
		{
			// day
			color = white;
			intensity = MAX_INT;
		}
		else if (TimeOfDay >= sunset-sunsize && TimeOfDay <= sunset+sunsize)
		{
			// dusk
			fraction = (float)(sunset+sunsize - TimeOfDay) / (sunsize*2.0f);
			color = yellow + ((white - yellow) * fraction);
			intensity = MAX_INT * fraction;
		}

		if (m_pDayDome)
			m_pDayDome->SetSunColor(color);

//		color = white;
		color *= intensity;
		m_pSunLight->m_pLight->SetColor(color);
		m_pSunLight->m_pLight->SetAmbient(RGBf(ambient, ambient, ambient));
	}
}

//
//
void vtSkyDome::SetDuskTimes(int start_hr, int start_min,
							 int end_hr, int end_min)
{
	DuskStartTime = TIME_TO_INT(start_hr, start_min, 0);
	DuskEndTime = TIME_TO_INT(end_hr, end_min, 0);

	if (m_pDayDome)
		m_pDayDome->SetDuskTimes(start_hr, start_min, end_hr, end_min);
	if (m_pStarDome)
		m_pStarDome->SetDuskTimes(start_hr, start_min, end_hr, end_min);
}

//
//
void vtSkyDome::SetDawnTimes(int start_hr, int start_min,
							 int end_hr, int end_min)
{
	DawnStartTime = TIME_TO_INT(start_hr, start_min, 0);
	DawnEndTime = TIME_TO_INT(end_hr, end_min, 0);

	if (m_pDayDome)
		m_pDayDome->SetDawnTimes(start_hr, start_min, end_hr, end_min);
	if (m_pStarDome)
		m_pStarDome->SetDawnTimes(start_hr, start_min, end_hr, end_min);
}


//
//
void vtSkyDome::SetDayColors(const RGBf &horizon, const RGBf &azimuth)
{
	if (m_pDayDome)
	{
		m_pDayDome->SetDayColors(horizon, azimuth);
		m_pDayDome->ApplyDayColors();
	}
}

//
//
void vtSkyDome::SetSunsetColor(const RGBf &sunset)
{
	if (m_pDayDome)
		m_pDayDome->SetSunsetColor(sunset);
}

//
//
void vtSkyDome::SetMaxSunsetAngle(float sunset_angle)
{
	if (m_pDayDome)
		m_pDayDome->SetMaxSunsetAngle(sunset_angle);
}

//
//
void vtSkyDome::SetInterpCutoff(float cutoff)
{
	if (m_pDayDome)
		m_pDayDome->SetInterpCutoff(cutoff);
}


//
//
void vtSkyDome::SetRadius(float radius)
{
	if (m_pDayDome)	m_pDayDome->SetRadius(radius);
	if (m_pStarDome)	m_pStarDome->SetRadius(radius);
}

bool vtSkyDome::SetTexture(const char *filename)
{
	if (m_pDayDome)
		return m_pDayDome->SetTexture(filename);
	return false;
}


///////////////////////////////////////////////////////////////////////////
// DayDome
//

vtDayDome::vtDayDome()
{
	m_pMats = NULL;
	m_pMat = NULL;
	m_pDomeGeom = NULL;
	m_pDomeMesh = NULL;
	m_pSunShape = NULL;
	m_pSunMat = NULL;
	m_pSunImage = NULL;
	SphVertices = NULL;
	m_bHasTexture = false;
}

vtDayDome::~vtDayDome()
{
	if (SphVertices) delete[] SphVertices;
	if (m_pSunImage)
		m_pSunImage->Release();
	if (m_pMats)
		m_pMats->Release();
}

void vtDayDome::Create(int depth, float radius, const char *sun_texture)
{
	SetName2("DayDome");
	m_pDomeGeom = new vtGeom();
	m_pDomeGeom->SetName2("DayDomeGeom");

	AddChild(m_pDomeGeom);

	// Only a single material is needed for the dome, since vertex colors are
	// used to change the color of the sky.
	m_pMats = new vtMaterialArray();
	m_pMat = new vtMaterial();
	m_pMat->SetLighting(false);
	m_pMat->SetCulling(false);
	m_pMats->Append(m_pMat);
	m_pDomeGeom->SetMaterials(m_pMats);

	int res = 16;
	m_pDomeMesh = new vtMesh(GL_TRIANGLE_STRIP, VT_Colors | VT_TexCoords, res*res);
	m_pDomeMesh->CreateEllipsoid(FPoint3(1.0f, 1.0f, 1.0f), res, true);
	m_pDomeGeom->AddMesh(m_pDomeMesh, 0);
	m_pDomeMesh->Release();	// pass ownership to Geometry

	NumVertices = m_pDomeMesh->GetNumVertices();
	SphVertices = new FPoint3[NumVertices];
	ConvertVertices();

	TimeOfDay = 0;
	Radius = radius;
	Scale3(Radius, Radius, Radius);

	// Set default horizon, azimuth and sunset colors
	DayHorizonCol.Set(0.5f, 1.0f, 1.0f);
	DayAzimuthCol.Set(0.3f, 0.3f, 0.9f);
	SunsetCol.Set(1.0f, 0.55f, 0.0f);
	SetMaxSunsetAngle(0.5f);
	SetSunModifier(0.33f);

	// Set default Interpolation cutoff
	SetInterpCutoff(0.3f);

	// Set Maximum rotational change dependent only on hours, minutes and secs
	MaxTimeOfDay = TIME_TO_INT(24, 0, 0);

	SetDawnTimes(5, 0, 7, 0);
	SetDuskTimes(17, 0, 19, 0);

	if (sun_texture && *sun_texture)
	{
		int idx = -1;

		m_pSunImage = new vtImage(sun_texture);
		if (m_pSunImage->LoadedOK())
		{
			idx = m_pMats->AddTextureMaterial(m_pSunImage,
							 false, false,	// culling, lighting
							 true, true,	// transp, additive
							 1.0f,			// diffuse
							 1.0f, 1.0f);	// alpha, emmisive
		}
		if (idx == -1)
			return;		// could not load texture, cannot have sun

		// Create sun
		vtGeom *pGeom = new vtGeom();
		m_pSunShape = new vtMovGeom(pGeom);
		vtMesh *SunMesh = new vtMesh(GL_TRIANGLE_FAN, VT_TexCoords, 4);

		m_pSunMat = m_pMats->GetAt(idx);

		SunMesh->CreateRectangle(0.50f, 0.50f);
		pGeom->SetMaterials(m_pMats);
		pGeom->AddMesh(SunMesh, idx);
		SunMesh->Release();	// pass ownership to Geometry

		m_pSunShape->SetName2("Sun");

		// Y translation
		FMatrix4 trans;
		trans.Identity();
		trans.Translate(FPoint3(0.0f, 0.90f, 0.0f));
		SunMesh->TransformVertices(trans);

		AddChild(m_pSunShape);
	}
}

//
//
void vtDayDome::SetRadius(float radius)
{
	Identity();
	Scale3(radius, radius, radius);
}

void vtDayDome::SetDayColors(const RGBf &horizon, const RGBf &azimuth)
{
	DayHorizonCol = horizon;
	DayAzimuthCol = azimuth;
}

void vtDayDome::SetSunsetColor(const RGBf &sunset)
{
	SunsetCol = sunset;
}

bool vtDayDome::SetTexture(const char *filename)
{
	vtImage *pImage = new vtImage(filename);
#ifndef VTLIB_PSM
	if (!pImage->LoadedOK())
		return false;
#endif

	// create and apply the texture material
	int index = m_pMats->AddTextureMaterial(pImage, false, false);
	pImage->Release();	// pass ownership to the Material

	// set the UV values to cylindrically project the texture onto the hemisphere
	int i, j;
	int verts = m_pDomeMesh->GetNumVertices();
	FPoint3 p;
	FPoint2 uv;
	FBox3 box;
	m_pDomeMesh->GetBoundBox(box);

	for (i = 0; i < verts; i++)
		m_pDomeMesh->SetVtxColor(i, RGBf(1,1,1));	// all white vertices

	// First way: do texture projection based on vertex position.  This is
	// not ideal for the case of sky domes because of cylindrical UV
	// wraparound: there will be a UV texture seam.
	/*
	for (i = 0; i < verts; i++)
	{
		p = m_pDomeMesh->GetVtxPos(i);
		uv.x = (float) atan2(p.z, p.x) / PI2d;
		uv.y = (p.y - box.min.y) / (box.max.y - box.min.y);
		uv.y = 1.0 - uv.y;	// flip
		m_pDomeMesh->SetVtxTexCoord(i, uv);
	}
	*/
	// Second way: use our special knowledge of how the sphere vertices are
	// constructed to set the UV values evently from 0 to 1 without
	// wraparound.
	int ysize = (int) (sqrt((double)verts/4));
	int xsize = ysize * 4;
	for (i =0; i < xsize; i++)
	{
		uv.x = (float)i / (float)(xsize-1);
		for (j = 0; j < ysize; j++)
		{
			uv.y = 1.0f - (float)j / (float)(ysize-1);
			m_pDomeMesh->SetVtxTexCoord(i*ysize+j, uv);
		}
	}

	m_pDomeGeom->SetMeshMatIndex(m_pDomeMesh, index);
	m_bHasTexture = true;

	return true;
}

/**
 * Sunrise and sunset cause a warm-colored circular glow at the point where
 * the sun is touching the horizon.  This function sets the radius of that
 * circle.
 *
 * \param sunset_angle The radius angle in radians.  A typical value is 0.5
 * (around 28 degrees).
 */
void vtDayDome::SetMaxSunsetAngle(float sunset_angle)
{
	MaxSunsetAngle = sunset_angle;
}

void vtDayDome::SetInterpCutoff(float cutoff)
{
	Cutoff = cutoff;
}

void vtDayDome::SetSunModifier(float sunpct)
{
	SunTimePctMod = sunpct;
}


/**
 * Sets the time of day (or night).
 * \param time			Time in seconds since midnight.
 * \param bFullRefresh	Pass true to force the sky colors to be updated;
 *	otherwise, they will only be updated when absolutely necessary, which
 * is during dawn and dusk.
 */
void vtDayDome::SetTimeOfDay(int time, bool bFullRefresh)
{
	TimeOfDay = time;

	// recolor vertices during dusk and dawn sequences
	if (bFullRefresh ||
		(TimeOfDay >= DawnStartTime && TimeOfDay <= DawnEndTime) ||
		(TimeOfDay >= DuskStartTime && TimeOfDay <= DuskEndTime))
		ApplyDayColors();

	// determine sun color
	if (m_pSunShape != NULL)
	{
		// move sun: Set the right transformation for the time of day
		m_pSunShape->Identity();

		Scale3(Radius, Radius, Radius);//
//		m_pSunShape->Rotate(TRANS_XAxis, PI/2.0f);
//		m_pSunShape->Turn(TRANS_YAxis, PI/2.0f);

		// Set the north star around 20 degrees above the horizon
		m_pSunShape->RotateLocal(FPoint3(1,0,0), 0.1f * PIf);

		// rotate to move across sky
		m_pSunShape->RotateLocal(FPoint3(0,0,1), ((float)TimeOfDay/MaxTimeOfDay) * PI2f - PIf);
	}
}

//
//
void vtDayDome::SetSunColor(const RGBf &color)
{
#ifdef VTLIB_PSM
	if (m_pSunMat)
		m_pSunMat->SetDiffuse1(color);
#else
	if (m_pSunMat)
		m_pSunMat->vtMaterialBase::SetDiffuse1(color);
#endif
}

//
//
void vtDayDome::ConvertVertices()
{
	FPoint3 p, psph;

	int num = m_pDomeMesh->GetNumVertices();
	for (int i = 0; i < num; i++)
	{
		p = m_pDomeMesh->GetVtxPos(i);
		PT_CART_TO_SPHERE(p, psph);
		SphVertices[i] = psph;
	}
}

const float NITE_GLO = 0.15f;

//
//
void vtDayDome::ApplyDayColors()
{
	RGBf vtxcol;
	FPoint3 psph;
	float phipct, phipct_cut, thetapct, sunpct;
	int i = 0;

	vtMesh *mesh = m_pDomeMesh;

	float duskpct = NITE_GLO + (1.0f - NITE_GLO) * (float)(DuskEndTime - TimeOfDay)/DuskDuration;
	float dawnpct = NITE_GLO + (1.0f - NITE_GLO) * (float)(TimeOfDay - DawnStartTime)/DawnDuration;
	float midseq, midseqtime, midseqpct;

	if (TimeOfDay >= DuskStartTime && TimeOfDay <= DuskEndTime)
	{
		// dusk
		m_fademod = duskpct;
		midseq = DuskMidSeq;
		midseqtime = DuskMidSeqTime;
		midseqpct = fabsf(midseqtime - (float)TimeOfDay)/midseq;
	}
	else if (TimeOfDay >= DawnStartTime && TimeOfDay <= DawnEndTime)
	{
		// dawn
		m_fademod = dawnpct;
		midseq = DawnMidSeq;
		midseqtime = DawnMidSeqTime;
		midseqpct = fabsf(midseqtime - (float)TimeOfDay)/midseq;
	}
	else if (TimeOfDay <= DawnStartTime || TimeOfDay >= DuskEndTime)
	{
		m_fademod = NITE_GLO;
	}
	else
		// day
		m_fademod = 1.0f;

	// Don't actually change the dome color if it already has a texture
	if (m_bHasTexture)
		return;

	// Set day colors
	for (i = 0; i < mesh->GetNumVertices(); i++)
	{
		FPoint3 p = mesh->GetVtxPos(i);
		psph = SphVertices[i];

		phipct = fabsf(PID2f - fabsf(psph.phi))/PID2f;
		thetapct = (PID2f - fabsf(psph.theta))/PID2f;

		// Ordering of color layering is important!!!
		// Color is interpolated along phi from 0 to Cutoff
		if ((1.0f - phipct) <= Cutoff)
		{
			phipct_cut = (1.0f - phipct)/Cutoff;
			vtxcol = (DayHorizonCol * (1.0f - phipct_cut)) + (DayAzimuthCol * phipct_cut);
		}
		else
		{
			vtxcol = DayAzimuthCol;
		}
		vtxcol *= m_fademod;

		if (TimeOfDay >= DuskStartTime && TimeOfDay <= DuskSeqEnd)
		{
			if (((1.0f - phipct) <= MaxSunsetAngle) && (fabsf(psph.theta) > PID2f))
			{
				phipct_cut = (1.0f - phipct)/MaxSunsetAngle;
				sunpct = (1.0f - phipct_cut) * (1.0f - midseqpct) * (-thetapct);
				vtxcol = (vtxcol * (1.0f - sunpct)) + (SunsetCol * sunpct);
			}
		}
		else if (TimeOfDay >= DawnSeqStart && TimeOfDay <= DawnEndTime)
		{
			if (((1.0f - phipct) <= MaxSunsetAngle) && (fabsf(psph.theta) <= PID2f))
			{
				phipct_cut = (1.0f - phipct)/MaxSunsetAngle;
				sunpct = (1.0f - phipct_cut) * (1.0f - midseqpct) * thetapct;
				vtxcol = (vtxcol * (1.0f - sunpct)) + (SunsetCol * sunpct);
			}
		}
		mesh->SetVtxColor(i, vtxcol);
	}
	mesh->ReOptimize();
}

//
//
void vtDayDome::SetDuskTimes(int start_hr, int start_min,
							 int end_hr, int end_min)
{
	DuskStartTime = TIME_TO_INT(start_hr, start_min, 0);
	DuskEndTime = TIME_TO_INT(end_hr, end_min, 0);
	DuskDuration = (float)(DuskEndTime - DuskStartTime);
	DuskSeqEnd = (float)DuskEndTime - (DuskDuration * SunTimePctMod);
	DuskMidSeq = (DuskSeqEnd - (float)DuskStartTime)/2.0f;
	DuskMidSeqTime = DuskSeqEnd - DuskMidSeq;
}

//
//
void vtDayDome::SetDawnTimes(int start_hr, int start_min,
							 int end_hr, int end_min)
{
	DawnStartTime = TIME_TO_INT(start_hr, start_min, 0);
	DawnEndTime = TIME_TO_INT(end_hr, end_min, 0);
	DawnDuration = (float)(DawnEndTime - DawnStartTime);
	DawnSeqStart = (float)DawnStartTime + (DawnDuration * SunTimePctMod);
	DawnMidSeq = (float)(DawnEndTime - DawnSeqStart)/2.0f;
	DawnMidSeqTime = (float)DawnEndTime - DawnMidSeq;
}


//
// vtStarDome
//
vtStarDome::vtStarDome()
{
	m_pMats = NULL;
	Starfield = NULL;
	m_pMoonImage = NULL;
}

vtStarDome::~vtStarDome()
{
	if (Starfield)
		delete[] Starfield;
	if (m_pMoonImage)
		m_pMoonImage->Release();
	if (m_pMats)
		m_pMats->Release();
}

void vtStarDome::Create(const char *starfile, float radius, float brightness,
					  const char *moon_texture)
{
	SetName2("StarDome");
	m_pStarGeom = new vtGeom();
	m_pStarGeom->SetName2("StarDomeGeom");

	TimeOfDay = 0;
	NumStars = 0;
	RelativeBrightness = brightness;

	// Read in the star data
	ReadStarData(starfile);

	m_pMats = new vtMaterialArray();
	int star_mat = m_pMats->AddRGBMaterial1(RGBf(0,0,0), false, false);
	vtMaterial *pMat = m_pMats->GetAt(star_mat);
	pMat->SetTransparent(true, true);

	// Need a material?
	m_pStarGeom->SetMaterials(m_pMats);

	m_pStarMesh = new vtMesh(GL_POINTS, VT_Colors, NumStars);
	AddStars(m_pStarMesh);
	m_pStarGeom->AddMesh(m_pStarMesh, star_mat);
	m_pStarMesh->Release();		// pass ownership to Geometry
	AddChild(m_pStarGeom);

	// Set Maximum rotational change dependent only on hours, minutes and secs
	MaxTimeOfDay = TIME_TO_INT(24, 0, 0);

	SetDawnTimes(5, 0, 7, 0);
	SetDuskTimes(17, 0, 19, 0);

	if (moon_texture && *moon_texture)
	{
		int idx = -1;
		m_pMoonImage = new vtImage(moon_texture);
		if (m_pMoonImage->LoadedOK())
		{
			idx = m_pMats->AddTextureMaterial(m_pMoonImage,
								 false, false,	// culling, lighting
								 true, true,	// transparent, additive
								 1.0f,			// diffuse
								 1.0f, 1.0f);	// alpha, emmisive
		}
		if (idx == -1)
			return;		// could not load texture, cannot have sun

		// Create moon
		vtGeom *pGeom = new vtGeom();
		m_pMoonGeom = new vtMovGeom(pGeom);
		vtMesh *MoonMesh = new vtMesh(GL_TRIANGLE_FAN, VT_TexCoords, 4);

		MoonMesh->CreateRectangle(0.1f, 0.1f);
		pGeom->SetMaterials(m_pMats);
		pGeom->AddMesh(MoonMesh, idx);
		MoonMesh->Release();	// pass ownership to Geometry

		m_pMoonGeom->SetName2("Moon");

		// Y translation
		FMatrix4 trans;
		trans.Identity();
		trans.Translate(FPoint3(0.0f, 0.90f, 0.0f));
		MoonMesh->TransformVertices(trans);

		AddChild(m_pMoonGeom);
	}
	SetRadius(radius);
}

//
//
void vtStarDome::SetRadius(float radius)
{
	Radius = radius;
	Identity();
	Scale3(radius, radius, radius);
}

/**
 * Sets the time of day (or night).
 * \param time			Time in seconds since midnight.
 */
void vtStarDome::SetTimeOfDay(int time)
{
	TimeOfDay = time;

	// Set the right transformation for the time of night
	Identity();
	Scale3(Radius*0.93f, Radius*0.93f, Radius*0.93f);

	// Set the north star around 20 degrees above the horizon
	RotateLocal(FPoint3(1,0,0), 0.1f * PIf);

	RotateLocal(FPoint3(0,0,1), ((float)TimeOfDay/MaxTimeOfDay) * PIf * 2.0f);

	FadeStars();
}

// Helper function to compare stars by magnitude
int	compare_mag(const void *star1, const void *star2)
{
	vtStarDome::Star *pstar1 = (vtStarDome::Star *)star1;
	vtStarDome::Star *pstar2 = (vtStarDome::Star *)star2;

	float magdiff = pstar1->mag - pstar2->mag;
	if (magdiff < 0) return -1;
	else if (magdiff > 0) return 1;
	else return 0;
}


//
// Add Stars to GeoMesh Vertex List ordered by magnitude
//
void vtStarDome::AddStars(vtMesh *mesh)
{
	FPoint3	temppt;
	RGBf	tempcol, white(1.0f, 1.0f, 1.0f);

	// Add each star to the GeoMesh's vertex array
	for (int i = 0; i < NumStars; i++)
	{
		mesh->AddVertex(Starfield[i].cartpt);
		tempcol = (Starfield[i].relmag > 1.0f) ? white : white * Starfield[i].relmag;
#if 0
		TRACE("Star #%d %f RGB(%f,%f,%f)\n", i, Starfield[i].relmag,
			tempcol.r, tempcol.g, tempcol.b);
#endif
		mesh->SetVtxColor(i, tempcol);
	}
}


/**
 * Add Constellations to GeoMesh Vertex List
 */
void vtStarDome::AddConstellation(vtMesh *mesh)
{
	// to do
}

/**
 * Determines how many vertices are need to be included in the mesh, based
 * on the time of day and when dawn and dusk are supposed to be.
 */
void vtStarDome::FadeStars()
{
	int numvalidstars = 0;

	vtMesh *mesh = m_pStarMesh;

	float duskpct, dawnpct;
	float magmod;

	duskpct = (float)(TimeOfDay - DuskStartTime)/DuskDuration;
	dawnpct = (float)(DawnEndTime - TimeOfDay)/DawnDuration;
	if (TimeOfDay >= DuskStartTime && TimeOfDay <= DuskEndTime)
		magmod = duskpct;
	else if (TimeOfDay >= DawnStartTime && TimeOfDay <= DawnEndTime)
		magmod = dawnpct;
	else magmod = 1.00f;

	for (int i = 0; i < NumStars; i++)
	{
		numvalidstars++;
		if (Starfield[i].mag >= (HighMag + MagRange * magmod)) break;
	}
//	mesh->SetNumVertices(numvalidstars);		// TODO
}


//
// Set Star File
//
void vtStarDome::SetStarFile(char *starpath)
{
	strcpy(StarFile, starpath);
}


/**
 * Read the star data files and throw into array structures.
 */
bool vtStarDome::ReadStarData(const char *starfile)
{
	int n, numstars = 0, num_file_stars = 0;
	float himag, lomag;
	int ra_h, ra_m, dec_d, dec_m, dec_s;
	float ra_s;

	FILE *starfp = NULL;
	Star *curstar;

	if (!(starfp = fopen(starfile, "r")))
	{
//		TRACE("Couldn't open %s\n", fname);
		return false;
	}

	if ((n = fscanf(starfp, "%d %f %f\n", &num_file_stars,
					  &himag, &lomag)) == EOF)
	{
//		TRACE("Couldn't read number of stars from %s\n", fname);
		return false;
	}

	Starfield = new Star[num_file_stars];

	while (numstars++ != num_file_stars)
	{
		curstar =  &Starfield[NumStars];
		n = fscanf(starfp, "%d %d %f %d %d %d %f\n", &ra_h,
			&ra_m, &ra_s, &dec_d, &dec_m, &dec_s, &curstar->mag);

		curstar->ra = (float)ra_h + (float)ra_m/60.0f + (float)ra_s/3600.0f;
		curstar->dec = (float)dec_d + (float)dec_m/60.0f + (float)dec_s/3600.0f;

		if (n == EOF)
			return false;

		ConvertStarCoord(curstar);

//		if (curstar->mag <= 3.35)	// convenient for finding the big dipper
		if (curstar->mag <= MAX_MAGNITUDE)
			NumStars++;
	}
	lomag = MAX_MAGNITUDE;

//	TRACE("CONFIRM NUM STARS = %d\n", NumStars);
	fclose(starfp);

	// sort the stars by magnitude
	qsort((void *)Starfield, NumStars, sizeof(Star), compare_mag);

	HighMag = himag;
	LowMag = lomag;
	MagRange = fabsf(HighMag) + LowMag;

	// Pre calculate the relative magnitude percentage of each star
	for (int i = 0; i < NumStars; i++)
	{
		Starfield[i].relmag = (MagRange - (Starfield[i].mag + fabsf(HighMag)))
			/(MagRange/RelativeBrightness);
	}
	return true;
}


/**
 * Convert star coordinate information to x,y,z coordinates.
 */
void vtStarDome::ConvertStarCoord(Star *star)
{
	FPoint3 spherept;

	// Make sure the stars are drawn inside of the sky dome
	spherept.rho = 0.95f; // calculated for a unit  sphere

	// First off convert from sexagesimal to spherical notation for RA
	// Right Ascension corresponds to theta  Range: 0 - 24 hours
	spherept.theta = star->ra * (PI2f/24.0f);

	// Declination corresponds to phi Range: -90 to 90 degrees
	spherept.phi = DEG_TO_RAD(star->dec);

	PT_SPHERE_TO_CART(spherept, star->cartpt);
}

/**
 * Sets the beginning, end, and duration of dusk.
 */
void vtStarDome::SetDuskTimes(int start_hr, int start_min,
						   int end_hr, int end_min)
{
	DuskStartTime = TIME_TO_INT(start_hr, start_min, 0);
	DuskEndTime = TIME_TO_INT(end_hr, end_min, 0);
	DuskDuration = (float)(DuskEndTime - DuskStartTime);
}

/**
 * Sets the beginning, end, and duration of dawn.
 */
void vtStarDome::SetDawnTimes(int start_hr, int start_min,
						   int end_hr, int end_min)
{
	DawnStartTime = TIME_TO_INT(start_hr, start_min, 0);
	DawnEndTime = TIME_TO_INT(end_hr, end_min, 0);
	DawnDuration = (float)(DawnEndTime - DawnStartTime);
}



