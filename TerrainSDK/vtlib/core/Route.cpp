//
// Route.cpp
//
// Creates route geometry, drapes on a terrain
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
//////////////////////////////////////////////////////////////////////
// 
// Define a route on a terrain given (at minimum) an inorder list of 
//	2D corner points.
//
// 2001.09.30 JD - cloned from Fence.cpp
//	2001.10.03 JD - added point capture
//
//////////////////////////////////////////////////////////////////////

#include "vtlib/vtlib.h"
#include "Light.h"
#include "Route.h"
#include "Terrain.h"
#include "Tower3D.h"

#define ROUTE_POLE_SPACING	(100f * m_fRouteScale)	// 100 meters

#define LONGEST_ROUTE 2000   // in meters

vtMaterialArray *vtRoute::m_pRouteMats;
float vtRoute::m_fRouteScale;	// route size is exaggerated by this amount

///////////////////

vtRoute::vtRoute(float fHeight, float fSpacing, float fOffR
		, float fOffL, float fStInc, vtString sName, vtTerrain* pT)
{
	m_bClosed = false;
	m_bBuilt = false;
	m_fHeight = fHeight;
	m_fSpacing = fSpacing;

	m_sBranchName = sName;
	m_fOffsetLeft = fOffL;
	m_fOffsetRight = fOffR;
	m_fStationIncrement = fStInc;
	m_pTheTerrain = pT;

	m_PostSize.Set(0.13f, m_fHeight, 0.13f);

	// convert to world coordinates
	m_PostSize *= (WORLD_SCALE * m_fRouteScale);

	m_pRouteGeom = new vtGeom;
	m_pRouteGeom->SetName2("Route");

	if (m_pRouteMats == NULL)
		CreateMaterials();
	m_pRouteGeom->SetMaterials(m_pRouteMats);

#if 0
	// wrap each route in a LOD node so that it is not drawn from far away
	m_pLOD = new vtLOD();
	float range = 2000.0f * WORLD_SCALE;
	m_pLOD->SetRanges(&range, 1);

	m_pLOD->AddChild(m_pRouteGroup);
#else
	m_pLOD = NULL;
#endif
}


void vtRoute::add_point(const DPoint2 &epos)
{
	PolePlacement place;
	place.m_Point = epos;
	place.m_StationIndex = -1;

	int numroutepts = m_aRoutePlaces.GetSize();

	// check distance
	if (numroutepts > 0)
	{
		DPoint2 LastPt = m_aRoutePlaces.GetAt(numroutepts - 1).m_Point;

		double distance = (LastPt - epos).Length();

		if (distance <= LONGEST_ROUTE)
			m_aRoutePlaces.Append(place);
	}
	else
		m_aRoutePlaces.Append(place);
}

int vtRoute::m_mi_woodpost;
int vtRoute::m_mi_wire;
int vtRoute::m_mi_metalpost;

void vtRoute::CreateMaterials()
{
	m_pRouteMats = new vtMaterialArray();

	// create wirefence post textured material (0)
	vtString str = vtTerrain::m_strDataPath + "Culture/fencepost_64.bmp";
	m_mi_woodpost = m_pRouteMats->AddTextureMaterial2(str,
		true, true, false, false,
		TERRAIN_AMBIENT,
		TERRAIN_DIFFUSE,
		1.0f,		// alpha
		TERRAIN_EMISSIVE);

	// add wire material (1)
	m_mi_wire = m_pRouteMats->AddRGBMaterial(RGBf(0.0f, 0.0f, 0.0f), // diffuse
		RGBf(0.5f, 0.5f, 0.5f),	// ambient
		false, true, false,		// culling, lighting, wireframe
		0.6f);					// alpha

}


//
// Builds (or rebuilds) the geometry for a fence.
//
void vtRoute::BuildGeometry(vtHeightField *pHeightField)
{
	if (m_bBuilt)
		DestroyGeometry();

	m_sDataPath = m_pTheTerrain->m_strDataPath;

	// create surface and shape
	AddRouteMeshes(pHeightField);

	m_bBuilt = true;
}

