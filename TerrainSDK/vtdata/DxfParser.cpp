//
// DxfParser.cpp
//
// Class for parsing a DXF File.
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "DxfParser.h"
#include "vtLog.h"

using std::vector;
using std::set;

/**
 * Constructor.  Pass in the filename to parse.
 */
DxfParser::DxfParser(const vtString &sFileName,
					   vector<DxfEntity> &entities,
					   vector<vtString> &layers) :
	m_entities(entities),
	m_layers(layers)
{
	m_sFileName = sFileName;
}

/**
 * This method will parse the entire file, collecting all entities
 *  that it finds.
 *
 * \param progress_callback	Pass a function to receive progress notification
 *		(values of 0 to 100) if desired.  Optional.
 *
 * \returns Success.  If failure, then call GetLastError() to get an
 *		informative error message.
 */
bool DxfParser::RetreiveEntities(void progress_callback(int))
{
	try
	{
		DxfCodeValue pair;

		if (m_sFileName.IsEmpty())
			return false;

		m_iLine = 0;
		m_pFile = fopen(m_sFileName, "r");
		if (m_pFile == NULL)
		{
			return false;
		}
		m_iCounter = 0;
		if (fseek(m_pFile, 0, SEEK_END) != 0)
		{
			fclose(m_pFile);
			return false;
		}
		m_iEndPosition = ftell(m_pFile);
		if (m_iEndPosition < 1)
		{
			fclose(m_pFile);
			return false;
		}
		rewind(m_pFile);

		// Process each section
		while (ReadCodeValue(pair))
		{
			if (pair.m_iCode == 0 && pair.m_sValue == "SECTION")
			{
				if (!ReadCodeValue(pair))
					throw "Unexpected end of file found.";

				if (pair.m_iCode != 2)
					throw "Expecting section type, but none encountered.";

				if (pair.m_sValue == "ENTITIES")
					ReadEntitySection(progress_callback);
				else
					SkipSection();

			}
			else if (pair.m_iCode == 0 && pair.m_sValue == "EOF")
			{
				break;
			}
			else
			{
				throw "Expecting section, but none encountered.";
			}
		}
	}
	catch (const char *msg)
	{
		m_strMessage.Format("DXF Parse Error: %s\nLine = %d\n", msg, m_iLine);

		fclose(m_pFile);
		return false;
	}
	catch (...)
	{
		m_strMessage.Format("DXF Parse Error: Unknown parse error.\nLine = %d\n", m_iLine);

		fclose(m_pFile);
		return false;
	}

	fclose(m_pFile);
	return true;
}

inline bool DxfParser::ReadCodeValue(DxfCodeValue & pair)
{
	// Read the code.
	char buf[1024];
	if (fgets(buf, 1024, m_pFile) == NULL)
		return false;
	vtString sVal = buf;
	sVal.TrimRight();
	sVal.TrimLeft();
	if (!sVal.IsNumber())
		throw "Non-numeric code encountered.";
	pair.m_iCode = atoi(sVal);

	// Read the value.
	if (fgets(buf, 1024, m_pFile) == NULL)
		throw "Unexpected end of line encountered.";
	sVal = buf;
	sVal.TrimRight();
	sVal.TrimLeft();
	pair.m_sValue = sVal;

	return true;
}

void DxfParser::SkipSection()
{
	DxfCodeValue pair;
	bool bFoundEnd = false;
	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0 && pair.m_sValue == "ENDSEC")
		{
			bFoundEnd = true;
			break;
		}
	}
	if (!bFoundEnd)
		throw "Unable to find end of section.";
}

void DxfParser::ReadEntitySection(void progress_callback(int))
{
	DxfCodeValue pair;
	bool bFoundEnd = false;
	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0 && pair.m_sValue == "ENDSEC")
		{
			bFoundEnd = true;
			break;
		}
		if (pair.m_iCode == 0 && pair.m_sValue == "POLYLINE")
		{
			ReadPolyline();
		}
		if (pair.m_iCode == 0 && pair.m_sValue == "POINT")
		{
			ReadPoint();
		}
		if (pair.m_iCode == 0 && pair.m_sValue == "LINE")
		{
			ReadLine();
		}
		if (pair.m_iCode == 0 && pair.m_sValue == "LWPOLYLINE")
		{
			ReadLWPolyline();
		}
		if (pair.m_iCode == 0 && pair.m_sValue == "3DFACE")
		{
			Read3DFace();
		}

		m_iCounter++;
		if (m_iCounter == 200)
		{
			long current = ftell(m_pFile);
			int iProgress =  (int) ((double)current / m_iEndPosition * 100);
			progress_callback(iProgress);
			m_iCounter = 0;
		}
	}
	if (!bFoundEnd)
		throw "Unable to find end of section.";
}

