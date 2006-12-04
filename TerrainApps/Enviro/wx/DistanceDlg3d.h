//
// Name: DistanceDlg3d.h
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __DistanceDlg3d_H__
#define __DistanceDlg3d_H__

#include "vtui/DistanceDlg.h"

//----------------------------------------------------------------------------
// DistanceDlg3d
//----------------------------------------------------------------------------

class DistanceDlg3d: public DistanceDlg
{
public:
	// constructors and destructors
	DistanceDlg3d( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	virtual void OnMode(bool bPath);
	virtual void Reset();
};

#endif
