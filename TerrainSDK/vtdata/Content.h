//
// Content.h
//
// Header for the Content Management classes.
//
// Copyright (c) 2001-2006 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_CONTENTH
#define VTDATA_CONTENTH

#include "xmlhelper/easyxml.hpp"
#include "vtString.h"
#include "MathTypes.h"

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

	bool operator==(const vtTag &v) const
	{
		return (name == v.name && value == v.value);
	}
	bool operator!=(const vtTag &v) const
	{
		return (name != v.name || value != v.value);
	}
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
	virtual ~vtTagArray() {}

	void AddTag(const vtTag &pTag);
	void AddTag(const char *name, const char *value);

	vtTag *FindTag(const char *szTagName);
	const vtTag *FindTag(const char *szTagName) const;

	vtTag *GetTag(int index);
	const vtTag *GetTag(int index) const;

	uint NumTags() const;
	void RemoveTag(int index);
	void RemoveTag(const char *szTagName);
	void Clear();

	// Set value
	void SetValueString(const char *szTagName, const vtString &string, bool bCreating = false);
	void SetValueBool(const char *szTagName, bool value, bool bCreating = false);
	void SetValueInt(const char *szTagName, int value, bool bCreating = false);
	void SetValueFloat(const char *szTagName, float value, bool bCreating = false);
	void SetValueDouble(const char *szTagName, double value, bool bCreating = false);
	void SetValueRGBi(const char *szTagName, const RGBi &value, bool bCreating = false);

	// Get value directly
	const char *GetValueString(const char *szTagName, bool bSuppressWarning = false) const;
	bool GetValueBool(const char *szTagName) const;
	int GetValueInt(const char *szTagName) const;
	float GetValueFloat(const char *szTagName) const;
	double GetValueDouble(const char *szTagName) const;
	RGBi GetValueRGBi(const char *szTagName) const;

	// Get by reference
	bool GetValueString(const char *szTagName, vtString &string) const;
	bool GetValueBool(const char *szTagName, bool &bValue) const;
	bool GetValueInt(const char *szTagName, int &iValue) const;
	bool GetValueFloat(const char *szTagName, float &fValue) const;
	bool GetValueDouble(const char *szTagName, double &dValue) const;
	bool GetValueRGBi(const char *szTagName, RGBi &color) const;

	// Operators
	vtTagArray &operator=(const vtTagArray &v);
	bool operator==(const vtTagArray &v) const;
	bool operator!=(const vtTagArray &v) const;

	// Copy each tag from one array to another
	void CopyTagsFrom(const vtTagArray &v);

	// File IO
	bool WriteToXML(const char *fname, const char *title) const;
	void WriteToXMLBody(FILE *fp, int iIndent) const;
	bool LoadFromXML(const char *fname);

	// Allow overriding values by subclasses
	virtual bool OverrideValue(const char *szTagName, const vtString &string)
	{
		return false;
	}
	virtual void WriteOverridesToXML(FILE *fp) const {}

	// Debugging info
	static void SetVerbose(bool value);
	void LogTags() const;

protected:
	std::vector<vtTag>	m_tags;
	static bool s_bVerbose;		// display debugging info
};


////////////////////////////////////////////////////////////////////////
// Visitor class for XML parsing of TagArray files.

class TagVisitor : public XMLVisitor
{
public:
	TagVisitor(vtTagArray *pArray) : m_level(0), m_pArray(pArray) {}
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement(const char *name);
	void data(const char *s, int length);

protected:
	int m_level;
	string m_data;
	vtTagArray *m_pArray;
};


/**
 * Represents a "culture" item.  A vtItem has a name and any number of tags
 * which provide description.  It also contains a set of models (vtModel)
 * which are polygonal models of the item at various LOD.
 */
class vtItem : public vtTagArray
{
public:
	vtItem();
	virtual ~vtItem();

	void Empty();
	void AddModel(vtModel *item) { m_models.Append(item); }
	void RemoveModel(vtModel *model);
	uint NumModels() { return m_models.GetSize(); }
	vtModel *GetModel(int i) { return m_models.GetAt(i); }

	vtString	m_name;
	FRECT		m_extents;

protected:
	vtArray<vtModel*>	m_models;
};

/**
 * The vtContentManager class keeps a list of 3d models, along with
 * information about what they are and how they should be loaded.
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
	virtual ~vtContentManager();

	void ReadXML(const char *filename);
	void WriteXML(const char *filename) const;

	void Empty();
	void AddItem(vtItem *item) { m_items.Append(item); }
	void RemoveItem(vtItem *item);
	uint NumItems() { return m_items.GetSize(); }
	virtual vtItem *NewItem() { return new vtItem; }
	vtItem *GetItem(int i) { return m_items.GetAt(i); }
	vtItem *FindItemByName(const char *name);
	vtItem *FindItemByType(const char *type, const char *subtype);
	vtString GetFilename() { return m_strFilename; }

protected:
	vtArray<vtItem*>	m_items;
	vtString m_strFilename;
};

#endif // VTDATA_CONTENTH

