//
// TripDub.h
//
// Module for Double-You-Double-You-Double-You support (WWW)
//
// Copyright (c) 2003-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_TRIPDUB_H
#define VTDATA_TRIPDUB_H

// The dependency on Libwww is optional.  If not desired, skip this file.
#if SUPPORT_HTTP

#include "vtString.h"

typedef struct _HTRequest HTRequest;
typedef struct _HTAnchor	HTAnchor;

class vtBytes
{
public:
	vtBytes() { m_data = NULL; }
	~vtBytes() { delete m_data; }

	void Put(unsigned char *data, size_t len)
	{
		if (m_data)
			delete [] m_data;
		m_data = new unsigned char[len];
		memcpy(m_data, data, len);
		m_len = len;
	}
	unsigned char *Get() { return m_data; }
	size_t Len() { return m_len; }

protected:
	unsigned char *m_data;
	size_t m_len;
};

class ReqContext
{
public:
	ReqContext();
	~ReqContext();

	void AddHeader(const char *token, const char *value);
	bool GetURL(const char *url, vtString &str);
	bool GetURL(const char *url, vtBytes &data);
	bool DoQuery(vtBytes &data, int redirects = 0);

	/// Set level of logging output: 0 (none) 1 (some) 2 (lots)
	void SetVerbosity(int i) { m_iVerbosity = i; }

	HTRequest *	m_request;
	HTAnchor *	m_anchor;
	char *		m_cwd;				  /* Current dir URL */

	static bool s_bFirst;
	int m_iVerbosity;
	void InitializeLibrary();
};

void AddCookie(const char *name, const char *value);

#endif // SUPPORT_HTTP

#endif // VTDATA_TRIPDUB_H

