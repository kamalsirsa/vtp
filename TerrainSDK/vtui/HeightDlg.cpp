/////////////////////////////////////////////////////////////////////////////
// Name:		HeightDlg.cpp
// Author:	  XX
// Created:	 XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
	#pragma implementation "HeightDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#include "HeightDlg.h"
#include "HeightGrid.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// CHeightDialog
//----------------------------------------------------------------------------

// WDR: event table for CHeightDialog

BEGIN_EVENT_TABLE(CHeightDialog, AutoDialog)
	EVT_BUTTON( ID_RECALCULATEHEIGHTS, CHeightDialog::OnRecalculateHeights )
	EVT_BUTTON( wxID_OK, CHeightDialog::OnOK )
END_EVENT_TABLE()

CHeightDialog::CHeightDialog( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	HeightDialogFunc( this, TRUE );
}

void CHeightDialog::Setup(vtBuilding  * const pBuilding, vtHeightField *pHeightField)
{
	int i;
	double dBaseline = pBuilding->CalculateBaseElevation(pHeightField);
	wxGridCellFloatEditor GCFE;

	m_bGridModified = false;

	m_pBuilding = pBuilding;
	m_pHeightField = pHeightField;
	m_pHeightGrid = GetHeightgrid();
	m_pScaleradiobox = GetScaleradiobox();
	m_pBaselineOffset = GetBaselineoffset();
	m_pHeightGrid->Setup(this);

	// Set up the baseline offset
	m_fBaselineOffset = m_pBuilding->GetElevationOffset();
	// TODO --- find an easy way of adding a KILL_FOCUS handler to this ctrl
	AddNumValidator(ID_BASELINEOFFSET, &m_fBaselineOffset);

	// Set up the radio box
	m_pScaleradiobox->SetSelection(SCALE_EVENLY);

	m_NumLevels = m_pBuilding->GetNumLevels();
	m_BottomRow = m_NumLevels - 1;
	m_dBaseLine = dBaseline;

	for (i = 0; i < (m_NumLevels - 1); i++)
		dBaseline += m_pBuilding->GetLevel(i)->m_fStoryHeight * m_pBuilding->GetLevel(i)->m_iStories;

	// Calculate elevation of top level baseline;

	// Set up the grid
	m_pHeightGrid->SetColLabelValue(BASELINE_COL, _T("Baseline"));
	m_pHeightGrid->SetColFormatFloat(BASELINE_COL);
	m_pHeightGrid->SetColLabelValue(ACTIVE_COL, _T("Active"));
	m_pHeightGrid->SetColFormatBool(ACTIVE_COL);
	m_pHeightGrid->SetColLabelValue(STOREYS_COL, _T("Storeys"));
	m_pHeightGrid->SetColFormatNumber(STOREYS_COL);
	m_pHeightGrid->SetColLabelValue(HEIGHT_COL, _T("Height"));
	m_pHeightGrid->SetColFormatFloat(HEIGHT_COL);
	m_pHeightGrid->AppendRows(m_NumLevels);

	wxString str;
	for (i = 0; i < m_NumLevels; i++)
	{
		int iCurrentLevel = (m_NumLevels - 1) - i;
		vtLevel  *pLevel = m_pBuilding->GetLevel(iCurrentLevel);
		float StoreyHeight = pLevel->m_fStoryHeight;
		int Storeys = pLevel->m_iStories;

		if (i != 0)
			dBaseline -= StoreyHeight * Storeys;

		str.Printf(_T("%d"), iCurrentLevel);
		m_pHeightGrid->SetRowLabelValue(i, str);

		str.Printf(_T("%f"), dBaseline);
		m_pHeightGrid->SetCellValue(i, BASELINE_COL, str);
		m_pHeightGrid->SetCellEditor(i, BASELINE_COL, GCFE.Clone());
		m_pHeightGrid->SetReadOnly(i, ACTIVE_COL);
		m_pHeightGrid->SetReadOnly(i, BASELINE_COL);
		if (0 == iCurrentLevel)
			m_pHeightGrid->SetCellValue(i, ACTIVE_COL, _T("1"));
		else
			m_pHeightGrid->SetCellValue(i, ACTIVE_COL, _T("0"));

		str.Printf(_T("%d"), Storeys);
		m_pHeightGrid->SetReadOnly(i, STOREYS_COL);
		m_pHeightGrid->SetCellValue(i, STOREYS_COL, str);

		str.Printf(_T("%f"), StoreyHeight);
		m_pHeightGrid->SetReadOnly(i, HEIGHT_COL);
		m_pHeightGrid->SetCellValue(i, HEIGHT_COL, str);
	}
}