void DxfParser::ReadPoint()
{
	DxfCodeValue pair;
	DxfEntity entity;

	bool bFoundEnd = false, bFoundLayer = false, bFoundX = false;
	bool bFoundY = false, bFoundZ = false;
	DPoint3 pt;

	long oldPos = ftell(m_pFile);

	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0)
		{
			bFoundEnd = true;

			// save the entity off if it has everything.
			if (bFoundLayer && bFoundX && bFoundY && bFoundZ)
			{
				entity.m_points.push_back(pt);
				entity.m_iType = DET_Point;
				m_entities.push_back(entity);				
			}

			// back that pointer up.
			// don't worry, we won't be here if current line is less than 4
			fseek(m_pFile, oldPos, SEEK_SET);
			break;
		}
		else if (pair.m_iCode == 8)
		{
			entity.m_iLayer = GetLayerIndex(pair.m_sValue);
			bFoundLayer = true;
		}
		else if (pair.m_iCode == 10)
		{
			pt.x = strtod(pair.m_sValue, NULL);
			bFoundX = true;
		}
		else if (pair.m_iCode == 20)
		{
			pt.y = strtod(pair.m_sValue, NULL);
			bFoundY = true;
		}
		else if (pair.m_iCode == 30)
		{
			pt.z = strtod(pair.m_sValue, NULL);
			bFoundZ = true;
		}
		oldPos = ftell(m_pFile);
	}
	if (!bFoundEnd)
		throw "Unable to find end of point entity.";
}

void DxfParser::ReadLine()
{
	DxfCodeValue pair;
	DxfEntity entity;
	entity.m_iType = DET_Polyline;
	DPoint3 startPt, endPt;
	bool bFoundEnd = false;
	bool bFoundLayer = false;
	bool bFoundStartX = false;
	bool bFoundStartY = false;
	bool bFoundStartZ = false;
	bool bFoundEndX = false;
	bool bFoundEndY = false;
	bool bFoundEndZ = false;

	long oldPos = ftell(m_pFile);

	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0)
		{
			bFoundEnd = true;

			// back that pointer up.
			// don't worry, we won't be here if current line is less than 4
			fseek(m_pFile, oldPos, SEEK_SET);
			entity.m_points.push_back(endPt);
			entity.m_points.push_back(startPt);

			// save the entity off if it has everything.
			if (bFoundLayer && bFoundStartX && bFoundStartY && bFoundStartZ &&
				bFoundEndX && bFoundEndY && bFoundEndZ && entity.m_points.size() > 0)
			{
				m_entities.push_back(entity);
			}
			break;
		}
		else if (pair.m_iCode == 8)
		{
			entity.m_iLayer = GetLayerIndex(pair.m_sValue);
			bFoundLayer = true;
		}
		else if (pair.m_iCode == 10)
		{
			startPt.x = strtod(pair.m_sValue, NULL);
			bFoundStartX = true;
		}
		else if (pair.m_iCode == 20)
		{
			startPt.y = strtod(pair.m_sValue, NULL);
			bFoundStartY = true;
		}
		else if (pair.m_iCode == 30)
		{
			startPt.z = strtod(pair.m_sValue, NULL);
			bFoundStartZ = true;
		}
		else if (pair.m_iCode == 11)
		{
			endPt.x = strtod(pair.m_sValue, NULL);
			bFoundEndX = true;
		}
		else if (pair.m_iCode == 21)
		{
			endPt.y = strtod(pair.m_sValue, NULL);
			bFoundEndY = true;
		}
		else if (pair.m_iCode == 31)
		{
			endPt.z = strtod(pair.m_sValue, NULL);
			bFoundEndZ = true;
		}
		oldPos = ftell(m_pFile);
	}
	if (!bFoundEnd)
		throw "Unable to find end of polyline entity.";
}

void DxfParser::ReadPolyline()
{
	DxfCodeValue pair;
	DxfEntity entity;
	bool bFoundEnd = false;
	bool bFoundLayer = false;
	bool bFoundType = false;

	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0 && pair.m_sValue == "SEQEND")
		{
			bFoundEnd = true;

			// save the entity off if it has everything.
			if (bFoundLayer && bFoundType && entity.m_points.size() > 0)
			{
				m_entities.push_back(entity);
			}
			break;
		}
		else if (pair.m_iCode == 8)
		{
			entity.m_iLayer = GetLayerIndex(pair.m_sValue);
			bFoundLayer = true;
		}
		else if (pair.m_iCode == 70)
		{
			if (atoi(pair.m_sValue) & 1)
				entity.m_iType = DET_Polygon;
			else
				entity.m_iType = DET_Polyline;
			bFoundType = true;
		}
		else if (pair.m_iCode == 0 && pair.m_sValue == "VERTEX")
		{
			ReadVertex(entity.m_points);
		}
	}
	if (!bFoundEnd)
		throw "Unable to find end of polyline entity.";
}

