//
// Name:	ModelDlg.cpp
//
// Copyright (c) 2001-2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "ModelDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef _MSC_VER
#pragma warning( disable : 4786 ) 
#endif

#include "vtlib/vtlib.h"

#include "ModelDlg.h"
#include "vtdata/Content.h"
#include "Frame.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ModelDlg
//----------------------------------------------------------------------------

// WDR: event table for ModelDlg

BEGIN_EVENT_TABLE(ModelDlg,AutoPanel)
	EVT_TEXT( ID_FILENAME, ModelDlg::OnTextFilename )
	EVT_TEXT_ENTER( ID_DISTANCE, ModelDlg::OnTextVisible )
	EVT_TEXT_ENTER( ID_SCALE, ModelDlg::OnTextScale )
END_EVENT_TABLE()

ModelDlg::ModelDlg( wxWindow *parent, wxWindowID id,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoPanel( parent, id, position, size, style )
{
	m_bUpdating = false;
	m_pCurrentModel = NULL;
	ModelDialogFunc( this, TRUE ); 

	AddValidator(ID_FILENAME, &m_strFilename);
	AddNumValidator(ID_DISTANCE, &m_fDistance);
	AddNumValidator(ID_SCALE, &m_fScale, 5);
	AddValidator(ID_STATUS, &m_strStatus);
}

// WDR: handler implementations for ModelDlg

void ModelDlg::OnTextScale( wxCommandEvent &event )
{
	UpdateFromControls();

	if (!m_pCurrentModel)
		return;

	// change in Scale should be passed to the 3d scene graph
	GetMainFrame()->UpdateScale(m_pCurrentModel);
	GetMainFrame()->ZoomToCurrentModel();
}

void ModelDlg::OnTextVisible( wxCommandEvent &event )
{
	if (m_bUpdating)
		return;

	UpdateFromControls();

	// change in LOD visibility distance should be passed to the 3d scene graph
	GetMainFrame()->UpdateCurrentModelLOD();
}

void ModelDlg::OnTextFilename( wxCommandEvent &event )
{
	UpdateFromControls();
}

void ModelDlg::SetCurrentModel(vtModel *model)
{
	if (model)
	{
		m_strFilename = model->m_filename;
		m_fDistance = model->m_distance;
		m_fScale = model->m_scale;
	}
	else
	{
		m_strFilename = "";
		m_fDistance = 0.0f;
		m_fScale = 1.0f;
		m_strStatus = "";
	}
	m_pCurrentModel = model;
	m_bUpdating = true;
	TransferDataToWindow();
	m_bUpdating = false;
}

void ModelDlg::SetModelStatus(const char *string)
{
	m_strStatus = string;
	TransferDataToWindow();
}

void ModelDlg::UpdateFromControls()
{
	if (m_bUpdating)
		return;

	TransferDataFromWindow();
	if (m_pCurrentModel)
	{
		m_pCurrentModel->m_filename = m_strFilename.mb_str();
		m_pCurrentModel->m_distance = m_fDistance;
		m_pCurrentModel->m_scale = m_fScale;
	}
}

