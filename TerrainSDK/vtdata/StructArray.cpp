//
// The vtStructureArray class is an array of Structure objects.
//
// It supports operations including loading and saving to a file
// and picking of building elements.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <stdlib.h>
#include <string.h>
#include "LocalConversion.h"
#include "shapelib/shapefil.h"
#include "xmlhelper/easyxml.hpp"
#include "StructArray.h"
#include "Building.h"
#include "Fence.h"

vtStructureArray g_DefaultStructures;


/////////////////////////////////////////////////////////////////////////////

vtStructureArray::vtStructureArray()
{
	m_strFilename = "Untitled.vtst";
	m_pEditBuilding = NULL;
}


// Factories
vtBuilding *vtStructureArray::NewBuilding()
{
	// Make sure that subsequent operations on this building are done in with
	// the correct local coordinate system
	vtBuilding::s_Conv.Setup(m_proj.GetUnits(), DPoint2(0, 0));

	return new vtBuilding;
}

vtFence *vtStructureArray::NewFence()
{
	return new vtFence;
}

vtStructInstance *vtStructureArray::NewInstance()
{
	return new vtStructInstance;
}

vtBuilding *vtStructureArray::AddNewBuilding()
{
	vtBuilding *nb = NewBuilding();
	Append(nb);
	return nb;
}

vtFence *vtStructureArray::AddNewFence()
{
	vtFence *nf = NewFence();
	Append(nf);
	return nf;
}

vtStructInstance *vtStructureArray::AddNewInstance()
{
	vtStructInstance *ni = NewInstance();
	Append(ni);
	return ni;
}

void vtStructureArray::DestructItems(int first, int last)
{
	for (int i = first; i <= last; i++)
		delete GetAt(i);
}

void vtStructureArray::SetEditedEdge(vtBuilding *bld, int lev, int edge)
{
	m_pEditBuilding = bld;
	m_iEditLevel = lev;
	m_iEditEdge = edge;
}

bool vtStructureArray::WriteSHP(const char* pathname)
{
	SHPHandle hSHP = SHPCreate ( pathname, SHPT_POINT );
	if (!hSHP)
		return false;

	int count = GetSize();
//	SHPObject *obj;
	for (int i = 0; i < count; i++)	//for each coordinate
	{
		// TODO
//		obj = SHPCreateSimpleObject(SHPT_POINT, 1, &temp.x, &temp.y, NULL);
//		SHPWriteObject(hSHP, -1, obj);
//		SHPDestroyObject(obj);
	}
	SHPClose(hSHP);
	return true;
}


/** Find the building corner closest to the given point, if it is within
 * 'epsilon' distance.  The building index, corner index, and distance from
 * the given point are all returned by reference.
 */
bool vtStructureArray::FindClosestBuildingCorner(const DPoint2 &point,
			double epsilon, int &building, int &corner, double &closest)
{
	if (IsEmpty())
		return false;

	building = -1;
	double dist;
	closest = 1E8;

	int i, j;
	for (i = 0; i < GetSize(); i++)
	{
		vtStructure *str = GetAt(i);
		if (str->GetType() != ST_BUILDING)
			continue;
		vtBuilding *bld = str->GetBuilding();

		const DLine2 &dl = bld->GetAtFootprint(0);
		for (j = 0; j < dl.GetSize(); j++)
		{
			dist = (dl.GetAt(j) - point).Length();
			if (dist > epsilon)
				continue;
			if (dist < closest)
			{
				building = i;
				corner = j;
				closest = dist;
			}
		}
	}
	return (building != -1);
}

/** Find the building center closest to the given point, if it is within
 * 'epsilon' distance.  The building index, and distance from the given
 * point are returned by reference.
 */
bool vtStructureArray::FindClosestBuildingCenter(const DPoint2 &point,
				double epsilon, int &building, double &closest)
{
	if (IsEmpty())
		return false;

	building = -1;
	DPoint2 loc;
	double dist;
	closest = 1E8;

	for (int i = 0; i < GetSize(); i++)
	{
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (!bld)
			continue;

		bld->GetBaseLevelCenter(loc);
		dist = (loc - point).Length();
		if (dist > epsilon)
			continue;
		if (dist < closest)
		{
			building = i;
			closest = dist;
		}
	}
	return (building != -1);
}

bool vtStructureArray::FindClosestLinearCorner(const DPoint2 &point, double epsilon,
					   int &structure, int &corner, double &closest)
{
	DPoint2 loc;
	double dist;
	int i, j;

	structure = -1;
	corner = -1;
	closest = 1E8;

	for (i = 0; i < GetSize(); i++)
	{
		vtStructure *str = GetAt(i);
		vtFence *fen = str->GetFence();
		if (!fen)
			continue;
		DLine2 &dl = fen->GetFencePoints();
		for (j = 0; j < dl.GetSize(); j++)
		{
			dist = (dl.GetAt(j) - point).Length();
			if (dist > epsilon)
				continue;
			if (dist < closest)
			{
				structure = i;
				corner = j;
				closest = dist;
			}
		}
	}
	return (structure != -1);
}

