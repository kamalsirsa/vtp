
#ifndef VTDATA_WFSCLIENT_H
#define VTDATA_WFSCLIENT_H

// The dependency on Libwww is optional.  If not desired, skip this file.
#if SUPPORT_HTTP

#include "vtString.h"

typedef struct _HTRequest HTRequest;
typedef struct _HTAnchor	HTAnchor;
typedef struct _HTCookie	HTCookie;

class ReqContext
{
public:
	ReqContext();
	~ReqContext();

	void AddHeader(const char *token, const char *value);
	void GetURL(const char *url, vtString &str);
	void DoQuery(vtString &str, int redirects = 0);

	HTRequest *	m_request;
	HTAnchor *	m_anchor;
	char *		m_cwd;				  /* Current dir URL */

//	char *		realm;			/* For automated authentication */
//	char *		user;
//	char *		password;

	static bool s_bFirst;
	void InitializeLibrary();
};

void AddCookie(const char *name, const char *value);

#endif // SUPPORT_HTTP

#endif // VTDATA_WFSCLIENT_H