void vtRoute::DestroyGeometry()
{
#if 1
	// Destroy the meshes so they can be re-made?
	while (m_pRouteGeom->GetNumMeshes())
	{
		vtMesh *pMesh = m_pRouteGeom->GetMesh(0);
		m_pRouteGeom->RemoveMesh(pMesh);
	}
#endif

	m_bBuilt = false;
}


void vtRoute::add_Pole(FPoint3 &p1, int iMatIdx)
{
//	vtString str("RouteData/Towers/test.obj");
//	vtNode* tower = m_pTheTerrain->LoadModel(str);
//
//	if (tower)
//	{
//		vtTransform* xform = new vtTransform;
//		xform->AddChild(tower);
//		m_pTheTerrain->PlantModelUTM(xform, p1.x,p1.y);
//	}

/*	// create pole block
	vtMesh *pPostMesh = new vtMesh(GL_TRIANGLE_FAN, VT_Normals | VT_TexCoords, 20);

	pPostMesh->CreateOptimizedBlock(m_PostSize);

	// scoot over and upwards to put it above ground
	FMatrix4 t;
	t.Identity();
	t.Translate(p1);
	pPostMesh->TransformVertices(t);

	m_pRouteGeom->AddMesh(pPostMesh, iMatIdx); */
}

void vtRoute::add_Pole(DPoint3 &p1, long lStationIndex)
{
//	vtString str("RouteData/Towers/test.obj");
	vtString str("RouteData/Towers/LatticeTower.obj");
//	vtNode* tower = m_pTheTerrain->LoadModel(str);
	if (lStationIndex<0)
	{
		// Add the profile without showing any structures
		return;	// no tower point here
	}
	vtNode* tower = m_StationArray.GetAt(lStationIndex).m_pTower;
	if (tower)
	{
		vtTransform* xform = new vtTransform;
		xform->AddChild(tower);
		xform->Scale3(WORLD_SCALE, WORLD_SCALE, WORLD_SCALE);
		// invert
		xform->RotateLocal(FPoint3(1,0,0), -PIf);
		// orient
		xform->RotateLocal(FPoint3(0,1,0), m_StationArray.GetAt(lStationIndex).dRadAzimuth+PID2f);

		m_pTheTerrain->PlantModelUTM(xform, p1.x,p1.y);
		tower->SetEnabled(true);
		m_pTheTerrain->m_pLodGrid->AppendToGrid(xform);
//		m_pTheTerrain->m_pTerrainGroup->AddChild(tower);
//		m_pLOD->AppendToGrid(xform);
	}

}