/**
 * Find the structure which is closest to the given point, if it is within
 * 'epsilon' distance.  The structure index and distance are returned by
 * reference.
 */
bool vtStructureArray::FindClosestStructure(const DPoint2 &point, double epsilon,
					   int &structure, double &closest, bool bSkipBuildings)
{
	structure = -1;
	closest = 1E8;

	if (IsEmpty())
		return false;

	DPoint2 loc;
	double dist;

	for (int i = 0; i < GetSize(); i++)
	{
		vtStructure *str = GetAt(i);

		// it might be a building
		vtBuilding *bld = str->GetBuilding();
		if (bld)
		{
			if (bSkipBuildings)
				continue;
			dist = bld->GetDistanceToInterior(point);
		}

		// or a fence
		vtFence *fen = str->GetFence();
		// or an instance
		vtStructInstance *inst = str->GetInstance();

		if (fen)
			dist = fen->GetDistanceToLine(point);
		if (inst)
		{
			loc = inst->m_p;
			dist = (loc - point).Length();
		}
		if (dist > epsilon)
			continue;

		if (dist < closest)
		{
			structure = i;
			closest = dist;
		}
	}
	return (structure != -1);
}


/**
 * Find the building which is closest to the given point, if it is within
 * 'epsilon' distance.  The structure index and distance are returned by
 * reference.
 */
bool vtStructureArray::FindClosestBuilding(const DPoint2 &point,
						double epsilon, int &structure, double &closest)
{
	structure = -1;
	closest = 1E8;

	DPoint2 loc;
	double dist;

	for (int i = 0; i < GetSize(); i++)
	{
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (!bld) continue;

		dist = bld->GetDistanceToInterior(point);
		if (dist > epsilon)
			continue;
		if (dist < closest)
		{
			structure = i;
			closest = dist;
		}
	}
	return (structure != -1);
}


void vtStructureArray::GetExtents(DRECT &rect)
{
	if (GetSize() == 0)
		return;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);

	DRECT rect2;
	int i, size = GetSize();
	for (i = 0; i < size; i++)
	{
		vtStructure *str = GetAt(i);
		if (str->GetExtents(rect2))
			rect.GrowToContainRect(rect2);
	}
}

int vtStructureArray::AddFoundations(vtHeightField *pHF)
{
	vtLevel *pLev, *pNewLev;
	int i, j, pts, built = 0;
	float fElev;

	int selected = NumSelected();
	int size = GetSize();
	for (i = 0; i < size; i++)
	{
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (!bld)
			continue;
		if (selected > 0 && !str->IsSelected())
			continue;

		// Get the footprint of the lowest level
		pLev = bld->GetLevel(0);
		const DLine2 &foot = pLev->GetFootprint();
		pts = foot.GetSize();

		float fMin = 1E9, fMax = -1E9;
		for (j = 0; j < pts; j++)
		{
			pHF->FindAltitudeAtPoint2(foot.GetAt(j), fElev);

			if (fElev < fMin) fMin = fElev;
			if (fElev > fMax) fMax = fElev;
		}
		float fDiff = fMax - fMin;

		// if there's less than 50cm of depth, don't bother building
		// a foundation
		if (fDiff < 0.5f)
			continue;

		// Create and add a foundation level
		pNewLev = new vtLevel();
		pNewLev->m_iStories = 1;
		pNewLev->m_fStoryHeight = fDiff;
		bld->InsertLevel(0, pNewLev);
		bld->SetFootprint(0, foot);
		pNewLev->SetEdgeMaterial(BMAT_NAME_CEMENT);
		pNewLev->SetEdgeColor(RGBi(255, 255, 255));
		built++;
	}
	return built;
}

void vtStructureArray::RemoveFoundations()
{
	vtLevel *pLev;
	int i, size = GetSize();
	for (i = 0; i < size; i++)
	{
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (!bld)
			continue;
		pLev = bld->GetLevel(0);
		const vtString *mat = pLev->GetEdge(0)->m_pMaterial;
		if (mat && *mat == BMAT_NAME_CEMENT)
		{
			bld->DeleteLevel(0);
		}
	}
}

int vtStructureArray::NumSelected()
{
	int sel = 0;
	for (int i = 0; i < GetSize(); i++)
	{
		if (GetAt(i)->IsSelected()) sel++;
	}
	return sel;
}

