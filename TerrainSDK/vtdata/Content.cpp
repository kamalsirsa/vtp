//
// Content.cpp
//
// Implements the ContentManager class, including the ability to read
// and write the contents to an XML file.
//
// Copyright (c) 2001-2004 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <stdio.h>
#include "xmlhelper/easyxml.hpp"
#include "Content.h"
#include "vtLog.h"


////////////////////////////////////////////////////////////////////////
// Implementation of class vtTagArray
//

void vtTagArray::AddTag(const vtTag &pTag)
{
	m_tags.push_back(pTag);
}

void vtTagArray::AddTag(const char *name, const char *value)
{
	vtTag tag;
	tag.name = name;
	tag.value = value;
	m_tags.push_back(tag);
}

vtTag *vtTagArray::FindTag(const char *name)
{
	int size = m_tags.size();
	for (int i = 0; i < size; i++)
	{
		vtTag *tag = &m_tags[i];
		if (!tag->name.CompareNoCase(name))
			return tag;
	}
	return NULL;
}

const vtTag *vtTagArray::FindTag(const char *name) const
{
	int size = m_tags.size();
	for (int i = 0; i < size; i++)
	{
		const vtTag *tag = &m_tags[i];
		if (!tag->name.CompareNoCase(name))
			return tag;
	}
	return NULL;
}

vtTag *vtTagArray::GetTag(int index)
{
	return &m_tags[index];
}

const vtTag *vtTagArray::GetTag(int index) const
{
	return &m_tags[index];
}

unsigned int vtTagArray::NumTags() const
{
	return m_tags.size();
}

void vtTagArray::RemoveTag(int index)
{
	m_tags.erase(m_tags.begin() + index);
}

void vtTagArray::RemoveTag(const char *szTagName)
{
	int size = m_tags.size();
	vtTag *tag;
	for (int i = 0; i < size; i++)
	{
		tag = &m_tags[i];
		if (!tag->name.CompareNoCase(szTagName))
		{
			m_tags.erase(m_tags.begin() + i);
			break;
		}
	}
}

void vtTagArray::Clear()
{
	m_tags.clear();
}

//
// Set
//

void vtTagArray::SetValueString(const char *szTagName, const vtString &string,
								bool bSuppressWarning)
{
	vtTag *tag = FindTag(szTagName);
	if (tag)
		tag->value = string;
	else
	{
		// might be overridden by a derived class
		if (OverrideValue(szTagName, string))
			return;

		// if not, add it as a new tag
		if (!bSuppressWarning)
			VTLOG("\tWarning: tag %s was not found, creating.\n", szTagName);
		AddTag(szTagName, string);
	}
}

void vtTagArray::SetValueBool(const char *szTagName, bool bValue)
{
	if (bValue)
		SetValueString(szTagName, "true");
	else
		SetValueString(szTagName, "false");
}

void vtTagArray::SetValueInt(const char *szTagName, int iValue)
{
	vtString str;
	str.Format("%d", iValue);
	SetValueString(szTagName, str);
}

void vtTagArray::SetValueFloat(const char *szTagName, float fValue)
{
	vtString str;
	str.Format("%f", fValue);
	SetValueString(szTagName, str);
}

void vtTagArray::SetValueDouble(const char *szTagName, double dValue)
{
	vtString str;
	str.Format("%lf", dValue);
	SetValueString(szTagName, str);
}

void vtTagArray::SetValueRGBi(const char *szTagName, const RGBi &color)
{
	vtString str;
	str.Format("%d %d %d", color.r, color.g, color.b);
	SetValueString(szTagName, str);
}


//
// Get values directly.  Convenient syntax, but can't report failure
//  if the tag doesn't exist.
//

const char *vtTagArray::GetValueString(const char *szTagName, bool bUTF8ToAnsi, bool bSuppressWarning) const
{
	const vtTag *tag = FindTag(szTagName);
	if (tag)
	{
#if SUPPORT_WSTRING
		if (bUTF8ToAnsi)
		{
			// The internal string is UTF8, but we want it as Ansi (iso-8859-1)
			wstring2 wide_string;
			wide_string.from_utf8(tag->value);
			return wide_string.eb_str();
		}
#endif
		return tag->value;
	}
	else
	{
		if (!bSuppressWarning)
			VTLOG("\tWarning: could not get tag %s, not found.\n", szTagName);
		return NULL;
	}
}

bool vtTagArray::GetValueBool(const char *szTagName) const
{
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		return (tag->value[0] == 't' || tag->value[0] == '1');
	return false;
}

