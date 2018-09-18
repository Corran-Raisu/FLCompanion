#if !defined(AFX_SPEEDDELAYSDLG_H__F84E57A0_8D47_43C3_8ABD_297EA1B794B7__INCLUDED_)
#define AFX_SPEEDDELAYSDLG_H__F84E57A0_8D47_43C3_8ABD_297EA1B794B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpeedDelaysDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpeedDelaysDlg dialog

class CSpeedDelaysDlg : public CDialog
{
// Construction
public:
	CSpeedDelaysDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSpeedDelaysDlg)
	enum { IDD = IDD_SPEEDDELAYS_DIALOG };
	UINT	m_engineSpeed;
	UINT	m_jumpDelay;
	UINT	m_laneDelay;
	UINT	m_laneSpeed;
	UINT	m_baseDelay;
	UINT	m_holeDelay;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpeedDelaysDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void SetDefaults();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSpeedDelaysDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPEEDDELAYSDLG_H__F84E57A0_8D47_43C3_8ABD_297EA1B794B7__INCLUDED_)
