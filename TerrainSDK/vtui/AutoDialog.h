//
// AutoDialog.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef AUTODIALOGH
#define AUTODIALOGH

#if wxUSE_VALIDATORS != 1
#error "wx was not configured with support for validators"
#endif

#include <wx/valgen.h>

#ifndef NUMERIC_VALIDATOR
#define NUMERIC_VALIDATOR

class wxNumericValidator: public wxValidator
{
public:
	wxNumericValidator(short *val);
	wxNumericValidator(int *val);
	wxNumericValidator(float *val, int digits = -1);
	wxNumericValidator(double *val, int digits = -1);
	wxNumericValidator(const wxNumericValidator& copyFrom);

	// Make a clone of this validator (or return NULL) - currently necessary
	// if you're passing a reference to a validator.
	// Another possibility is to always pass a pointer to a new validator
	// (so the calling code can use a copy constructor of the relevant class).
	virtual wxObject *Clone() const { return new wxNumericValidator(*this); }
	bool Copy(const wxNumericValidator& val);

	// Called when the value in the window must be validated.
	// This function can pop up an error message.
	virtual bool Validate(wxWindow *WXUNUSED(parent)) { return TRUE; };

	// Called to transfer data to the window
	virtual bool TransferToWindow();

	// Called to transfer data to the window
	virtual bool TransferFromWindow();

	void Enable(bool bTrue) { m_bEnabled = bTrue; }

protected:
	void Initialize();

	short	*m_pValShort;
	int		*m_pValInt;
	float	*m_pValFloat;
	double	*m_pValDouble;

	// For a floating-point value, specify the number of digits to display.
	// A value of -1 means to use printf's default (6 digits for floats,
	//	more digits for doubles)
	int		m_iDigits;

	// You can disable these validators
	bool	m_bEnabled;
};

#endif // NUMERIC_VALIDATOR


class AutoDialog : public wxDialog
{
public:
	AutoDialog() {}
	AutoDialog(wxWindow *parent, wxWindowID id,
			 const wxString& title,
			 const wxPoint& pos = wxDefaultPosition,
			 const wxSize& size = wxDefaultSize,
			 long style = wxDEFAULT_DIALOG_STYLE) :
	  wxDialog(parent, id, title, pos, size, style) {}

	void AddValidator(long id, wxString *sptr);
	void AddValidator(long id, bool *bptr);
	void AddValidator(long id, int *iptr);
	wxNumericValidator *AddNumValidator(long id, short *sptr);
	wxNumericValidator *AddNumValidator(long id, int *iptr);
	wxNumericValidator *AddNumValidator(long id, float *fptr, int digits = -1);
	wxNumericValidator *AddNumValidator(long id, double *dptr, int digits = -1);
};

// And forms of the methods which don't require subclassing from AutoDialog
void AddValidator(wxDialog *dlg, long id, wxString *sptr);
void AddValidator(wxDialog *dlg, long id, bool *bptr);
void AddValidator(wxDialog *dlg, long id, int *iptr);
wxNumericValidator *AddNumValidator(wxDialog *dlg, long id, short *sptr);
wxNumericValidator *AddNumValidator(wxDialog *dlg, long id, int *iptr);
wxNumericValidator *AddNumValidator(wxDialog *dlg, long id, float *fptr, int digits = -1);
wxNumericValidator *AddNumValidator(wxDialog *dlg, long id, double *dptr, int digits = -1);

class AutoPanel : public wxPanel
{
public:
	AutoPanel() {}
	AutoPanel(wxWindow *parent, wxWindowID id,
			 const wxPoint& pos = wxDefaultPosition,
			 const wxSize& size = wxDefaultSize,
			 long style = wxDEFAULT_DIALOG_STYLE) :
	  wxPanel(parent, id, pos, size, style) {}

	void AddValidator(long id, wxString *sptr);
	void AddValidator(long id, bool *bptr);
	void AddValidator(long id, int *iptr);
	wxNumericValidator *AddNumValidator(long id, int *iptr);
	wxNumericValidator *AddNumValidator(long id, float *fptr, int digits = -1);
	wxNumericValidator *AddNumValidator(long id, double *dptr, int digits = -1);
};

// And forms of the methods which don't require subclassing from AutoPanel
void AddValidator(wxPanel *pan, long id, wxString *sptr);
void AddValidator(wxPanel *pan, long id, bool *bptr);
void AddValidator(wxPanel *pan, long id, int *iptr);
wxNumericValidator *AddNumValidator(wxPanel *pan, long id, short *sptr);
wxNumericValidator *AddNumValidator(wxPanel *pan, long id, int *iptr);
wxNumericValidator *AddNumValidator(wxPanel *pan, long id, float *fptr, int digits = -1);
wxNumericValidator *AddNumValidator(wxPanel *pan, long id, double *dptr, int digits = -1);


#endif