void vtRoute::AddRouteMeshes(vtHeightField *pHeightField)
{
	Array<DPoint2> posts;
	DPoint2 diff, dp;
	int i, nposts;
	vtStation stp;
	int numiterations = 300;	//for catenary spans

	int numroutepts = m_aRoutePlaces.GetSize();
	float fCurrentSpacing = m_fSpacing * m_fRouteScale;

	// first determine where the poles go, for this whole line
	for (i = 0; i < numroutepts; i++)
	{
			posts.Append(m_aRoutePlaces[i].m_Point);
	}

	// convert post positions to world-coordinate ground locations
	nposts = posts.GetSize();

	Array<FPoint3> p3;	// poles
	Array<FPoint3> wireAtt;

	FPoint3 fp;
	p3.SetSize(nposts);
	for (i = 0; i < nposts; i++)
	{
		float x, z;
		dp = posts[i];
		g_Proj.ConvertFromEarth(dp, x, z);


		// plant the pole on the terrain
		p3[i].x = x;
		p3[i].z = z;
		pHeightField->FindAltitudeAtPoint(p3[i], p3[i].y);
	}

	// generate the posts
	for (i = 0; i < nposts; i++)
	{
		DPoint3 dp3; 
		dp3.x = posts[i].x; dp3.y = posts[i].y;
		add_Pole(dp3, m_aRoutePlaces[i].m_StationIndex);
	}

	// and the wires
	if (nposts > 1)
	{
		for (long i=1; i<nposts; i++)
		{
			StringWires(i,pHeightField);
		}

/*		FPoint3 fpLast;

		vtMesh *pWireMesh = new vtMesh(GL_LINE_STRIP, 0, nposts*numiterations);
		int vidx = 0;
		for (j=0; j<7; j++)	//max number of wires
		{
			int start = vidx;
			for (i = 0; i < nposts; i++)
			{	// for each structure get the wire position
				long lInd0 = m_aRoutePlaces[i].m_StationIndex;
				vtStation st0 = m_StationArray[lInd0];
				if(j>=st0.m_iNUmWires) continue;	// skip if no wire.

				// catenary calcs go here.  got to get world coords
				FPoint3 fp0 = p3[i] + FPoint3(st0.m_fpWireAtt[j].x*WORLD_SCALE
					, (st0.m_fTowerHeight- st0.m_fpWireAtt[j].y)*WORLD_SCALE
					, st0.m_fpWireAtt[j].z*WORLD_SCALE);
					pWireMesh->AddVertex(fp0);
					vidx++;
//				if (i>0)
//				{
//					double dist = FPoint3(fp-fpLast).Length()/100;
//					DrawCat(fp0, fp1, m_StationArray[lInd0].dRadAzimuth, 3000.0, numiterations, pWireMesh);
//					vidx+=numiterations;
//				}

//				vidx++;
				fpLast = fp;
			}
			pWireMesh->AddStrip2(nposts, start);
		}
		m_pRouteGeom->AddMesh(pWireMesh, m_mi_wire);
*/	}
}

void vtRoute::StringWires(long ll, vtHeightField *pHeightField)
{	// pick pole numbers i and i-1 and string a wire between them
	long numiterations = 100;
	
	FPoint3 fp0, fp1;

	long lInd0 = m_aRoutePlaces[ll-1].m_StationIndex;
	long lInd1 = m_aRoutePlaces[ll].m_StationIndex;

	vtStation st0 = m_StationArray[lInd0];
	vtStation st1 = m_StationArray[lInd1];
//	g_Proj.ConvertFromEarth(st0.m_dpStationPoint, fp0);
//	g_Proj.ConvertFromEarth(st1.m_dpStationPoint, fp1);
		
	FPoint2 p0 = m_aRoutePlaces[ll-1].m_Point;
	
	FPoint2 p1 = m_aRoutePlaces[ll].m_Point;

	for (int j = 0; j< 7; j++)
	{
		if(j>=st1.m_iNUmWires) continue;	// skip if no wire.
		// catenary calcs go here.  got to get world coords

		vtMesh *pWireMesh = new vtMesh(GL_LINE_STRIP, 0, numiterations+1);

		g_Proj.ConvertFromEarth(p0, fp0.x, fp0.z);
		pHeightField->FindAltitudeAtPoint(fp0, fp0.y);
		fp0 = fp0 + FPoint3(st0.m_fpWireAtt[j].x*WORLD_SCALE
			, (st0.m_fTowerHeight- st0.m_fpWireAtt[j].y)*WORLD_SCALE
			, st0.m_fpWireAtt[j].z*WORLD_SCALE);
			pWireMesh->AddVertex(fp0);

		g_Proj.ConvertFromEarth(p1, fp1.x, fp1.z);
		pHeightField->FindAltitudeAtPoint(fp1, fp1.y);
		fp1 = fp1 + FPoint3(st1.m_fpWireAtt[j].x*WORLD_SCALE
			, (st1.m_fTowerHeight- st1.m_fpWireAtt[j].y)*WORLD_SCALE
			, st1.m_fpWireAtt[j].z*WORLD_SCALE);

		DrawCat(fp0, fp1, st0.dRadAzimuth, 30.0, numiterations, pWireMesh);
		pWireMesh->AddVertex(fp1);
		pWireMesh->AddStrip2(numiterations, 0);
		m_pRouteGeom->AddMesh(pWireMesh, m_mi_wire);
	}

}

