//
// ImportOSM.cpp: The main Builder class of the VTBuilder
//
// Copyright (c) 2006-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <map>

#include "vtui/Helper.h"

#include "Builder.h"

// Layers
//#include "RawLayer.h"
#include "RoadLayer.h"
#include "StructLayer.h"
#include "WaterLayer.h"

////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of an OpenStreetMap file.
//

struct OSMNode {
	DPoint2 p;
	bool signal_lights;
};

class VisitorOSM : public XMLVisitor
{
public:
	VisitorOSM(RoadMapEdit *rm) : m_state(0), m_pMap(rm) {}
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement(const char *name);
	void data(const char *s, int length);
	void SetSignalLights();

private:
	//string m_data;
	int m_state;
	int m_rec;

	typedef std::map<int, OSMNode> NodeMap;
	NodeMap m_nodes;
	std::vector<int> m_refs;

	RoadMapEdit *m_pMap;
	LayerType	m_WayType;

	int			m_iRoadLanes;
	SurfaceType m_eSurfaceType;
};

void VisitorOSM::SetSignalLights()
{
	// For all the nodes which have signal lights, set the state
	for (NodeMap::iterator it = m_nodes.begin(); it != m_nodes.end(); it++)
	{
		OSMNode &node = it->second;
		if (node.signal_lights)
		{
			TNode *tnode = m_pMap->FindNodeByID(it->first);
			if (tnode)
			{
				for (int j = 0; j < tnode->NumLinks(); j++)
					tnode->SetIntersectType(j, IT_LIGHT);
			}
		}
	}
	m_pMap->GuessIntersectionTypes();
}

void VisitorOSM::startElement(const char *name, const XMLAttributes &atts)
{
	const char *val;

	if (m_state == 0)
	{
		if (!strcmp(name, "node"))
		{
			DPoint2 p;
			int id;

			val = atts.getValue("id");
			if (val)
				id = atoi(val);

			val = atts.getValue("lon");
			if (val)
				p.x = atof(val);

			val = atts.getValue("lat");
			if (val)
				p.y = atof(val);

			OSMNode node;
			node.p = p;
			node.signal_lights = false;
			m_nodes[id] = node;

			m_state = 1;
		}
		else if (!strcmp(name, "way"))
		{
			m_refs.clear();
			m_state = 2;
			m_WayType = LT_UNKNOWN;

			// Defaults
			m_iRoadLanes = 2;
			m_eSurfaceType = SURFT_PAVED;
		}
	}
	else if (m_state == 1 && !strcmp(name, "tag"))
	{
		vtString key, value;

		val = atts.getValue("k");
		if (val)
			key = val;

		val = atts.getValue("v");
		if (val)
			value = val;

		// Node key/value
		if (key == "highway")
		{
			if (value == "traffic_signals")	//
			{
				m_nodes[m_nodes.size()-1].signal_lights = true;
			}
		}
	}
	else if (m_state == 2)
	{
		if (!strcmp(name, "nd"))
		{
			val = atts.getValue("ref");
			if (val)
			{
				int ref = atoi(val);
				m_refs.push_back(ref);
			}
		}
		else if (!strcmp(name, "tag"))
		{
			vtString key, value;

			val = atts.getValue("k");
			if (val)
				key = val;

			val = atts.getValue("v");
			if (val)
				value = val;

			// There are hundreds of possible Way tags
			if (key == "natural")	// value is coastline, marsh, etc.
				m_WayType = LT_UNKNOWN;

			if (key == "route" && value == "ferry")
				m_WayType = LT_UNKNOWN;

			if (key == "highway")
			{
				m_WayType = LT_ROAD;
				if (value == "motorway")	// like a freeway
					m_iRoadLanes = 4;
				if (value == "motorway_link")	// on/offramp
					m_iRoadLanes = 1;
				if (value == "unclassified")	// lowest form of the interconnecting grid network.
					m_iRoadLanes = 1;
				if (value == "unsurfaced")
					m_eSurfaceType = SURFT_DIRT;
				if (value == "track")
				{
					m_iRoadLanes = 1;
					m_eSurfaceType = SURFT_2TRACK;
				}
				if (value == "bridleway")
					m_eSurfaceType = SURFT_GRAVEL;
				if (value == "footway")
				{
					m_iRoadLanes = 1;
					m_eSurfaceType = SURFT_GRAVEL;
				}
				if (value == "primary") // An actual highway, or arterial
					m_iRoadLanes = 2;	// Doesn't tell us much useful
			}
			if (key == "waterway")
				m_WayType = LT_WATER;
			if (key == "railway")
				m_eSurfaceType = SURFT_RAILROAD;
			if (key == "aeroway")
				m_WayType = LT_UNKNOWN;		// Airport features, like runways
			if (key == "aerialway")
				m_WayType = LT_UNKNOWN;
			if (key == "power")
				m_WayType = LT_UNKNOWN;
			if (key == "man_made")
				m_WayType = LT_UNKNOWN;		// Piers, towers, windmills, etc.
			if (key == "leisure")
				m_WayType = LT_UNKNOWN;		// gardens, golf courses, public lawns, etc.
			if (key == "amenity")
				m_WayType = LT_UNKNOWN;		// mostly, types of building classified by use
			if (key == "abutters")
			{
				// describes the predominant usage of land along a road or other way
			}
			if (key == "surface")
			{
				if (value == "asphalt")
					m_eSurfaceType = SURFT_PAVED;
				if (value == "compacted")
					m_eSurfaceType = SURFT_GRAVEL;
				if (value == "concrete")
					m_eSurfaceType = SURFT_PAVED;
				if (value == "dirt")
					m_eSurfaceType = SURFT_DIRT;
				if (value == "earth")
					m_eSurfaceType = SURFT_DIRT;
				if (value == "fine_gravel")
					m_eSurfaceType = SURFT_GRAVEL;
				if (value == "ground")
					m_eSurfaceType = SURFT_2TRACK;	// or SURFT_TRAIL
				if (value == "gravel")
					m_eSurfaceType = SURFT_GRAVEL;
				if (value == "paved")
					m_eSurfaceType = SURFT_PAVED;
				if (value == "sand")
					m_eSurfaceType = SURFT_DIRT;
				if (value == "unpaved")
					m_eSurfaceType = SURFT_GRAVEL;	// or SURFT_DIRT
			}
			if (key == "lanes")
				m_iRoadLanes = atoi(value);
		}
	}
}

