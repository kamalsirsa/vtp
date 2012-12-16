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
	VisitorOSM();
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement(const char *name);
	void data(const char *s, int length) {}	// OSM doesn't use actual XML data
	void SetSignalLights();

	vtRoadLayer *m_road_layer;
	vtStructureLayer *m_struct_layer;

private:
	void MakeRoad();
	void MakeStructure();
	void MakeBuilding();
	void MakeLinear();
	void ParseOSMTag(const vtString &key, const vtString &value);

	enum ParseState {
		PS_NONE,
		PS_NODE,
		PS_WAY
	} m_state;
	int m_rec;

	typedef std::map<int, OSMNode> NodeMap;
	NodeMap m_nodes;
	std::vector<int> m_refs;

	vtProjection m_proj;

	LayerType	m_WayType;

	int			m_iRoadLanes;
	SurfaceType m_eSurfaceType;

	vtStructureType m_eStructureType;
	vtLinearStyle	m_eLinearStyle;
	int				m_iNumStories;
	float			m_fHeight;
	RoofType		m_RoofType;
};

VisitorOSM::VisitorOSM() : m_state(PS_NONE)
{
	m_road_layer = NULL;
	m_struct_layer = NULL;

	// OSM is always in Geo WGS84
	m_proj.SetWellKnownGeogCS("WGS84");
}

void VisitorOSM::SetSignalLights()
{
	// For all the nodes which have signal lights, set the state
	for (NodeMap::iterator it = m_nodes.begin(); it != m_nodes.end(); it++)
	{
		OSMNode &node = it->second;
		if (node.signal_lights)
		{
			TNode *tnode = m_road_layer->FindNodeByID(it->first);
			if (tnode)
			{
				for (int j = 0; j < tnode->NumLinks(); j++)
					tnode->SetIntersectType(j, IT_LIGHT);
			}
		}
	}
	m_road_layer->GuessIntersectionTypes();
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

			m_state = PS_NODE;
		}
		else if (!strcmp(name, "way"))
		{
			m_refs.clear();
			m_state = PS_WAY;

			// Defaults
			m_WayType = LT_UNKNOWN;
			m_iRoadLanes = 2;
			m_eSurfaceType = SURFT_PAVED;
			m_eStructureType = ST_NONE;
			m_iNumStories = -1;
			m_fHeight = -1;
			m_RoofType = NUM_ROOFTYPES;
		}
	}
	else if (m_state == PS_NODE && !strcmp(name, "tag"))
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
			if (value == "traffic_signals")
			{
				m_nodes[m_nodes.size()-1].signal_lights = true;
			}
		}
	}
	else if (m_state == PS_WAY)
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

			ParseOSMTag(key, value);
		}
	}
}

void VisitorOSM::endElement(const char *name)
{
	if (m_state == PS_NODE && !strcmp(name, "node"))
	{
		m_state = PS_NONE;
	}
	else if (m_state == PS_WAY && !strcmp(name, "way"))
	{
		// Look at the referenced nodes, turn them into a vt link
		uint refs = m_refs.size();

		// must have at least 2 refs
		if (m_refs.size() >= 2)
		{
			if (m_WayType == LT_ROAD)
				MakeRoad();
			if (m_WayType == LT_STRUCTURE)
				MakeStructure();
		}
		m_state = PS_NONE;
	}
}

