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

#include "Content.h"

typedef std::vector<vtTagArray *> WFSLayerArray;
bool GetLayersFromWFS(const char *szServerURL, WFSLayerArray &layers);

// for now, handle WMS here as well
bool GetLayersFromWMS(const char *szServerURL, WFSLayerArray &layers);

#endif // VTDATA_WFSCLIENT_H

