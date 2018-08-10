// AboutDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "FLCompanion.h"
#include "FLCompanionDlg.h"
#include "AboutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog


CAboutDlg::CAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAboutDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAboutDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_ABOUTICON, m_icon);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg message handlers

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_icon.SetWindowPos(NULL, 0, 0, 48, 48, SWP_NOMOVE|SWP_NOZORDER);
	m_icon.SetIcon((HICON) LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDR_MAINFRAME),IMAGE_ICON,48,48,LR_SHARED));
	CString text;
	GetDlgItemText(IDC_ABOUTTEXT, text);
	text.Format(text + '\0', g_mainDlg->m_version);
	SetDlgItemText(IDC_ABOUTTEXT, text);

	return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CAboutDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	switch (reinterpret_cast<LPNMHDR>(lParam)->code)
	{
	case NM_RETURN:
	case NM_CLICK:
		ShellExecute(*this, L"open", reinterpret_cast<PNMLINK>(lParam)->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
		return 1;
	default:
		return CDialog::OnNotify(wParam, lParam, pResult);
	}
}