bool vtRoute::close_route()
{
	if (m_aRoutePlaces.GetSize() > 2 && !m_bClosed)
	{
		DPoint2 FirstRoutePoint = m_aRoutePlaces.GetAt(0).m_Point;
		add_point(FirstRoutePoint);
		m_bClosed = true;
		return true; // redraw = true
	}
	else
		return false;
}

bool vtRoute::load(FILE *fp)
{
	// Unimplemented.

	return false;
}

void vtRoute::save(FILE *fp)
{
	// Print routes using TLCADD ground interface file.
	if (fp)
	{
		//first header line
//		fprintf(fp, "!  Branch  !            Location             !   Elevation    !   Elevation    !   Elevation    !\n");
//		fprintf(fp, "!  Name    !      X         !       Y        !      Left      !    Center      !     Right      !\n");

		for (long ll = 0; ll < m_StationArray.GetSize(); ll++)
		{
			fprintf(fp, "! %s   !  %12.2f  !  %12.2f  !  %12.2f  !  %12.2f  !  %12.2f  ! %d \n"
				, m_sBranchName
				, m_StationArray[ll].m_dpStationPoint.x
				, m_StationArray[ll].m_dpStationPoint.y
				, m_StationArray[ll].m_dpOffsetElevations.x
				, m_StationArray[ll].m_dpStationPoint.z
				, m_StationArray[ll].m_dpOffsetElevations.y 
				, m_StationArray[ll].m_iRoutePointNumber 
				);
		}
	}
}

bool vtRoute::AddStation(vtHeightField* pHeightField, vtStation &sp)
{
	// Get and add Station data
	return true;
}

bool vtRoute::AddStations(vtHeightField* pHeightField, RoutePoint rp)
{
	// Get and add Station data
	return true;
}

bool vtRoute::TestReader(FILE* f)
{	// Read the testload.txt file
	DPoint2 dpTemp;
	PolePlacement place;
	long lStationNumber=0;
	long lDeadEndOrAngle=0;	// for debugging
	float fTemp;
	int iTemp=0;
	char sBang[2]; 
	char sTemp[18];
	vtStation NewStation;

	if(!f)
		return false;

	while (!feof(f))
	{
		fTemp=0;iTemp=-1;
		// read the record.
		fscanf(f, "%s", sBang);
		if (strcmp("!", sBang)!=0)break;	//bad data. Go away.
		fscanf(f, "%s", sTemp);
			m_sBranchName = sTemp;
		fscanf(f, "%s", sBang);
		fscanf(f, "%f", &fTemp);
			NewStation.m_dpStationPoint.x=fTemp;
		fscanf(f, "%s", sBang);
		fscanf(f, "%f", &fTemp);
			NewStation.m_dpStationPoint.y=fTemp;
		fscanf(f, "%s", sBang);
		fscanf(f, "%f", &fTemp);
			NewStation.m_dpOffsetElevations.x=fTemp;
		fscanf(f, "%s", sBang);
		fscanf(f, "%f", &fTemp);
			NewStation.m_dpStationPoint.z=fTemp;
		fscanf(f, "%s", sBang);
		fscanf(f, "%f", &fTemp);
			NewStation.m_dpOffsetElevations.y=fTemp;
		fscanf(f, "%s", sBang);
		fscanf(f, "%d", &iTemp);
			NewStation.m_iRoutePointNumber=iTemp;

		m_StationArray.Append(NewStation);

		// Figure out the "corner" points


		if (NewStation.m_iRoutePointNumber>=0)
		{
			dpTemp.Set(NewStation.m_dpStationPoint.x
				, NewStation.m_dpStationPoint.y);
			place.m_Point = dpTemp;
			place.m_StationIndex = m_StationArray.GetSize()-1;
			m_aRoutePlaces.Append(place);
			lDeadEndOrAngle++;
		}
	}
	return true;
}

