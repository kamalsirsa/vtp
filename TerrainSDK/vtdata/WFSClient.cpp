//
// WFSClient.cpp
//
// Web Feature Server Client
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "config_vtdata.h"
#include "Features.h"

// The dependency on Libwww is optional.  If not desired, skip this file.
#if SUPPORT_HTTP

#include "FilePath.h"
#include "vtLog.h"
#include "TripDub.h"
#include "WFSClient.h"
#include "xmlhelper/easyxml.hpp"


/////////////////////////////////////////////////////////////////////////////
// vtFeatures methods
//

vtFeatureSet *vtFeatureLoader::ReadFeaturesFromWFS(const char *szServerURL, const char *layername)
{
	vtString url = szServerURL;
	url += "GetFeature?typeName=";
	url += layername;

	vtString str;
	ReqContext cl;
	cl.GetURL(url, str);
	if (str == "")
		return false;

	char *temp_fname = "C:/temp/gml_temp.gml";
	FILE *fp = fopen(temp_fname, "wb");
	if (!fp)
		return false;

	fwrite((const char *)str, 1, str.GetLength(), fp);
	fclose(fp);

	return LoadWithOGR(temp_fname);
}


////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of WFS Layer List files.
//

class LayerListVisitor : public XMLVisitor
{
public:
	LayerListVisitor(WFSLayerArray &pLayers) : m_Layers(pLayers)
	{
	}

	virtual ~LayerListVisitor () {}

	void startXML() { _level = 0; m_pCurrent = NULL; }
	void endXML() { _level = 0; }
	void startElement (const char * name, const XMLAttributes &atts);
	void endElement (const char * name);
	void data (const char * s, int length);

private:
	string _data;
	int _level;

	WFSLayerArray &m_Layers;
	WFSLayerArray m_Stack;
	vtTagArray *m_pCurrent;
};

void LayerListVisitor::startElement (const char * name, const XMLAttributes &atts)
{
	if (string(name) == "Layer")
	{
		_level++;
		m_pCurrent = new vtTagArray;
		m_Layers.push_back(m_pCurrent);
		m_Stack.push_back(m_pCurrent);
	}
	_data = "";
}

void LayerListVisitor::endElement(const char *name)
{
	if (string(name) == "Layer")
	{
		_level--;
		m_Stack.pop_back();
		int num = m_Stack.size();
		if (num)
			m_pCurrent = m_Stack[num-1];
		else
			m_pCurrent = NULL;
	}
	else if (m_pCurrent != NULL)
	{
		const char *value = _data.c_str();
		m_pCurrent->AddTag(name, value);
	}
}

void LayerListVisitor::data(const char *s, int length)
{
	_data.append(string(s, length));
}

//
//
//
bool GetLayersFromWFS(const char *szServerURL, WFSLayerArray &layers)
{
	vtString url = szServerURL;
	url += "GetCapabilities?version=0.0.14";

	vtString str;
	ReqContext cl;
	cl.GetURL(url, str);
	if (str == "")
		return false;

	char *temp_fname = "C:/temp/layers_temp.xml";
	FILE *fp = fopen(temp_fname, "wb");
	if (!fp)
		return false;
	fwrite((const char *)str, 1, str.GetLength(), fp);
	fclose(fp);

	LayerListVisitor visitor(layers);
	try
	{
		readXML(temp_fname, visitor);
	}
	catch (xh_exception &)
	{
		// TODO: would be good to pass back the error message.
		return false;
	}
	return true;
}


//
// for now, handle WMS in this module as well
//
bool GetLayersFromWMS(const char *szServerURL, WFSLayerArray &layers)
{
//	char *temp_fname = "C:/temp/elsalvador.xml";

	vtString url = szServerURL;
	url += "?REQUEST=GetCapabilities&version=1.1.1&SERVICE=WMS";

	ReqContext cl;
	vtString str;
	cl.GetURL(url, str);
	if (str == "")
		return false;

	char *temp_fname = "C:/temp/layers_temp.xml";
	FILE *fp = fopen(temp_fname, "wb");
	if (!fp)
		return false;
	fwrite((const char *)str, 1, str.GetLength(), fp);
	fclose(fp);

	LayerListVisitor visitor(layers);
	try
	{
		readXML(temp_fname, visitor);
	}
	catch (xh_exception &)
	{
		// TODO: would be good to pass back the error message.
		return false;
	}
	return true;
}

#else
// no support for HTTP
bool vtFeatures::ReadFeaturesFromWFS(const char *, const char *)
{
	return false;
}
#endif	// SUPPORT_HTTP
