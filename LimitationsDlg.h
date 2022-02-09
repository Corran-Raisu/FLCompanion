#if !defined(AFX_LIMITATIONSDLG_H__F6E713EE_8E1E_4029_AE97_75B4D0A8C1A5__INCLUDED_)
#define AFX_LIMITATIONSDLG_H__F6E713EE_8E1E_4029_AE97_75B4D0A8C1A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LimitationsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLimitationsDlg dialog

class CLimitationsDlg : public CDialog
{
// Construction
public:
	CLimitationsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLimitationsDlg)
	enum { IDD = IDD_LIMITATIONS_DIALOG };
	CCheckListBox	m_goodsList;
	CCheckListBox	m_factionsList;
	CCheckListBox	m_systemsList;
	CComboBox       m_factionID;
	UINT	m_cargoSize;
	BOOL	m_avoidLockedGates;
	BOOL	m_jumptrade;
	BOOL	m_avoidHoles;
	BOOL	m_avoidGates;
	BOOL	m_avoidLanes;
	BOOL	m_avoidUnstableHoles;
	BOOL	m_isTransport;
	CString	m_investmentText;
	UINT	m_maxInvestment;
	CString	m_distanceText;
	UINT	m_maxDistance;
	CString	m_jumptradeText;
	UINT	m_jumptradeTime;
	CString	m_minCSUText;
	UINT	m_minCSU;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLimitationsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLimitationsDlg)
	afx_msg void OnChangeInvestment();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSwitchAllFactions();
	afx_msg void OnSwitchAllSystems();
	afx_msg void OnSwitchAllGoods();
	afx_msg void OnChkchangeList(UINT nID);
	afx_msg void OnChangeDistance();
	afx_msg void OnChangeJumpTrade();
	afx_msg void OnChangeFactionID();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void SwitchAll(CCheckListBox& listbox, int clearallID);
	CWndResizer m_resizer;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIMITATIONSDLG_H__F6E713EE_8E1E_4029_AE97_75B4D0A8C1A5__INCLUDED_)
