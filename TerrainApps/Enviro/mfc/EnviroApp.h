//
// EnviroApp.h : main header file for the Enviro application
//

#ifndef ENVIROAPPH
#define ENVIROAPPH

#include "resource.h"	// main symbols
#include "StartupDlg.h"
#include "HtmlVw.h"

class EnviroView;

//
// EnviroApp
//
class EnviroApp : public CWinApp
{
public:
	EnviroApp();
	~EnviroApp();

	// The two views
	EnviroView		*m_pMainView;
	CMfcieView		*m_pHTMLView;

	void LoadOptions();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EnviroApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(EnviroApp)
	virtual afx_msg void OnAppAbout();
	//}}AFX_MSG

protected:
	// Overrides
	BOOL OnIdle(LONG lCount);
	bool OnInit();
	DECLARE_MESSAGE_MAP()
public:
	virtual int ExitInstance();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // ENVIROAPPH
