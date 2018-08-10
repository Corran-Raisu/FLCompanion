#if !defined(AFX_GOODSREPODLG_H__40E78ED3_D9BD_4A6C_AC54_5C538A0D35C4__INCLUDED_)
#define AFX_GOODSREPODLG_H__40E78ED3_D9BD_4A6C_AC54_5C538A0D35C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GoodsRepoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGoodsRepoDlg dialog

class CGoodsRepoDlg : public CDialog
{
// Construction
public:
	CGoodsRepoDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGoodsRepoDlg)
	enum { IDD = IDD_GOODSREPO_DIALOG };
	CListCtrl	m_goodsList;
	CListCtrl	m_basesList;
	CComboBox	m_kindCombo;
	//}}AFX_DATA

	CBase* m_selectedBase;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGoodsRepoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGoodsRepoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeKindCombo();
	afx_msg void OnDestroy();
	afx_msg void OnItemclickBasesList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomDrawBasesList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeGoodsList();
	afx_msg void OnItemchangedGoodsList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemactivateBasesList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CString m_kind;
	CString m_item;
	BOOL m_showAllBases;
	float m_defaultPrice;
	static CGoodsRepoDlg *g_goodsRepoDlg;
	BOOL FindEquipKind(const CString &iniFilename);
	static BOOL _FindEquipKind(const CString &iniFilename) { return g_goodsRepoDlg->FindEquipKind(iniFilename); }
	BOOL FindEquipNames(const CString &iniFilename);
	static BOOL _FindEquipNames(const CString &iniFilename) { return g_goodsRepoDlg->FindEquipNames(iniFilename); }
	BOOL FindItemRefs(const CString &iniFilename);
	static BOOL _FindItemRefs(const CString &iniFilename) { return g_goodsRepoDlg->FindItemRefs(iniFilename); }
	void EmptyGoods();
	void DisplayGoodPrices(CGood* good);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GOODSREPODLG_H__40E78ED3_D9BD_4A6C_AC54_5C538A0D35C4__INCLUDED_)
