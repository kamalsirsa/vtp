//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#if WIN32

#include "win32/psmwin.h"
#include "vtlib/vtlib.h"

bool vtScene::HasWinInfo()
{
	PSWorld3D* world = PSWorld3D::Get();
	if (world == NULL)
		return false;
	PSDevInfo* devinfo = (PSDevInfo*) world->GetProp(PROP_DevInfo);
	return (devinfo != NULL);
}

void vtScene::SetWinInfo(void *handle, void *context)
{
	PSWorld3D* world = PSWorld3D::Get();
	if (world == NULL)
		return;
	PSDevInfo* devinfo = (PSDevInfo*) world->GetProp(PROP_DevInfo);

	VTlsData*	tls = VTlsData::Get();		// establish thread storage
	devinfo = new PSDevInfo;				// remember low-level info
	world->AddProp(devinfo);
	devinfo->WinHandle = (HWND) handle;
	devinfo->Device = context;

	world->Run(devinfo->WinHandle);			// start scene threads
}

#endif
