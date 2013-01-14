//
// UtilityMap.cpp
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include "UtilityMap.h"
#include "vtString.h"
#include "vtLog.h"
#include "shapelib/shapefil.h"
#include "xmlhelper/easyxml.hpp"


void vtLine::MakePolyline(DLine2 &polyline)
{
	const uint num = m_poles.size();
	polyline.SetSize(num);
	for (uint i = 0; i < num; i++)
		polyline[i] = m_poles[i]->m_p;
}

vtUtilityMap::vtUtilityMap()
{
}

vtUtilityMap::~vtUtilityMap()
{
	for (uint i = 0; i < m_Poles.size(); i++)
		delete m_Poles[i];
	m_Poles.clear();

	for (uint i = 0; i < m_Lines.size(); i++)
		delete m_Lines[i];
	m_Lines.clear();
}

vtPole *vtUtilityMap::ClosestPole(const DPoint2 &p)
{
	uint npoles = m_Poles.size();
	if (npoles == 0)
		return NULL;

	uint k, ret=0;
	double	dist, closest = 1E9;

	for (k = 0; k < npoles; k++)
	{
		dist = (p - m_Poles[k]->m_p).Length();
		if (dist < closest)
		{
			closest = dist;
			ret = k;
		}
	}
	return m_Poles[ret];
}

void vtUtilityMap::GetPoleExtents(DRECT &rect)
{
	if (m_Poles.empty())
		return;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);

	const int size = m_Poles.size();
	for (int i = 0; i < size; i++)
	{
		vtPole *pole = m_Poles[i];
		rect.GrowToContainPoint(pole->m_p);
	}
}

#if 0
////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of Content files.
////////////////////////////////////////////////////////////////////////

class UtilityVisitor : public XMLVisitor
{
public:
	UtilityVisitor(vtUtilityMap *umap)
	: m_pUM(umap) { m_state = 0; }

	virtual ~UtilityVisitor() {}

	void startXML();
	void endXML();
	void startElement(const char * name, const XMLAttributes &atts);
//	void endElement(const char * name);
	void data(const char * s, int length);
	void SetName(const char *s) { m_name = s; }
	const char *GetName() { return(m_name); }
	void SetReference(const char *s) { m_reference = s; }
	const char *GetReference() { return(m_reference); }
	void SetRotation(float f) { m_rotation = f; }
	float SetRotation() { return(m_rotation); }

private:
	string m_data;
	int m_state;

	vtString m_name, m_reference;
	float m_rotation;

	vtUtilityMap *m_pUM;
};

void UtilityVisitor::startXML ()
{
}

void UtilityVisitor::endXML ()
{
}


bool vtUtilityMap::ReadXML(const char *pathname, bool progress_callback(int))
{
	// The locale might be set to something European that interprets '.' as ','
	//  and vice versa, which would break our usage of sscanf/atof terribly.
	//  So, push the 'standard' locale, it is restored when it goes out of scope.
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	bool success = false;
	UtilityVisitor visitor(this);
	try
	{
		readXML(pathname, visitor, progress_callback);
		success = true;
	}
	catch (xh_exception &ex)
	{
		// TODO: would be good to pass back the error message.
		VTLOG("XML Error: ");
		VTLOG(ex.getFormattedMessage().c_str());
		return false;
	}

	return success;
}

void UtilityVisitor::startElement(const char *name, const XMLAttributes &atts)
{
	const char *attval;

	// clear data at the start of each element
	m_data = "";

	if (m_state == 0)
	{
		if (!strcmp(name, "UtilityCollection"))
		{
			m_state = 1;
		}
		else
		{
			string message = "Root element name is ";
			message += name;
			message += "; expected UtilityCollection";
			throw xh_io_exception(message, "XML Reader");
		}
		return;
	}
	if (m_state == 1)
	{
		if (!strcmp(name, "OverheadLine"))
		{
			m_pRoute = new vtRoute(GetCurrentTerrain());		// Create new route (should this be in vtUtilityMap ??)
			attval = atts.getValue("name");
			if (attval)
				m_pRoute->SetName(attval);		// TODO Set Name of the overhead line
			GetCurrentTerrain()->AddRoute(m_pRoute);
			m_state = 2;
		}
		return;
	}

	if (m_state == 2)	// Conductor
	{
		if (!strcmp(name, "Conductor"))
		{
			attval = atts.getValue("width");
			if (attval)
				m_pRoute->SetThickness(atof(attval));
			attval = atts.getValue("color");
			if (attval)
				m_pRoute->SetColor(ParseHexColor(attval));
			m_state = 3;
		}
		return;
	}

	if (m_state == 3)	// Pylon
	{
		if (!strcmp(name, "Pylon"))
		{
			attval = atts.getValue("name");
			if (attval)
				SetName(attval);
			attval = atts.getValue("reference");
			if (attval)
				SetReference(attval);
			attval = atts.getValue("rotation");
			if (attval)
				SetRotation(atof(attval));

			m_state = 4;	// now read in the coordinate ...
		}
		else
			m_state = 1;	// then it is a new overhead line
		return;
	}
}

void UtilityVisitor::endElement(const char *name)
{
	const char *data = m_data.c_str();

	if (m_state == 4)	// Coordinate of the pylon
	{
		if (!strcmp(name, "gml:coordinates"))
		{
			double x, y;
			sscanf(data, "%lf,%lf", &x, &y);
			m_pRoute->AddPoint(DPoint2(x,y), GetReference());
					// now set also the rotation !!!! HOW?? TODO
			m_state = 3;
		}
	}
}

void UtilityVisitor::data(const char *s, int length)
{
	m_data.append(string(s, length));
}
#endif
