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
#define MIN_AMB		0.0f
#define MAX_AMB		1.0f
#define AMB_RANGE	(MAX_AMB-MIN_AMB)

// maximum directional light intensity
#define MAX_INT		1.0f

// only show stars up to this magnitude
#define MAX_MAGNITUDE	5.8f

#define SQR(x)	x * x
#define TWO_PI (PI * 2.0f)
#define QUARTER_PI	(PI * 0.25f)

// Radians <-> Degrees Macros, single angle
#define RAD_TO_DEG(x) (x * (180.0f/PIf))
#define DEG_TO_RAD(x) (x * (PIf/180.0f))

// Coordinate change macros for a point class
// Here x = rho, y = theta, z = phi
#define rho		x
#define theta	y
#define phi		z

#define PT_CART_TO_SPHERE(A, B) {\
	B.rho = sqrtf(SQR(A.x) + SQR(A.y) + SQR(A.z));\
	B.theta = atan2f(-A.z, A.x);\
	B.phi = atan2f(A.y, sqrt(SQR(A.x) + SQR(A.z)));\
}

#define PT_SPHERE_TO_CART(A, B) {\
	B.x = A.rho * cosf(A.phi) * sinf(A.theta);\
	B.y = A.rho * cosf(A.phi) * cosf(A.theta);\
	B.z = -A.rho * sinf(A.phi);\
}

#define TIME_TO_INT(hr, min, sec) ((hr * 60 + min) * 60 + sec)

///////////////////////////////////////////////////////////////////////////
// SkyDome
//

//
// creates a complete SkyDome, which includes a DayDome and a StarDome.
//
void vtSkyDome::Create(const char *starfile, int depth, float radius,
					 const char *sun_texture, const char *moon_texture)
{
	Radius = radius;

	// Create the vtDayDome
	m_pDayDome = new vtDayDome();
	m_pDayDome->Create(depth, Radius, sun_texture);
	m_pDayDome->ApplyDayColors();
	m_pDayDome->SetName2("DayDome");

	// Create the vtStarDome
	m_pStarDome = new vtStarDome();
	m_pStarDome->Create(starfile, Radius, 2.0f, moon_texture);
	m_pStarDome->SetName2("StarDome");

	// build heirarchy
	this->AddChild(m_pDayDome);
	this->AddChild(m_pStarDome);

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

//
//
void vtSkyDome::SetTimeOfDay(int hr, int min, int sec)
{
	SetTimeOfDay(TIME_TO_INT(hr, min, sec));
}

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
		m_pSunLight->GetLight()->SetColor2(color);
		m_pSunLight->GetLight()->SetAmbient2(RGBf(ambient, ambient, ambient));
	}
}

//
//
void vtSkyDome::SetDuskTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec)
{
	DuskStartTime = TIME_TO_INT(start_hr, start_min, start_sec);
	DuskEndTime = TIME_TO_INT(end_hr, end_min, end_sec);

	if (m_pDayDome)
		m_pDayDome->SetDuskTimes(start_hr, start_min, start_sec, end_hr, end_min, end_sec);
	if (m_pStarDome)
		m_pStarDome->SetDuskTimes(start_hr, start_min, start_sec, end_hr, end_min, end_sec);
}

//
//
void vtSkyDome::SetDawnTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec)
{
	DawnStartTime = TIME_TO_INT(start_hr, start_min, start_sec);
	DawnEndTime = TIME_TO_INT(end_hr, end_min, end_sec);

	if (m_pDayDome)
		m_pDayDome->SetDawnTimes(start_hr, start_min, start_sec, end_hr, end_min, end_sec);
	if (m_pStarDome)
		m_pStarDome->SetDawnTimes(start_hr, start_min, start_sec, end_hr, end_min, end_sec);
}


vtSkyDome::vtSkyDome()
{
	m_pDayDome = NULL;
	m_pStarDome = NULL;
	m_pSunLight = NULL;
}

//
//
void vtSkyDome::SetDayColors(RGBf& horizon, RGBf& azimuth)
{
	if (m_pDayDome)
	{
		m_pDayDome->SetDayColors(horizon, azimuth);
		m_pDayDome->ApplyDayColors();
	}
}

