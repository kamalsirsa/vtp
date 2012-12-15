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
	int id;
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

	std::vector<OSMNode> m_nodes;
	int find_node(int id)
	{
		for (size_t i = 0; i < m_nodes.size(); i++)
			if (m_nodes[i].id == id)
				return (int)i;
		return -1;
	}
	std::vector<int> m_refs;

	RoadMapEdit *m_pMap;
	LinkEdit *m_pLink;
	bool	m_bAddLink;
};

void VisitorOSM::SetSignalLights()
{
	// For all the nodes which have signal lights, set the state
	for (uint i = 0; i < m_nodes.size(); i++)
	{
		OSMNode &node = m_nodes[i];
		if (node.signal_lights)
		{
			TNode *tnode = m_pMap->FindNodeByID(node.id);
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

			//TNode *node = m_pMap->NewNode();
			//node->m_p = p;
			//node->m_id = id;
			//m_pMap->AddNode(node)

			OSMNode node;
			node.p = p;
			node.id = id;
			node.signal_lights = false;
			m_nodes.push_back(node);

			m_state = 1;
		}
		else if (!strcmp(name, "way"))
		{
			m_pLink = m_pMap->NewLink();
			m_pLink->m_iLanes = 2;

			m_refs.clear();
			m_state = 2;
			m_bAddLink = true;
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
				m_bAddLink = false;

			if (key == "route" && value == "ferry")
				m_bAddLink = false;

			if (key == "highway")
			{
				if (value == "motorway")	// like a freeway
					m_pLink->m_iLanes = 4;
				if (value == "motorway_link")	// on/offramp
					m_pLink->m_iLanes = 1;
				if (value == "unclassified")	// lowest form of the interconnecting grid network.
					m_pLink->m_iLanes = 1;
				if (value == "unsurfaced")
					m_pLink->m_Surface = SURFT_DIRT;
				if (value == "track")
				{
					m_pLink->m_iLanes = 1;
					m_pLink->m_Surface = SURFT_2TRACK;
				}
				if (value == "bridleway")
					m_pLink->m_Surface = SURFT_GRAVEL;
				if (value == "footway")
				{
					m_pLink->m_iLanes = 1;
					m_pLink->m_Surface = SURFT_GRAVEL;
				}
			}
			if (key == "waterway")
				m_bAddLink = false;
			if (key == "railway")
				m_pLink->m_Surface = SURFT_RAILROAD;
			if (key == "aeroway")
				m_bAddLink = false;
			if (key == "aerialway")
				m_bAddLink = false;
			if (key == "power")
				m_bAddLink = false;
			if (key == "man_made")
				m_bAddLink = false;
			if (key == "leisure")
				m_bAddLink = false;
			if (key == "amenity")
				m_bAddLink = false;
			if (key == "abutters")
				m_bAddLink = false;
			if (key == "surface")
			{
				if (value == "paved")
					m_pLink->m_Surface = SURFT_PAVED;
				if (value == "unpaved")
					m_pLink->m_Surface = SURFT_GRAVEL;
			}
			if (key == "lanes")
				m_pLink->m_iLanes = atoi(value);
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
		if (refs >= 2 && m_bAddLink == true)
		{
			int ref_first = m_refs[0];
			int ref_last = m_refs[refs-1];

			int idx_first = find_node(ref_first);
			int idx_last = find_node(ref_last);

			TNode *node0 = m_pMap->FindNodeByID(m_nodes[idx_first].id);
			if (!node0)
			{
				// doesn't exist, create it
				node0 = m_pMap->NewNode();
				node0->SetPos(m_nodes[idx_first].p);
				node0->m_id = m_nodes[idx_first].id;
				m_pMap->AddNode(node0);
			}
			m_pLink->SetNode(0, node0);

			TNode *node1 = m_pMap->FindNodeByID(m_nodes[idx_last].id);
			if (!node1)
			{
				// doesn't exist, create it
				node1 = m_pMap->NewNode();
				node1->SetPos(m_nodes[idx_last].p);
				node1->m_id = m_nodes[idx_last].id;
				m_pMap->AddNode(node1);
			}
			m_pLink->SetNode(1, node1);

			// Copy all the points
			for (uint r = 0; r < refs; r++)
			{
				int idx = find_node(m_refs[r]);
				m_pLink->Append(m_nodes[idx].p);
			}

			m_pMap->AddLink(m_pLink);

			// point node to links
			node0->AddLink(m_pLink);
			node1->AddLink(m_pLink);

			m_pLink->ComputeExtent();

			m_pLink = NULL;
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
