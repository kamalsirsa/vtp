//
// The vtStructureArray class is an array of Structure objects.
//
// It supports operations including loading and saving to a file
// and picking of building elements.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma warning( disable : 4786 )  

#include <stdlib.h>
#include <string.h>
#include "vtdata/shapelib/shapefil.h"
#include "xmlhelper/easyxml.hpp"
#include "StructArray.h"
#include "Building.h"
#include "Fence.h"

void vtStructureArray::AddBuilding(vtBuilding *bld)
{
	vtStructure *s = NewStructure();
	s->SetBuilding(bld);
	Append(s);
}

// Factories
vtBuilding *vtStructureArray::NewBuilding()
{
	return new vtBuilding;
}

vtFence *vtStructureArray::NewFence()
{
	return new vtFence;
}

void vtStructureArray::DestructItems(int first, int last)
{
	for (int i = first; i <= last; i++)
		delete GetAt(i);
}

bool vtStructureArray::ReadBCF(const char* pathname)
{
	FILE* fp;
	if ( (fp = fopen(pathname, "rb")) == NULL )
		return false;

	char buf[4];
	fread(buf, 3, 1, fp);
	if (strncmp(buf, "bcf", 3))
	{
		// not current bcf, try reading old format
		rewind(fp);
		return ReadBCF_Old(fp);
	}

	float version;
	fscanf(fp, "%f\n", &version);

	if (version < BCFVERSION_SUPPORTED)
	{
		// too old, unsupported version
		fclose(fp);
		return false;
	}

	int zone = 1;
	if (version == 1.2f)
	{
		fscanf(fp, "utm_zone %d\n", &zone);
	}
	m_proj.SetUTM(zone);

	int i, j, count;
	DPoint2 p;
	int points;
	char key[80];
	RGBi color;

	fscanf(fp, "buildings %d\n", &count);
	for (i = 0; i < count; i++)	//for each building
	{
		vtBuilding *bld = NewBuilding();

		BldShape type;
		fscanf(fp, "type %d\n", &type);
		bld->SetShape((BldShape) type);

		int stories = 1;
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
				bld->SetLocation(loc);
			}
			else if (!strcmp(key, "rot"))
			{
				float rot;
				fscanf(fp, "%f\n", &rot);
				bld->SetRotation(rot);
			}
			else if (!strcmp(key, "stories"))
			{
				fscanf(fp, "%d\n", &stories);

				if (stories < 1 || stories > 10) stories = 1;	// TEMPORARY FIX!
				bld->SetStories(stories);
			}
			else if (!strcmp(key, "color"))
			{
				fscanf(fp, "%hd %hd %hd\n", &color.r, &color.g, &color.b);
				bld->SetColor(BLD_BASIC, color);
			}
			else if (!strcmp(key, "color_roof"))
			{
				fscanf(fp, "%hd %hd %hd\n", &color.r, &color.g, &color.b);
				bld->SetColor(BLD_ROOF, color);
			}
			else if (!strcmp(key, "color_trim"))
			{
				fscanf(fp, "%hd %hd %hd\n", &color.r, &color.g, &color.b);
				bld->SetColor(BLD_MOULDING, color);
			}
			else if (!strcmp(key, "size"))
			{
				float w, d;
				fscanf(fp, "%f %f\n", &w, &d);
				bld->SetRectangle(w, d);
			}
			else if (!strcmp(key, "radius"))
			{
				float rad;
				fscanf(fp, "%f\n", &rad);
				bld->SetRadius(rad);
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
				bld->SetFootprint(dl);
			}
			else if (!strcmp(key, "trim"))
			{
				int trim;
				fscanf(fp, "%d\n", &trim);
				bld->m_bMoulding = (trim != 0);
			}
			else if (!strcmp(key, "elev"))
			{
				int elev;
				fscanf(fp, "%d\n", &elev);
				bld->m_bElevated = (elev != 0);
			}
			else if (!strcmp(key, "roof_type"))
			{
				int rt;
				fscanf(fp, "%d\n", &rt);
				bld->m_RoofType = (RoofType) rt;
			}
		}
		AddBuilding(bld);
	}
	fclose(fp);
	return true;
}

bool vtStructureArray::ReadBCF_Old(FILE *fp)
{
	int ncoords;
	int num = fscanf(fp, "%d\n", &ncoords);
	if (num != 1)
		return false;

	DPoint2 point;
	for (int i = 0; i < ncoords; i++)
	{
		fscanf(fp, "%lf %lf\n", &point.x, &point.y);
		vtBuilding *bld = NewBuilding();
		bld->SetLocation(point);
		AddBuilding(bld);
	}

	fclose(fp);
	return true;
}