void VisitorOSM::endElement(const char *name)
{
	if (m_state == 1 && !strcmp(name, "node"))
	{
		m_state = 0;
	}
	else if (m_state == 2 && !strcmp(name, "way"))
	{
		// Look at the referenced nodes, turn them into a vt link
		uint refs = (uint)m_refs.size();

		// must have at least 2 refs
		if (refs >= 2 && m_WayType == LT_ROAD)
		{
			LinkEdit *link = m_pMap->NewLink();

			link->m_iLanes = m_iRoadLanes;
			link->m_Surface = m_eSurfaceType;

			int ref_first = m_refs[0];
			int ref_last = m_refs[refs-1];

			TNode *node0 = m_pMap->FindNodeByID(ref_first);
			if (!node0)
			{
				// doesn't exist, create it
				node0 = m_pMap->NewNode();
				node0->SetPos(m_nodes[ref_first].p);
				node0->m_id = ref_first;
				m_pMap->AddNode(node0);
			}
			link->SetNode(0, node0);

			TNode *node1 = m_pMap->FindNodeByID(ref_last);
			if (!node1)
			{
				// doesn't exist, create it
				node1 = m_pMap->NewNode();
				node1->SetPos(m_nodes[ref_last].p);
				node1->m_id = ref_last;
				m_pMap->AddNode(node1);
			}
			link->SetNode(1, node1);

			// Copy all the points
			for (uint r = 0; r < refs; r++)
			{
				int idx = m_refs[r];
				link->Append(m_nodes[idx].p);
			}

			m_pMap->AddLink(link);

			// point node to links
			node0->AddLink(link);
			node1->AddLink(link);

			link->ComputeExtent();
		}
		m_state = 0;
	}
}

void VisitorOSM::data(const char *s, int length)
{
	//m_data.append(string(s, length));
}

/**
 * Import what we can from OpenStreetMap.
 */
void Builder::ImportDataFromOSM(const wxString &strFileName)
{
	// Avoid trouble with '.' and ',' in Europe
	//  OSM always has English punctuation
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	std::string fname_local = strFileName.ToUTF8();

	// OSM is always in Geo WGS84
	vtProjection proj;
	proj.SetWellKnownGeogCS("WGS84");

	vtRoadLayer *rlayer = new vtRoadLayer;
	rlayer->SetProjection(proj);

	VisitorOSM visitor(rlayer);
	try
	{
		readXML(fname_local, visitor, NULL);
	}
	catch (xh_exception &ex)
	{
		DisplayAndLog(ex.getFormattedMessage().c_str());
		return;
	}
	visitor.SetSignalLights();

	bool success = AddLayerWithCheck(rlayer, true);
}
