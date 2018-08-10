// ModInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FLCompanion.h"
#include "ModInfoDlg.h"
#include "Datas.h"
#include <CommCtrl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModInfoDlg dialog


CModInfoDlg::CModInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModInfoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CModInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModInfoDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CModInfoDlg)
	ON_BN_CLICKED(IDC_RADIO_DESCRIPTION, OnRadioClick)
	ON_BN_CLICKED(IDC_RADIO_FLCWARNING, OnRadioClick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModInfoDlg message handlers

BOOL CModInfoDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CString str, str2;
	GetWindowText(str);
	str.FormatMessage(str, g_modInfo.name);
	SetWindowText(str);
	SetDlgItemText(IDC_AUTHORS,		L"Author(s): "+g_modInfo.author);
	GetDlgItemText(IDC_WEBLINK, str);
	str2.Format(L"<a href=\"%s\">%s</a>", g_modInfo.url, str);
	SetDlgItemText(IDC_WEBLINK,	str2);
	if (g_modInfo.FLC_info.IsEmpty())
	{
		EnableDlgItem(*this, IDC_RADIO_FLCWARNING, FALSE);
		EnableDlgItem(*this, IDC_RADIO_DESCRIPTION, FALSE);
		CheckDlgButton(IDC_RADIO_DESCRIPTION, BST_CHECKED);
		SetDlgItemText(IDC_ABOUTTEXT,	g_modInfo.description);
	}
	else
	{
		CheckDlgButton(IDC_RADIO_FLCWARNING, BST_CHECKED);
		SetDlgItemText(IDC_ABOUTTEXT,	g_modInfo.FLC_info);
		if (!g_modInfo.FLC_info_name.IsEmpty())
			SetDlgItemText(IDC_RADIO_FLCWARNING,	g_modInfo.FLC_info_name);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CModInfoDlg::OnRadioClick() 
{
	int iRadio = GetCheckedRadioButton(IDC_RADIO_DESCRIPTION, IDC_RADIO_FLCWARNING);
	if (iRadio == IDC_RADIO_DESCRIPTION)
	{
		SendDlgItemMessage(IDC_ABOUTTEXT, EM_SETTEXTMODE,  TM_PLAINTEXT);
		SetDlgItemText(IDC_ABOUTTEXT,	g_modInfo.description);
	}
	else
		SetDlgItemText(IDC_ABOUTTEXT,	g_modInfo.FLC_info);
}

BOOL CModInfoDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
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