void vtStructureArray::DeselectAll()
{
	for (int i = 0; i < GetSize(); i++)
		GetAt(i)->Select(false);
}

void vtStructureArray::DeleteSelected()
{
	for (int i = 0; i < GetSize();)
	{
		vtStructure *str = GetAt(i);
		if (str->IsSelected())
		{
			DestroyStructure(i);
			delete str;
			RemoveAt(i);
		}
		else
			i++;
	}
}


////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of Structure files.
////////////////////////////////////////////////////////////////////////

// helper
RGBi ParseRGB(const char *str)
{
	RGBi color;
	sscanf(str, "%hd %hd %hd\n", &color.r, &color.g, &color.b);
	return color;
}

class StructureVisitor : public XMLVisitor
{
public:
	StructureVisitor(vtStructureArray *sa) :
		 _level(0), m_pSA(sa) {}

	virtual ~StructureVisitor () {}

	void startXML ();
	void endXML ();
	void startElement (const char * name, const XMLAttributes &atts);
	void endElement (const char * name);
	void data (const char * s, int length);

private:
	struct State
	{
		State () : item(0), type("") {}
		State (vtStructure * _item, const char * _type)
			: item(_item), type(_type) {}
		vtStructure * item;
		string type;
		RGBi wall_color;
	};

	State &state () { return _state_stack[_state_stack.size() - 1]; }

	void push_state (vtStructure *_item, const char *type)
	{
		if (type == 0)
			_state_stack.push_back(State(_item, "unspecified"));
		else
			_state_stack.push_back(State(_item, type));
		_level++;
		_data = "";
	}

	void pop_state () {
		_state_stack.pop_back();
		_level--;
	}

	string _data;
	int _level;
	vector<State> _state_stack;

	vtStructureArray *m_pSA;
};

void StructureVisitor::startXML ()
{
  _level = 0;
  _state_stack.resize(0);
}

void StructureVisitor::endXML ()
{
  _level = 0;
  _state_stack.resize(0);
}