void CHeightDialog::OnLeftClickGrid( wxGridEvent &event )
{
	int iRow = event.GetRow();
	long lValue;
	// Check to if user has clicked on "locked column"
	// but not on bottom row
	if ((event.GetCol() == ACTIVE_COL) && (iRow != m_BottomRow))
	{
		// Toggle the lock
		m_pHeightGrid->GetCellValue(iRow, ACTIVE_COL).ToLong(&lValue);
		if (lValue)
		{
			m_pHeightGrid->SetReadOnly(iRow, BASELINE_COL);
			m_pHeightGrid->SetCellValue(iRow, ACTIVE_COL, _T("0"));
		}
		else
		{
			m_pHeightGrid->SetReadOnly(iRow, BASELINE_COL, false);
			m_pHeightGrid->SetCellValue(iRow, ACTIVE_COL, _T("1"));
		}
	}
}

void CHeightDialog::OnGridEditorHidden( wxGridEvent &event )
{
	// Should probably do some validation here !!!!
	// Check if baseline >= the first active one below it
	// if not then display a message, set cell value to first locked one below it
	// and reset the editor.

	m_bGridModified = true;
	return;

	// None of the following will work until I find a way of getting the actual data that is now in the editor
/*
	char string[50];
	double dBaseLine, dLowerBaseLine;
	int iRow = event.GetRow();

	m_bGridModified = true;
	m_pHeightGrid->GetCellValue(iRow, BASELINE_COL).ToDouble(&dBaseLine);
	// Find next active cell below
	for (int i = iRow; true; i++)
	{
		long lValue;

		m_pHeightGrid->GetCellValue(i, ACTIVE_COL).ToLong(&lValue);
		if (lValue)
			break;
	}
	// i will always be valid because bottom cell is always active
	m_pHeightGrid->GetCellValue(i, BASELINE_COL).ToDouble(&dLowerBaseLine);

	if (dBaseLine < dLowerBaseLine)
	{
		// Bad food display message box
		wxMessageDialog(this,
						_T("Active baseline must be greater or equal to next lower active baseline"),
						_T("Error"),
						wxOK|wxICON_ERROR);
		m_pHeightGrid->SetCellValue(i, BASELINE_COL, _gcvt(dLowerBaseLine, 10, string));
		m_pHeightGrid->ShowCellEditControl();
	}*/
}


// WDR: handler implementations for CHeightDialog

void CHeightDialog::OnOK( wxCommandEvent &event )
{
	if (m_bGridModified)
		OnRecalculateHeights(event);

	wxDialog::OnOK(event);
}

