//
// The vtTowerArray class is an array of Tower objects.
//
// It supports operations including loading and saving to a file
// and picking of Tower elements.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include <string.h>
#include "vtdata/shapelib/shapefil.h"
#include "TowerArray.h"

void vtTowerArray::DestructItems(int first, int last)
{
	for (int i = first; i <= last; i++)
		delete GetAt(i);
}

bool vtTowerArray::ReadUTL(const char* pathname)
{
	FILE* fp;
	if ( (fp = fopen(pathname, "rb")) == NULL )
		return false;

	char buf[4];
	fread(buf, 3, 1, fp);
	float version;
	fscanf(fp, "%f\n", &version);

	if (version < UTLVERSION_SUPPORTED)
	{
		// too old, unsupported version
		return false;
	}

	int zone = 11;
	if (version == 1.1f)
	{
		fscanf(fp, "utm_zone %d\n", &zone);
	}
	m_proj.SetUTM(true, zone);

	int i, j, count;
	DPoint2 p;
	int points;
	char key[80];
	RGBi color;

	fscanf(fp, "towers %d\n", &count);
	for (i = 0; i < count; i++)	//for each building
	{
		vtTower *tower = NewTower();

		TowerShape type;
		fscanf(fp, "type %d\n", &type);
		tower->SetShape((TowerShape) type);

		//int arms = 1;
		while (1)
		{
			long start = ftell(fp);

			int result = fscanf(fp, "%s ", key);
			if (result == -1)
				break;

			if (!strcmp(key, "type"))
			{
				fseek(fp, start, SEEK_SET);
				break;
			}
			if (!strcmp(key, "loc"))
			{
				DPoint2 loc;
				fscanf(fp, "%lf %lf\n", &loc.x, &loc.y);
				tower->SetLocation(loc);
			}
			else if (!strcmp(key, "rot"))
			{
				float rot;
				fscanf(fp, "%f\n", &rot);
				tower->SetRotation(rot);
			}
			else if (!strcmp(key, "color"))
			{
				fscanf(fp, "%hd %hd %hd\n", &color.r, &color.g, &color.b);
				tower->SetColor( color);
			}
			else if (!strcmp(key, "size"))
			{
				float w, d;
				fscanf(fp, "%f %f\n", &w, &d);
				tower->SetRectangle(w, d);
			}
			else if (!strcmp(key, "radius"))
			{
				float rad;
				fscanf(fp, "%f\n", &rad);
				tower->SetRadius(rad);
			}
			else if (!strcmp(key, "footprint"))
			{
				DLine2 dl;
				fscanf(fp, "%d", &points);
				dl.SetSize(points);

				for (j = 0; j < points; j++)
				{
					fscanf(fp, " %lf %lf", &p.x, &p.y);
					dl.SetAt(j, p);
				}
				fscanf(fp, "\n");
				tower->SetFootprint(dl);
			}
			else if (!strcmp(key, "elev"))
			{
				int elev;
				fscanf(fp, "%d\n", &elev);
				tower->m_bElevated = (elev != 0);
			}
			else if (!strcmp(key, "tower_type"))
			{
				int tt;
				fscanf(fp, "%d\n", &tt);
				tower->m_TowerType = (TowerType) tt;
			}
			else if(!strcmp(key,"tower_material"))
			{
				int tm;
				fscanf(fp,"%d\n",&tm);
				tower->m_TowerMaterial=(TowerMaterial)tm;
			}
			else if (!strcmp(key,"tower_index"))
			{
				int idx;
				fscanf(fp,"5d\n",&idx);
				tower->m_iTowerTypeIndex=idx;
			}
		}
		AddTower(tower);
	}
	fclose(fp);
	return true;
}


bool vtTowerArray::ReadSHP(const char* pathname)
{
	SHPHandle hSHP = SHPOpen(pathname, "rb");
	if (hSHP == NULL)
	{
		// try to read the old format
		return ReadUTL(pathname);
	}

    int		nEntities, nShapeType;
    double 	adfMinBound[4], adfMaxBound[4];
	DPoint2 point;

	SHPGetInfo(hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound);
	if (nShapeType != SHPT_POINT)
		return false;

	for (int i = 0; i < nEntities; i++)
	{
		SHPObject *psShape = SHPReadObject(hSHP, i);
		point.x = psShape->padfX[0];
		point.y = psShape->padfY[0];
		vtTower *tower = NewTower();
		tower->SetLocation(point);
		Append(tower);
		SHPDestroyObject(psShape);
	}
	SHPClose(hSHP);
	return true;
}