//
//
void vtSkyDome::SetSunsetColor(RGBf& sunset)
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


///////////////////////////////////////////////////////////////////////////
// DayDome
//

void vtDayDome::Create(int depth, float radius,
					 const char *sun_texture)
{
	SetName2("DayDome");
	m_pDomeGeom = new vtGeom();
	m_pDomeGeom->SetName2("DayDomeGeom");

	AddChild(m_pDomeGeom);

	m_pMats = new vtMaterialArray();
	m_pMat = new vtMaterial();
	m_pMat->SetLighting(false);
	m_pMat->SetCulling(false);
	m_pMats->Append(m_pMat);
	m_pDomeGeom->SetMaterials(m_pMats);

	int res = 16;
	m_pDomeMesh = new vtMesh(GL_TRIANGLE_STRIP, VT_Colors, res*res);
	m_pDomeMesh->CreateEllipsoid(FPoint3(1.0f, 1.0f, 1.0f), res, true);
	m_pDomeGeom->AddMesh(m_pDomeMesh, 0);

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

	SetDawnTimes(5, 0, 0, 7, 0, 0);
	SetDuskTimes(17, 0, 0, 19, 0, 0);

	if (sun_texture)
	{
		// Create sun
		vtGeom *pGeom = new vtGeom();
		m_pSunShape = new vtMovGeom(pGeom);
		vtMesh *SunMesh = new vtMesh(GL_TRIANGLE_FAN, VT_TexCoords, 4);

//		vtImage *pImage = new vtImage(sun_texture, GL_RGBA4);
		int idx = m_pMats->AddTextureMaterial2(sun_texture,
							 false, false,	// culling, lighting
							 true, true,	// transp, additive
							 1.0f,			// diffuse
							 1.0f, 1.0f);	// alpha, emmisive

		m_SunApp = m_pMats->GetAt(idx);

		SunMesh->CreateRectangle(0.50f, 0.50f);
		pGeom->SetMaterials(m_pMats);
		pGeom->AddMesh(SunMesh, idx);

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

//
//
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
void vtDayDome::SetSunColor(RGBf &color)
{
	if (m_SunApp)
		m_SunApp->vtMaterialBase::SetDiffuse1(color);
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

#define NITE_GLO 0.15f

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
void vtDayDome::SetDuskTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec)
{
	DuskStartTime = TIME_TO_INT(start_hr, start_min, start_sec);
	DuskEndTime = TIME_TO_INT(end_hr, end_min, end_sec);
	DuskDuration = (float)(DuskEndTime - DuskStartTime);
	DuskSeqEnd = (float)DuskEndTime - (DuskDuration * SunTimePctMod);
	DuskMidSeq = (DuskSeqEnd - (float)DuskStartTime)/2.0f;
	DuskMidSeqTime = DuskSeqEnd - DuskMidSeq;
}

//
//
void vtDayDome::SetDawnTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec)
{
	DawnStartTime = TIME_TO_INT(start_hr, start_min, start_sec);
	DawnEndTime = TIME_TO_INT(end_hr, end_min, end_sec);
	DawnDuration = (float)(DawnEndTime - DawnStartTime);
	DawnSeqStart = (float)DawnStartTime + (DawnDuration * SunTimePctMod);
	DawnMidSeq = (float)(DawnEndTime - DawnSeqStart)/2.0f;
	DawnMidSeqTime = (float)DawnEndTime - DawnMidSeq;
}


//
//
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

	vtMaterialArray *pApps = new vtMaterialArray();
	vtMaterial *pMat = new vtMaterial();
	pMat->SetCulling(false);
	pMat->SetLighting(false);
	pMat->SetDiffuse(0.0f, 0.0f, 0.0f, 1.0f);
	pMat->SetSpecular(0.0f, 0.0f, 0.0f);
	pMat->SetAmbient(0.0f, 0.0f, 0.0f);
	pMat->SetEmission(0.0f, 0.0f, 0.0f);
	pMat->SetTransparent(true, true);
	int star_mat = pApps->Append(pMat);

	// Need a material?
	m_pStarGeom->SetMaterials(pApps);

	m_pStarMesh = new vtMesh(GL_POINTS, VT_Colors, NumStars);
	AddStars(m_pStarMesh);
	m_pStarGeom->AddMesh(m_pStarMesh, star_mat);
	AddChild(m_pStarGeom);

	// Set Maximum rotational change dependent only on hours, minutes and secs
	MaxTimeOfDay = TIME_TO_INT(24, 0, 0);

	SetDawnTimes(5, 0, 0, 7, 0, 0);
	SetDuskTimes(17, 0, 0, 19, 0, 0);

	if (moon_texture)
	{
		// Create moon
		vtGeom *pGeom = new vtGeom();
		m_pMoonGeom = new vtMovGeom(pGeom);
		vtMesh *MoonMesh = new vtMesh(GL_TRIANGLE_FAN, VT_TexCoords, 4);

		int moon_mat = pApps->AddTextureMaterial2(moon_texture,
							 false, false,	// culling, lighting
							 true, true,	// transparent, additive
							 1.0f,			// diffuse
							 1.0f, 1.0f);	// alpha, emmisive

		MoonMesh->CreateRectangle(0.1f, 0.1f);
		pGeom->SetMaterials(pApps);
		pGeom->AddMesh(MoonMesh, moon_mat);

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

//
//
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
int	compare_mag(const void* star1, const void* star2)
{
	Star *pstar1 = (Star *)star1;
	Star *pstar2 = (Star *)star2;

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
		mesh->SetVtxPos(i, Starfield[i].cartpt);
		tempcol = (Starfield[i].relmag > 1.0f) ? white : white * Starfield[i].relmag;
#if 0
		TRACE("Star #%d %f RGB(%f,%f,%f)\n", i, Starfield[i].relmag,
			tempcol.r, tempcol.g, tempcol.b);
#endif
		mesh->SetVtxColor(i, tempcol);
	}
}


//
// Add Constellations to GeoMesh Vertex List
//
void vtStarDome::AddConstellation(vtMesh *mesh)
{
	// to do
}

//
// determines how many vertices need to be included in the mesh, based
// on the time of day and when dawn and dusk are supposed to be
//
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


//
// Read the star data files and throw into array structures
//
void vtStarDome::ReadStarData(const char *starfile)
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
		return;
	}

	if ((n = fscanf(starfp, "%d %f %f\n", &num_file_stars,
		              &himag, &lomag)) == EOF)
	{
//		TRACE("Couldn't read number of stars from %s\n", fname);
		return;
	}

	if (!(Starfield = new Star[num_file_stars]))
	{
//		TRACE("Couldn't create space for stars\n");
		return;
	}

	while (numstars++ != num_file_stars)
	{
		curstar =  &Starfield[NumStars];
		n = fscanf(starfp, "%d %d %f %d %d %d %f\n", &ra_h,
			&ra_m, &ra_s, &dec_d, &dec_m, &dec_s, &curstar->mag);

		curstar->ra = (float)ra_h + (float)ra_m/60.0f + (float)ra_s/3600.0f;
		curstar->dec = (float)dec_d + (float)dec_m/60.0f + (float)dec_s/3600.0f;

		if (n == EOF)
			return;

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
}


//
// converts star coordinate information to x,y,z coordinates
//
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

//
// sets the beginning, end, and duration of dusk
//
void vtStarDome::SetDuskTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec)
{
	DuskStartTime = TIME_TO_INT(start_hr, start_min, start_sec);
	DuskEndTime = TIME_TO_INT(end_hr, end_min, end_sec);
	DuskDuration = (float)(DuskEndTime - DuskStartTime);
}

//
// sets the beginning, end, and duration of dawn
//
void vtStarDome::SetDawnTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec)
{
	DawnStartTime = TIME_TO_INT(start_hr, start_min, start_sec);
	DawnEndTime = TIME_TO_INT(end_hr, end_min, end_sec);
	DawnDuration = (float)(DawnEndTime - DawnStartTime);
}



