//
// Content.h
//
// Header for the Content Management classes.
//
// Copyright (c) 2001-2003 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_CONTENTH
#define VTDATA_CONTENTH

#include "vtString.h"
#include "Array.h"

/**
 * vtModel contains a reference to a 3d polygonal model: a filename, the
 * scale of the model, and the distance at which this LOD should be used.
 */
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

/**
 * Each tag has two strings: a Name and a Value.
 * This is similar to the concept of a tag in XML.
 */
class vtTag
{
public:
	vtString name;
	vtString value;
};

/**
 * A simple set of tags.  Each tag (vtTag) has two strings: a Name and a Value.
 * This is similar to the concept of a tag in XML.
 * \par
 * If this gets used for something more performance-sensitive, we could replace
 * the linear lookup with a hash map.
 */
class vtTagArray
{
public:
	void AddTag(const vtTag &pTag);
	void AddTag(const char *name, const char *value);

	vtTag *FindTag(const char *szTagName);
	vtTag *GetTag(int index);
	unsigned int NumTags();
	void RemoveTag(int index);
	void RemoveTag(const char *szTagName);

	// Set value
	void SetValueString(const char *szTagName, const vtString &string);
	void SetValueBool(const char *szTagName, bool value);
	void SetValueInt(const char *szTagName, int value);
	void SetValueFloat(const char *szTagName, float value);
	void SetValueDouble(const char *szTagName, double value);

	// Get value directly
	const char *GetValueString(const char *szTagName);
	bool GetValueBool(const char *szTagName);
	int GetValueInt(const char *szTagName);
	float GetValueFloat(const char *szTagName);
	double GetValueDouble(const char *szTagName);

	// Get by reference
	bool GetValueString(const char *szTagName, vtString &string);
	bool GetValueBool(const char *szTagName, bool &bValue);
	bool GetValueInt(const char *szTagName, int &iValue);
	bool GetValueFloat(const char *szTagName, float &fValue);
	bool GetValueDouble(const char *szTagName, double &dValue);

protected:
	std::vector<vtTag>	m_tags;
};

/**
 * Represents a "culture" item.  A vtItem has a name and any number of tags
 * which provide description.  It also contains a set of models (vtModel)
 * which are polygonal models of the item at various LOD.
 */
class vtItem : public vtTagArray
{
public:
	~vtItem();

	void Empty();
	void AddModel(vtModel *item) { m_models.Append(item); }
	void RemoveModel(vtModel *model);
	unsigned int NumModels() { return m_models.GetSize(); }
	vtModel *GetModel(int i) { return m_models.GetAt(i); }

	vtString		m_name;

protected:
	Array<vtModel*>	m_models;
};

/**
 * The vtContentManager class keeps a list of 3d models,
 * along with information about what they are and how they should be loaded.
 * It consists of a set of Content Items (vtItem) which each represent a
 * particular object, which in turn consist of Models (vtModel) which are a
 * particular 3D geometry for that Item.  An Item can have several Models
 * which represent different levels of detail (LOD).
 * \par
 * To load a set of content  from a file, first create a
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
	unsigned int NumItems() { return m_items.GetSize(); }
	virtual vtItem *NewItem() { return new vtItem; }
	vtItem *GetItem(int i) { return m_items.GetAt(i); }
	vtItem *FindItemByName(const char *name);
	vtItem *FindItemByType(const char *type, const char *subtype);

protected:
	Array<vtItem*>	m_items;
};

#endif // VTDATA_CONTENTH

