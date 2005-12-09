// easyxml.cpp - implementation of EasyXML interfaces.

#pragma warning( disable : 4786 )

#include <string.h>		// strcmp()
#include <fstream>
using namespace std;

#include "easyxml.hpp"
#include "xmlparse.h"


////////////////////////////////////////////////////////////////////////
// Implementation of XMLAttributes.
////////////////////////////////////////////////////////////////////////

XMLAttributes::XMLAttributes ()
{
}

XMLAttributes::~XMLAttributes ()
{
}

int XMLAttributes::findAttribute (const char * name) const
{
	int s = size();
	for (int i = 0; i < s; i++) {
		if (strcmp(name, getName(i)) == 0)
			return i;
	}
	return -1;
}

bool XMLAttributes::hasAttribute (const char * name) const
{
	return (findAttribute(name) != -1);
}

const char *XMLAttributes::getValue (const char * name) const
{
	int pos = findAttribute(name);
	if (pos >= 0)
		return getValue(pos);
	else
		return 0;
}

////////////////////////////////////////////////////////////////////////
// Implementation of XMLAttributesDefault.
////////////////////////////////////////////////////////////////////////

XMLAttributesDefault::XMLAttributesDefault ()
{
}

XMLAttributesDefault::XMLAttributesDefault (const XMLAttributes &atts)
{
	int s = atts.size();
	for (int i = 0; i < s; i++)
		addAttribute(atts.getName(i), atts.getValue(i));
}

XMLAttributesDefault::~XMLAttributesDefault ()
{
}

int XMLAttributesDefault::size () const
{
	return _atts.size() / 2;
}

const char *XMLAttributesDefault::getName (int i) const
{
	return _atts[i*2].c_str();
}

const char *XMLAttributesDefault::getValue (int i) const
{
	return _atts[i*2+1].c_str();
}

void XMLAttributesDefault::addAttribute (const char * name, const char * value)
{
	_atts.push_back(name);
	_atts.push_back(value);
}

void XMLAttributesDefault::setName (int i, const char * name)
{
	_atts[i*2] = name;
}

void XMLAttributesDefault::setValue (int i, const char * name)
{
	_atts[i*2+1] = name;
}

void XMLAttributesDefault::setValue (const char * name, const char * value)
{
	int pos = findAttribute(name);
	if (pos >= 0) {
		setName(pos, name);
		setValue(pos, value);
	} else {
		addAttribute(name, value);
	}
}


////////////////////////////////////////////////////////////////////////
// Attribute list wrapper for Expat.
////////////////////////////////////////////////////////////////////////

class ExpatAtts : public XMLAttributes
{
public:
	ExpatAtts (const char ** atts) : _atts(atts) {}

	virtual int size () const;
	virtual const char * getName (int i) const;
	virtual const char * getValue (int i) const;

private:
	const char ** _atts;
};

int ExpatAtts::size () const
{
	int s = 0;
	for (int i = 0; _atts[i] != 0; i += 2)
		s++;
	return s;
}

const char *ExpatAtts::getName (int i) const
{
	return _atts[i*2];
}

const char *ExpatAtts::getValue (int i) const
{
	return _atts[i*2+1];
}


////////////////////////////////////////////////////////////////////////
// Static callback functions for Expat.
////////////////////////////////////////////////////////////////////////

#define VISITOR (*((XMLVisitor *)userData))

static void start_element (void * userData, const char * name, const char ** atts)
{
	VISITOR.startElement(name, ExpatAtts(atts));
}

static void end_element (void * userData, const char * name)
{
	VISITOR.endElement(name);
}

static void character_data (void * userData, const char * s, int len)
{
	VISITOR.data(s, len);
}

static void processing_instruction (void * userData,
			const char * target,
			const char * data)
{
	VISITOR.pi(target, data);
}

#undef VISITOR


////////////////////////////////////////////////////////////////////////
// Implementation of XMLReader.
////////////////////////////////////////////////////////////////////////