void StructureVisitor::startElement (const char * name, const XMLAttributes &atts)
{
	int i;
	State &st = state();

	if (_level == 0)
	{
		if (string(name) != (string)"structures-file")
		{
			string message = "Root element name is ";
			message += name;
			message += "; expected structures-file";
			throw xh_io_exception(message, "XML Reader");
		}
		push_state(NULL, "top");
		return;
	}

	if (_level == 1)
	{
		if (string(name) == (string)"coordinates")
		{
			const char *type  = atts.getValue("type");
			const char *value = atts.getValue("value");
			m_pSA->m_proj.SetTextDescription(type, value);
			g_Conv.Setup(m_pSA->m_proj.GetUnits(), DPoint2(0,0));
		}
		else if (string(name) == (string)"structures")
		{
			push_state(NULL, "structures");
		}
		else
		{
			// Unknown element: ignore
			push_state(NULL, "dummy");
		}
		return;
	}

	const char * attval;

	if (_level == 2)
	{
		vtStructure *pStruct = NULL;
		if (string(name) == (string)"structure")
		{
			// Get the name.
			attval = atts.getValue("type");
			if (attval != NULL)
			{
				if (string(attval) == (string)"building")
				{
					vtBuilding *bld = m_pSA->NewBuilding();
					pStruct = bld;
				}
				if (string(attval) == (string)"linear")
				{
					vtFence *fen = m_pSA->NewFence();
					pStruct = fen;
				}
				if (string(attval) == (string)"instance")
				{
					vtStructInstance *inst = m_pSA->NewInstance();
					pStruct = inst;
				}
			}
			push_state(pStruct, "structure");
		}
		else
		{
			// Unknown field, ignore.
			pStruct = NULL;
			push_state(pStruct, "dummy");
		}
		return;
	}

	vtStructure *pStruct = st.item;
	if (!pStruct)
		return;
	vtFence *fen = pStruct->GetFence();
	vtBuilding *bld = pStruct->GetBuilding();
	vtStructInstance *inst = pStruct->GetInstance();

	if (_level == 3 && bld != NULL)
	{
		if (string(name) == (string)"height")
		{
			const char *sto_str = atts.getValue("stories");
			if (sto_str)
			{
				// height in stories ("floors")
				int stories = atoi(sto_str);
				if (bld)
					bld->SetStories(stories);
			}
		}
		if (string(name) == (string)"walls")
		{
			const char *color = atts.getValue("color");
			if (bld && color)
				st.wall_color = ParseRGB(color);
		}
		if (string(name) == (string)"shapes")
		{
			push_state(pStruct, "shapes");
			return;
		}
		if (string(name) == (string)"roof")
		{
			// hack to postpone setting building color until now
			bld->SetColor(BLD_BASIC, st.wall_color);

			const char *type = atts.getValue("type");
			if (bld && (string)type == (string)"flat")
				bld->SetRoofType(ROOF_FLAT);
			if (bld && (string)type == (string)"shed")
				bld->SetRoofType(ROOF_SHED);
			if (bld && (string)type == (string)"gable")
				bld->SetRoofType(ROOF_GABLE);
			if (bld && (string)type == (string)"hip")
				bld->SetRoofType(ROOF_HIP);
			const char *color = atts.getValue("color");
			if (bld && color)
				bld->SetColor(BLD_ROOF, ParseRGB(color));
		}
		if (string(name) == (string)"points")
		{
			const char *num = atts.getValue("num");
		}
		return;
	}
	if (_level == 3 && fen != NULL)
	{
		if (string(name) == (string)"points")
		{
			int points;
			const char *num = atts.getValue("num");
			points = atoi(num);

			DLine2 &fencepts = fen->GetFencePoints();

			DPoint2 loc;
			const char *coords = atts.getValue("coords");
			const char *cp = coords;
			for (i = 0; i < points; i++)
			{
				sscanf(cp, "%lf %lf", &loc.x, &loc.y);
				fencepts.Append(loc);
				cp = strchr(cp, ' ');
				cp++;
				cp = strchr(cp, ' ');
				cp++;
			}
		}
		if (string(name) == (string)"height")
		{
			// absolute height in meters
			const char *abs_str = atts.getValue("abs");
			if (abs_str)
				fen->SetHeight((float)atof(abs_str));
		}
		if (string(name) == (string)"posts")
		{
			// this linear structure has posts
			const char *type = atts.getValue("type");
			if (0 == strcmp(type, "wood"))
				fen->SetFenceType(FT_WIRE);
			else if (0 == strcmp(type, "steel"))
				fen->SetFenceType(FT_CHAINLINK);
			else if (0 == strcmp(type, "hedgerow"))
				fen->SetFenceType(FT_HEDGEROW);
			else if (0 == strcmp(type, "drystone"))
				fen->SetFenceType(FT_DRYSTONE);
			else if (0 == strcmp(type, "privet"))
				fen->SetFenceType(FT_PRIVET);
			else if (0 == strcmp(type, "stone"))
				fen->SetFenceType(FT_STONE);
			else if (0 == strcmp(type, "beech"))
				fen->SetFenceType(FT_BEECH);
			else
				fen->SetFenceType(FT_WIRE);

			const char *size = atts.getValue("size");
			FPoint3 postsize;
			postsize.y = fen->GetHeight();
			sscanf(size, "%f, %f", &postsize.x, &postsize.z);
			fen->SetPostSize(postsize);

			const char *spacing = atts.getValue("spacing");
			if (spacing)
				fen->SetSpacing((float)atof(spacing));
		}
		if (string(name) == (string)"connect")
		{
			const char *type = atts.getValue("type");
			// not yet supported; currently implied by post type
		}
		return;
	}
	if (_level == 3 && inst != NULL)
	{
		if (string(name) == (string)"placement")
		{
			const char *loc = atts.getValue("location");
			if (loc)
				sscanf(loc, "%lf %lf", &inst->m_p.x, &inst->m_p.y);
			const char *rot = atts.getValue("rotation");
			if (rot)
				sscanf(rot, "%f", &inst->m_fRotation);
		}
		else
			_data = "";
	}
	if (_level == 4 && bld != NULL)
	{
		if (string(name) == (string)"rect")
		{
			DPoint2 loc;
			FPoint2 size2;
			const char *ref_point = atts.getValue("ref_point");
			if (ref_point)
			{
				sscanf(ref_point, "%lf %lf", &loc.x, &loc.y);
				bld->SetRectangle(loc, 10, 10);
			}
			float fRotation = 0.0f;
			const char *rot = atts.getValue("rot");
			if (rot)
			{
				fRotation = (float)atof(rot);
			}
			const char *size = atts.getValue("size");
			if (size)
			{
				sscanf(size, "%f, %f", &size2.x, &size2.y);
				bld->SetRectangle(loc, size2.x, size2.y, fRotation);
			}
		}
		if (string(name) == (string)"circle")
		{
			DPoint2 loc;
			const char *ref_point = atts.getValue("ref_point");
			if (ref_point)
			{
				sscanf(ref_point, "%lf %lf", &loc.x, &loc.y);
				bld->SetCircle(loc, 10);
			}
			const char *radius = atts.getValue("radius");
			if (radius)
			{
				bld->SetCircle(loc, (float)atof(radius));
			}
		}
		if (string(name) == (string)"poly")
		{
			int points;
			const char *num = atts.getValue("num");
			points = atoi(num);

			DLine2 foot;
			DPoint2 loc;
			const char *coords = atts.getValue("coords");
			const char *cp = coords;
			for (i = 0; i < points; i++)
			{
				sscanf(cp, "%lf %lf", &loc.x, &loc.y);
				foot.Append(loc);
				cp = strchr(cp, ' ');
				cp++;
				cp = strchr(cp, ' ');
				cp++;
			}
			bld->SetFootprint(0, foot);
		}
	}
}