bool vtRoute::logReader(FILE* f)
{	// Read the .log file's first two entries
	DPoint2 dpTemp;
	long lStationNumber=0;
	long lDeadEndOrAngle=0;	// for debugging
	float fTemp;
	int iTemp=0;
	char sBang[2]; 
	char sTemp[18];
	vtStation NewStation;
	Array<vtStation> TempStation;

	if(!f)
		return false;

	while (!feof(f) && lStationNumber++ <2)
	{
		fTemp=0;iTemp=-1;
		// read the record.
		fscanf(f, "%s", sBang);
		if (strcmp("!", sBang)!=0)break;	//bad data. Go away.
		fscanf(f, "%s", sTemp);
			m_sBranchName = sTemp;
		fscanf(f, "%s", sBang);
		fscanf(f, "%f", &fTemp);
			NewStation.m_dpStationPoint.x=fTemp;
		fscanf(f, "%s", sBang);
		fscanf(f, "%f", &fTemp);
			NewStation.m_dpStationPoint.y=fTemp;
		fscanf(f, "%s", sBang);
		fscanf(f, "%f", &fTemp);
			NewStation.m_dpOffsetElevations.x=fTemp;
		fscanf(f, "%s", sBang);
		fscanf(f, "%f", &fTemp);
			NewStation.m_dpStationPoint.z=fTemp;
		fscanf(f, "%s", sBang);
		fscanf(f, "%f", &fTemp);
			NewStation.m_dpOffsetElevations.y=fTemp;
		fscanf(f, "%s", sBang);
		fscanf(f, "%d", &iTemp);
			NewStation.m_iRoutePointNumber=iTemp;
		NewStation.m_pTower = 0;
		NewStation.sStructure[0] = '\0';
		TempStation.Append(NewStation);
	}

	// Figure out the header contents.
	m_phTheHeader.dp2Station0UTM.x = TempStation[0].m_dpStationPoint.x;
	m_phTheHeader.dp2Station0UTM.y = TempStation[0].m_dpStationPoint.y;
	DPoint2 dpDiff;
	dpTemp.x =  TempStation[1].m_dpStationPoint.x;
	dpTemp.y =  TempStation[1].m_dpStationPoint.y;
	dpDiff = dpTemp-m_phTheHeader.dp2Station0UTM;
	m_phTheHeader.dStation0Azimuth = -atan2(dpDiff.y,dpDiff.x);

	return true;
}

