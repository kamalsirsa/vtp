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
#include "vtLog.h"
#include "WFSClient.h"

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
#pragma comment(lib, "wwwhttp.lib")
#endif

#define DEFAULT_TIMEOUT	90				  /* timeout in secs */
#define MILLIES			1000

#include "xmlhelper/easyxml.hpp"

struct MyCookie
{
	vtString name;
	vtString value;
};
std::vector<MyCookie> g_cookies;
static int s_last_status = 0;
static HTAnchor *new_anchor = NULL;

void AddCookie(const char *name, const char *value)
{
	unsigned int i, size = g_cookies.size();
	for (i = 0; i < size; i++)
	{
		if (g_cookies[i].name == name)
		{
			g_cookies[i].value = value;
			return;
		}
	}
	// otherwise we need to create a new one
	MyCookie mc;
	mc.name = name;
	mc.value = value;
	g_cookies.push_back(mc);
}



#if 1
PRIVATE int printer (const char * fmt, va_list pArgs)
{
	char buf[20000];
	int ret = vsprintf(buf, fmt, pArgs);
	g_Log._Log(buf);
	return ret;
}

PRIVATE int tracer (const char * fmt, va_list pArgs)
{
	char buf[20000];
	int ret = vsprintf(buf, fmt, pArgs);
	g_Log._Log(buf);
	return ret;
}
#endif

PRIVATE int term_handler (HTRequest * request, HTResponse * response,
				   void * param, int status)
{
	/* Check for status */
	VTLOG("status %d\n", status);

	s_last_status = status;
    new_anchor = HTResponse_redirection(response);

	/* we're not handling other requests */
	HTEventList_stopLoop ();

	/* stop here */
	return HT_ERROR;
}


PRIVATE BOOL setCookie (HTRequest * request, HTCookie * cookie, void * param)
{
    if (cookie)
	{
#if 0
		char * addr = HTAnchor_address((HTAnchor *) HTRequest_anchor(request));
		VTLOG("While accessing `%s\', we received a cookie with parameters:\n", addr);
		if (HTCookie_name(cookie))
			VTLOG("\tName   : `%s\'\n", HTCookie_name(cookie));
		if (HTCookie_value(cookie))
			VTLOG("\tValue  : `%s\'\n", HTCookie_value(cookie));
		if (HTCookie_domain(cookie))
			VTLOG("\tDomain : `%s\'\n", HTCookie_domain(cookie));
		if (HTCookie_path(cookie))
			VTLOG("\tPath   : `%s\'\n", HTCookie_path(cookie));
		if (HTCookie_expiration(cookie) > 0) {
			time_t t = HTCookie_expiration(cookie);
			VTLOG("\tExpires: `%s\'\n", HTDateTimeStr(&t, NO));
		}
		VTLOG("\tCookie is %ssecure\n\n", HTCookie_isSecure(cookie) ? "" : "not ");
		HT_FREE(addr);
#endif

		// Work around bug in libwww cookie parsing: when it gets a line like this:
		//		Set-Cookie: foo=;Expires=Thu. 01-Jan-1970...
		// rather than give a blank string for the value it gives "Expires"
		if (!strcmp(HTCookie_value(cookie), "Expires"))
			return YES;

		// add it to our own list
		AddCookie(HTCookie_name(cookie), HTCookie_value(cookie));
    }

    return YES;
}

PRIVATE HTAssocList *findCookie(HTRequest * request, void * param)
{
//	VTLOG(" findCookie %s:\n", param);

    HTAssocList * alist = HTAssocList_new();	/* Is deleted by the cookie module */
	unsigned int i;
	for (i = 0; i < g_cookies.size(); i++)
	{
	    HTAssocList_addObject(alist, g_cookies[i].name, g_cookies[i].value);
//		VTLOG("   adding (%d) %s=%s\n", i,
//			(const char *)g_cookies[i].name,
//			(const char *)g_cookies[i].value);
	}
    return alist;
}



//-------------------------------------------------------------------------
// ReqContext class
//


bool ReqContext::s_bFirst = true;

