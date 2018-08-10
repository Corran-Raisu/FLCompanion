#if !defined(AFX_BASEINFODLG_H__4EF18059_D081_4AC4_8A8E_9CB2A7876E98__INCLUDED_)
#define AFX_BASEINFODLG_H__4EF18059_D081_4AC4_8A8E_9CB2A7876E98__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// BaseInfoDlg.h : header file
//

#include "Base.h"
#include "ResourceProvider.h"

/////////////////////////////////////////////////////////////////////////////
// CBaseInfoDlg dialog

class CBaseInfoDlg : public CDialog
{
// Construction
public:
	CBaseInfoDlg(CBase *base, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBaseInfoDlg)
	enum { IDD = IDD_BASEINFO_DIALOG };
	CListCtrl	m_equipList;
	CListCtrl	m_goodList;
	CListCtrl	m_shipList;
	CComboBox	m_baseCombo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBaseInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBaseInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeBaseCombo();
	afx_msg void OnItemclickGoodsList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	static CBaseInfoDlg *g_baseInfoDlg;
	BOOL LoadBaseMarkets(const CString &iniFilename);
	static BOOL _LoadBaseMarkets(const CString &iniFilename) { return g_baseInfoDlg->LoadBaseMarkets(iniFilename); }
	BOOL FindEquipNames(const CString &iniFilename);
	static BOOL _FindEquipNames(const CString &iniFilename) { return g_baseInfoDlg->FindEquipNames(iniFilename); }
private:
	CBase *m_base;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BASEINFODLG_H__4EF18059_D081_4AC4_8A8E_9CB2A7876E98__INCLUDED_)