bool vtRoute::p3DReader(FILE* fp3D)
{	// Read the .log to get the coords of the 1st two stations.
	//	then read the p3D to get the rest of the stations.
	DPoint2 dpTemp;
	PolePlacement place;
	long lStationNumber=0;
	long lRoutePoint=0;	
	double fTemp;
	int iTemp=0;
	char sTemp[80];
	ProfileRecord NewPoint;
	vtStation NewStation;
	char sline[200], *s,*end;
	int i,icomma =0;

	if(!fp3D)
		return false;

	while (!feof(fp3D))
	{
		s=sline;
		icomma=i=0;

		fgets(s, 200, fp3D);
		fTemp=0;iTemp=-1;
		// check for empty line
		if (sline[icomma]=='\0')break;
		// read the record.
		while((sTemp[i++] = sline[icomma++])!=',') ;
		sTemp[i]='\0'; 
		end = &sTemp[i];
		i=0;
		fTemp = strtod(sTemp, &end);
			NewPoint.dStationNumber = fTemp*12/39;

		// Elevation Data
		while((sTemp[i++] = sline[icomma++])!=',') ;
		sTemp[--i]='\0'; end = &sTemp[--i];i=0;
		fTemp = strtod(sTemp, &end);
			NewPoint.dElevationLeft = fTemp;

		while((sTemp[i++] = sline[icomma++])!=',') ;
		sTemp[--i]='\0'; end = &sTemp[--i];i=0;
		fTemp = strtod(sTemp, &end);
			NewPoint.dElevationCenter = fTemp;

		while((sTemp[i++] = sline[icomma++])!=',') ;
		sTemp[--i]='\0'; end = &sTemp[--i];i=0;
		fTemp = strtod(sTemp, &end);
			NewPoint.dElevationRight = fTemp;

		// Offset Data
		while((sTemp[i++] = sline[icomma++])!=',') ;
		sTemp[--i]='\0'; end = &sTemp[--i];i=0;
		fTemp = strtod(sTemp, &end);
			NewPoint.OffsetLeft = fTemp;

		while((sTemp[i++] = sline[icomma++])!=',') ;
		sTemp[--i]='\0'; end = &sTemp[--i];i=0;
		fTemp = strtod(sTemp, &end);
			NewPoint.OffsetRight = fTemp;

		// Line Angle Data
		while((sTemp[i++] = sline[icomma++])!=',') ;
		sTemp[--i]='\0'; end = &sTemp[--i];i=0;
		fTemp = strtod(sTemp, &end);
			NewPoint.LineAngleDeg = fTemp;

		while((sTemp[i++] = sline[icomma++])!=',') ;
		sTemp[--i]='\0'; end = &sTemp[--i];i=0;
		fTemp = strtod(sTemp, &end);
			NewPoint.LineAngleMin = fTemp;

		while((sTemp[i++] = sline[icomma++])!=',') ;
		sTemp[--i]='\0'; end = &sTemp[--i];i=0;
		fTemp = strtod(sTemp, &end);
			NewPoint.LineAngleSec = fTemp;

		while((sTemp[i++] = sline[icomma++])!=',' && sline[icomma]!= '\0') ;
		sTemp[--i]='\0'; end = &sTemp[--i];i=0;
		if (strlen(sTemp)>1)
			strcpy(NewPoint.strStructureFamily, sTemp);
		else
			memset(NewPoint.strStructureFamily,0,80);

		m_StationRefArray.Append(NewPoint);
		lStationNumber++;
		sline[0] = '\0';
	}

	// First station is built using the header
	NewPoint = m_StationRefArray.GetAt(0);
	NewStation.m_pTower = 0;

	NewStation.m_dpStationPoint.x = m_phTheHeader.dp2Station0UTM.x;
	NewStation.m_dpStationPoint.x += NewPoint.dStationNumber*sin(m_phTheHeader.dStation0Azimuth);

	NewStation.m_dpStationPoint.y = m_phTheHeader.dp2Station0UTM.y;
	NewStation.m_dpStationPoint.y += NewPoint.dStationNumber*cos(m_phTheHeader.dStation0Azimuth);
//	NewStation.m_dpStationPoint.x += NewPoint.dStationNumber*sin(m_phTheHeader.dStation0Azimuth);
	
	NewStation.m_dpStationPoint.z = NewPoint.dElevationCenter;

	//Elevation at offset
	NewStation.m_dpOffsetElevations.x = NewPoint.dElevationLeft;
	NewStation.m_dpOffsetElevations.y = NewPoint.dElevationRight;

	//Offset Distance
	NewStation.m_dpOffsetDistances.x = NewPoint.OffsetLeft;
	NewStation.m_dpOffsetDistances.y = NewPoint.OffsetRight;

	// Line Angle
	if (NewPoint.LineAngleDeg==0 && NewPoint.LineAngleMin==0 && NewPoint.LineAngleSec==0)
		NewStation.dRadLineAngle = 0;
	else
		NewStation.dRadLineAngle = PI/180*(NewPoint.LineAngleDeg + 1/60*(NewPoint.LineAngleMin + 1/60*NewPoint.LineAngleSec));
	NewStation.dRadAzimuth = m_phTheHeader.dStation0Azimuth + NewStation.dRadLineAngle;
	NewStation.lRoutePointIndex=-1;

	if (NewPoint.strStructureFamily[0]!='\0')
	{
		dpTemp.Set(NewStation.m_dpStationPoint.x
			, NewStation.m_dpStationPoint.y);
		place.m_Point = dpTemp;
		place.m_StationIndex = m_StationArray.GetSize();
		m_aRoutePlaces.Append(place);
		NewStation.lRoutePointIndex=lRoutePoint++;
		strcpy(NewStation.sStructure, NewPoint.strStructureFamily);
//		NewStation.sStructure  = structfamily;
	}

	m_StationArray.Append(NewStation);


	// Use the previous station to build each element of the Station Array
	for (long ll=1; ll<m_StationRefArray.GetSize(); ll++)
	{
		// Get the previous station point for reference coords
		vtStation LastStation = m_StationArray.GetAt(ll-1);
		NewPoint = m_StationRefArray.GetAt(ll);

		NewStation.m_pTower=0;
		// absolute coordinates
		NewStation.m_dpStationPoint.x = LastStation.m_dpStationPoint.x;
		NewStation.m_dpStationPoint.x += NewPoint.dStationNumber*sin(LastStation.dRadAzimuth);

		NewStation.m_dpStationPoint.y = LastStation.m_dpStationPoint.y;
		NewStation.m_dpStationPoint.y += NewPoint.dStationNumber*cos(LastStation.dRadAzimuth);
//		NewStation.m_dpStationPoint.x += NewPoint.dStationNumber*sin(LastStation.dRadAzimuth);
		
		NewStation.m_dpStationPoint.z = NewPoint.dElevationCenter;

		//Elevation at offset
		NewStation.m_dpOffsetElevations.x = NewPoint.dElevationLeft;
		NewStation.m_dpOffsetElevations.y = NewPoint.dElevationRight;

		//Offset Distance
		NewStation.m_dpOffsetDistances.x = NewPoint.OffsetLeft;
		NewStation.m_dpOffsetDistances.y = NewPoint.OffsetRight;

		// Line Angle
		if (NewPoint.LineAngleDeg==0 && NewPoint.LineAngleMin==0 && NewPoint.LineAngleSec==0)
			NewStation.dRadLineAngle = 0;
		else
			NewStation.dRadLineAngle = PI/180*(NewPoint.LineAngleDeg + 1/60*(NewPoint.LineAngleMin + 1/60*NewPoint.LineAngleSec));
		NewStation.dRadAzimuth = LastStation.dRadAzimuth + NewStation.dRadLineAngle;
		NewStation.lRoutePointIndex=-1;

		if (NewPoint.strStructureFamily[0]!='\0')
		{
			dpTemp.Set(NewStation.m_dpStationPoint.x
				, NewStation.m_dpStationPoint.y);
			place.m_Point = dpTemp;
			place.m_StationIndex = m_StationArray.GetSize();
			m_aRoutePlaces.Append(place);
			NewStation.lRoutePointIndex=lRoutePoint++;
			strcpy(NewStation.sStructure, NewPoint.strStructureFamily);
//			NewStation.sStructure  = structfamily;
		}
		m_StationArray.Append(NewStation);

	}
	return true;
}

