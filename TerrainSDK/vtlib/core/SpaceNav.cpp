//
// Name: SpaceNav.cpp
// Purpose: Implements Win32-specific support for the SpaceNavigator 6DOF device.
//
// Copyright (c) 2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// Based on: WM_INPUTExample.cpp, 3Dconnexion Inc. May 2007
//
// Uses WM_INPUT/RawInput to get data from 3Dx devices.
// You can disconnect and reconnect devices at will while the program is running.
// You can connect more than one device at a time, and distinguish the arriving
// data between the different devices.
//

#ifdef _MSC_VER
#define _WIN32_WINNT 0x0501
#include <windows.h>	// unfortunately
#endif

// Headers for the VTP libraries
#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"

#include "SpaceNav.h"


vtSpaceNav::vtSpaceNav()
{
#if WIN32
	g_pRawInputDeviceList = NULL;
	g_pRawInputDevices = NULL;
#endif
	m_fSpeed = 100.0f;
	m_bAllowRoll = true;
}

vtSpaceNav::~vtSpaceNav()
{
#if WIN32
	if (g_pRawInputDeviceList)
		free(g_pRawInputDeviceList);
	if (g_pRawInputDevices)
		free(g_pRawInputDevices);
#endif
}

bool vtSpaceNav::InitRawDevices()
{
#if WIN32
	// Find the Raw Devices
	UINT nDevices;
	// Get Number of devices attached
	if (GetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST)) != 0)
	{ 
		VTLOG("No RawInput devices attached\n");
		return false;
	}
	// Create list large enough to hold all RAWINPUTDEVICE structs
	if ((g_pRawInputDeviceList = (PRAWINPUTDEVICELIST)malloc(sizeof(RAWINPUTDEVICELIST) * nDevices)) == NULL)
	{
		VTLOG("Error mallocing RAWINPUTDEVICELIST\n");
		return false;
	}
	// Now get the data on the attached devices
	if (GetRawInputDeviceList(g_pRawInputDeviceList, &nDevices, sizeof(RAWINPUTDEVICELIST)) == -1) 
	{
		VTLOG("Error from GetRawInputDeviceList\n");
		return false;
	}

	g_pRawInputDevices = (PRAWINPUTDEVICE)malloc( nDevices * sizeof(RAWINPUTDEVICE) );
	g_nUsagePage1Usage8Devices = 0;

	// Look through device list for RIM_TYPEHID devices with UsagePage == 1, Usage == 8
	for(UINT i=0; i<nDevices; i++)
	{
		if (g_pRawInputDeviceList[i].dwType == RIM_TYPEHID)
		{
			UINT nchars = 300;
			TCHAR deviceName[300];
			if (GetRawInputDeviceInfo(g_pRawInputDeviceList[i].hDevice,
									  RIDI_DEVICENAME, deviceName, &nchars) >= 0)
				VTLOG("Device[%d]: handle=0x%x name = %S\n", i,
					g_pRawInputDeviceList[i].hDevice, deviceName);
			RID_DEVICE_INFO dinfo;
			UINT sizeofdinfo = sizeof(dinfo);
			dinfo.cbSize = sizeofdinfo;
			if (GetRawInputDeviceInfo(g_pRawInputDeviceList[i].hDevice,
									  RIDI_DEVICEINFO, &dinfo, &sizeofdinfo ) >= 0)
			{
				if (dinfo.dwType == RIM_TYPEHID)
				{
					RID_DEVICE_INFO_HID *phidInfo = &dinfo.hid;
					VTLOG("VID = 0x%x\n", phidInfo->dwVendorId);
					VTLOG("PID = 0x%x\n", phidInfo->dwProductId);
					VTLOG("Version = 0x%x\n", phidInfo->dwVersionNumber);
					VTLOG("UsagePage = 0x%x\n", phidInfo->usUsagePage);
					VTLOG("Usage = 0x%x\n", phidInfo->usUsage);

					// Add this one to the list of interesting devices?
					// Actually only have to do this once to get input from all usage 1, usagePage 8 devices
					// This just keeps out the other usages.
					// You might want to put up a list for users to select amongst the different devices.
					// In particular, to assign separate functionality to the different devices.
					if (phidInfo->usUsagePage == 1 && phidInfo->usUsage == 8)
					{
						g_pRawInputDevices[g_nUsagePage1Usage8Devices].usUsagePage = phidInfo->usUsagePage;
						g_pRawInputDevices[g_nUsagePage1Usage8Devices].usUsage     = phidInfo->usUsage;
						g_pRawInputDevices[g_nUsagePage1Usage8Devices].dwFlags     = 0;
						g_pRawInputDevices[g_nUsagePage1Usage8Devices].hwndTarget  = NULL;
						g_nUsagePage1Usage8Devices++;
					}
				}
			}
		}
	}
	// Register for input from the devices in the list
	if (RegisterRawInputDevices( g_pRawInputDevices, g_nUsagePage1Usage8Devices, sizeof(RAWINPUTDEVICE) ) == FALSE )
	{
		VTLOG("Error calling RegisterRawInputDevices\n");
		return false;
	}
	return true;
