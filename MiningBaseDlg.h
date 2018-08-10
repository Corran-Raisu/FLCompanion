#if !defined(AFX_MININGBASEDLG_H__95E18AE3_51F6_44BD_B900_1C9ACDC8620A__INCLUDED_)
#define AFX_MININGBASEDLG_H__95E18AE3_51F6_44BD_B900_1C9ACDC8620A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MiningBaseDlg.h : header file
//

#include "System.h"

/////////////////////////////////////////////////////////////////////////////
// CMiningBaseDlg dialog

class CMiningBaseDlg : public CDialog
{
// Construction
public:
	CMiningBaseDlg(CAsteroids* asteroids, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMiningBaseDlg)
	enum { IDD = IDD_MININGBASE_DIALOG };
	CEdit	m_miningSpeedEdit;
	CComboBox	m_goodsCombo;
	double	m_lootStat;
	double	m_miningSpeed;
	UINT m_goodIndex;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMiningBaseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMiningBaseDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeStats();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CAsteroids* m_asteroids;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MININGBASEDLG_H__95E18AE3_51F6_44BD_B900_1C9ACDC8620A__INCLUDED_)