void CHeightDialog::OnRecalculateHeights( wxCommandEvent &event )
{
	int iScaleType = m_pScaleradiobox->GetSelection();
	int i, j;
	int iBaseLevel = 0;
	double dBaseLine =  m_dBaseLine;
	vtLevel *pLevel = m_pBuilding->GetLevel(0);
	float fCurrentHeight = pLevel->m_fStoryHeight * pLevel->m_iStories;
	wxString str;

	if (!m_bGridModified)
		return;

	// Recalculate the storey heights
	for (i = 1; i < m_NumLevels; i++)
	{
		long lValue;
		int iCurrentRow = m_NumLevels - 1 - i;

		pLevel = m_pBuilding->GetLevel(i);

		m_pHeightGrid->GetCellValue(iCurrentRow, ACTIVE_COL).ToLong(&lValue);
		if (lValue)
		{
			double dNewBaseLine;

			// Scale the intervening heights to match
			m_pHeightGrid->GetCellValue(iCurrentRow, BASELINE_COL).ToDouble(&dNewBaseLine);
			if (dNewBaseLine < dBaseLine)
			{
				// For the time being just give up
				wxMessageDialog(this,
								_T("Active baselines must be greater or equal to next lower active baseline"),
								_T("Error"),
								wxOK|wxICON_ERROR);
				return;
			}
			if (fCurrentHeight != (float)(dNewBaseLine - dBaseLine))
			{
				switch (iScaleType)
				{
					case SCALE_EVENLY:
					{
						float fScaleFactor = (float)(dNewBaseLine - dBaseLine) / fCurrentHeight;

						for (j = iBaseLevel; j < i; j++)
						{
							m_pBuilding->GetLevel(j)->m_fStoryHeight *= fScaleFactor;
							str.Printf(_T("%f"), m_pBuilding->GetLevel(j)->m_fStoryHeight);
							m_pHeightGrid->SetCellValue(m_NumLevels - 1 - j, HEIGHT_COL, str);
						}
						break;
					}
					case SCALE_FROM_BOTTOM:
					{
						float fDifference = (float)(dNewBaseLine - dBaseLine) - fCurrentHeight;

						for (j = iBaseLevel; fDifference != 0; j++)
						{
							vtLevel *pCurrentLevel = m_pBuilding->GetLevel(j);
							float fCurrentLevelHeight = pCurrentLevel->m_fStoryHeight * pCurrentLevel->m_iStories;

							if (fDifference >= fCurrentLevelHeight)
							{
								pCurrentLevel->m_fStoryHeight = 0.0;
								fDifference -= fCurrentLevelHeight;
							}
							else
							{
								float fScaleFactor = fDifference / fCurrentLevelHeight;
								m_pBuilding->GetLevel(j)->m_fStoryHeight *= fScaleFactor;
								fDifference = 0;
							}
							str.Printf(_T("%f"), m_pBuilding->GetLevel(j)->m_fStoryHeight);
							m_pHeightGrid->SetCellValue(m_NumLevels - 1 - j, HEIGHT_COL, str);
						}
						break;
					}
					case SCALE_FROM_TOP:
						float fDifference = (float)(dNewBaseLine - dBaseLine) - fCurrentHeight;

						for (j = i - 1; fDifference != 0; j--)
						{
							vtLevel *pCurrentLevel = m_pBuilding->GetLevel(j);
							float fCurrentLevelHeight = pCurrentLevel->m_fStoryHeight * pCurrentLevel->m_iStories;

							if (fDifference >= fCurrentLevelHeight)
							{
								pCurrentLevel->m_fStoryHeight = 0.0;
								fDifference -= fCurrentLevelHeight;
							}
							else
							{
								float fScaleFactor = fDifference / fCurrentLevelHeight;
								m_pBuilding->GetLevel(j)->m_fStoryHeight *= fScaleFactor;
								fDifference = 0;
							}
							str.Printf(_T("%f"), m_pBuilding->GetLevel(j)->m_fStoryHeight);
							m_pHeightGrid->SetCellValue(m_NumLevels - 1 - j, HEIGHT_COL, str);
						}
						break;
						break;
				}
			}
			dBaseLine = dNewBaseLine;
			iBaseLevel = i;
			fCurrentHeight = 0;
		}
		fCurrentHeight += pLevel->m_fStoryHeight * pLevel->m_iStories;
	}
	m_bGridModified = false;
}

void CHeightDialog::OnInitDialog(wxInitDialogEvent& event)
{
	wxDialog::OnInitDialog(event);  // calls TransferDataToWindow()
}