#else
	// not implemented
	return false;
#endif
}

#if WIN32
void vtSpaceNav::ProcessWM_INPUTEvent(LPARAM lParam, vtTransform *target)
{
	#ifdef SHOW_DETAILS
	VTLOG("WM_INPUT lParam=0x%x\n", lParam );
	#endif

	RAWINPUTHEADER header;
	UINT size = sizeof(header);
	if ( GetRawInputData( (HRAWINPUT)lParam, RID_HEADER, &header,  &size, sizeof(RAWINPUTHEADER) ) == -1)
	{
		VTLOG("Error from GetRawInputData(RID_HEADER)\n");
		return;
	}
	//else
		//VTLOG("rawEvent.header: hDevice = 0x%x\n", header.hDevice );

	// Set aside enough memory for the full event
	size = header.dwSize;
	LPRAWINPUT evt = (LPRAWINPUT)malloc(size);
	if (GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, evt, &size, sizeof(RAWINPUTHEADER) ) == -1)
	{
		VTLOG("Error from GetRawInputData(RID_INPUT)\n");
		free(evt);
		return;
	}
	else
	{
		if (evt->header.dwType == RIM_TYPEHID)
		{
			static BOOL bGotTranslation = FALSE, 
				        bGotRotation    = FALSE;
			static int all6DOFs[6] = {0};
			LPRAWHID pRawHid = &evt->data.hid;

			#ifdef SHOW_DETAILS
			VTLOG("rawInput count: %d\n", pRawHid->dwCount);
			VTLOG("          size: %d\n", pRawHid->dwSizeHid);
			for(UINT i=0; i<pRawHid->dwSizeHid; i++)
			{
				VTLOG("%d ", pRawHid->bRawData[i] );
			}
			_RPT0( _CRT_WARN, "\n" );
			#endif

			// Translation or Rotation packet?  They come in two different packets.
			if (pRawHid->bRawData[0] == 1) // Translation vector
			{
				all6DOFs[0] = (pRawHid->bRawData[1] & 0x000000ff) | ((signed short)(pRawHid->bRawData[2]<<8) & 0xffffff00); 
				all6DOFs[1] = (pRawHid->bRawData[3] & 0x000000ff) | ((signed short)(pRawHid->bRawData[4]<<8) & 0xffffff00); 
				all6DOFs[2] = (pRawHid->bRawData[5] & 0x000000ff) | ((signed short)(pRawHid->bRawData[6]<<8) & 0xffffff00);
				bGotTranslation = TRUE;
			}
			else if (pRawHid->bRawData[0] == 2) // Rotation vector
			{
				all6DOFs[3] = (pRawHid->bRawData[1] & 0x000000ff) | ((signed short)(pRawHid->bRawData[2]<<8) & 0xffffff00); 
				all6DOFs[4] = (pRawHid->bRawData[3] & 0x000000ff) | ((signed short)(pRawHid->bRawData[4]<<8) & 0xffffff00); 
				all6DOFs[5] = (pRawHid->bRawData[5] & 0x000000ff) | ((signed short)(pRawHid->bRawData[6]<<8) & 0xffffff00);
				bGotRotation = TRUE;
			}
			else if (pRawHid->bRawData[0] == 3) // Buttons (display most significant byte to least)
			{
				//VTLOG("Button mask: %.2x %.2x %.2x\n",(unsigned char)pRawHid->bRawData[3],(unsigned char)pRawHid->bRawData[2],(unsigned char)pRawHid->bRawData[1]);
				OnButtons((unsigned char)pRawHid->bRawData[3],
						  (unsigned char)pRawHid->bRawData[2],
						  (unsigned char)pRawHid->bRawData[1]);
			}

			if (bGotTranslation && bGotRotation)
			{
				bGotTranslation = bGotRotation = FALSE;
				//VTLOG("all6DOFs: %d %d %d ", all6DOFs[0], all6DOFs[1], all6DOFs[2]);
				//VTLOG(          "%d %d %d\n", all6DOFs[3], all6DOFs[4], all6DOFs[5]);
				FPoint3 trans((float) all6DOFs[0]/256*m_fSpeed,
							  (float)-all6DOFs[2]/256*m_fSpeed,
							  (float) all6DOFs[1]/256*m_fSpeed);
				target->TranslateLocal(trans);
				target->RotateLocal(FPoint3(1,0,0), (float) all6DOFs[3]/25600);
				target->RotateLocal(FPoint3(0,1,0), (float)-all6DOFs[5]/25600);
				if (m_bAllowRoll)
					target->RotateLocal(FPoint3(0,0,1), (float) all6DOFs[4]/25600);
			}
		}
	}
	free(evt);
}

#endif	// WIN32

