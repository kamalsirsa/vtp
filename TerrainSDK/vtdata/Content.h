//
// Content.h
//
// Header for the ContentManager class.
//
// Copyright (c) 2001 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_CONTENTH
#define VTDATA_CONTENTH

#include "vtString.h"
#include "Array.h"
#include "Array.inl"

class vtModel
{
public:
	vtModel()
	{
		m_distance = 0.0f;
		m_scale = 1.0f;
		m_attempted_load = false;
	}

	vtString	m_filename;
	float		m_distance;
	float		m_scale;	// meters per unit (e.g. cm = .01)
	bool		m_attempted_load;
};

class vtItem
{
public:
	~vtItem();

	void Empty();
	void AddModel(vtModel *item) { m_models.Append(item); }
	void RemoveModel(vtModel *model);
	int NumModels() { return m_models.GetSize(); }
	vtModel *GetModel(int i) { return m_models.GetAt(i); }

	vtString	m_name;
	vtString	m_type;
	vtString	m_subtype;
	vtString	m_url;

protected:
	Array<vtModel*>	m_models;
};

/**  The vtContentManager class is useful for keeping a list of 3d models,
 * along with information about what they are and how they should be loaded.
 * It consists of a set of Content Items (vtItem) which each represent a
 * particular object, which in turn consist of Models (vtModel) which are a
 * particular 3d geometry for that Item.  An Item can have several Models
 * which represent different levels of detail (LOD).
 * \par
 * To load a set of content  from a file, first create an empty
 * vtContentManager object, then call ReadXML() with the name of name of a
 * VT Content file (.vtco).
 */
class vtContentManager
{
public:
	~vtContentManager();

	void ReadXML(const char *filename);
	void WriteXML(const char *filename);

	void Empty();
	void AddItem(vtItem *item) { m_items.Append(item); }
	void RemoveItem(vtItem *item);
	int NumItems() { return m_items.GetSize(); }
	vtItem *GetItem(int i) { return m_items.GetAt(i); }
	vtItem *FindItemByName(const char *name);
	vtItem *FindItemByType(const char *type, const char *subtype);

protected:
	Array<vtItem*>	m_items;
};

#define NUM_ITEMTYPES	4
#define NUM_SUBTYPES	15
extern const char *ItemTypeNames[NUM_ITEMTYPES];
extern const char *SubTypeNames[NUM_SUBTYPES];

#endif // VTDATA_CONTENTH

