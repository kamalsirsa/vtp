//
// Location classes
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Location.h"

///////////////////////////////

//vtLocation

///////////////////////////////

vtLocationSaver::vtLocationSaver()
{
	m_pTarget = NULL;
}

vtLocationSaver::vtLocationSaver(const char* fname)
{
	Read(fname);
}

vtLocationSaver::~vtLocationSaver()
{
}

bool vtLocationSaver::Read(const char *fname)
{
	int i, j, k;
	char buf[200];
	float f;

	FILE *fp = fopen(fname, "rb");
	if (!fp) return false;

	int num;
	float ver;
	int result = fscanf(fp, "loc v%f\n", &ver);
	if (result != 1)
	{
		fclose(fp);
		return false;
	}
	fscanf(fp, "num %d\n", &num);
	for (i = 0; i < num; i++)
	{
		vtLocation *loc = new vtLocation;

		fgets(buf, 200, fp);
		buf[strlen(buf)-1] = 0;	// trim LF
		loc->m_name = buf;

		fscanf(fp, "matrix");
		for (j = 0; j < 4; j++)
		{
			for (k = 0; k < 4; k++)
			{
				fscanf(fp, " %f", &f);
				loc->m_mat.Set(j, k, f);
			}
		}
		fscanf(fp, "\n");
		m_loc.Append(loc);
	}
	m_strFilename = fname;
	return true;
}

bool vtLocationSaver::Write(const char *fname)
{
	int i, j ,k;

	if (fname == NULL)
		fname = m_strFilename;

	FILE *fp = fopen(fname, "wb");
	if (!fp) return false;

	int num = m_loc.GetSize();
	fprintf(fp, "loc v1.0\n");
	fprintf(fp, "num %d\n", num);
	for (i = 0; i < num; i++)
	{
		fprintf(fp, "%s\n", (pcchar) m_loc[i]->m_name);
		fprintf(fp, "matrix");
		for (j = 0; j < 4; j++)
		{
			for (k = 0; k < 4; k++)
			{
				float f = m_loc[i]->m_mat.Get(j, k);
				fprintf(fp, " %f", f);
			}
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
	return true;
}

void vtLocationSaver::StoreTo(int num, const char *name)
{
	vtLocation *loc;
	if (num < m_loc.GetSize())
		loc = m_loc.GetAt(num);
	else
		loc = new vtLocation();

	m_pTarget->GetTransform1(loc->m_mat);
	if (name) loc->m_name = name;

	m_loc.SetAt(num, loc);
}

void vtLocationSaver::RecallFrom(int num)
{
	m_pTarget->SetTransform1(m_loc[num]->m_mat);
}

void vtLocationSaver::Remove(int num)
{
	vtLocation *loc = m_loc.GetAt(num);
	delete loc;
	m_loc.RemoveAt(num);
}

