//
// WFSClient.h
//
// Web Feature Server Client
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_WFSCLIENT_H
#define VTDATA_WFSCLIENT_H

// The dependency on Libwww is optional.  If not desired, skip this file.
#if SUPPORT_HTTP

#include "vtString.h"

typedef struct _HTRequest HTRequest;
typedef struct _HTAnchor	HTAnchor;

class ReqContext
{
public:
	ReqContext();
	~ReqContext();

	void AddHeader(const char *token, const char *value);
	void GetURL(const char *url, vtString &str);
	void DoQuery(vtString &str, int redirects = 0);

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

#endif // VTDATA_WFSCLIENT_H