void VisitorOSM::ParseOSMTag(const vtString &key, const vtString &value)
{
	// There are hundreds of possible Way tags
	if (key == "aeroway")
	{
		if (value == "terminal")
		{
			m_WayType = LT_STRUCTURE;
			m_eStructureType = ST_BUILDING;
		}
	}
	if (key == "aerialway")
		m_WayType = LT_UNKNOWN;

	if (key == "abutters")
	{
		// describes the predominant usage of land along a road or other way,
		// not directly useful for visualization
	}
	if (key == "amenity")
	{
		// Mostly, types of building classified by use (like a post office,
		//  library, school, hospital..)
		// Except for some non-building values.
		if (value == "parking" ||
			value == "bench" ||
			value == "fuel" ||
			value == "grave_yard" ||
			value == "post_box")
		{
			// Nothing yet.
		}
		if (value == "school" ||
			value == "place_of_worship" ||
			value == "restaurant" ||
			value == "bank" ||
			value == "fast_food" ||
			value == "cafe" ||
			value == "kindergarten" ||
			value == "public_building" ||
			value == "hospital" ||
			value == "post_office")
		{
			m_WayType = LT_STRUCTURE;
			m_eStructureType = ST_BUILDING;
		}
	}
	if (key == "barrier")
	{
		// Some kinds of barrier are structures.
		if (value == "city_wall" || value == "wall")
		{
			m_WayType = LT_STRUCTURE;
			m_eStructureType = ST_LINEAR;
			m_eLinearStyle = FS_STONE;
		}
		if (value == "fence")
		{
			m_WayType = LT_STRUCTURE;
			m_eStructureType = ST_LINEAR;
			m_eLinearStyle = FS_METAL_POSTS_WIRE;
		}
		if (value == "hedge")
		{
			m_WayType = LT_STRUCTURE;
			m_eStructureType = ST_LINEAR;
			m_eLinearStyle = FS_PRIVET;
		}
	}
	if (key == "building")
	{
		// Values may be "yes" (94%), "house", "residential", "hut", "garage"..
		m_WayType = LT_STRUCTURE;
		m_eStructureType = ST_BUILDING;
	}
	if (key == "building:levels")
	{
		m_iNumStories = atoi(value);
	}
	if (key == "height")
	{
		m_fHeight = atof((const char *)value);
	}
	if (key == "highway")
	{
		m_WayType = LT_ROAD;
		// Common types include: residential, service, unclassified, primary,
		//  secondary; none of which say anything about how the road looks.
		// Look for values that do.
		if (value == "bridleway")
			m_eSurfaceType = SURFT_GRAVEL;
		if (value == "footway")
		{
			m_iRoadLanes = 1;
			m_eSurfaceType = SURFT_GRAVEL;
		}
		if (value == "motorway")	// like a freeway
			m_iRoadLanes = 4;
		if (value == "motorway_link")	// on/offramp
			m_iRoadLanes = 1;
		if (value == "path")
		{
			m_iRoadLanes = 1;
			m_eSurfaceType = SURFT_TRAIL;
		}
		if (value == "track")
		{
			// Roads "for agricultural use": farm roads, forest tracks, etc.
			m_iRoadLanes = 1;
			m_eSurfaceType = SURFT_2TRACK;
		}
		if (value == "unclassified")	// lowest form of the interconnecting grid network.
			m_iRoadLanes = 1;
		if (value == "unsurfaced")
			m_eSurfaceType = SURFT_DIRT;
	}
	if (key == "lanes")
		m_iRoadLanes = atoi(value);

	if (key == "leisure")
		m_WayType = LT_UNKNOWN;		// gardens, golf courses, public lawns, etc.

	if (key == "man_made")
		m_WayType = LT_UNKNOWN;		// Piers, towers, windmills, etc.

	if (key == "power")
		m_WayType = LT_UNKNOWN;

	if (key == "natural")	// value is coastline, marsh, etc.
		m_WayType = LT_UNKNOWN;

	if (key == "railway")
		m_eSurfaceType = SURFT_RAILROAD;

	if (key == "roof:shape")
	{
		// http://wiki.openstreetmap.org/wiki/Key:roof:shape#Roof
		if (value == "flat") m_RoofType = ROOF_FLAT;
		if (value == "gabled") m_RoofType = ROOF_GABLE;
		if (value == "hipped") m_RoofType = ROOF_HIP;
		// Less-common values gnored: half-hipped, pyramidal, gambrel, mansard, dome.
	}

	if (key == "route" && value == "ferry")
		m_WayType = LT_UNKNOWN;

	if (key == "shop")
	{
		// Values may be "supermarket"..
		m_WayType = LT_STRUCTURE;
		m_eStructureType = ST_BUILDING;
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
	if (key == "waterway")
		m_WayType = LT_WATER;
}

void VisitorOSM::MakeRoad()
{
	if (!m_road_layer)
	{
		m_road_layer = new vtRoadLayer;
		m_road_layer->SetProjection(m_proj);
	}

	LinkEdit *link = m_road_layer->NewLink();

	link->m_iLanes = m_iRoadLanes;
	link->m_Surface = m_eSurfaceType;

	int ref_first = m_refs[0];
	int ref_last = m_refs[m_refs.size() - 1];

	TNode *node0 = m_road_layer->FindNodeByID(ref_first);
	if (!node0)
	{
		// doesn't exist, create it
		node0 = m_road_layer->NewNode();
		node0->SetPos(m_nodes[ref_first].p);
		node0->m_id = ref_first;
		m_road_layer->AddNode(node0);
	}
	link->SetNode(0, node0);

	TNode *node1 = m_road_layer->FindNodeByID(ref_last);
	if (!node1)
	{
		// doesn't exist, create it
		node1 = m_road_layer->NewNode();
		node1->SetPos(m_nodes[ref_last].p);
		node1->m_id = ref_last;
		m_road_layer->AddNode(node1);
	}
	link->SetNode(1, node1);

	// Copy all the points
	for (uint r = 0; r < m_refs.size(); r++)
	{
		int idx = m_refs[r];
		link->Append(m_nodes[idx].p);
	}

	m_road_layer->AddLink(link);

	// point node to links
	node0->AddLink(link);
	node1->AddLink(link);

	link->ComputeExtent();
}

void VisitorOSM::MakeStructure()
{
	if (!m_struct_layer)
	{
		m_struct_layer = new vtStructureLayer;
		m_struct_layer->SetProjection(m_proj);
	}
	if (m_eStructureType == ST_BUILDING)
		MakeBuilding();

	if (m_eStructureType == ST_LINEAR)
		MakeLinear();
}

void VisitorOSM::MakeBuilding()
{
	vtBuilding *bld = m_struct_layer->AddNewBuilding();

	// Apply footprint
	DLine2 foot(m_refs.size());
	for (uint r = 0; r < m_refs.size(); r++)
	{
		int idx = m_refs[r];
		foot[r] = m_nodes[idx].p;
	}
	bld->SetFootprint(0, foot);

	// Apply a default style of building
	vtBuilding *pDefBld = GetClosestDefault(bld);
	if (pDefBld)
		bld->CopyFromDefault(pDefBld, true);
	else
	{
		bld->SetStories(1);
		bld->SetRoofType(ROOF_FLAT);
	}

	// Apply other building info, if we have it.
	if (m_fHeight != -1)
		bld->GetLevel(0)->m_fStoryHeight = m_fHeight;
	if (m_iNumStories != -1)
		bld->SetStories(m_iNumStories);
	if (m_RoofType != NUM_ROOFTYPES)
		bld->SetRoofType(m_RoofType);
}

void VisitorOSM::MakeLinear()
{
	vtFence *ls = m_struct_layer->AddNewFence();

	// Apply footprint
	DLine2 foot(m_refs.size());
	for (uint r = 0; r < m_refs.size(); r++)
	{
		int idx = m_refs[r];
		foot[r] = m_nodes[idx].p;
	}
	ls->SetFencePoints(foot);

	// Apply style;
	ls->ApplyStyle(m_eLinearStyle);
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

	VisitorOSM visitor;
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

	if (visitor.m_road_layer)
		AddLayerWithCheck(visitor.m_road_layer, true);

	if (visitor.m_struct_layer)
		AddLayerWithCheck(visitor.m_struct_layer, true);
}