int vtTagArray::GetValueInt(const char *szTagName) const
{
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		return atoi((const char *)tag->value);
	return 0;
}

float vtTagArray::GetValueFloat(const char *szTagName) const
{
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		return (float) atof((const char *)tag->value);
	return 0.0f;
}

double vtTagArray::GetValueDouble(const char *szTagName) const
{
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		return atof((const char *)tag->value);
	return 0.0;
}

RGBi vtTagArray::GetValueRGBi(const char *szTagName) const
{
	const vtTag *tag = FindTag(szTagName);
	if (tag)
	{
		RGBi color;
		if (sscanf(tag->value, "%d %d %d", &color.r, &color.g, &color.b) == 3)
			return color;
	}
	return RGBi(-1, -1, -1);
}

//
// Get by reference
//
bool vtTagArray::GetValueString(const char *szTagName, vtString &string) const
{
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		string = tag->value;
	return (tag != NULL);
}

bool vtTagArray::GetValueBool(const char *szTagName, bool &bValue) const
{
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		bValue = (tag->value[0] == 't' || tag->value[0] == '1');
	return (tag != NULL);
}

bool vtTagArray::GetValueInt(const char *szTagName, int &iValue) const
{
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		iValue = atoi((const char *)tag->value);
	return (tag != NULL);
}

bool vtTagArray::GetValueFloat(const char *szTagName, float &fValue) const
{
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		fValue = (float) atof((const char *)tag->value);
	return (tag != NULL);
}

bool vtTagArray::GetValueDouble(const char *szTagName, double &dValue) const
{
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		dValue = atof((const char *)tag->value);
	return (tag != NULL);
}

bool vtTagArray::GetValueRGBi(const char *szTagName, RGBi &color) const
{
	const vtTag *tag = FindTag(szTagName);
	if (tag)
	{
		int r, g, b;
		if (sscanf(tag->value, "%d %d %d", &r, &g, &b) == 3)
			color.Set(r, g, b);
		else
			return false;
	}
	return (tag != NULL);
}


//
// Operators
//
vtTagArray &vtTagArray::operator=(const vtTagArray &v)
{
	m_tags = v.m_tags;
	return *this;
}

bool vtTagArray::operator==(const vtTagArray &v) const
{
	unsigned int size = m_tags.size();
	if (size != v.m_tags.size())
		return false;

	for (unsigned int i = 0; i < size; i++)
	{
		const vtTag *tag = GetTag(i);
		const vtTag *tag2 = v.FindTag(tag->name);
		if (!tag2)
			return false;
		if (*tag != *tag2)
			return false;
	}
	return true;
}

bool vtTagArray::operator!=(const vtTagArray &v) const
{
//	return (m_tags != v.m_tags);
	return true;
}

void vtTagArray::CopyTagsFrom(const vtTagArray &v)
{
	for (unsigned int i = 0; i < v.NumTags(); i++)
	{
		const vtTag *tag = v.GetTag(i);
		SetValueString(tag->name, tag->value, true);	// suppress warn
	}
}

// File IO
bool vtTagArray::WriteToXML(const char *fname, const char *title)
{
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	FILE *fp = fopen(fname, "wb");
	if (!fp)
		return false;

	fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	fprintf(fp, "<%s>\n", title);

	unsigned int i, size = NumTags();
	for (i = 0; i < size; i++)
	{
		vtTag *tag = GetTag(i);
		fprintf(fp, "\t<%s>%s</%s>\n", (const char *) tag->name,
			(const char *) EscapeStringForXML(tag->value), (const char *) tag->name);
	}
	WriteOverridesToXML(fp);
	fprintf(fp, "</%s>\n", title);
	fclose(fp);
	return true;
}

////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of Content files.
////////////////////////////////////////////////////////////////////////

class TagVisitor : public XMLVisitor
{
public:
	TagVisitor(vtTagArray *pArray) :
		m_level(0), _hasException(false), m_pArray(pArray) {}
	virtual ~TagVisitor() {}

	void startElement(const char *name, const XMLAttributes &atts);
	void endElement(const char *name);
	void data(const char *s, int length);

	bool hasException() const { return _hasException; }
	xh_io_exception &getException() { return _exception; }
	void setException(const xh_io_exception &exception)
	{
		_exception = exception;
		_hasException = true;
	}

private:
	int m_level;
	string m_data;
	xh_io_exception _exception;
	bool _hasException;

	vtTagArray *m_pArray;
};

void TagVisitor::startElement(const char *name, const XMLAttributes &atts)
{
	m_level++;
	m_data = "";
}