void DxfParser::ReadVertex(vector<DPoint3> & points)
{
	DxfCodeValue pair;

	bool bFoundEnd = false, bFoundLayer = false, bFoundX = false;
	bool bFoundY = false, bFoundZ = false;
	DPoint3 pt;

	long oldPos = ftell(m_pFile);
	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0)
		{
			bFoundEnd = true;

			// save the entity off if it has everything.
			if (bFoundX && bFoundY && bFoundZ)
			{
				points.push_back(pt);
			}

			// back that pointer up.
			// don't worry, we won't be here if current line is less than 4
			fseek(m_pFile, oldPos, SEEK_SET);
			break;
		}
		else if (pair.m_iCode == 10)
		{
			pt.x = strtod(pair.m_sValue, NULL);
			bFoundX = true;
		}
		else if (pair.m_iCode == 20)
		{
			pt.y = strtod(pair.m_sValue, NULL);
			bFoundY = true;
		}
		else if (pair.m_iCode == 30)
		{
			pt.z = strtod(pair.m_sValue, NULL);
			bFoundZ = true;
		}
		oldPos = ftell(m_pFile);
	}
	if (!bFoundEnd)
		throw "Unable to find end of vertex entity.";
}

int DxfParser::GetLayerIndex(const vtString & sLayer)
{
	unsigned int iLayers = m_layers.size();
	for (unsigned int i = 0; i < iLayers; ++i)
	{
		if (sLayer == m_layers[i])
			return i;
	}
	m_layers.push_back(sLayer);
	return iLayers;
}


void DxfParser::ReadLWPolyline()
{
	long oldPos = ftell(m_pFile);

	// we assume that the x,y values come in that order (i.e. 10 20 10 20 etc.)
	DxfCodeValue pair;
	DxfEntity entity;
	double dElevation = 0.0;
	bool bFoundElevation = false;
	bool bFoundEnd = false;
	bool bFoundLayer = false;
	bool bFoundType = false;
	int iCurrIndex = 0;

	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0)
		{
			bFoundEnd = true;

			// save the entity off if it has everything.
			if (bFoundElevation && bFoundLayer && bFoundType && entity.m_points.size() > 0)
			{
				for (int i = 0; i < iCurrIndex; ++i)
					entity.m_points[i].z = dElevation;
				m_entities.push_back(entity);
			}

			// back that pointer up.
			// don't worry, we won't be here if current line is less than 4
			fseek(m_pFile, oldPos, SEEK_SET);
			break;
		}
		else if (pair.m_iCode == 8)
		{
			entity.m_iLayer = GetLayerIndex(pair.m_sValue);
			bFoundLayer = true;
		}
		else if (pair.m_iCode == 10)
		{
			DPoint3 pt;
			pt.x = strtod(pair.m_sValue, NULL);
			entity.m_points.push_back(pt);
			++iCurrIndex;
		}
		else if (pair.m_iCode == 20)
		{
			entity.m_points[iCurrIndex].y = strtod(pair.m_sValue, NULL);
		}
		else if (pair.m_iCode == 38)
		{
			dElevation = strtod(pair.m_sValue, NULL);
			bFoundElevation = true;
		}
		else if (pair.m_iCode == 70)
		{
			if (atoi(pair.m_sValue) & 1)
				entity.m_iType = DET_Polygon;
			else
				entity.m_iType = DET_Polyline;
			bFoundType = true;
		}
		oldPos = ftell(m_pFile);
	}
	if (!bFoundEnd)
		throw "Unable to find end of lwpolyline entity.";
}

void DxfParser::Read3DFace()
{
	DxfCodeValue pair;
	DxfEntity entity;
	bool bFoundEnd = false;
	bool bFoundLayer = false;

	entity.m_iType = DET_3DFace;

	// There are always four points in a 3DFACE...
	DPoint3 pt1, pt2, pt3, pt4;

	long oldPos = ftell(m_pFile);
	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0)
		{
			bFoundEnd = true;

			// back that pointer up.
			// don't worry, we won't be here if current line is less than 4
			fseek(m_pFile, oldPos, SEEK_SET);

			// save the entity off if it has everything.
			break;
		}
		else if (pair.m_iCode == 8)
		{
			entity.m_iLayer = GetLayerIndex(pair.m_sValue);
			bFoundLayer = true;
		}

		else if (pair.m_iCode == 10)
			pt1.x = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 20)
			pt1.y = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 30)
			pt1.z = strtod(pair.m_sValue, NULL);

		else if (pair.m_iCode == 11)
			pt2.x = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 21)
			pt2.y = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 31)
			pt2.z = strtod(pair.m_sValue, NULL);

		else if (pair.m_iCode == 12)
			pt3.x = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 22)
			pt3.y = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 32)
			pt3.z = strtod(pair.m_sValue, NULL);

		else if (pair.m_iCode == 13)
			pt4.x = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 23)
			pt4.y = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 33)
			pt4.z = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 70)
		{
			// Invisible edge flags.  Ignore these?
			//  Bit 1,2,3,4 means 1st,2nd,3rd,4th edge is invisible
			int mask = atoi(pair.m_sValue);
		}
		oldPos = ftell(m_pFile);
	}

	if (!bFoundEnd)
		throw "Unable to find end of polyline entity.";

	// ...although the forth point may be the same as the third,
	//  in which case ignore it and treat this as a triangle.
	entity.m_points.push_back(pt1);
	entity.m_points.push_back(pt2);
	entity.m_points.push_back(pt3);
	if (pt4 != pt3)
		entity.m_points.push_back(pt4);

	if (bFoundLayer)
		m_entities.push_back(entity);
}
