//
// Content.cpp
//
// Implements the ContentManager class, including the ability to read
// and write the contents to an XML file.
//
// Copyright (c) 2001 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <stdio.h>
#include "xmlhelper/easyxml.hpp"
#include "Content.h"


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

vtTag *vtTagArray::GetTag(int index)
{
	return &m_tags[index];
}

unsigned int vtTagArray::NumTags()
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

void vtTagArray::SetValue(const char *name, const char *szValue)
{
	vtTag *tag = FindTag(name);
	if (tag)
		tag->value = szValue;
	else
		AddTag(name, szValue);
}

void vtTagArray::SetValue(const char *name, int iValue)
{
	vtString str;
	str.Format("%d", iValue);
	SetValue(name, str);
}

void vtTagArray::SetValue(const char *name, double dValue)
{
	vtString str;
	str.Format("%lf", dValue);
	SetValue(name, str);
}

const char *vtTagArray::GetValue(const char *name)
{
	vtTag *tag = FindTag(name);
	if (tag)
		return tag->value;
	else
		return NULL;
}

bool vtTagArray::GetValue(const char *name, vtString &string)
{
	vtTag *tag = FindTag(name);
	if (tag)
		string = tag->value;
	return (tag != NULL);
}

bool vtTagArray::GetValue(const char *name, int &value)
{
	vtTag *tag = FindTag(name);
	if (tag)
		value = atoi((const char *)tag->value);
	return (tag != NULL);
}

bool vtTagArray::GetValue(const char *name, double &value)
{
	vtTag *tag = FindTag(name);
	if (tag)
		value = atof((const char *)tag->value);
	return (tag != NULL);
}

////////////////////////////////////////////////////////////////////////
// Implementation of class vtItem
//

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
	string _base;
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
		if (string(name) != (string)"model")
		{
			// save all other tags as literal strings
			vtTag tag;
			tag.name = name;
			tag.value = _data.c_str();

			vtItem *pItem = st.item;
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
		const char *name = pItem->m_name;
		fprintf(fp, "\t<item name=\"%s\">\n", name);
#if 0
		if (pItem->m_type != "")
		{
			const char *type = pItem->m_type;
			fprintf(fp, "\t\t<classification type=\"%s\"", type);
			if (pItem->m_subtype != "")
			{
				const char *subtype = pItem->m_subtype;
				fprintf(fp, " subtype=\"%s\"", subtype);
			}
			fprintf(fp, " />\n");
		}
		if (pItem->m_url != "")
		{
			const char *url = pItem->m_url;
			fprintf(fp, "\t\t<link utl=\"%s\" />\n", url);
		}
#endif
		for (j = 0; j < pItem->NumTags(); j++)
		{
			vtTag *tag = pItem->GetTag(j);
			fprintf(fp, "\t\t<%s>%s</%s>\n", (const char *)tag->name,
				(const char *)tag->value, (const char *)tag->name);
		}
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

