#if !defined(AFX_MODINFODLG_H__E658ECD5_1E9C_4177_8CD9_20E989767644__INCLUDED_)
#define AFX_MODINFODLG_H__E658ECD5_1E9C_4177_8CD9_20E989767644__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModInfoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CModInfoDlg dialog

class CModInfoDlg : public CDialog
{
// Construction
public:
	CModInfoDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModInfoDlg)
	enum { IDD = IDD_MODINFO_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioClick();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODINFODLG_H__E658ECD5_1E9C_4177_8CD9_20E989767644__INCLUDED_)
