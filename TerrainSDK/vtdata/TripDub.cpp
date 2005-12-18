//
// TripDub.cpp
//
// Module for Double-You-Double-You-Double-You support (WWW)
// Ref: http://web.yost.com/Misc/nix-on-www.html
//
// Copyright (c) 2002-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "config_vtdata.h"

// The dependency on LibCURL is optional.  If not desired, skip this file.
#if SUPPORT_HTTP

#include "TripDub.h"
#include "vtdata/vtLog.h"

#if 0

// Old cookie stuff, here just fstruct MyCookie
{
	vtString name;
	vtString value;
};

std::vector<MyCookie> g_cookies;
static int s_last_status = 0;

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


void ReqContext::AddHeader(const char *token, const char *value)
{
}

bool ReqContext::DoQuery(vtBytes &data, int redirects)
{
}

#endif	// Old cookie stuff

/////////////////////////////////////////////////////////////////////////////
// LibCurl begins here

#include "curl/curl.h"

#ifdef _MSC_VER
// We will now need to link with the LibCurl library.  This can
// be done automatically with MSVC as follows:
#pragma comment(lib, "libcurl.lib")
#endif

size_t write_as_string( void *ptr, size_t size, size_t nmemb, void *stream)
{
	ReqContext *context = (ReqContext *)stream;
	size_t length = size * nmemb;

	context->m_pDataString->Concat((pcchar) ptr, length);
	return length;
}

size_t write_as_bytes( void *ptr, size_t size, size_t nmemb, void *stream)
{
	ReqContext *context = (ReqContext *)stream;
	size_t length = size * nmemb;

	context->m_pDataBytes->Append((unsigned char *) ptr, length);
	return length;
}

bool ReqContext::s_bFirst = true;

ReqContext::ReqContext()
{
	if (s_bFirst)
		InitializeLibrary();

	m_curl = curl_easy_init(); 

	CURLcode result;
	result = curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);

	// This tells CURL to give us an error if we get an e.g. 404, instead
	//  of giving us the HTML page from the server, e.g. "The page cannot
	//  be found..."
	result = curl_easy_setopt(m_curl, CURLOPT_FAILONERROR, 1);
}

ReqContext::~ReqContext()
{
	curl_easy_cleanup(m_curl); 
}

bool ReqContext::GetURL(const char *url, vtString &str)
{
	m_pDataString = &str;

	CURLcode result;
	char errorbuf[CURL_ERROR_SIZE];
	result = curl_easy_setopt(m_curl, CURLOPT_URL, url);
	result = curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, write_as_string);
	result = curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, errorbuf);
	result = curl_easy_perform(m_curl);

	if (result == 0)	// 0 means everything was ok
		return true;
	else
	{
		str = errorbuf;
		return false;
	}
}

bool ReqContext::GetURL(const char *url, vtBytes &data)
{
	m_pDataBytes = &data;

	CURLcode result;
	result = curl_easy_setopt(m_curl, CURLOPT_URL, url);
	result = curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, write_as_bytes);
	result = curl_easy_perform(m_curl);

	return (result == 0);	// 0 means everything was ok
}

bool ReqContext::DoQuery(vtBytes &data, int redirects)
{
	return false;
}

void ReqContext::InitializeLibrary()
{
	s_bFirst = false;
	long flags = CURL_GLOBAL_ALL;	// Initialize everything possible. This sets all known bits. 
	CURLcode result = curl_global_init(flags); 
	if (result != 0)
	{
		// error
	}
}

#endif	// SUPPORT_HTTP