void StructureVisitor::endElement(const char * name)
{
	State &st = state();
	vtStructure *pStruct = st.item;

	if (string(name) == (string)"structures")
	{
		pop_state();
	}
	if (string(name) == (string)"structure")
	{
		if (st.item != NULL)
			m_pSA->Append(st.item);
		pop_state();
	}
	if (string(name) == (string)"shapes")
	{
		// currently, building wall information is not saved or restored, so
		// we must manually indicate that detail should be implied upon loading
		vtBuilding *bld = pStruct->GetBuilding();
		bld->AddDefaultDetails();

		pop_state();
	}
	if (_level == 3)
	{
		if (pStruct->GetType() == ST_INSTANCE &&
			string(name) != (string)"placement")
		{
			// save all other tags as literal strings
			vtTag tag;
			tag.name = name;
			tag.value = _data.c_str();

			pStruct->AddTag(tag);
		}
	}
}

void StructureVisitor::data(const char *s, int length)
{
	if (state().item != NULL)
		_data.append(string(s, length));
}


////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of GML Structure files.
////////////////////////////////////////////////////////////////////////

// helper
RGBi ParseHexColor(const char *str)
{
	RGBi color;
	sscanf(str,   "%2hx", &color.r);
	sscanf(str+2, "%2hx", &color.g);
	sscanf(str+4, "%2hx", &color.b);
	return color;
}

class StructVisitorGML : public XMLVisitor
{
public:
	StructVisitorGML(vtStructureArray *sa) : m_state(0), m_pSA(sa) {}
	void startXML() { m_state = 0; }
	void endXML() { m_state = 0; }
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement(const char *name);
	void data(const char *s, int length);

private:
	string m_data;
	int m_state;

	vtStructureArray *m_pSA;
	vtStructure *m_pStructure;
	vtBuilding *m_pBuilding;
	vtStructInstance *m_pInstance;
	vtFence *m_pFence;
	vtLevel *m_pLevel;
	vtEdge *m_pEdge;

	int m_iLevel;
	int m_iEdge;
};

