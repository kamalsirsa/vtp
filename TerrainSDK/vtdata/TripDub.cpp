//
// TripDub.cpp
//
// Module for Double-You-Double-You-Double-You support (WWW)
// Ref: http://web.yost.com/Misc/nix-on-www.html
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "config_vtdata.h"

// The dependency on Libwww is optional.  If not desired, skip this file.
#if SUPPORT_HTTP

#include "TripDub.h"
#include "vtdata/vtLog.h"

//
// Includes for libwww (used for HTTP requests)
//

// first, avoid preprocessor conflicts between GDAL(cpl) and WWWLib
#undef HAVE_VPRINTF
#undef STDC_HEADERS
// avoid preprocessor conflict between MSVC errno.h and WWWlib
#undef EINVAL

#ifdef UNIX
#include "w3c-libwww/WWWLib.h"
#include "w3c-libwww/WWWInit.h"
#else
#include "WWWLib.h"
#include "WWWInit.h"
#endif

#ifdef _MSC_VER
// We will now need to link with several of the Libwww libraries.  This can
// be done automatically with MSVC as follows:
#pragma comment(lib, "wwwapp.lib")
#pragma comment(lib, "wwwcore.lib")
#pragma comment(lib, "wwwinit.lib")
#pragma comment(lib, "wwwutils.lib")
#pragma comment(lib, "wwwhttp.lib")
#endif

#define DEFAULT_TIMEOUT	90		// timeout in secs
#define MILLIES			1000

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

PRIVATE int term_handler (HTRequest * request, HTResponse * response,
				   void * param, int status)
{
	ReqContext *context = (ReqContext *) HTRequest_context(request);

	// Check for status
	if (context->m_iVerbosity > 0)
		VTLOG("status %d\n", status);

	s_last_status = status;
	new_anchor = HTResponse_redirection(response);

	// we're not handling other requests
	HTEventList_stopLoop ();

	// stop here
	return HT_ERROR;
}


PRIVATE BOOL setCookie(HTRequest * request, HTCookie * cookie, void * param)
{
	ReqContext *context = (ReqContext *) HTRequest_context(request);

	if (!cookie)
		return YES;

	if (context->m_iVerbosity > 1)
	{
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
	}

	// Work around bug in libwww cookie parsing: when it gets a line like this:
	//		Set-Cookie: foo=;Expires=Thu. 01-Jan-1970...
	// rather than give a blank string for foo, it gives "Expires"
	if (!strcmp(HTCookie_value(cookie), "Expires"))
		return YES;

	// add it to our own list
	AddCookie(HTCookie_name(cookie), HTCookie_value(cookie));

	return YES;
}

PRIVATE HTAssocList *findCookie(HTRequest * request, void * param)
{
	ReqContext *context = (ReqContext *) HTRequest_context(request);

	if (context->m_iVerbosity > 1)
		VTLOG(" findCookie %s:\n", param);

	HTAssocList * alist = HTAssocList_new(); // is deleted by the cookie module
	unsigned int i;
	for (i = 0; i < g_cookies.size(); i++)
	{
		HTAssocList_addObject(alist, g_cookies[i].name, g_cookies[i].value);
		if (context->m_iVerbosity > 1)
			VTLOG("   adding (%d) %s=%s\n", i,
				(const char *)g_cookies[i].name,
				(const char *)g_cookies[i].value);
	}
	return alist;
}


//---------------------------------------------------------------------------
// ReqContext class
//

bool ReqContext::s_bFirst = true;

ReqContext::ReqContext()
{
	m_iVerbosity = 1;

	if (s_bFirst)
		InitializeLibrary();

	m_cwd = HTGetCurrentDirectoryURL();

	// Bind the ConLine object together with the Request Object
	m_request = HTRequest_new();
	HTRequest_setOutputFormat(m_request, WWW_SOURCE);

	// Setting preemptive to NO doesn't seem to make a difference
	HTRequest_setPreemptive(m_request, YES);

	// Set a backpointer so we can find ourselves later
	HTRequest_setContext(m_request, this);
}

ReqContext::~ReqContext()
{
	HTRequest_delete(m_request);
	HT_FREE(m_cwd);
}


void ReqContext::InitializeLibrary()
{
	// This one call does most of the work
	HTProfile_newPreemptiveRobot("vtdata", __DATE__);
//	HTProfile_newNoCacheClient("vtdata", __DATE__);

	// Need our own trace and print functions
	HTPrint_setCallback(printer);
	HTTrace_setCallback(tracer);

	HTAlertInit();
	HTAlert_setInteractive(NO);

#ifdef WIN32
	// This is important!  Things don't work right on Windows without it.
	HTEventInit();
#endif

	// Add our own filter to handle termination
	HTNet_addAfter(term_handler, NULL, NULL, HT_ALL, HT_FILTER_LAST);

	// Setup cookies
	HTCookie_init();
	HTCookie_setCallbacks(setCookie, NULL, findCookie, NULL);

	// Don't prompt for cookies - just accept them!
	//	HTCookieMode mode = HTCookie_cookieMode();
	HTCookie_setCookieMode((HTCookieMode) (HT_COOKIE_ACCEPT | HT_COOKIE_SEND));

	// Set event timeout
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

bool ReqContext::DoQuery(vtBytes &data, int redirects)
{
	const char *address;

	address = HTAnchor_address(m_anchor);
	if (m_iVerbosity > 0)
		VTLOG("  Opening: '%s' ... ", address);

	HTChunk *chunk = HTLoadAnchorToChunk(m_anchor, m_request);

	// chunk had better not be NULL, that's where the data will go
	if (!chunk)
		return false;

	// wait until the request is over
	HTEventList_loop(m_request);

	// check status - redirect?
	if (s_last_status == HT_FOUND)	// also known as 302 Redirect
	{
		address = HTAnchor_address(new_anchor);
		if (m_iVerbosity == 2)
			VTLOG("  Redirected To: %s\n", address);
		if (m_iVerbosity == 1)
			VTLOG("  (Redirected)");

		m_anchor = new_anchor;

		// Delete any auth credendials as they get regenerated
		HTRequest_deleteCredentialsAll(m_request);
		HTRequest_deleteExtraHeaderAll(m_request);

		return DoQuery(data, redirects + 1);
	}
	if (s_last_status == HT_TIMEOUT)
	{
		// too long, give up
		if (m_iVerbosity > 0)
			VTLOG("  Timeout: more than %d seconds\n", DEFAULT_TIMEOUT);
		return false;
	}

	char *dataptr = HTChunk_data(chunk);
	int len = HTChunk_size(chunk);

	data.Put((unsigned char *)dataptr, len);
	return true;
}

bool ReqContext::GetURL(const char *url, vtString &str)
{
	char *absolute_url = HTParse(url, m_cwd, PARSE_ALL);
	if (!absolute_url)
		return false;

	m_anchor = HTAnchor_findAddress(absolute_url);
	HT_FREE(absolute_url);

	vtBytes bytes;
	if (DoQuery(bytes, 0))
	{
		str = bytes.Get();
		return true;
	}
	else
		return false;
}

bool ReqContext::GetURL(const char *url, vtBytes &data)
{
	char *absolute_url = HTParse(url, m_cwd, PARSE_ALL);
	if (!absolute_url)
		return false;

	m_anchor = HTAnchor_findAddress(absolute_url);
	HT_FREE(absolute_url);

	return DoQuery(data, 0);
}

#endif	// SUPPORT_HTTP