ReqContext::ReqContext()
{
	if (s_bFirst)
		InitializeLibrary();

	m_cwd = HTGetCurrentDirectoryURL();

	/* Bind the ConLine object together with the Request Object */
	m_request = HTRequest_new();
	HTRequest_setOutputFormat(m_request, WWW_SOURCE);

	// Setting preemptive to NO doesn't seem to make a difference
	HTRequest_setPreemptive(m_request, YES);

	HTRequest_setContext(m_request, this);
}

ReqContext::~ReqContext()
{
	HTRequest_delete(m_request);
	HT_FREE(m_cwd);
}


void ReqContext::InitializeLibrary()
{
	// This one call does most of the work?
	HTProfile_newPreemptiveRobot("vtdata", __DATE__);
//	HTProfile_newNoCacheClient("vtdata", "1.0");

	/* Need our own trace and print functions */
	HTPrint_setCallback(printer);
	HTTrace_setCallback(tracer);

    HTAlertInit();
    HTAlert_setInteractive(NO);

#ifdef WIN32
	HTEventInit();
#endif

	/* Add our own filter to handle termination */
	HTNet_addAfter(term_handler, NULL, NULL, HT_ALL, HT_FILTER_LAST);

    /* Setup cookies */
    HTCookie_init();
    HTCookie_setCallbacks(setCookie, NULL, findCookie, NULL);

	// Don't prompt for cookies - just accept them!
	//	HTCookieMode mode = HTCookie_cookieMode();
	HTCookie_setCookieMode((HTCookieMode) (HT_COOKIE_ACCEPT | HT_COOKIE_SEND));

	/* Setting event timeout */
	int timer = DEFAULT_TIMEOUT*MILLIES;
	HTHost_setEventTimeout(timer);

	// Don't know what this will do, just trying:
	HTHost_setActiveTimeout(timer);

	s_bFirst = false;
}

void ReqContext::AddHeader(const char *token, const char *value)
{
	HTRequest_addExtraHeader(m_request, (char *) token, (char *) value);
}

void ReqContext::DoQuery(vtString &str, int redirects)
{
	const char *address;

	address = HTAnchor_address(m_anchor);
	VTLOG("  Opening: '%s' ... ", address);

	HTChunk *chunk = HTLoadAnchorToChunk(m_anchor, m_request);

	/* chunk had better not be NULL, that's where the data will go */
	if (!chunk)
		return;

	char *string;
	/* wait until the request is over */
	HTEventList_loop(m_request);

	// check status - redirect?
	if (s_last_status == HT_FOUND)	// also known as 302 Redirect
	{
		address = HTAnchor_address(new_anchor);
//		VTLOG("  Redirected To: %s\n", address);
		VTLOG("  (Redirected)");

		m_anchor = new_anchor;

		/* Delete any auth credendials as they get regenerated */
		HTRequest_deleteCredentialsAll(m_request);
		HTRequest_deleteExtraHeaderAll(m_request);

		DoQuery(str, redirects + 1);
		return;
	}
	if (s_last_status == HT_TIMEOUT)
	{
		// too long, give up
		VTLOG("  Timeout: more than %d seconds\n", DEFAULT_TIMEOUT);
		return;
	}

	string = HTChunk_toCString(chunk);
//	if (string)
//		g_Log._Log(string);
	str = string;
	HT_FREE(string);
}

void ReqContext::GetURL(const char *url, vtString &str)
{
	str = "";
	char *absolute_url = HTParse(url, m_cwd, PARSE_ALL);
	if (!absolute_url)
		return;

	m_anchor = HTAnchor_findAddress(absolute_url);
	HT_FREE(absolute_url);

	DoQuery(str, 0);
}


/////////////////////////////////////////////////////////////////////////////
// vtFeatures methods
//

bool vtFeatures::ReadFeaturesFromWFS(const char *szServerURL, const char *layername)
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
	LayerListVisitor(WFSLayerArray *pLayers) :
		m_pLayers(pLayers), _level(0) {}

	virtual ~LayerListVisitor () {}

	void startXML() { _level = 0; }
	void endXML() { _level = 0; }
	void startElement (const char * name, const XMLAttributes &atts);
	void endElement (const char * name);
	void data (const char * s, int length);

private:
	string _data;
	int _level;

	WFSLayerArray *m_pLayers;
	vtTagArray *m_pTags;
};

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
