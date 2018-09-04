// FLCompanionDlg.h : header file
//

#if !defined(AFX_FLCOMPANIONDLG_H__6D763B2E_AAC2_45DD_BA3A_FE9ED46D3937__INCLUDED_)
#define AFX_FLCOMPANIONDLG_H__6D763B2E_AAC2_45DD_BA3A_FE9ED46D3937__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "System.h"

#define WM_RECALC		(WM_APP+2)

/////////////////////////////////////////////////////////////////////////////
// CFLCompanionDlg dialog

class CFLCompanionDlg : public CDialog
{
// Construction
public:
	CFLCompanionDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CFLCompanionDlg)
	enum { IDD = IDD_FLCOMPANION_DIALOG };
	CButton	m_switchBtn;
	CButton	m_jumpsBtn;
	CButton	m_backBtn;
	CButton	m_TRaddBtn;
	CButton	m_TRremBtn;
	CComboBox	m_asteroidsCombo;
	CListCtrl	m_systemWaypoints;
	CComboBox	m_destsystemCombo;
	CComboBox	m_destbaseCombo;
	CListCtrl	m_waypoints;
	CComboBox	m_systemCombo;
	CListCtrl	m_routes;
	CComboBox	m_baseCombo;
	CListCtrl   m_traderoute;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFLCompanionDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CFLCompanionDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangeBaseCombo();
	afx_msg void OnItemclickRoutes(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemactivateRoutes(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemactivateWaypoints(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeSystemCombo();
	afx_msg void OnBack();
	afx_msg void OnItemchangedRoutes(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedWaypoints(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeDestsystemCombo();
	afx_msg void OnSelchangeDestbaseCombo();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBegintrackRoutes(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDumpBaseTimes();
	afx_msg void OnBaseInfo();
	afx_msg void OnGoodsRepo();
	afx_msg void OnAbout();
	afx_msg void OnVisitForum();
	afx_msg void OnVersionHistory();
	afx_msg void OnSetFLDir();
	afx_msg void OnSwitchNicknames();
	afx_msg void OnSpeedDelays();
	afx_msg void OnLimitations();
	afx_msg void OnUpdateNicknames(CCmdUI* pCmdUI);
	afx_msg void OnVisitWebsite();
	afx_msg void OnSystemMap();
	afx_msg void OnMining();
	afx_msg void OnSelchangeAsteroidsCombo();
	afx_msg void OnUpdateGameLaunch(CCmdUI* pCmdUI);
	afx_msg void OnGameLaunch();
	afx_msg void OnModInfo();
	afx_msg void OnUpdateModInfo(CCmdUI* pCmdUI);
	afx_msg void OnJumps();
	afx_msg void OnUpdateGameImport(CCmdUI* pCmdUI);
	afx_msg void OnGameImportAbout();
	afx_msg void OnGameImport(UINT nID);
	afx_msg void OnGameImportCheckall();
	afx_msg void OnUpdateGameImportCheckall(CCmdUI* pCmdUI);
	afx_msg void OnActivateApp(BOOL bActive, DWORD hTask);
	afx_msg void OnSwitch();
	afx_msg void OnTR_Add();
	afx_msg void OnTR_Rem();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu);
	afx_msg LRESULT OnRecalc(WPARAM, LPARAM);
	
public:
	enum {
		RECALC_SOLUTIONS	= 0x01,
		RECALC_PATHS		= 0x02,
	};
	void Recalc(DWORD flags);
	BOOL SetMaxInvestment(UINT investment);
	BOOL SetMaxDistance(UINT distance);
	BOOL SetMinCSU(UINT minCSU);
	BOOL SetCargoSize(UINT cargoSize);
private:
	static int SelComboByData(CComboBox &combo, void *selData);
	void DrawMap(CDC &dc);
	void LoadMaps();
	void MatchGates();
	void CalcDistances();
	void ResetMapZoom();
	void ShowAllSolutions();
	void AddSolutionsForBase(CBase* base);
	void AddSolution(int goodIndex, double destbuy, double srcsell, CBase *srcbase, CBase *destbase, LONG distance);
	void JumptoBase(CBase *base);
	void InitSystemCombos();
	void ImportFromGame();
	void Calc_TotalRow();
public:
	BOOL m_displayNicknames;
	UINT m_cargoSize;
	CString m_version;
	int SelectedItem;
	int g_traderouteTotal;
private:
	HACCEL m_hAccel;
	UINT m_maxInvestment;
	UINT m_maxDistance;
	UINT m_minCSU;
	BOOL m_showAllSolutions;
	CAsteroids* m_curAsteroids;
	CList<CBase*,CBase*> m_history;
	CPoint m_mapMouseCoords;
	int m_mapmax;
	CPoint m_mapOrigin;
	int m_zoom;
	CSystem *m_drawnSystem;
	DWORD m_importFromGame;
	LONG m_recalcFlags;
};

extern CFLCompanionDlg* g_mainDlg;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLCOMPANIONDLG_H__6D763B2E_AAC2_45DD_BA3A_FE9ED46D3937__INCLUDED_)