bool vtStructureArray::ReadSHP(const char* pathname)
{
	SHPHandle hSHP = SHPOpen(pathname, "rb");
	if (hSHP == NULL)
		return false;

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
		vtBuilding *bld = NewBuilding();
		bld->SetLocation(point);
		vtStructure *s = NewStructure();
		s->SetBuilding(bld);
		Append(s);
		SHPDestroyObject(psShape);
	}
	SHPClose(hSHP);
	return true;
}

bool vtStructureArray::WriteSHP(const char* pathname)
{
	char *ext = strrchr(pathname, '.');

    SHPHandle hSHP = SHPCreate ( pathname, SHPT_POINT );
    if (!hSHP)
		return false;

	int count = GetSize();
	DPoint2 temp;
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


//
// find the corner of a building closest to the given point
//
// return true if found
//
bool vtStructureArray::FindClosestBuildingCorner(const DPoint2 &point,
			double error, int &building, int &corner, double &closest)
{
	if (IsEmpty())
		return false;

	building = -1;
	DPoint2 loc;
	double dist;
	closest = 1E8;

	int i, j;
	for (i = 0; i < GetSize(); i++)
	{
		vtStructure *str = GetAt(i);
		if (str->GetType() != ST_BUILDING)
			continue;
		vtBuilding *bld = str->GetBuilding();
		switch (bld->GetShape())
		{
		case SHAPE_RECTANGLE:
			if (bld->GetFootprint().GetSize() == 0)
				bld->RectToPoly();

		case SHAPE_POLY:
			{
				DLine2 &dl = bld->GetFootprint();
				for (j = 0; j < dl.GetSize(); j++)
				{
					dist = (dl.GetAt(j) - point).Length();
					if (dist > error)
						continue;
					if (dist < closest)
					{
						building = i;
						corner = j;
						closest = dist;
					}
				}
			}
			break;
		case SHAPE_CIRCLE:
			loc = bld->GetLocation();
			dist = fabs((point - loc).Length() - bld->GetRadius());
			if (dist > error)
				continue;
			if (dist < closest)
			{
				building = i;
				corner = 0;
				closest = dist;
			}
			break;
		}
	}
	return (building != -1);
}

//
// find the corner of a building closest to the given point
//
// return true if found
//
bool vtStructureArray::FindClosestBuildingCenter(const DPoint2 &point,
				double error, int &building, double &closest)
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
		if (str->GetType() != ST_BUILDING)
			continue;
		vtBuilding *bld = str->GetBuilding();
		loc = bld->GetLocation();
		dist = (loc - point).Length();
		if (dist > error)
			continue;
		if (dist < closest)
		{
			building = i;
			closest = dist;
		}
	}
	return (building != -1);
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
	sscanf(str, "%d %d %d\n", &color.r, &color.g, &color.b);
	return color;
}

class StructureVisitor : public XMLVisitor
{
public:
	StructureVisitor(vtStructureArray *sa)
	: m_pSA(sa), _level(0), _hasException(false) {}

	virtual ~StructureVisitor () {}

	void startXML ();
	void endXML ();
	void startElement (const char * name, const XMLAttributes &atts);
	void endElement (const char * name);
	void data (const char * s, int length);
	void warning (const char * message, int line, int column);

	bool hasException () const { return _hasException; }
	xh_io_exception &getException () { return _exception; }
	void setException (const xh_io_exception &exception) {
		_exception = exception;
		_hasException = true;
	}

private:
	struct State
	{
		State () : item(0), type("") {}
		State (vtStructure * _item, const char * _type)
			: item(_item), type(_type) {}
		vtStructure * item;
		string type;
	};

	State &state () { return _state_stack[_state_stack.size() - 1]; }