void TagVisitor::endElement(const char *name)
{
	if (m_level == 2)
		m_pArray->SetValueString(name, m_data.c_str());

	m_level--;
}

void TagVisitor::data(const char *s, int length)
{
	m_data.append(string(s, length));
}

bool vtTagArray::LoadFromXML(const char *fname)
{
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	TagVisitor visitor(this);
	try
	{
		readXML(fname, visitor);
	}
	catch (xh_io_exception &ex)
	{
//		throw visitor.getException();
		const string msg = ex.getFormattedMessage();
		VTLOG(" XML problem: %s\n", msg.c_str());
		return false;
	}
	return true;
}


////////////////////////////////////////////////////////////////////////
// Implementation of class vtItem
//

vtItem::vtItem()
{
	m_extents.Empty();
}

vtItem::~vtItem()
{
	// clean up
	Empty();
}

void vtItem::Empty()
{
	for (unsigned int i = 0; i < m_models.GetSize(); i++)
	{
		delete m_models.GetAt(i);
	}
	m_models.Empty();
}

void vtItem::RemoveModel(vtModel *model)
{
	int i = m_models.Find(model);
	m_models.RemoveAt(i);
	delete model;
}

////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of Content files.
////////////////////////////////////////////////////////////////////////

class ContentVisitor : public XMLVisitor
{
public:
	ContentVisitor(vtContentManager *man)
	: _level(0), _hasException(false), m_pMan(man) {}

	virtual ~ContentVisitor() {}

	void startXML();
	void endXML();
	void startElement(const char * name, const XMLAttributes &atts);
	void endElement(const char * name);
	void data(const char * s, int length);

	bool hasException() const { return _hasException; }
	xh_io_exception &getException() { return _exception; }
	void setException(const xh_io_exception &exception)
	{
		_exception = exception;
		_hasException = true;
	}

private:
	struct State
	{
		State () : item(0), type("") {}
		State (vtItem * _item, const char * _type)
			: item(_item), type(_type) {}
		vtItem * item;
		string type;
	};

	State &state () { return _state_stack[_state_stack.size() - 1]; }

	void push_state(vtItem * _item, const char * type)
	{
		if (type == 0)
			_state_stack.push_back(State(_item, "unspecified"));
		else
			_state_stack.push_back(State(_item, type));
		_level++;
		_data = "";
	}

	void pop_state()
	{
		_state_stack.pop_back();
		_level--;
	}

	string _data;
	int _level;
	vector<State> _state_stack;
	xh_io_exception _exception;
	bool _hasException;

	vtContentManager *m_pMan;
};

void ContentVisitor::startXML ()
{
  _level = 0;
  _state_stack.resize(0);
}

void ContentVisitor::endXML ()
{
  _level = 0;
  _state_stack.resize(0);
}


void ContentVisitor::startElement (const char * name, const XMLAttributes &atts)
{
	State &st = state();

	if (_level == 0)
	{
		if (string(name) != (string)"vtp-content") {
			string message = "Root element name is ";
			message += name;
			message += "; expected vtp-content";
			throw xh_io_exception(message, "XML Reader");
		}
		push_state(NULL, "top");
		return;
	}

	const char * attval;

	if (_level == 1)
	{
		if (string(name) == (string)"item")
		{
			vtItem *pItem = m_pMan->NewItem();
			// Get the name.
			attval = atts.getValue("name");
			if (attval != NULL) {
				pItem->m_name = attval;
			}
			push_state(pItem, "item");
		}
		else
		{
			// Unknown field, ignore.
			push_state(NULL, "dummy");
		}
		return;
	}

	if (_level == 2)
	{
		vtItem *pItem = st.item;

		if (string(name) == (string)"model")
		{
			vtModel *pModel = new vtModel();
			pModel->m_filename = atts.getValue("filename");

			attval = atts.getValue("distance");
			if (attval && *attval)
				pModel->m_distance = (float)atof(attval);
			else
				pModel->m_distance = 0.0f;

			attval = atts.getValue("scale");
			if (attval && *attval)
				pModel->m_scale = (float)atof(attval);

			pItem->AddModel(pModel);
		}
		else
			_data = "";
	}
}