void StructVisitorGML::startElement(const char *name, const XMLAttributes &atts)
{
	const char *attval;

	// clear data at the start of each element
	m_data = "";

	if (m_state == 0 && !strcmp(name, "StructureCollection"))
	{
		m_state = 1;
		return;
	}

	if (m_state == 1)
	{
		if (!strcmp(name, "Building"))
		{
			m_pBuilding = m_pSA->NewBuilding();
			m_pStructure = m_pBuilding;
			m_state = 2;
			m_iLevel = 0;
		}
		else if (!strcmp(name, "Linear"))
		{
			m_pFence = m_pSA->NewFence();
			m_pStructure = m_pFence;

			attval = atts.getValue("Height");
			if (attval)
				m_pFence->SetHeight((float) atof(attval));

			m_state = 10;
		}
		else if (!strcmp(name, "Imported"))
		{
			m_pInstance = m_pSA->NewInstance();
			m_pStructure = m_pInstance;

			m_state = 20;
		}
		attval = atts.getValue("ElevationOffset");
		if (attval)
			m_pStructure->SetElevationOffset((float) atof(attval));
		attval = atts.getValue("OriginalElevation");
		if (attval)
			m_pStructure->SetOriginalElevation((float) atof(attval));
		return;
	}

	if (m_state == 2)	// Building
	{
		if (!strcmp(name, "Level"))
		{
			m_pLevel = m_pBuilding->CreateLevel();
			attval = atts.getValue("FloorHeight");
			if (attval)
				m_pLevel->m_fStoryHeight = (float) atof(attval);
			attval = atts.getValue("StoryCount");
			if (attval)
				m_pLevel->m_iStories = atoi(attval);
			m_state = 3;
			m_iEdge = 0;
		}
		return;
	}

	if (m_state == 3)	// Level
	{
		if (!strcmp(name, "Footprint"))
			m_state = 4;
		else if (!strcmp(name, "Edge"))
		{
			m_pEdge = m_pLevel->GetEdge(m_iEdge);
			m_pEdge->m_Features.clear();

			attval = atts.getValue("Material");
			if (attval)
				m_pEdge->m_pMaterial = GetGlobalMaterials()->FindName(attval);
			attval = atts.getValue("Color");
			if (attval)
				m_pEdge->m_Color = ParseHexColor(attval);
			attval = atts.getValue("Slope");
			if (attval)
				m_pEdge->m_iSlope = atoi(attval);
			attval = atts.getValue("EaveLength");
			if (attval)
				m_pEdge->m_fEaveLength = (float) atof(attval);
			attval = atts.getValue("Facade");
			if (attval)
				m_pEdge->m_Facade = attval;

			m_state = 5;
		}
		return;
	}

	if (m_state == 4)	// Footprint
	{
		// nothing necessary here, catch the end of element
	}

	if (m_state == 5)	// Edge
	{
		if (!strcmp(name, "EdgeElement"))
		{
			vtEdgeFeature ef;

			attval = atts.getValue("Type");
			if (attval)
				ef.m_code = vtBuilding::GetEdgeFeatureValue(attval);
			attval = atts.getValue("Begin");
			if (attval)
				ef.m_vf1 = (float) atof(attval);
			attval = atts.getValue("End");
			if (attval)
				ef.m_vf2 = (float) atof(attval);
			m_pEdge->m_Features.push_back(ef);
		}
	}

	if (m_state == 10)	// Linear
	{
		if (!strcmp(name, "Path"))
		{
			m_state = 11;
		}
		else if (!strcmp(name, "Posts"))
		{
			// this linear structure has posts
			const char *type = atts.getValue("Type");
			if (0 == strcmp(type, "wood"))
				m_pFence->SetFenceType(FT_WIRE);
			else if (0 == strcmp(type, "steel"))
				m_pFence->SetFenceType(FT_CHAINLINK);
			else if (0 == strcmp(type, "hedgerow"))
				m_pFence->SetFenceType(FT_HEDGEROW);
			else if (0 == strcmp(type, "drystone"))
				m_pFence->SetFenceType(FT_DRYSTONE);
			else if (0 == strcmp(type, "privet"))
				m_pFence->SetFenceType(FT_PRIVET);
			else if (0 == strcmp(type, "stone"))
				m_pFence->SetFenceType(FT_STONE);
			else if (0 == strcmp(type, "beech"))
				m_pFence->SetFenceType(FT_BEECH);
			else
				m_pFence->SetFenceType(FT_WIRE);

			const char *size = atts.getValue("Size");
			FPoint3 postsize;
			postsize.y = m_pFence->GetHeight();
			sscanf(size, "%f, %f", &postsize.x, &postsize.z);
			m_pFence->SetPostSize(postsize);

			const char *spacing = atts.getValue("Spacing");
			if (spacing)
				m_pFence->SetSpacing((float)atof(spacing));
		}
		else if (!strcmp(name, "Connect"))
		{
			// not yet supported; currently implied by post type
		}
	}
	if (m_state == 20)	// Imported
	{
		if (!strcmp(name, "Location"))
		{
			m_state = 21;
		}
	}
}

void StructVisitorGML::endElement(const char *name)
{
	bool bGrabAttribute = false;
	const char *data = m_data.c_str();

	if (m_state == 5 && !strcmp(name, "Edge"))
	{
		m_iEdge++;
		m_state = 3;
	}

	else if (m_state == 4)
	{
		if (!strcmp(name, "gml:coordinates"))
		{
			DLine2 line;
			double x, y;
			while (sscanf(data, "%lf,%lf", &x, &y) == 2)
			{
				line.Append(DPoint2(x,y));
				data = strchr(data, ' ');
				if (data)
					data++;
				else
					break;
			}
			m_pBuilding->SetFootprint(m_iLevel, line);
		}
		else if (!strcmp(name, "Footprint"))
			m_state = 3;
	}

	else if (m_state == 3 && !strcmp(name, "Level"))
	{
		m_state = 2;
		m_iLevel ++;
	}

	else if (m_state == 2)
	{
		if (!strcmp(name, "Building"))
		{
			m_state = 1;

			m_pSA->Append(m_pStructure);
			m_pStructure = NULL;
		}
		else
			bGrabAttribute = true;
	}
	else if (m_state == 1 && (!strcmp(name, "SRS")))
	{
		m_pSA->m_proj.SetTextDescription("wkt", data);

		// This seems wrong - why would each .vtst file reset the global projection?
		// g_Conv.Setup(m_pSA->m_proj.GetUnits(), DPoint2(0,0));
	}
	else if (m_state == 10)
	{
		if (!strcmp(name, "Linear"))
		{
			m_state = 1;

			m_pSA->Append(m_pStructure);
			m_pStructure = NULL;
		}
	}
	else if (m_state == 11)
	{
		if (!strcmp(name, "gml:coordinates"))
		{
			DLine2 &fencepts = m_pFence->GetFencePoints();
			double x, y;
			while (sscanf(data, "%lf,%lf", &x, &y) == 2)
			{
				fencepts.Append(DPoint2(x,y));
				data = strchr(data, ' ');
				if (data)
					data++;
				else
					break;
			}
		}
		else if (!strcmp(name, "Path"))
			m_state = 10;
	}
	else if (m_state == 20)
	{
		if (!strcmp(name, "Imported"))
		{
			m_state = 1;
			m_pSA->Append(m_pStructure);
			m_pStructure = NULL;
		}
		else if (!strcmp(name, "Rotation"))
		{
			sscanf(data, "%f", &m_pInstance->m_fRotation);
		}
		else if (!strcmp(name, "Scale"))
		{
			sscanf(data, "%f", &m_pInstance->m_fScale);
		}
		else
			bGrabAttribute = true;
	}
	else if (m_state == 21)
	{
		if (!strcmp(name, "gml:coordinates"))
		{
			double x, y;
			sscanf(data, "%lf,%lf", &x, &y);
			m_pInstance->m_p.Set(x,y);
		}
		else if (!strcmp(name, "Location"))
			m_state = 20;
	}

	// first check for Attribute nodes
	if (bGrabAttribute)
	{
		// save these elements as literal strings
		vtTag tag;
		tag.name = name;
		tag.value = data;
		m_pStructure->AddTag(tag);		// where does the tag go?
	}
}