void readXML (istream &input, XMLVisitor &visitor, const string &path,
			  bool progress_callback(int))
{
	XML_Parser parser = XML_ParserCreate(0);
	XML_SetUserData(parser, &visitor);
	XML_SetElementHandler(parser, start_element, end_element);
	XML_SetCharacterDataHandler(parser, character_data);
	XML_SetProcessingInstructionHandler(parser, processing_instruction);

	visitor.startXML();

	int progress = 0;
	char buf[16384];
	while (!input.eof())
	{
		// FIXME: get proper error string from system
		if (!input.good())
		{
			XML_ParserFree(parser);
			throw xh_io_exception("Problem reading file",
					xh_location(path,
					XML_GetCurrentLineNumber(parser),
					XML_GetCurrentColumnNumber(parser)),
					"XML Parser");
		}

		input.read(buf,16384);
		if (!XML_Parse(parser, buf, input.gcount(), false))
		{
			const XML_LChar *message = XML_ErrorString(XML_GetErrorCode(parser));
			int line = XML_GetCurrentLineNumber(parser);
			int col = XML_GetCurrentColumnNumber(parser);
			XML_ParserFree(parser);
			throw xh_io_exception(message,
					xh_location(path, line, col),
					"XML Parser");
		}
		if (progress_callback != NULL)
		{
			progress++;
			if (progress == 400)
				progress = 0;
			progress_callback(progress/4);
		}
	}

	// Verify end of document.
	if (!XML_Parse(parser, buf, 0, true)) {
		XML_ParserFree(parser);
		throw xh_io_exception(XML_ErrorString(XML_GetErrorCode(parser)),
				xh_location(path,
							XML_GetCurrentLineNumber(parser),
							XML_GetCurrentColumnNumber(parser)),
				"XML Parser");
	}

	XML_ParserFree(parser);
}

void readXML (const string &path, XMLVisitor &visitor,
			  bool progress_callback(int))
{
	gzFile fp = gzopen(path.c_str(), "rb");

	if (!fp)
		throw xh_io_exception("Failed to open file", xh_location(path),
					"XML Parser");
	try
	{
		readCompressedXML(fp, visitor, path, progress_callback);
	}
	catch (xh_io_exception &e)
	{
			gzclose(fp);
			throw e;
	}
	catch (xh_throwable &t)
	{
			gzclose(fp);
			throw t;
	}
	// If it gets here, it succeeded
	gzclose(fp);
}

void readCompressedXML (gzFile fp, XMLVisitor &visitor, const string& path,
						bool progress_callback(int))
{
	XML_Parser parser = XML_ParserCreate(0);
	XML_SetUserData(parser, &visitor);
	XML_SetElementHandler(parser, start_element, end_element);
	XML_SetCharacterDataHandler(parser, character_data);
	XML_SetProcessingInstructionHandler(parser, processing_instruction);

	visitor.startXML();

	int progress = 0;
	char buf[16384];
	while (!gzeof(fp))
	{
		int iCount = gzread(fp, buf, 16384);
		if (iCount > 0)
		{
			if (!XML_Parse(parser, buf, iCount, false))
			{
				const XML_LChar *message = XML_ErrorString(XML_GetErrorCode(parser));
				int line = XML_GetCurrentLineNumber(parser);
				int col = XML_GetCurrentColumnNumber(parser);
				XML_ParserFree(parser);
				throw xh_io_exception(message,
						xh_location(path, line, col),
						"XML Parser");
			}
			if (progress_callback != NULL)
			{
				progress++;
				if (progress == 400)
					progress = 0;
				progress_callback(progress/4);
			}
		}
		else if (iCount < 0)
		{
			XML_ParserFree(parser);
			throw xh_io_exception("Problem reading file",
					xh_location(path,
					XML_GetCurrentLineNumber(parser),
					XML_GetCurrentColumnNumber(parser)),
					"XML Parser");
		}
	}

	// Verify end of document.
	if (!XML_Parse(parser, buf, 0, true))
	{
		XML_Error errcode = XML_GetErrorCode(parser);
		const XML_LChar *errstr = XML_ErrorString(errcode);
		int line = XML_GetCurrentLineNumber(parser);
		int column = XML_GetCurrentColumnNumber(parser);

		XML_ParserFree(parser);

		throw xh_io_exception(errstr,
				xh_location(path,
							line,
							column),
				"XML Parser");
	}

	XML_ParserFree(parser);
}