void ContentVisitor::endElement(const char * name)
{
	State &st = state();

	if (string(name) == (string)"item")
	{
		if (st.item != NULL)
		{
			m_pMan->AddItem(st.item);
		}
		pop_state();
	}
	if (_level == 2)
	{
		vtItem *pItem = st.item;
		if (string(name) == (string)"model")
		{
		}
		else if (string(name) == (string)"extents")
		{
			FRECT ext;
			int result = sscanf(_data.c_str(), "%f, %f, %f, %f", &ext.left,
				&ext.top, &ext.right, &ext.bottom);
			if (result == 4)
				pItem->m_extents = ext;
		}
		else
		{
			// save all other tags as literal strings
			vtTag tag;
			tag.name = name;
			tag.value = _data.c_str();
			pItem->AddTag(tag);
		}
	}
}

void ContentVisitor::data(const char * s, int length)
{
	if (state().item != NULL)
		_data.append(string(s, length));
}


////////////////////////////////////////////////////////////////////////
// vtContentManager class implementation.
////////////////////////////////////////////////////////////////////////


vtContentManager::~vtContentManager()
{
	// clean up
	Empty();
}

void vtContentManager::Empty()
{
	for (unsigned int i = 0; i < m_items.GetSize(); i++)
	{
		delete m_items.GetAt(i);
	}
	m_items.Empty();
}

void vtContentManager::RemoveItem(vtItem *item)
{
	int i = m_items.Find(item);
	m_items.RemoveAt(i);
	delete item;
}

vtItem *vtContentManager::FindItemByName(const char *name)
{
	for (unsigned int i = 0; i < m_items.GetSize(); i++)
	{
		vtItem *pItem = m_items.GetAt(i);
		if (!pItem->m_name.CompareNoCase(name))
			return pItem;
	}
	return NULL;
}

vtItem *vtContentManager::FindItemByType(const char *type, const char *subtype)
{
	for (unsigned int i = 0; i < m_items.GetSize(); i++)
	{
		vtItem *pItem = m_items.GetAt(i);
		vtTag *tag1 = pItem->FindTag("type");
		if (tag1 && !tag1->value.CompareNoCase(type))
		{
			if (subtype)
			{
				vtTag *tag2 = pItem->FindTag("subtype");
				if (tag2 && !tag2->value.CompareNoCase(subtype))
					return pItem;
			}
			else
				return pItem;
		}
	}
	return NULL;
}

/**
 * Read content items from an XML file.
 *
 * There is no return value because if there is an error, an
 * xh_io_exception will be thrown.
 *
 * \param filename A string containing the file path.
 */
void vtContentManager::ReadXML(const char *filename)
{
	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	m_strFilename = filename;

	ContentVisitor visitor(this);
	readXML(filename, visitor);
	if (visitor.hasException())
		throw visitor.getException();
}


/**
 * Write content items to an XML file.
 *
 * There is no return value because if there is an error, an
 * xh_io_exception will be thrown.
 *
 * \param filename A string containing the file path.
 */
void vtContentManager::WriteXML(const char *filename)
{
	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	unsigned int i, j;
	FILE *fp = fopen(filename, "wb");
	if (!fp)
	{
		throw xh_io_exception("Failed to open file", xh_location(filename),
				"XML Parser");
	}

	fprintf(fp, "<?xml version=\"1.0\"?>\n\n");

	fprintf(fp, "<vtp-content file-format-version=\"1.1\">\n");
	for (i = 0; i < m_items.GetSize(); i++)
	{
		vtItem *pItem = m_items.GetAt(i);

		// Write name
		const char *name = pItem->m_name;
		fprintf(fp, "\t<item name=\"%s\">\n", name);

		// Write tags
		for (j = 0; j < pItem->NumTags(); j++)
		{
			vtTag *tag = pItem->GetTag(j);
			fprintf(fp, "\t\t<%s>%s</%s>\n", (const char *)tag->name,
				(const char *)tag->value, (const char *)tag->name);
		}

		// Write extents
		FRECT ext = pItem->m_extents;
		if (!ext.IsEmpty())
		{
			fprintf(fp, "\t\t<extents>%.2f, %.2f, %.2f, %.2f</extents>\n",
				ext.left, ext.top, ext.right, ext.bottom);
		}

		// Write models
		for (j = 0; j < pItem->NumModels(); j++)
		{
			vtModel *pMod = pItem->GetModel(j);
			const char *filename = pMod->m_filename;
			fprintf(fp, "\t\t<model filename=\"%s\"", filename);
			if (pMod->m_distance != 0.0f)
				fprintf(fp, " distance=\"%g\"", pMod->m_distance);
			if (pMod->m_scale != 1.0f)
				fprintf(fp, " scale=\"%g\"", pMod->m_scale);
			fprintf(fp, " />\n");
		}
		fprintf(fp, "\t</item>\n");
	}
	fprintf(fp, "</vtp-content>\n");
	fclose(fp);
}

