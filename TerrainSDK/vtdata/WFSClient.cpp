//
// WFSClient.cpp
//
// Web Feature Server Client
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// C:\APIs\w3c-libwww-5.4.0\Library\src

#include "Features.h"
#include "config_vtdata.h"
#include "FilePath.h"

// The dependency on Libwww is optional.  If not desired, skip this file.
#if SUPPORT_HTTP

// Includes for libwww (used for HTTP requests)
// first, avoid preprocessor conflicts between GDAL(cpl) and WWWLib
#undef HAVE_VPRINTF
#undef STDC_HEADERS
// avoid preprocessor conflict between MSVC errno.h and WWWlib
#undef EINVAL
#include "WWWLib.h"
#include "WWWInit.h"

#ifdef _MSC_VER
// We will now need to link with several of the Libwww libraries.  This can
// be done automatically with MSVC as follows:
#pragma comment(lib, "wwwapp.lib")
#pragma comment(lib, "wwwcore.lib")
#pragma comment(lib, "wwwinit.lib")
#pragma comment(lib, "wwwutils.lib")
#endif

#define DEFAULT_TIMEOUT	10				  /* timeout in secs */
#define MILLIES			1000

#include "xmlhelper/easyxml.hpp"

class ReqContext
{
public:
	ReqContext();
	~ReqContext();
	char *GetURL(const char *url);

	HTRequest *	request;
	HTAnchor *	anchor;
	char *		cwd;				  /* Current dir URL */

	char *		realm;			/* For automated authentication */
	char *		user;
	char *		password;
};

PRIVATE int printer (const char * fmt, va_list pArgs)
{
	char buf[20000];
	int ret = vsprintf(buf, fmt, pArgs);
	OutputDebugString(buf);
	return ret;
}

PRIVATE int tracer (const char * fmt, va_list pArgs)
{
	char buf[20000];
	int ret = vsprintf(buf, fmt, pArgs);
	OutputDebugString(buf);
	return ret;
}

PRIVATE BOOL PromptUsernameAndPassword (HTRequest * request, HTAlertOpcode op,
					int msgnum, const char * dfault,
					void * input, HTAlertPar * reply)
{
	return NO;
}

PRIVATE int term_handler (HTRequest * request, HTResponse * response,
				   void * param, int status)
{
	/* Check for status */
	HTPrint("Load resulted in status %d\n", status);
	
	/* we're not handling other requests */
	HTEventList_stopLoop ();

	/* stop here */
	return HT_ERROR;
}


//-------------------------------------------------------------------------

ReqContext::ReqContext()
{
	static bFirst = true;
	if (bFirst)
	{
		HTList * converters = HTList_new();		/* List of converters */
		HTList * encodings = HTList_new();		/* List of encoders */
		
		/* Initialize libwww core */
		HTLibInit("vtdata", __DATE__);

		/* Need our own trace and print functions */
		HTPrint_setCallback(printer);
		HTTrace_setCallback(tracer);

		/* On windows we must always set up the eventloop */
#ifdef WIN32
		HTEventInit();
#endif

		/* Register the default set of transport protocols */
		HTTransportInit();

		/* Register the default set of protocol modules */
		HTProtocolInit();

		/* Register the default set of BEFORE and AFTER callback functions */
		HTNetInit();

		/* Register the default set of converters */
		HTConverterInit(converters);
		HTFormat_setConversion(converters);

		/* Register the default set of transfer encoders and decoders */
		HTTransferEncoderInit(encodings);
		HTFormat_setTransferCoding(encodings);

		/* Register the default set of MIME header parsers */
		HTMIMEInit();

		/* Add our own filter to handle termination */
		HTNet_addAfter(term_handler, NULL, NULL, HT_ALL, HT_FILTER_LAST);

		/* Setting event timeout */
		int timer = DEFAULT_TIMEOUT*MILLIES;
		HTHost_setEventTimeout(timer);

		// Don't know what this will do, just trying:
		HTHost_setActiveTimeout(timer);

		bFirst = false;
	}

	cwd = HTGetCurrentDirectoryURL();

	/* Bind the ConLine object together with the Request Object */
	request = HTRequest_new();
	HTRequest_setOutputFormat(request, WWW_SOURCE);

	// Setting preemptive to NO doesn't seem to make a difference
	HTRequest_setPreemptive(request, YES);

	HTRequest_setContext(request, this);
}

ReqContext::~ReqContext()
{
	HTRequest_delete(request);
	HT_FREE(cwd);
}

char *ReqContext::GetURL(const char *url)
{
	HTChunk * chunk = NULL;

	char *absolute_url = HTParse(url, cwd, PARSE_ALL);
	anchor = HTAnchor_findAddress(absolute_url);
	chunk = HTLoadAnchorToChunk(anchor, request);
	HT_FREE(absolute_url);

	/* If chunk != NULL then we have the data */
	if (!chunk)
		return NULL;

	char *string;
	/* wait until the request is over */
	HTEventList_loop(request);
	string = HTChunk_toCString(chunk);
//	HTPrint("%s", string ? string : "no text");
	return string;
}

bool vtFeatures::ReadFeaturesFromWFS(const char *szServerURL, const char *layername)
{
	vtString url = szServerURL;
	url += "GetFeature?typeName=";
	url += layername;

	ReqContext cl;
	char *string = cl.GetURL(url);
	if (!string)
		return false;

	char *temp_fname = "C:/temp/gml_temp.gml";
	FILE *fp = fopen(temp_fname, "wb");
	if (!fp)
		return false;

	fwrite(string, 1, strlen(string), fp);
	fclose(fp);
	HT_FREE(string);

	return LoadWithOGR(temp_fname);
}

////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of WFS Layer List files.
////////////////////////////////////////////////////////////////////////

class LayerListVisitor : public XMLVisitor
{
public:
	LayerListVisitor(WFSLayerArray *pLayers) :
		m_pLayers(pLayers), _level(0) {}

	virtual ~LayerListVisitor () {}

	void startXML ();
	void endXML ();
	void startElement (const char * name, const XMLAttributes &atts);
	void endElement (const char * name);
	void data (const char * s, int length);

private:
	string _data;
	int _level;

	WFSLayerArray *m_pLayers;
	vtTagArray *m_pTags;
};

void LayerListVisitor::startXML ()
{
	_level = 0;
}

void LayerListVisitor::endXML ()
{
	_level = 0;
}

void LayerListVisitor::startElement (const char * name, const XMLAttributes &atts)
{
	if (_level == 0)
	{
		if (string(name) == "FeatureType")
		{
			_level = 1;
			m_pTags = new vtTagArray;
		}
	}
	_data = "";
}

void LayerListVisitor::endElement(const char * name)
{
	if (_level == 1)
	{
		if (string(name) == "FeatureType")
		{
			m_pLayers->Append(m_pTags);
			_level = 0;
			return;
		}
		const char *value = _data.c_str();
		m_pTags->AddTag(name, value);
	}
}

void LayerListVisitor::data(const char *s, int length)
{
	if (_level == 1)
		_data.append(string(s, length));
}

//
//
//
bool GetLayersFromWFS(const char *szServerURL, WFSLayerArray &layers)
{
	vtString url = szServerURL;
	url += "GetCapabilities?version=0.0.14";

	ReqContext cl;
	char *string = cl.GetURL(url);
	if (!string)
		return false;

	char *temp_fname = "C:/temp/layers_temp.xml";
	FILE *fp = fopen(temp_fname, "wb");
	if (!fp)
		return false;
	fwrite(string, 1, strlen(string), fp);
	HT_FREE(string);
	fclose(fp);

	LayerListVisitor visitor(&layers);
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
