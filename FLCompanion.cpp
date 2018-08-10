// FLCompanion.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "FLCompanion.h"
#include "FLCompanionDlg.h"
#include "Datas.h"
#include "IniFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define APP_GUID _T("FLCompanion-{0A8EB4BA-8147-460B-9B0C-6D5B32F3FF41}")

/////////////////////////////////////////////////////////////////////////////
// CFLCompanionApp

BEGIN_MESSAGE_MAP(CFLCompanionApp, CWinApp)
	//{{AFX_MSG_MAP(CFLCompanionApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFLCompanionApp construction

CFLCompanionApp::CFLCompanionApp() : CWinApp(L"FLCompanion")
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CFLCompanionApp object

CFLCompanionApp theApp;
	
/////////////////////////////////////////////////////////////////////////////
// CFLCompanionApp initialization

BOOL CFLCompanionApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
	InitCommonControls();

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
	AfxEnableControlContainer();
	AfxInitRichEdit();
	AfxOleInit();

	SetRegistryKey(L"Wizou");
	
	int counter = GetProfileInt(L"Settings", L"RunCounter",0);
	/*if (counter%10 == 0)
		if (AfxMessageBox(L"WARNING ! This tool can spoil all the fun of playing Freelancer !\n\n"
						L"It is recommended for the gameplay and the community that you don't use it too much",
						MB_OKCANCEL|MB_ICONINFORMATION) == IDCANCEL)
			return FALSE;*/
	WriteProfileInt(L"Settings", L"RunCounter",counter+1);
	
	CFLCompanionDlg dlg;

	m_pMainWnd = &dlg;

	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

CString MinuteSeconds(UINT time, BOOL forceMin)
{
	time = (time+500)/1000;
	if (forceMin || (time > 60))
	{
		CString result;
		result.Format(L"%d min %02d sec", time/60, time%60);
		return result;
	}
	else
		return IntToString(time)+" sec";
}