double vtRoute::isangle(DLine3 L)
{
	// Given three points in a line, are they colinear or do they 
	//	contain an angle? Preserve orientation.
	DPoint3 P0, P1, P2;
	DPoint3 diff1, diff2;
	double a1,a2;
	if (L.GetSize()<3)
		return 0.0F;

	P0=L.GetAt(0);
	P1=L.GetAt(1);
	diff1 = P1-P0;
	P2=L.GetAt(2);
	diff2 = P2-P1;

	if (diff2.x== diff1.x && diff2.y== diff1.y)
		return 0.0F;
	
	a1=atan2(diff1.y, diff1.x); // this calculates the quadrant correctly
	a2=atan2(diff2.y, diff2.y); // unlike vb's atn.
	return a2-a1;
	
}

double vtRoute::isangle(DPoint3 P0, DPoint3 P1, DPoint3 P2)
{
	DLine3 L;
	L.Append(P0);
	L.Append(P1);
	L.Append(P2);
	return isangle(L);
}

bool vtRoute::StructureReader(vtString sPath)
{	// Read through the station array, find stations with structures,
	//	Load structures.

	//sPath identifies the path to the Route data

//	vtString sStructureDataPath = sPath + "Towers/";
	vtString sStructureDataPath = "RouteData/Towers/";
	vtString sStructure = sStructureDataPath;
	vtString sWires = sPath + sStructureDataPath;

	//Read the Station array
	for (long ll=0; ll<m_StationArray.GetSize();ll++)
	{
		if (m_StationArray.GetAt(ll).lRoutePointIndex>=0)
		{
			vtStation& WithStructure = m_StationArray.GetAt(ll);
//			if (strcmp(WithStructure.sStructure, "1U11EC3F-1000-120-LD9") == 0)
			if (strncmp(WithStructure.sStructure, "1U11EC3F", 8) == 0)
			{
				sStructure = sStructureDataPath + "LatticeTower.obj";
				sWires = sStructureDataPath + "LatticeTower.wire";
			}
			else // default tower type
			{
				sStructure = sStructureDataPath + "test.obj";
				sWires = sStructureDataPath + "test.wire";
//				return false;	// No such structure
			}
			WithStructure.m_pTower = m_pTheTerrain->LoadModel(sStructure);
			WireReader("Data/" + sWires, ll);
		}
	}
	return true;

}


