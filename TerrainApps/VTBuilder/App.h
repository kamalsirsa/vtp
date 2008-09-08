//
// App.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

class BuilderApp: public wxApp
{
public:
	void Args(int argc, wxChar **argv);
	bool OnInit();
	int OnExit();
	wxString GetLanguageCode() { return m_locale.GetCanonicalName(); }

protected:
	void SetupLocale();

	wxLocale m_locale; // locale we'll be using
	vtString m_locale_name;
};