void StructVisitorGML::data(const char *s, int length)
{
	m_data.append(string(s, length));
}

/////////////////////////////////////////////////////////////////////////


bool vtStructureArray::WriteXML_Old(const char* filename)
{
	int i;
	FILE *fp = fopen(filename, "wb");
	if (!fp)
	{
		throw xh_io_exception("Failed to open file", xh_location(filename),
				"XML Writer");
	}

	fprintf(fp, "<?xml version=\"1.0\"?>\n\n");
	fprintf(fp, "<structures-file file-format-version=\"1.0\">\n");

	// Write projection
	char type[20], tempvalue[2000];
	//RFJ This needs to be processed !!!!!!!
	m_proj.GetTextDescription(type, tempvalue);
	vtString value = EscapeStringForXML(tempvalue);

	fprintf(fp, "<coordinates type=\"%s\" value=\"%s\" />\n", type,
		(const char *) value);

	bool bDegrees = (m_proj.IsGeographic() == 1);

	fprintf(fp, "<structures>\n");
	for (i = 0; i < GetSize(); i++)
	{
		vtStructure *str = GetAt(i);
		str->WriteXML_Old(fp, bDegrees);
	}
	fprintf(fp, "</structures>\n");
	fprintf(fp, "</structures-file>\n");
	fclose(fp);
	return true;
}


/////////////////////////////////////////////////////////////////////////


bool vtStructureArray::WriteXML(const char* filename)
{
	int i;
	FILE *fp = fopen(filename, "wb");
	if (!fp)
	{
		throw xh_io_exception("Failed to open file", xh_location(filename),
				"XML Writer");
	}

	fprintf(fp, "<?xml version=\"1.0\"?>\n");
	fprintf(fp, "\n");

	fprintf(fp, "<StructureCollection xmlns=\"http://www.openplans.net\"\n"
		"\t\t\t\t\t xmlns:gml=\"http://www.opengis.net/gml\"\n"
		"\t\t\t\t\t xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
		"\t\t\t\t\t xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
		"\t\t\t\t\t xsi:schemaLocation=\"http://www.openplans.net/buildings.xsd\">\n");
	fprintf(fp, "\n");

	// Write the extents (required by gml:StructureCollection)
	DRECT ext;
	GetExtents(ext);
	fprintf(fp, "\t<gml:boundedBy>\n");
	fprintf(fp, "\t\t<gml:Box>\n");
	fprintf(fp, "\t\t\t<gml:coordinates>");
	fprintf(fp, "%lf,%lf %lf,%lf", ext.left, ext.bottom, ext.right, ext.top);
	fprintf(fp, "</gml:coordinates>\n");
	fprintf(fp, "\t\t</gml:Box>\n");
	fprintf(fp, "\t</gml:boundedBy>\n");
	fprintf(fp, "\n");

	// Write projection
	char *wkt;
	OGRErr err = m_proj.exportToWkt(&wkt);
	if (err != OGRERR_NONE)
		return false;
	fprintf(fp, "\t<SRS>%s</SRS>\n", wkt);
	fprintf(fp, "\n");
	OGRFree(wkt);

	bool bDegrees = (m_proj.IsGeographic() == 1);

	for (i = 0; i < GetSize(); i++)
	{
		vtStructure *str = GetAt(i);
		str->WriteXML(fp, bDegrees);
	}
	fprintf(fp, "</StructureCollection>\n");
	fclose(fp);
	return true;
}