bool vtRoute::WireReader(vtString s, long lStation)
{
	// Read wire locations from a file into the station record
	int icount = 0;
	vtStation& st = m_StationArray.GetAt(lStation);
	FILE* f = fopen(s, "r");
	float height=0;
	if (!f)
		return false;
	if (!feof(f))
		fscanf(f, "%f", &(height));
	st.m_fTowerHeight = height;
	while (!feof(f))
	{	// read wire positions relative to the tower
		float a1,a2,a3;
		FPoint3 fpTemp;
		fscanf(f, "%f", &a1);
		fscanf(f, "%f", &a2);
		fscanf(f, "%f", &a3);
		fpTemp.Set(a1, a2, a3);
		st.m_fpWireAtt[icount++] = fpTemp;
	}
	st.m_iNUmWires=icount;

	fclose(f);
	return true;
}

void vtRoute::DrawCat(FPoint3 pt0, FPoint3 pt1, double Az
					  , double catenary, double dNumSegs, vtMesh *pWireMesh)
{
	// Draw catenary curve between condutor attachment points
	//	from the current tower to the previous tower.

	FPoint3 diff = pt1-pt0;
	long ll=0;
	double pt0xz = sqrt(pt0.z*pt0.z+pt0.x*pt0.x); //project pt0
	double xz=sqrt(diff.z*diff.z+diff.x*diff.x);	//	distance in the xz plane
	double y=diff.y;

	double dLengths=xz/dNumSegs;

    //Calculate the parabolic constants.
    double parabolicConst =(((xz) / 2) - ((y) * (catenary / (xz))));
    
	FPoint3 ptCur(0,0,0);
	double dist=0;
	
	//Iterate along the xz-plane
//	while (fabs(ptCur.x*ptCur.x + ptCur.z*ptCur.z - xz*xz) > 0.0001)	// check on x or z works
	while (fabs(dist - xz*xz) > 0.000001)	// check on x or z works
	{
		ptCur.x+=dLengths*sin(Az);
		ptCur.z+=dLengths*-cos(Az);
		dist = sqrt(ptCur.x*ptCur.x + ptCur.z*ptCur.z);
		if(dist > xz) 
		{
			break;
		}
		ptCur.y = (((dist) / (2*catenary)) * ((dist) - (2*parabolicConst)));
		
		FPoint3 ptNew(pt0+ptCur);
		pWireMesh->AddVertex(ptNew);
		ll++;
	}

	ll=ll;
}
