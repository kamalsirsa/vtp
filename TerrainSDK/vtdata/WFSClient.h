//
// WFSClient.h
//
// Web Feature Server Client
//
// Copyright (c) 2003-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_WFSCLIENT_H
#define VTDATA_WFSCLIENT_H

#include "Content.h"

typedef std::vector<vtTagArray *> OGCLayerArray;

struct OGCServer
{
	vtString m_url;
	OGCLayerArray m_layers;
};

typedef std::vector<OGCServer> OGCServerArray;

bool GetLayersFromWFS(const char *szServerURL, OGCLayerArray &layers);

// for now, handle WMS here as well
bool GetLayersFromWMS(const char *szServerURL, OGCLayerArray &layers, vtString &msg);

#endif // VTDATA_WFSCLIENT_H

