#if !defined(AFX_SYSTEMMAP_H__BDFA4861_55EC_4818_9F47_A015AB27CCDB__INCLUDED_)
#define AFX_SYSTEMMAP_H__BDFA4861_55EC_4818_9F47_A015AB27CCDB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SystemMap.h : header file
//

#include "System.h"

/////////////////////////////////////////////////////////////////////////////
// CSystemMap dialog

class CSystemMap : public CDialog
{
// Construction
public:
	BOOL m_displayNicknames;
	CSystem *m_system;
	CSystem *m_destsystem;
	CSystemMap(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSystemMap)
	enum { IDD = IDD_SYSTEMMAP_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSystemMap)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSystemMap)
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int m_minX,m_minY;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSTEMMAP_H__BDFA4861_55EC_4818_9F47_A015AB27CCDB__INCLUDED_)
