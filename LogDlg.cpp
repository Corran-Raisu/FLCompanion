// LogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FLCompanion.h"
#include "LogDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CLogDlg g_logDlg;

void LogText(LPCTSTR lpMsg)
{
	if (g_logDlg.m_hWnd == NULL)
	{
		g_logDlg.Create(g_logDlg.IDD, AfxGetMainWnd());
		g_logDlg.ShowWindow(SW_SHOWNORMAL);
	}
	g_logDlg.m_log.SetSel(MAXLONG,MAXLONG);
	g_logDlg.m_log.ReplaceSel(lpMsg);
	OutputDebugString(lpMsg);
	g_logDlg.m_log.RedrawWindow();
}

void Log(LPCTSTR lpFormat, ...)
{
	TCHAR msg[1024];
	va_list arglist;
	va_start(arglist, lpFormat);
	_vstprintf(msg, lpFormat, arglist);
	_tcscat(msg, _T("\r\n"));
	LogText(msg);
}

/////////////////////////////////////////////////////////////////////////////
// CLogDlg dialog


CLogDlg::CLogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLogDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLogDlg)
	DDX_Control(pDX, IDC_LOG, m_log);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLogDlg, CDialog)
	//{{AFX_MSG_MAP(CLogDlg)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogDlg message handlers

void CLogDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	if (m_log.m_hWnd)
		m_log.SetWindowPos(NULL,0,0,cx,cy,SWP_NOMOVE|SWP_NOZORDER);
	
}

void CLogDlg::OnSysCommand(UINT nID, LPARAM lParam) 
{
	switch (nID & 0xFFF0)
	{
	case SC_CLOSE:
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, lParam);
		break;
	default:
		CDialog::OnSysCommand(nID, lParam);
	}
	
}

void CLogDlg::OnDestroy() 
{
	theApp.WriteProfileInt(L"Settings", L"LogMinimize", IsIconic());
	CDialog::OnDestroy();
}
