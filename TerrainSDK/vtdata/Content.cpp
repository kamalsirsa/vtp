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


//////////////////////////////////////////////////////////////////////////
// Classification
// (not yet used - we simply accept all strings rather than check)
//

const char *ItemTypeNames[NUM_ITEMTYPES] =
{
	"unspecified",
	"structure",
	"transportation",
	"vehicle"
};

const char *SubTypeNames[NUM_SUBTYPES] =
{
	"unspecified",

	// structure subtypes
	"observatory",
	"lighthouse",
	"commercial",
	"residence",
	"tent",

	// transportation subtypes
	"stopsign",
	"yieldsign",
	"signalpole",

	// vehicle subtypes
	"car",
	"pickup",
	"truck",
	"suv",
	"bus",
	"aircraft"
};


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
	for (int i = 0; i < m_models.GetSize(); i++)
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
	: m_pMan(man), _level(0), _hasException(false) {}

	virtual ~ContentVisitor () {}

	void startXML ();
	void endXML ();
	void startElement (const char * name, const XMLAttributes &atts);
	void endElement (const char * name);
	void data (const char * s, int length);
	void warning (const char * message, int line, int column);

	bool hasException () const { return _hasException; }
	xh_io_exception &getException () { return _exception; }
	void setException (const xh_io_exception &exception)
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

	void push_state (vtItem * _item, const char * type)
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

	vtContentManager *m_pMan;
};

void
ContentVisitor::startXML ()
{
  _level = 0;
  _state_stack.resize(0);
}

void
ContentVisitor::endXML ()
{
  _level = 0;
  _state_stack.resize(0);
}


void
ContentVisitor::startElement (const char * name, const XMLAttributes &atts)
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
		vtItem *pItem;
		if (string(name) == (string)"item")
		{
			pItem = new vtItem();
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
			pItem = NULL;
			push_state(pItem, "dummy");
		}
		return;
	}

	if (_level == 2)
	{
		vtItem *pItem = st.item;

		if (string(name) == (string)"classification")
		{
			pItem->m_type = atts.getValue("type");
			pItem->m_subtype = atts.getValue("subtype");
		}
		if (string(name) == (string)"link")
		{
			pItem->m_url = atts.getValue("url");
		}
		if (string(name) == (string)"model")
		{
			vtModel *pModel = new vtModel();
			pModel->m_filename = atts.getValue("filename");
			attval = atts.getValue("distance");
			if (attval && *attval)
			{
				pModel->m_distance = atof(attval);
			}
			else
				pModel->m_distance = 0.0f;
			pItem->AddModel(pModel);
		}
	}
}

void
ContentVisitor::endElement (const char * name)
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
}

void
ContentVisitor::data (const char * s, int length)
{
	if (state().item != NULL)
		_data.append(string(s, length));
}

void
ContentVisitor::warning (const char * message, int line, int column)
{
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
	for (int i = 0; i < m_items.GetSize(); i++)
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
	for (int i = 0; i < m_items.GetSize(); i++)
	{
		vtItem *pItem = m_items.GetAt(i);
		if (!pItem->m_name.CompareNoCase(name))
			return pItem;
	}
	return NULL;
}

vtItem *vtContentManager::FindItemByType(const char *type, const char *subtype)
{
	for (int i = 0; i < m_items.GetSize(); i++)
	{
		vtItem *pItem = m_items.GetAt(i);
		if (!pItem->m_type.CompareNoCase(type) &&
			(subtype == NULL || !pItem->m_subtype.CompareNoCase(subtype)))
			return pItem;
	}
	return NULL;
}

/**
 * Read content items from an XML file.
 *
 * @param file A string containing the file path.
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
 * @param file A string containing the file path.
 */
void vtContentManager::WriteXML(const char *filename)
{
	int i, j;
	FILE *fp = fopen(filename, "wb");
	if (!fp)
	{
		throw xh_io_exception("Failed to open file", xh_location(filename),
				"XML Parser");
	}

	fprintf(fp, "<?xml version=\"1.0\"?>\n\n");

	fprintf(fp, "<vtp-content file-format-version=\"1.0\">\n");
	for (i = 0; i < m_items.GetSize(); i++)
	{
		vtItem *pItem = m_items.GetAt(i);
		const char *name = pItem->m_name;
		fprintf(fp, "\t<item name=\"%s\">\n", name);
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
		for (j = 0; j < pItem->NumModels(); j++)
		{
			vtModel *pMod = pItem->GetModel(j);
			const char *filename = pMod->m_filename;
			fprintf(fp, "\t\t<model filename=\"%s\"", filename);
			if (pMod->m_distance != 0.0f)
				fprintf(fp, " distance=\"%g\"", pMod->m_distance);
			fprintf(fp, " />\n");
		}
		fprintf(fp, "\t</item>\n");
	}
	fprintf(fp, "</vtp-content>\n");
	fclose(fp);
}