bool vtStructureArray::ReadXML(const char *pathname)
{
	// check to see if it's old or new format
	bool bOldFormat = false;
	FILE *fp = fopen(pathname, "r");
	if (!fp) return false;

	m_strFilename = pathname;

	fseek(fp, 24, SEEK_SET);
	char buf[10];
	fread(buf, 10, 1, fp);
	if (!strncmp(buf, "structures", 10))
		bOldFormat = true;
	else
	{
		// RFJ quick hack for extra carriage returns
		fseek(fp, 26, SEEK_SET);
		fread(buf, 10, 1, fp);
		if (!strncmp(buf, "structures", 10))
			bOldFormat = true;
	}

	fclose(fp);

	if (bOldFormat)
	{
		StructureVisitor visitor(this);
		try
		{
			readXML(pathname, visitor);
		}
		catch (xh_exception &)
		{
			// TODO: would be good to pass back the error message.
			return false;
		}
	}
	else
	{
		StructVisitorGML visitor(this);
		try
		{
			readXML(pathname, visitor);
		}
		catch (xh_exception &)
		{
			// TODO: would be good to pass back the error message.
			return false;
		}
	}
	return true;
}


/////////////////////
// Helpers

int GetSHPType(const char *filename)
{
	SHPHandle hSHP = SHPOpen(filename, "rb");
	if (hSHP == NULL)
		return SHPT_NULL;

	int		nEntities, nShapeType;
	double 	adfMinBound[4], adfMaxBound[4];

	SHPGetInfo(hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound);
	SHPClose(hSHP);
	return nShapeType;
}

vtBuilding *GetClosestDefault(vtBuilding *pBld)
{
	// For now, just grab the first building from the defaults
	int i, num = g_DefaultStructures.GetSize();
	for (i = 0; i < num; i++)
	{
		vtStructure *pStr = g_DefaultStructures[i];
		vtBuilding *pDefBld = pStr->GetBuilding();
		if (pDefBld)
			return pDefBld;
	}
	return NULL;
}

vtFence *GetClosestDefault(vtFence *pFence)
{
	// For now, just grab the first fence from the defaults
	int i, num = g_DefaultStructures.GetSize();
	for (i = 0; i < num; i++)
	{
		vtStructure *pStr = g_DefaultStructures[i];
		vtFence *pDefFence = pStr->GetFence();
		if (pDefFence)
			return pDefFence;
	}
	return NULL;
}

vtStructInstance *GetClosestDefault(vtStructInstance *pInstance)
{
	// For now, just grab the first instance from the defaults
	int i, num = g_DefaultStructures.GetSize();
	for (i = 0; i < num; i++)
	{
		vtStructure *pStr = g_DefaultStructures[i];
		vtStructInstance *pDefInstance = pStr->GetInstance();
		if (pDefInstance)
			return pDefInstance;
	}
	return NULL;
}

bool SetupDefaultStructures(const char *fname)
{
	if (!fname)
		fname = "DefaultStructures.vtst";
	if (g_DefaultStructures.ReadXML(fname))
		return true;

	// else supply some internal defaults and let the user know the load failed
	vtBuilding *pBld = g_DefaultStructures.NewBuilding();
	vtLevel *pLevel;
	DLine2 DefaultFootprint; // Single edge
	DefaultFootprint.Append(DPoint2(0.0, 0.0));
	DefaultFootprint.Append(DPoint2(0.0, 1.0));

	// The default building is NOT a complete building
	// Alter the code here to set different hard coded
	// defaults for use in other operations
	// First set any structure tags needed
	//
	// NONE
	//
	// Now create the required number of levels
	// and set the values
	//
	// Level 0
	pLevel = pBld->CreateLevel(DefaultFootprint);
	pLevel->m_iStories = 1;
	pLevel->m_fStoryHeight = 3.20f;
	pLevel->SetEdgeMaterial(BMAT_NAME_PLAIN);
	pLevel->SetEdgeColor(RGBi(255,0,0)); // Red
	pLevel->GetEdge(0)->m_iSlope = 90;
	// Level 1
	pLevel = pBld->CreateLevel(DefaultFootprint);
	pLevel->m_iStories = 1;
	pLevel->m_fStoryHeight = 3.20f;
	pLevel->SetEdgeMaterial(BMAT_NAME_PLAIN);
	pLevel->SetEdgeColor(RGBi(255,240,225)); // Tan
	pLevel->GetEdge(0)->m_iSlope = 0;		 // Flat

	g_DefaultStructures.Append(pBld);

	return false;
}