bool vtTowerArray::WriteUTL(const char* pathname)
{
	FILE *fp = fopen(pathname, "wb");
	if (!fp) return false;

	int count = GetSize();
	TowerShape type;
	DPoint2 loc;
	vtTower *tower;
	RGBi color;
	int i, j, points;

	fprintf(fp, "utl%1.1f\n", UTLVERSION_CURRENT);
	fprintf(fp, "utm_zone %d\n", m_proj.GetUTMZone());
	fprintf(fp, "towers%d\n", count);
	for (i = 0; i < count; i++)	//for each coordinate
	{
		tower = GetAt(i);
		type = tower->GetShape();

		fprintf(fp, "type %d\n", type);
		loc = tower->GetLocation();
		fprintf(fp, "\tloc %lf %lf\n", loc.x, loc.y);

		color = tower->GetColor();
		fprintf(fp, "\tcolor %d %d %d\n", color.r, color.g, color.b);

		if (tower->m_bElevated)
		{
			fprintf(fp, "\telev 1\n");
		}

		// don't write
		if (type == TSHAPE_RECTANGLE)
		{
			float w, d, rot;
			tower->GetRectangle(w, d);
			tower->GetRotation(rot);
			fprintf(fp, "\tsize %f %f\n", w, d);
			if (rot != -1.0f)
				fprintf(fp, "\trot %f\n", rot);
			fprintf(fp, "\ttower_type %d\n", tower->m_TowerType);
		}
		if (type == TSHAPE_CIRCLE)
		{
			float rad = tower->GetRadius();
			fprintf(fp, "\tradius %f\n", rad);
		}
		if (type == TSHAPE_POLY)
		{
			DLine2 &dl = tower->GetFootprint();
			points = dl.GetSize();
			fprintf(fp, "\tfootprint %d", points);
			for (j = 0; j < points; j++)
			{
				DPoint2 p = dl.GetAt(j);
				fprintf(fp, " %lf %lf", p.x, p.y);
			}
			fprintf(fp, "\n");
		}
	}
	fclose(fp);
	return true;
}

// int nSHPType, int nShapeId, int nParts,
//	int * panPartStart, int * panPartType,
//	int nVertices, double * padfX, double * padfY,
//	double * padfZ, double * padfM )

bool vtTowerArray::WriteSHP(const char* pathname)
{
	char *ext = strrchr(pathname, '.');
	if (!strcmp(ext, ".bcf"))
		return WriteUTL(pathname);

    SHPHandle hSHP = SHPCreate ( pathname, SHPT_POINT );
    if (!hSHP)
		return false;

	int count = GetSize();
	DPoint2 temp;
	SHPObject *obj;
	for (int i = 0; i < count; i++)	//for each coordinate
	{
		vtTower *ptr = GetAt(i);
		temp = ptr->GetLocation();
		obj = SHPCreateSimpleObject(SHPT_POINT, 1, &temp.x, &temp.y, NULL);
		SHPWriteObject(hSHP, -1, obj);
		SHPDestroyObject(obj);
	}
	SHPClose(hSHP);
	return true;
}

int vtTowerArray::IsTowerThere(const DPoint2 &point, double error)
{
	if (IsEmpty())
		return -1;

	for (int i = 0; i < GetSize(); i++)
	{
		vtTower *bld = GetAt(i);
		DPoint2 temp = bld->GetLocation();
		if ((fabs(temp.x-point.x) <= error) && (fabs(temp.y-point.y) <= error)) //yes it is
			return i;
	}
	return -1;
}

//
// find the corner of a building closest to the given point
//
// return true if found
//
bool vtTowerArray::FindClosestCorner(const DPoint2 &point, double error,
									 int &tower, int &corner, double &closest)
{
	if (IsEmpty())
		return false;

	tower = -1;
	DPoint2 loc;
	double dist;
	closest = 1E8;

	int i, j;
	for (i = 0; i < GetSize(); i++)
	{
		vtTower *bld = GetAt(i);
		switch (bld->GetShape())
		{
		case TSHAPE_RECTANGLE:
			if (bld->GetFootprint().GetSize() == 0)
				bld->RectToPoly();

		case TSHAPE_POLY:
			{
				DLine2 &dl = bld->GetFootprint();
				for (j = 0; j < dl.GetSize(); j++)
				{
					dist = (dl.GetAt(j) - point).Length();
					if (dist > error)
						continue;
					if (dist < closest)
					{
						tower = i;
						corner = j;
						closest = dist;
					}
				}
			}
			break;
		case TSHAPE_CIRCLE:
			loc = bld->GetLocation();
			dist = fabs((point - loc).Length() - bld->GetRadius());
			if (dist > error)
				continue;
			if (dist < closest)
			{
				tower = i;
				corner = 0;
				closest = dist;
			}
			break;
		}
	}
	return (tower != -1);
}

//
// find the corner of a building closest to the given point
//
// return true if found
//
bool vtTowerArray::FindClosestCenter(const DPoint2 &point, double error,
									 int &tower, double &closest)
{
	if (IsEmpty())
		return false;

	tower = -1;
	DPoint2 loc;
	double dist;
	closest = 1E8;

	for (int i = 0; i < GetSize(); i++)
	{
		vtTower *bld = GetAt(i);
		loc = bld->GetLocation();
		dist = (loc - point).Length();
		if (dist > error)
			continue;
		if (dist < closest)
		{
			tower = i;
			closest = dist;
		}
	}
	return (tower != -1);
}

void vtTowerArray::GetExtents(DRECT &rect)
{
	if (GetSize() == 0)
		return;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);

	int i, size = GetSize();
	for (i = 0; i < size; i++)
	{
		vtTower *ptr = GetAt(i);
		rect.GrowToContainPoint(ptr->GetLocation());
	}
}

int vtTowerArray::NumSelected()
{
	int sel = 0;
	for (int i = 0; i < GetSize(); i++)
	{
		if (GetAt(i)->IsSelected()) sel++;
	}
	return sel;
}

void vtTowerArray::DeselectAll()
{
	for (int i = 0; i < GetSize(); i++)
		GetAt(i)->Select(false);
}

void vtTowerArray::DeleteSelected()
{
	for (int i = 0; i < GetSize();)
	{
		vtTower *bld = GetAt(i);
		if (bld->IsSelected())
		{
			delete bld;
			RemoveAt(i);
		}
		else
			i++;
	}
}