	void push_state (vtStructure * _item, const char * type)
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
	string _base;
	xh_io_exception _exception;
	bool _hasException;

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
		vtStructure *pItem = NULL;
		if (string(name) == (string)"structure")
		{
			// Get the name.
			attval = atts.getValue("type");
			if (attval != NULL)
			{
				if (string(attval) == (string)"building")
				{
					vtBuilding *bld = m_pSA->NewBuilding();
					pItem = m_pSA->NewStructure();
					pItem->SetBuilding(bld);
				}
				if (string(attval) == (string)"linear")
				{
					vtFence *fen = m_pSA->NewFence();
					pItem = m_pSA->NewStructure();
					pItem->SetFence(fen);
				}
			}
			push_state(pItem, "structure");
		}
		else
		{
			// Unknown field, ignore.
			pItem = NULL;
			push_state(pItem, "dummy");
		}
		return;
	}

	vtStructure *pItem = st.item;
	if (!pItem)
		return;
	vtFence *fen = pItem->GetFence();
	vtBuilding *bld = pItem->GetBuilding();

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
				bld->SetColor(BLD_BASIC, ParseRGB(color));
		}
		if (string(name) == (string)"trim")
		{
			if (bld)
				bld->m_bMoulding = true;
			const char *color = atts.getValue("color");
			if (bld && color)
				bld->SetColor(BLD_MOULDING, ParseRGB(color));
		}
		if (string(name) == (string)"shapes")
		{
			push_state(pItem, "shapes");
			return;
		}
		if (string(name) == (string)"roof")
		{
			const char *type = atts.getValue("type");
			if (bld && (string)type == (string)"flat")
				bld->m_RoofType = ROOF_FLAT;
			if (bld && (string)type == (string)"shed")
				bld->m_RoofType = ROOF_SHED;
			if (bld && (string)type == (string)"gable")
				bld->m_RoofType = ROOF_GABLE;
			if (bld && (string)type == (string)"hip")
				bld->m_RoofType = ROOF_HIP;
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
				fen->SetHeight(atof(abs_str));
		}
		if (string(name) == (string)"posts")
		{
			// this linear structure has posts
			const char *type = atts.getValue("type");
			if (!strcmp(type, "wood"))
				fen->SetFenceType(FT_WIRE);
			else
				fen->SetFenceType(FT_CHAINLINK);

			const char *size = atts.getValue("size");
			FPoint3 postsize;
			postsize.y = fen->GetHeight();
			sscanf(size, "%f, %f", &postsize.x, &postsize.z);
			fen->SetPostSize(postsize);

			const char *spacing = atts.getValue("spacing");
			if (spacing)
				fen->SetSpacing(atof(spacing));
		}
		if (string(name) == (string)"connect")
		{
			const char *type = atts.getValue("type");
			// not yet supported; currently implied by post type
		}
		return;
	}
	if (_level == 4 && bld != NULL)
	{
		vtStructure *pItem = st.item;
		vtBuilding *bld = pItem->GetBuilding();

		if (string(name) == (string)"rect")
		{
			bld->SetShape(SHAPE_RECTANGLE);

			DPoint2 loc;
			FPoint2 size2;
			const char *ref_point = atts.getValue("ref_point");
			if (ref_point)
			{
				sscanf(ref_point, "%lf %lf", &loc.x, &loc.y);
				bld->SetLocation(loc);
			}
			const char *size = atts.getValue("size");
			if (size)
			{
				sscanf(size, "%f, %f", &size2.x, &size2.y);
				bld->SetRectangle(size2.x, size2.y);
			}
			const char *rot = atts.getValue("rot");
			if (rot)
			{
				bld->SetRotation(atof(rot));
			}
		}
		if (string(name) == (string)"circle")
		{
			bld->SetShape(SHAPE_CIRCLE);

			DPoint2 loc;
			const char *ref_point = atts.getValue("ref_point");
			if (ref_point)
			{
				sscanf(ref_point, "%lf %lf", &loc.x, &loc.y);
				bld->SetLocation(loc);
			}
			const char *radius = atts.getValue("radius");
			if (radius)
			{
				bld->SetRadius(atof(radius));
			}
		}
		if (string(name) == (string)"poly")
		{
			int points;
			const char *num = atts.getValue("num");
			points = atoi(num);

			bld->SetShape(SHAPE_POLY);
			DLine2 &foot = bld->GetFootprint();

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
			bld->SetCenterFromPoly();
		}
	}
}

void StructureVisitor::endElement (const char * name)
{
	State &st = state();

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
		// currentl, wall information is not saved or restored, so we must
		// manually indicate that walls should be implied upon loading
		vtStructure *pItem = st.item;
		vtBuilding *bld = pItem->GetBuilding();
		bld->RebuildWalls();

		pop_state();
	}
}

void StructureVisitor::data (const char * s, int length)
{
	if (state().item != NULL)
		_data.append(string(s, length));
}

void StructureVisitor::warning (const char * message, int line, int column)
{
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

	fprintf(fp, "<?xml version=\"1.0\"?>\n\n");
	fprintf(fp, "<structures-file file-format-version=\"1.0\">\n");

	// Write projection
	char type[20], value[2000];
	m_proj.GetTextDescription(type, value);
	fprintf(fp, "<coordinates type=\"%s\" value=\"%s\" />\n", type, value);

	bool bDegrees = (m_proj.IsGeographic() == 1);

	fprintf(fp, "<structures>\n");
	for (i = 0; i < GetSize(); i++)
	{
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (bld)
			bld->WriteXML(fp, bDegrees);
		vtFence *fen = str->GetFence();
		if (fen)
			fen->WriteXML(fp, bDegrees);
		// TODO: a more elegant solution; e.g. a common base class with a
		// virtual Serialize method?
	}
	fprintf(fp, "</structures>\n");
	fprintf(fp, "</structures-file>\n");
	fclose(fp);
	return true;
}

bool vtStructureArray::ReadXML(const char* pathname)
{
	StructureVisitor visitor(this);
	try
	{
		readXML(pathname, visitor);
	}
	catch (xh_io_exception &e)
	{
		// TODO: would be good to pass back the error message.
		return false;
	}
	if (visitor.hasException())
		throw visitor.getException();
	return true;
}

