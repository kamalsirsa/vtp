//
// AutoDialog.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef AUTODIALOGH
#define AUTODIALOGH

#include <wx/valgen.h>

class wxNumericValidator: public wxValidator
{
public:
	wxNumericValidator(int* val);
	wxNumericValidator(float* val);
	wxNumericValidator(double* val);
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

protected:
	void Initialize();

	int		*m_pValInt;
	float	*m_pValFloat;
	double	*m_pValDouble;
};


class AutoDialog : public wxDialog
{
public:
	AutoDialog() {}
//	AutoDialog(wxWindow *parent, const wxString& title, bool modal) :
//	  wxDialog(parent, title, modal) {}
	AutoDialog(wxWindow *parent, wxWindowID id,
			 const wxString& title,
			 const wxPoint& pos = wxDefaultPosition,
			 const wxSize& size = wxDefaultSize,
			 long style = wxDEFAULT_DIALOG_STYLE) :
	  wxDialog(parent, id, title, pos, size, style) {}

	virtual void OnInitDialog(wxInitDialogEvent& event) = 0;

	void AddValidator(long id, wxString *sptr);
	void AddValidator(long id, bool *bptr);
	void AddValidator(long id, int *iptr);
	void AddNumValidator(long id, int *iptr);
	void AddNumValidator(long id, float *fptr);
	void AddNumValidator(long id, double *dptr);

	DECLARE_EVENT_TABLE()
};

#endif
