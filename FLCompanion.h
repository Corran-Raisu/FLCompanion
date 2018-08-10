// FLCompanion.h : main header file for the FLCOMPANION application
//

#if !defined(AFX_FLCOMPANION_H__C419B14C_ADFC_46AE_95C5_CD1E738E425D__INCLUDED_)
#define AFX_FLCOMPANION_H__C419B14C_ADFC_46AE_95C5_CD1E738E425D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CFLCompanionApp:
// See FLCompanion.cpp for the implementation of this class
//

class CFLCompanionApp : public CWinApp
{
public:
	CFLCompanionApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFLCompanionApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CFLCompanionApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CFLCompanionApp theApp;

CString MinuteSeconds(UINT time, bool forceMin = false);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLCOMPANION_H__C419B14C_ADFC_46AE_95C5_CD1E738E425D__INCLUDED_)
