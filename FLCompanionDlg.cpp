// FLCompanionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FLCompanion.h"
#include "FLCompanionDlg.h"
#include "ResourceProvider.h"
#include "Datas.h"
#include "SpeedDelaysDlg.h"
#include "LimitationsDlg.h"
#include "SystemMap.h"
#include "BaseInfoDlg.h"
#include "GoodsRepoDlg.h"
#include "DynEconDlg.h"
#include "AboutDlg.h"
#include "MiningBaseDlg.h"
#include "ModInfoDlg.h"
#include <locale>
#include <iostream>
#include <iomanip>

#define M_PI       3.1415926535897932384626433832795L
#define M_2PI      6.283185307179586476925286766559L

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFLCompanionDlg* g_mainDlg;
CBase* g_miningbase;
bool blnMap;
bool blnMultiBaseSolution;
int solutionCount;
int gcx, gcy;
#ifdef _DEBUG
#define ALL_TRADING_ROUTES
#endif

struct space_out : std::numpunct<char> {
	char do_thousands_sep()   const { return ' '; }  // separate with spaces
	std::string do_grouping() const { return "\1"; } // groups of 1 digit
};

/////////////////////////////////////////////////////////////////////////////
// CFLCompanionDlg dialog

CFLCompanionDlg::CFLCompanionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFLCompanionDlg::IDD, pParent), m_mapOrigin(0,0), m_recalcFlags(0)
{
	setlocale(LC_NUMERIC, "");
	//{{AFX_DATA_INIT(CFLCompanionDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = theApp.LoadIcon(IDR_MAINFRAME);
	m_zoom = 1;
	m_mapMouseCoords.x = 0;
	m_cargoSize = theApp.GetProfileInt(L"Settings", L"CargoSize", 1);
	m_maxInvestment = theApp.GetProfileInt(L"Settings", L"MaxInvestment", 0);
	m_maxDistance = theApp.GetProfileInt(L"Settings", L"MaxDistance", 0);
	m_jumptradeTime = theApp.GetProfileInt(L"Settings", L"JumpTradeTime", 0);
	m_minCSU = theApp.GetProfileInt(L"Settings", L"MinCSU", 0);
	m_displayNicknames = FALSE;
	m_showAllSolutions = true;
	g_avoidLockedGates = theApp.GetProfileInt(L"Settings", L"AvoidLockedGates", TRUE);
	g_jumptrade = theApp.GetProfileInt(L"Settings", L"JumpTrade", FALSE);
	g_avoidHoles = theApp.GetProfileInt(L"Settings", L"AvoidHoles", FALSE);
	g_avoidUnstableHoles = theApp.GetProfileInt(L"Settings", L"AvoidUnstableHoles", FALSE);
	g_avoidGates = theApp.GetProfileInt(L"Settings", L"AvoidGates", FALSE);
	g_avoidLanes = theApp.GetProfileInt(L"Settings", L"AvoidLanes", FALSE);
	g_isTransport = theApp.GetProfileInt(L"Settings", L"IsTransport", FALSE);
	ENGINE_SPEED = theApp.GetProfileInt(L"Settings", L"EngineSpeed", 280);
	LANE_SPEED = theApp.GetProfileInt(L"Settings", L"LaneSpeed", 1900);
	JUMP_DELAY = theApp.GetProfileInt(L"Settings", L"JumpDelay", 15)*1000;
	HOLE_DELAY = theApp.GetProfileInt(L"Settings", L"HoleDelay", JUMP_DELAY/1000)*1000;
	LANE_DELAY = theApp.GetProfileInt(L"Settings", L"LaneDelay", 10)*1000;
	BASE_DELAY = theApp.GetProfileInt(L"Settings", L"BaseDelay", 20)*1000;
	m_importFromGame = theApp.GetProfileInt(L"Settings", L"ImportFromGame", 0);
	// ugly replacement code to fetch version:
	LPVOID resVersion = LockResource(LoadResource(NULL, FindResource(NULL, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION)));
	VS_FIXEDFILEINFO* ffi = (VS_FIXEDFILEINFO*)VOID_OFFSET(resVersion, 40);
	m_version.Format(L"%d.%02d", HIWORD(ffi->dwFileVersionMS), LOWORD(ffi->dwFileVersionMS));
}

void CFLCompanionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFLCompanionDlg)
	DDX_Control(pDX, IDC_SWITCH, m_switchBtn);
	DDX_Control(pDX, IDC_TR_ADD, m_TRaddBtn);
	DDX_Control(pDX, IDC_TR_REM, m_TRremBtn);
	DDX_Control(pDX, IDC_JUMPS, m_jumpsBtn);
	DDX_Control(pDX, IDC_OPENMAP, m_mapBtn);
	DDX_Control(pDX, IDC_BACK, m_backBtn);
	DDX_Control(pDX, IDC_ASTEROIDS_COMBO, m_asteroidsCombo);
	DDX_Control(pDX, IDC_SYSTEM_WAYPOINTS, m_systemWaypoints);
	DDX_Control(pDX, IDC_DESTSYSTEM_COMBO, m_destsystemCombo);
	DDX_Control(pDX, IDC_DESTBASE_COMBO, m_destbaseCombo);
	DDX_Control(pDX, IDC_WAYPOINTS, m_waypoints);
	DDX_Control(pDX, IDC_SYSTEM_COMBO, m_systemCombo);
	DDX_Control(pDX, IDC_ROUTES, m_routes);
	DDX_Control(pDX, IDC_BASE_COMBO, m_baseCombo);
	DDX_Control(pDX, IDC_TRADEROUTES, m_traderoute);
	DDX_Control(pDX, IDC_SRCDEST_SWITCH, m_SrcDestSwitch);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFLCompanionDlg, CDialog)
	//{{AFX_MSG_MAP(CFLCompanionDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_BASE_COMBO, OnSelchangeBaseCombo)
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnItemclickRoutes)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_ROUTES, OnItemactivateRoutes)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_WAYPOINTS, OnItemactivateWaypoints)
	ON_CBN_SELCHANGE(IDC_SYSTEM_COMBO, OnSelchangeSystemCombo)
	ON_CBN_SELCHANGE(IDC_SRCDEST_SWITCH, OnSelchangeSrcDestCombo)
	ON_BN_CLICKED(IDC_BACK, OnBack)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ROUTES, OnItemchangedRoutes)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_TRADEROUTES, OnTRItemchangedRoutes)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_WAYPOINTS, OnItemchangedWaypoints)
	ON_CBN_SELCHANGE(IDC_DESTSYSTEM_COMBO, OnSelchangeDestsystemCombo)
	ON_CBN_SELCHANGE(IDC_DESTBASE_COMBO, OnSelchangeDestbaseCombo)
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_WINDOWPOSCHANGED()
	ON_NOTIFY(HDN_BEGINTRACK, 0, OnBegintrackRoutes)
	ON_COMMAND(ID_DUMP_BASE_TIMES, OnDumpBaseTimes)
	ON_COMMAND(ID_DUMP_SOLUTIONS, OnDumpSolutions)
	ON_COMMAND(ID_BASE_INFO, OnBaseInfo)
	ON_COMMAND(ID_GOODS_REPO, OnGoodsRepo)
	ON_COMMAND(ID_ABOUT, OnAbout)
	ON_COMMAND(ID_VISIT_FORUM, OnVisitForum)
	ON_COMMAND(ID_VERSION_HISTORY, OnVersionHistory)
	ON_COMMAND(ID_SET_FL_DIR, OnSetFLDir)
	ON_COMMAND(ID_NICKNAMES, OnSwitchNicknames)
	ON_COMMAND(ID_SPEEDDELAYS, OnSpeedDelays)
	ON_COMMAND(ID_LIMITATIONS, OnLimitations)
	ON_UPDATE_COMMAND_UI(ID_NICKNAMES, OnUpdateNicknames)
	ON_COMMAND(ID_VISIT_WEBSITE, OnVisitWebsite)
	ON_COMMAND(ID_SYSTEM_MAP, OnSystemMap)
	ON_COMMAND(ID_CLIENTREFRESH,OnRefreshClient)
	ON_COMMAND(ID_VIEW_LOG, ShowHideLog)
	ON_BN_CLICKED(IDC_MINING, OnMining)
	ON_CBN_SELCHANGE(IDC_ASTEROIDS_COMBO, OnSelchangeAsteroidsCombo)
	ON_UPDATE_COMMAND_UI(ID_GAME_LAUNCH, OnUpdateGameLaunch)
	ON_COMMAND(ID_GAME_LAUNCH, OnGameLaunch)
	ON_COMMAND(ID_MOD_INFO, OnModInfo)
	ON_UPDATE_COMMAND_UI(ID_MOD_INFO, OnUpdateModInfo)
	ON_BN_CLICKED(IDC_JUMPS, OnJumps)
	ON_BN_CLICKED(IDC_OPENMAP, OnMap)
	ON_UPDATE_COMMAND_UI_RANGE(ID_GAME_IMPORT_PRICES, ID_GAME_IMPORT_CARGOHOLD, OnUpdateGameImport)
	ON_COMMAND(ID_GAME_IMPORT_ABOUT, OnGameImportAbout)
	ON_COMMAND_RANGE(ID_GAME_IMPORT_PRICES, ID_GAME_IMPORT_CARGOHOLD, OnGameImport)
	ON_COMMAND(ID_GAME_IMPORT_CHECKALL, OnGameImportCheckall)
	ON_UPDATE_COMMAND_UI(ID_GAME_IMPORT_CHECKALL, OnUpdateGameImportCheckall)
	ON_WM_ACTIVATEAPP()
	ON_BN_CLICKED(IDC_SWITCH, OnSwitch)
	ON_BN_CLICKED(IDC_TR_ADD, OnTR_Add)
	ON_BN_CLICKED(IDC_TR_REM, OnTR_Rem)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_RECALC, OnRecalc)
	ON_WM_INITMENUPOPUP()
END_MESSAGE_MAP()




/////////////////////////////////////////////////////////////////////////////
// CFLCompanionDlg message handlers

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CFLCompanionDlg::OnSize(UINT nType, int cx, int cy)
{
	RECT rect;
	double routes_heightmultiplier = 0.35; // 0.49
	double left_widthmultiplier = blnMap ?  0.65 : 0.992;
	double left_min = 710;
	gcx = cx;
	gcy = cy;
	GetDlgItem(IDC_ROUTES)->SetWindowPos(NULL, 5, 30, max(cx*left_widthmultiplier, left_min), cy*routes_heightmultiplier, SWP_NOACTIVATE | SWP_NOZORDER); //cx*0.63
	GetDlgItem(IDC_ROUTES)->GetClientRect(&rect);

	GetDlgItem(IDC_TRADEROUTES)->SetWindowPos(NULL, 5, cy*routes_heightmultiplier + 55, max(cx*left_widthmultiplier, left_min), cy - ((cy*routes_heightmultiplier + 100) + (cy - (cy*routes_heightmultiplier + 225))) + +(cy - (cy*routes_heightmultiplier + 205 + min(cy - (cy*routes_heightmultiplier + 205), 225))), SWP_NOACTIVATE | SWP_NOZORDER); //cx*0.63
	GetDlgItem(IDC_TR_ADD)->SetWindowPos(NULL, 600, cy*routes_heightmultiplier + 32, 24, 20, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_TR_REM)->SetWindowPos(NULL, 623, cy*routes_heightmultiplier + 32, 24, 20, SWP_NOACTIVATE | SWP_NOZORDER);

	GetDlgItem(IDC_WAYPOINTS)->SetWindowPos(NULL, 5, cy*routes_heightmultiplier + 205 + (cy - (cy*routes_heightmultiplier + 205 + min(cy - (cy*routes_heightmultiplier + 205), 225))), 618, min(cy - (cy*routes_heightmultiplier + 205), 225), SWP_NOACTIVATE | SWP_NOZORDER); //  cx*0.58

	GetDlgItem(IDC_BUY_PRICE)->SetWindowPos(NULL, 10, cy*routes_heightmultiplier + 32, 170, 13, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_SELL_PRICE)->SetWindowPos(NULL, 425, cy*routes_heightmultiplier + 32, 170, 13, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_PERISHABLE)->SetWindowPos(NULL, 290, cy*routes_heightmultiplier + 32, 65, 13, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_MAP)->SetWindowPos(NULL, max((cx*left_widthmultiplier) + 10, left_min + 10), 30, cx - ((cx*left_widthmultiplier)+15), min(cy-75,cx - ((cx*left_widthmultiplier) + 15)), SWP_NOACTIVATE | SWP_NOZORDER);
	//GetDlgItem(IDC_MAP)->GetWindowRect(&rect);
	GetDlgItem(IDC_MINING_LABEL)->SetWindowPos(NULL, max((cx*left_widthmultiplier) + 10, left_min + 10), min(cy - 75, cx - ((cx*left_widthmultiplier) + 15)) + 55, 75, 13, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_ASTEROIDS_COMBO)->SetWindowPos(NULL, max((cx*left_widthmultiplier) + 10, left_min + 10) + 75, min(cy - 75, cx - ((cx*left_widthmultiplier) + 15)) + 50, 155, 13, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_MINING)->SetWindowPos(NULL, max((cx*left_widthmultiplier) + 10, left_min + 10) + 250, min(cy - 75, cx - ((cx*left_widthmultiplier) + 15)) + 50, 75, 18, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_MAPINFO)->SetWindowPos(NULL, max((cx*left_widthmultiplier) + 10, left_min + 10) + 25, min(cy - 75, cx - ((cx*left_widthmultiplier) + 15)) + 35, blnMap ? 355 : 0 , 13, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_MAPNAME)->SetWindowPos(NULL, max((cx*left_widthmultiplier) + 10, left_min + 10) + ((cx - ((cx*left_widthmultiplier) + 355) )/2), 10, blnMap ? 355 : 0, 13, SWP_NOACTIVATE);
	//GetDlgItem(IDC_MAPINFO)->SetWindowPos(NULL,)

		GetDlgItem(IDC_MAPINFO)->ShowWindow(blnMap ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_MAPNAME)->ShowWindow(blnMap ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_MINING)->ShowWindow(blnMap ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_ASTEROIDS_COMBO)->ShowWindow(blnMap ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_MINING_LABEL)->ShowWindow(blnMap ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_JUMPS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_OPENMAP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BACK)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_DESTSYSTEM_LABEL)->SetWindowPos(NULL, 60, cy*routes_heightmultiplier + 184 + (cy - (cy*routes_heightmultiplier + 205 + min(cy - (cy*routes_heightmultiplier + 205), 225))), 75, 13, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_DESTBASE_LABEL)->SetWindowPos(NULL, 265, cy*routes_heightmultiplier + 184 + (cy - (cy*routes_heightmultiplier + 205 + min(cy - (cy*routes_heightmultiplier + 205), 225))), 35, 13, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_DESTSYSTEM_COMBO)->SetWindowPos(NULL, 135, cy*routes_heightmultiplier + 182 + (cy - (cy*routes_heightmultiplier + 205 + min(cy - (cy*routes_heightmultiplier + 205), 225))), 128, 13, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_DESTBASE_COMBO)->SetWindowPos(NULL, 300, cy*routes_heightmultiplier + 182 + (cy - (cy*routes_heightmultiplier + 205 + min(cy - (cy*routes_heightmultiplier + 205), 225))), 203, 13, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_DESTFACTION)->SetWindowPos(NULL, 510, cy*routes_heightmultiplier + 184 + (cy - (cy*routes_heightmultiplier + 205 + min(cy - (cy*routes_heightmultiplier + 205), 225))), 150, 13, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_SWITCH)->SetWindowPos(NULL, 668, cy*routes_heightmultiplier + 182 + (cy - (cy*routes_heightmultiplier + 205 + min(cy - (cy*routes_heightmultiplier + 205), 225))), 28, 20, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_OPENMAP)->SetWindowPos(NULL, max((cx*left_widthmultiplier) + 10, left_min + 10) - 25, 8, 20, 20, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_JUMPS)->SetWindowPos(NULL, max((cx*left_widthmultiplier) + 10, left_min + 10), 8, 20, 20, SWP_NOACTIVATE | SWP_NOZORDER);
	GetDlgItem(IDC_JUMPS)->ShowWindow(blnMap ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_OPENMAP)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BACK)->ShowWindow(SW_SHOW);
	ResetMapZoom();
}

void CFLCompanionDlg::ShowHideLog()
{
	if (g_logDlg.IsWindowVisible())
	{
		theApp.WriteProfileInt(L"Settings", L"LogMinimize", TRUE);
		g_logDlg.ShowWindow(SW_HIDE);
	}
	else
	{
		theApp.WriteProfileInt(L"Settings", L"LogMinimize", FALSE);
		g_logDlg.ShowWindow(SW_SHOW);
	}

}

void CFLCompanionDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	if (IsIconic())
	{
		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
		DrawMap(dc);
	}
}

// from mk:@MSITStore:C:\Program%20Files\Microsoft%20Visual%20Studio\MSDN\2001OCT\1033\kbvc.chm::/Source/visualc/q242577.htm
void CFLCompanionDlg::OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu)
{
    ASSERT(pPopupMenu != NULL);
    // Check the enabled state of various menu items.

    CCmdUI state;
    state.m_pMenu = pPopupMenu;
    ASSERT(state.m_pOther == NULL);
    ASSERT(state.m_pParentMenu == NULL);

    // Determine if menu is popup in top-level menu and set m_pOther to
    // it if so (m_pParentMenu == NULL indicates that it is secondary popup).
    HMENU hParentMenu;
    if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
        state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
    else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
    {
        CWnd* pParent = this;
           // Child windows don't have menus--need to go to the top!
        if (pParent != NULL &&
           (hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
        {
           int nIndexMax = ::GetMenuItemCount(hParentMenu);
           for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
           {
            if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
            {
                // When popup is found, m_pParentMenu is containing menu.
                state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
                break;
            }
           }
        }
    }

    state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
    for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
      state.m_nIndex++)
    {
        state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
        if (state.m_nID == 0)
           continue; // Menu separator or invalid cmd - ignore it.

        ASSERT(state.m_pOther == NULL);
        ASSERT(state.m_pMenu != NULL);
        if (state.m_nID == (UINT)-1)
        {
           // Possibly a popup menu, route to first item of that popup.
           state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
           if (state.m_pSubMenu == NULL ||
            (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
            state.m_nID == (UINT)-1)
           {
            continue;       // First item of popup can't be routed to.
           }
           state.DoUpdate(this, TRUE);   // Popups are never auto disabled.
        }
        else
        {
           // Normal menu item.
           // Auto enable/disable if frame window has m_bAutoMenuEnable
           // set and command is _not_ a system command.
           state.m_pSubMenu = NULL;
           state.DoUpdate(this, FALSE);
        }

        // Adjust for menu deletions and additions.
        UINT nCount = pPopupMenu->GetMenuItemCount();
        if (nCount < state.m_nIndexMax)
        {
           state.m_nIndex -= (state.m_nIndexMax - nCount);
           while (state.m_nIndex < nCount &&
            pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
           {
            state.m_nIndex++;
           }
        }
        state.m_nIndexMax = nCount;
    }
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFLCompanionDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CFLCompanionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_mainDlg = this;
	CString title;
	GetWindowText(title);
	title.Format(title + '\0', m_version);
	SetWindowText(title);
	if (!LoadAppDatas(this))
		return FALSE;
	Recalc(RECALC_PATHS);

	m_hAccel = LoadAccelerators(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_ACCELERATOR));
	
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	m_backBtn.SetIcon  ((HICON) LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_BACK_ARROW),IMAGE_ICON,16,16,LR_SHARED));
	m_jumpsBtn.SetIcon ((HICON) LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_JUMP),IMAGE_ICON,16,16,LR_SHARED));
	m_mapBtn.SetIcon   ((HICON) LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_MAP), IMAGE_ICON, 16, 16, LR_SHARED));
	m_switchBtn.SetIcon((HICON) LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_SWITCH_ARROW),IMAGE_ICON,16,16,LR_SHARED));
	m_TRaddBtn.SetIcon ((HICON) LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_DOWN_ARROW), IMAGE_ICON, 16, 16, LR_SHARED));
	m_TRremBtn.SetIcon ((HICON) LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_UP_ARROW), IMAGE_ICON, 16, 16, LR_SHARED));

	m_routes.InsertColumn(0, L"Commodity", LVCFMT_LEFT, 125);
	m_routes.InsertColumn(1, L"From", LVCFMT_RIGHT, 0);			// "From" base caption
	m_routes.InsertColumn(2, L"To", LVCFMT_LEFT, 220);
	m_routes.InsertColumn(3, L"Profit", LVCFMT_RIGHT, 70);
	m_routes.InsertColumn(4, L"Distance", LVCFMT_RIGHT, 80);
	m_routes.InsertColumn(5, L"Profit/Distance", LVCFMT_RIGHT, 90);
	m_routes.InsertColumn(8, L"CSU", LVCFMT_CENTER, 75);
	m_routes.InsertColumn(6, NULL, LVCFMT_RIGHT, 0);			// Good index
	m_routes.InsertColumn(7, NULL, LVCFMT_RIGHT, 0);			// "From" base pointer
	INT order[] = { 6, 7, 1, 0, 2, 3, 4, 5, 8 }; // place 0-width columns at the beginning (so it doesn't disturb dividers dragging)
	m_routes.SetColumnOrderArray(_countof(order), order);
	m_routes.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT|LVS_EX_UNDERLINEHOT);

	m_traderoute.InsertColumn(0, L"Commodity", LVCFMT_LEFT, 125);
	m_traderoute.InsertColumn(1, L"From", LVCFMT_RIGHT, 220);			// "From" base caption
	m_traderoute.InsertColumn(2, L"To", LVCFMT_LEFT, 220);
	m_traderoute.InsertColumn(3, L"Profit", LVCFMT_RIGHT, 70);
	m_traderoute.InsertColumn(4, L"Distance", LVCFMT_RIGHT, 80);
	m_traderoute.InsertColumn(5, L"Profit/Distance", LVCFMT_RIGHT, 90);
	m_traderoute.InsertColumn(8, L"CSU", LVCFMT_CENTER, 75);
	m_traderoute.InsertColumn(6, NULL, LVCFMT_RIGHT, 0);			// Good index
	m_traderoute.InsertColumn(7, NULL, LVCFMT_RIGHT, 0);			// "From" base pointer
	m_traderoute.SetColumnOrderArray(_countof(order), order);
	m_traderoute.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT);

	m_waypoints.InsertColumn(0, L"System", LVCFMT_LEFT, 125);
	m_waypoints.InsertColumn(1, L"Route", LVCFMT_LEFT, 220);
	m_waypoints.InsertColumn(2, L"Time", LVCFMT_RIGHT, 80);
	m_waypoints.InsertColumn(3, L"Av.Speed", LVCFMT_RIGHT, 62);
	m_waypoints.InsertColumn(4, L"Location", LVCFMT_LEFT, 105);
	m_waypoints.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	m_systemWaypoints.InsertColumn(0, L"Route steps", LVCFMT_LEFT, 200);
	m_systemWaypoints.InsertColumn(1, L"Location", LVCFMT_RIGHT, 80);
	m_systemWaypoints.InsertColumn(2, L"Distance", LVCFMT_RIGHT, 80);
	m_systemWaypoints.InsertColumn(3, L"Speed", LVCFMT_RIGHT, 70);
	m_systemWaypoints.InsertColumn(4, L"Time", LVCFMT_RIGHT, 80);
	m_systemWaypoints.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	m_systemCombo.SetDroppedWidth(m_systemCombo.GetDroppedWidth()*3/2);
	m_baseCombo.SetDroppedWidth(m_baseCombo.GetDroppedWidth()*3/2);
	m_destsystemCombo.SetDroppedWidth(m_destsystemCombo.GetDroppedWidth()*3/2);
	m_destbaseCombo.SetDroppedWidth(m_destbaseCombo.GetDroppedWidth()*3/2);

	m_SrcDestSwitch.InsertString(0, L"Source");
	m_SrcDestSwitch.InsertString(1, L"Destination");
	m_SrcDestSwitch.SetCurSel(0);

	InitSystemCombos();
	CSystem* system;
	if (g_systemsByNick.Lookup(theApp.GetProfileString(L"Settings", L"SystemCombo", L""), system))
		SelComboByData(m_systemCombo, system);
	OnSelchangeSystemCombo();
	CBase* base;
	if (g_basesByNick.Lookup(theApp.GetProfileString(L"Settings", L"BaseCombo", L""), base))
		SelComboByData(m_baseCombo, base);
	m_baseCombo.SetFocus();

	if (g_logDlg.m_hWnd && (theApp.GetProfileInt(L"Settings", L"LogMinimize", FALSE) == 1))
		g_logDlg.ShowWindow(SW_HIDE);

	if (g_modInfo.largeMod && !g_modInfo.FLC_info.IsEmpty())
	{
		DWORD hash = FLHash(g_modInfo.FLC_info);
		if (hash != theApp.GetProfileInt(L"Settings", L"DisplayModInfo", 0))
			PostMessage(WM_COMMAND, ID_MOD_INFO);
	}
	ImportFromGame();
	return FALSE;  // return TRUE  unless you set the focus to a control
}

void CFLCompanionDlg::InitSystemCombos()
{
	m_systemCombo.ResetContent();
	int nIndex;
	nIndex = m_systemCombo.AddString(L" (All systems)");
#ifdef ALL_TRADING_ROUTES
	nIndex = m_systemCombo.AddString(L" (All trading routes)");
	m_systemCombo.SetItemData(nIndex, 2);
#endif
	m_destsystemCombo.ResetContent();
	m_destsystemCombo.AddString(L" (All systems)");
	for (int index = 0; index < SYSTEMS_COUNT; index++)
	{
		CSystem &system = g_systems[index];
		//if (!system.m_hasSell) continue;
		CString caption = m_displayNicknames ? system.m_nickname : system.m_caption;
		caption.TrimLeft();
		nIndex = m_systemCombo.AddString(caption);
		m_systemCombo.SetItemDataPtr(nIndex, &system);
		nIndex = m_destsystemCombo.AddString(caption);
		m_destsystemCombo.SetItemDataPtr(nIndex, &system);
	}
}

int CFLCompanionDlg::SelComboByData(CComboBox &combo, void *selData)
{
	if (selData)
	{
		int nIndex = 0;
		void *data;
		while ((data = combo.GetItemDataPtr(nIndex)) != (void*) CB_ERR)
		{
			if (data == selData)
			{
				combo.SetCurSel(nIndex);
				return nIndex;
			}
			nIndex++;
		}
	}
	return CB_ERR;
}

void CFLCompanionDlg::OnSystemMap() 
{
	CSystemMap systemMap;
	systemMap.m_displayNicknames = m_displayNicknames;
	int nIndex = m_baseCombo.GetCurSel();
	if (nIndex != CB_ERR)
		systemMap.m_system = reinterpret_cast<CBase*>(m_baseCombo.GetItemDataPtr(nIndex))->m_system;
	else
	{
		nIndex = m_systemCombo.GetCurSel();
		if (nIndex != CB_ERR)
			systemMap.m_system = reinterpret_cast<CSystem*>(m_systemCombo.GetItemDataPtr(nIndex));
	}
	nIndex = m_destbaseCombo.GetCurSel();
	if (nIndex != CB_ERR)
		systemMap.m_destsystem = reinterpret_cast<CBase*>(m_destbaseCombo.GetItemDataPtr(nIndex))->m_system;
	else
	{
		nIndex = m_destsystemCombo.GetCurSel();
		if (nIndex != CB_ERR)
			systemMap.m_destsystem = reinterpret_cast<CSystem*>(m_destsystemCombo.GetItemDataPtr(nIndex));
	}
	int result = systemMap.DoModal();
	if (result == 100)
	{
		SelComboByData(m_systemCombo,systemMap.m_system);
		OnSelchangeSystemCombo();
	}
	else if (result == 101)
	{
		SelComboByData(m_destsystemCombo,systemMap.m_system);
		OnSelchangeDestsystemCombo();
	}
}

void CFLCompanionDlg::OnSelchangeSrcDestCombo()
{
	INT destorder[] = { 6, 7, 1, 0, 2, 3, 4, 5, 8 }; // place 0-width columns at the beginning (so it doesn't disturb dividers dragging)
	INT srcorder[] = { 6, 7, 2, 0, 1, 3, 4, 5, 8 }; // place 0-width columns at the beginning (so it doesn't disturb dividers dragging)
	switch (m_SrcDestSwitch.GetCurSel())
	{
	case 0:
		SetDlgItemText(IDC_DESTSYSTEM_LABEL, L"Destination: ");
		m_routes.SetColumnOrderArray(_countof(destorder), destorder);
		m_routes.SetColumnWidth(2, 220);
		m_routes.SetColumnWidth(1, 0);
		OnSelchangeBaseCombo();
		break;
	case 1:
		SetDlgItemText(IDC_DESTSYSTEM_LABEL, L"Source: ");
		m_routes.SetColumnOrderArray(_countof(srcorder), srcorder);
		m_routes.SetColumnWidth(2, 0);
		m_routes.SetColumnWidth(1, 220);
		OnSelchangeBaseCombo();
		break;
	}
}

void CFLCompanionDlg::OnSelchangeSystemCombo() 
{
	int nIndex = m_systemCombo.GetCurSel();
#ifdef ALL_TRADING_ROUTES
reload:
#endif
	if (nIndex == CB_ERR)
		nIndex = m_systemCombo.SelectString(-1, L" (All systems)");
	CSystem *system = reinterpret_cast<CSystem*>(m_systemCombo.GetItemDataPtr(nIndex));

	nIndex = m_baseCombo.GetCurSel();
	CBase *base = (nIndex == CB_ERR) ? NULL : (CBase*) m_baseCombo.GetItemDataPtr(nIndex);

	CString nickname;
	m_baseCombo.ResetContent();

	m_showAllSolutions = (system == (CSystem*) 2); // (All routes)
	theApp.WriteProfileString(L"Settings", L"SystemCombo", system >= g_systems ? system->m_nickname : L"");
#ifdef ALL_TRADING_ROUTES
	if (m_showAllSolutions)
	{
		if (AfxMessageBox(L"Showing all trading routes can take some time to calculate!\r\n\r\nAre you sure?", MB_YESNO|MB_ICONEXCLAMATION) == IDNO)
		{
			nIndex = CB_ERR;
			goto reload;
		}
		system = NULL;
		m_routes.SetColumnWidth(1, 220);			// "From" base caption
	}
	else
	{
		m_routes.SetColumnWidth(1, 0);			// "From" base caption
	}
#endif
	
	for (UINT index = 0; index < BASES_COUNT; index++)
	{
		CBase &base = g_bases[index];
		//if (!base.m_hasSell) continue;
		if (system == NULL)
			if (m_displayNicknames)
				nIndex = m_baseCombo.AddString(base.m_caption+L" ("+base.m_nickname+L')');
			else 
				nIndex = m_baseCombo.AddString(base.m_caption+L" ("+base.m_system->m_caption+L')');
		else if (base.m_system == system)
			if (m_displayNicknames)
				nIndex = m_baseCombo.AddString(base.m_caption+L" ("+base.m_nickname+L')');
			else 
				nIndex = m_baseCombo.AddString(m_displayNicknames ? base.m_nickname : base.m_caption);
		else
			continue;
		m_baseCombo.SetItemDataPtr(nIndex, &base);
	}

	if (SelComboByData(m_baseCombo, base) == CB_ERR)
		m_baseCombo.SetCurSel(0);

#ifdef ALL_TRADING_ROUTES
	if (m_showAllSolutions)
		ShowAllSolutions();
	else
#endif
		Recalc(RECALC_SOLUTIONS);
}

void CFLCompanionDlg::AddSolution(int goodIndex, double destbuy, double srcsell, CBase *srcbase, CBase *destbase, LONG distance)
{
	LONG profit;
	UINT units = m_cargoSize == 1 ? 1 : UINT(m_cargoSize/g_goods[goodIndex].m_volume);
	if (units == 0)
		return;
	if ((m_maxInvestment > 0) && (srcsell*units > m_maxInvestment))
		units = UINT(m_maxInvestment/srcsell);
	if (g_goods[goodIndex].m_decay_time == 0)
		profit = UINT(destbuy-srcsell)*units;
	else
	{ // lets compute the effect of decaying goods:
		UINT decay_units = distance/g_goods[goodIndex].m_decay_time;
		if (m_cargoSize != 1)
		{
			if (decay_units > units)
				return; // all cargo would have decayed, so drop this solution
			profit = INT(destbuy*(units-decay_units)-srcsell*units);
		}
		else // user wants a price per cargo unit => we evaluate an average profit per unit, based on a cargo of 100 units
		{
			if (decay_units > 100)
				return;
			profit = INT(destbuy-srcsell-destbuy*decay_units/100.0);
		}
		if (profit < 0)
			return; // negative profit if decay is taken in account, drop it
	}
	if (distance == 0)
		return;
	if (((profit / units) * 100000 / distance) < m_minCSU && destbase!=g_miningbase && goodIndex!=0)
		return;
	int nItem = m_routes.InsertItem(MAXLONG, m_displayNicknames ? g_goods[goodIndex].m_nickname : g_goods[goodIndex].m_caption);
//#ifdef ALL_TRADING_ROUTES
	if (m_displayNicknames)
		m_routes.SetItemText(nItem, 1, srcbase->m_system->m_nickname+": "+srcbase->m_nickname);
	else
		m_routes.SetItemText(nItem, 1, srcbase->m_system->m_caption+": "+srcbase->m_caption);
//#endif
	if (m_displayNicknames)
		m_routes.SetItemText(nItem, 2, destbase->m_system->m_nickname+": "+destbase->m_nickname);
	else
		m_routes.SetItemText(nItem, 2, destbase->m_system->m_caption+": "+destbase->m_caption);
	TCHAR buf[32];
	_stprintf(buf, L"$%d", profit);
	m_routes.SetItemText(nItem, 3, buf);
	m_routes.SetItemText(nItem, 4, MinuteSeconds(distance, true));
	if (m_cargoSize == 1)
	{
		CString ratio = DoubleToString((profit / g_goods[goodIndex].m_volume) * 100000 / distance);
		int index = ratio.Find('.');
		if (index > 0) ratio = ratio.Left(index + 3);
		_stprintf(buf, L"%s ¢/sec", LPCTSTR(ratio));
	}
	else
	{
		CString ratio = DoubleToString(profit*1000.0/distance);
		int index = ratio.Find('.');
		if (index > 0) ratio = ratio.Left(index+3);
		_stprintf(buf, L"%s $/sec", LPCTSTR(ratio));
	}
	m_routes.SetItemText(nItem, 5, buf);

	CString ratio = DoubleToString((profit / units / g_goods[goodIndex].m_volume) * 100000 / distance);
	int index = ratio.Find('.');
	if (index > 0) ratio = ratio.Left(index + 3);
	_stprintf(buf, L"%s ¢/sec", LPCTSTR(ratio));

	//_stprintf(buf, L"%d ¢/sec", "");
	m_routes.SetItemText(nItem, 8, buf);
	_stprintf(buf, L"%d", goodIndex);
	m_routes.SetItemText(nItem, 6, buf);
	_stprintf(buf, L"%d", (int) srcbase);
	m_routes.SetItemText(nItem, 7, buf);
	m_routes.SetItemData(nItem, (DWORD) destbase);
	solutionCount++;
}

void CFLCompanionDlg::AddSolutionsForBase(CBase* base)
{
	if (m_traderoute.GetItemCount() != 0) 
// && m_traderoute.GetItemText(0, 1) == m_traderoute.GetItemText(m_traderoute.GetItemCount() - 1, 2))
	{
		//Add the Return with No Commodity solution as the first in the list.
		CBase *destbase = (CBase*)_ttoi(m_traderoute.GetItemText(0, 7));
		AddSolution(0, 0, 0, base, destbase, base->m_distanceToBase[destbase - g_bases] + base->GetDockingDelay());
	}
	// build up the buying price array
	float *sell = base->m_sell;
	if (!blnMultiBaseSolution)
		solutionCount = 0;
	for (int index = 0; index < SYSTEMS_COUNT; index++)
	{
		CSystem *system = &g_systems[index];
		if (system->m_avoid || base->m_faction->m_avoid == true)
			continue;
		// scan all bases in this system
		POSITION pos = system->m_bases.GetHeadPosition();
		while (pos)
		{
			CBase *destbase = system->m_bases.GetNext(pos);
			if (!destbase->m_hasSell)
				continue; // not interesting to go to bases that doesn't sell anything
			if (base->m_shortestPath[destbase-g_bases] == NULL)
				continue; // no route to destination;
			if (m_maxDistance && (base->m_distanceToBase[destbase-g_bases] >= m_maxDistance))
				continue; // route too distant
			if (m_SrcDestSwitch.GetCurSel() == 1 && destbase != (CBase*)m_baseCombo.GetItemDataPtr(m_baseCombo.GetCurSel()))
				continue;
			// scan the buying prices for this destination base
			for (UINT goodIndex = 0; goodIndex < GOODS_COUNT; goodIndex++)
			{
				if (g_goods[goodIndex].m_avoid)
					continue;
				double destbuy = max(destbase->m_buy[goodIndex], 1);
				double srcsell = max(sell[goodIndex], 1);
				if (destbase->m_buy[goodIndex] > sell[goodIndex]) // destination buying price is higher than initial sell price
					AddSolution(goodIndex, destbuy, srcsell, base, destbase, g_jumptrade ? m_jumptradeTime : 
						base->m_distanceToBase[destbase-g_bases]+base->GetDockingDelay());
						//base.m_system->m_distances[destbase->m_system-g_systems]);
			}
		}
	}
	if(g_miningbase != NULL)
		AddSolution(0, 0, 0, base, g_miningbase, base->m_distanceToBase[g_miningbase-g_bases]+base->GetDockingDelay());
	if(!blnMultiBaseSolution)
		Log(L"Solutions for %s (as source) loaded: %d" , m_displayNicknames ? base->m_nickname : base->m_caption, solutionCount);
}

//#ifdef ALL_TRADING_ROUTES
void CFLCompanionDlg::ShowAllSolutions() 
{
	blnMultiBaseSolution = true;
	solutionCount = 0;
	UpdateData();
	m_waypoints.DeleteAllItems();
	m_systemWaypoints.DeleteAllItems();
	ResetMapZoom();

	m_routes.SetRedraw(FALSE);
	m_routes.DeleteAllItems();
	SetDlgItemText(IDC_BUY_PRICE, L"");
	SetDlgItemText(IDC_SELL_PRICE, L"");
	SetDlgItemText(IDC_PERISHABLE, L"");

	BeginWaitCursor();
	for (int index = 0; index < BASES_COUNT; index++)
	{
		CBase &base = g_bases[index];
		if (!base.m_hasSell || (g_isTransport && base.m_isfreighteronly) || base.m_system->m_avoid) continue;

		AddSolutionsForBase(&base);
	}
	EndWaitCursor();

	HD_NOTIFY hdn;
	hdn.hdr.hwndFrom = m_routes.GetHeaderCtrl()->m_hWnd;
	hdn.iItem = 1;

	if (m_routes.GetColumnWidth(5) > 0)
		ListView_HeaderSort(m_routes, 5, true);
	else
		ListView_HeaderSort(m_routes, 3, true);

	m_routes.SetRedraw();
	PostMessage(WM_COMMAND, MAKELONG(IDC_DESTBASE_COMBO, CBN_SELCHANGE), (LPARAM) m_destbaseCombo.m_hWnd);
	int nIndex = m_baseCombo.GetCurSel();
	CBase *base = (CBase*)m_baseCombo.GetItemDataPtr(nIndex);

	if (m_SrcDestSwitch.GetCurSel() == 1)
		Log(L"Solutions for %s (as destination) loaded: %d", m_displayNicknames ? base->m_nickname : base->m_caption, solutionCount);
	else
		Log(L"All solutions loaded: %d", solutionCount);
	blnMultiBaseSolution = false;
}
//#endif // ALL_TRADING_ROUTES

void CFLCompanionDlg::OnSelchangeBaseCombo() 
{
	UpdateData();
	int nIndex = m_baseCombo.GetCurSel();
	CBase* base = nIndex == CB_ERR ? NULL : (CBase*) m_baseCombo.GetItemDataPtr(nIndex);
	theApp.WriteProfileString(L"Settings", L"BaseCombo", base ? base->m_nickname : L"");
	
	m_waypoints.DeleteAllItems();
	m_systemWaypoints.DeleteAllItems();
	ResetMapZoom();

	m_routes.SetRedraw(FALSE);
	m_routes.DeleteAllItems();
	SetDlgItemText(IDC_BUY_PRICE, L"");
	SetDlgItemText(IDC_SELL_PRICE, L"");
	SetDlgItemText(IDC_PERISHABLE, L"");
	if (base)
	{
		CSystem* system = base->m_system;
		SetDlgItemText(IDC_FACTION, base->m_faction ? (m_displayNicknames ? base->m_faction->m_nickname : base->m_faction->m_caption) : L"");
		//SetDlgItemText(IDC_ROUTE_STATIC, L"Route from "+base->m_caption+L" to...");

		if (m_history.IsEmpty() || m_history.GetTail() != base)
		{
			m_history.AddTail(base);
			if (m_history.GetCount() > 20) m_history.RemoveHead();
		}
		switch (m_SrcDestSwitch.GetCurSel())
		{
		case 0:
			AddSolutionsForBase(base);
			break;
		case 1:
			ShowAllSolutions();
			break;
		}

		HD_NOTIFY hdn;
		hdn.hdr.hwndFrom = m_routes.GetHeaderCtrl()->m_hWnd;
		hdn.iItem = 1;

		if (m_routes.GetColumnWidth(5) > 0)
			ListView_HeaderSort(m_routes, 5, true);
		else
			ListView_HeaderSort(m_routes, 3, true);

		m_asteroidsCombo.ResetContent();
		m_curAsteroids = NULL;
		CString dummy;
		POSITION pos = system->m_asteroids.GetStartPosition();
		while (pos)
		{
			CAsteroids &asteroids = system->m_asteroids.GetNextAssoc(pos, dummy);
			if (asteroids.m_good)
			{
				CString str;
				str.Format(L"~%.4g %s/asteroid", asteroids.m_lootStat, asteroids.m_good->m_caption);
				int nIndex = m_asteroidsCombo.AddString(str);
				m_asteroidsCombo.SetItemDataPtr(nIndex, &asteroids);
			}
		}
		int nIndex = m_asteroidsCombo.GetCount();
		EnableDlgItem(*this, IDC_MINING, nIndex != 0);
		EnableDlgItem(*this, IDC_ASTEROIDS_COMBO, nIndex != 0);
		if (nIndex != 0)
		{
			m_asteroidsCombo.SetCurSel(nIndex-1);
			m_curAsteroids = reinterpret_cast<CAsteroids*>(m_asteroidsCombo.GetItemDataPtr(nIndex-1));
		}
	}
	m_routes.SetRedraw();
	PostMessage(WM_COMMAND, MAKELONG(IDC_DESTBASE_COMBO, CBN_SELCHANGE), (LPARAM) m_destbaseCombo.m_hWnd);
}

void CFLCompanionDlg::ResetMapZoom()
{
	SetDlgItemText(IDC_MAPINFO, L"(click map and drag, or use the mouse wheel to scroll & zoom)");
	::ShowWindow(::GetDlgItem(*this, IDC_MAPINFO), SW_SHOW);
	m_mapOrigin = 0; m_zoom = 1;
	CClientDC dc(this);
	DrawMap(dc);
}

/*
Commodity Profit Distance Profit/Distance To
*/

void CFLCompanionDlg::OnItemclickRoutes(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (pNMHDR->hwndFrom == m_routes.GetHeaderCtrl()->m_hWnd)
		ListView_HeaderSort(pNMHDR, pResult);
}

void CFLCompanionDlg::JumptoBase(CBase *base) 
{
	SelComboByData(m_systemCombo, base->m_system);
	OnSelchangeSystemCombo();
	SelComboByData(m_baseCombo, base);
}

void CFLCompanionDlg::OnItemactivateRoutes(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	switch (m_SrcDestSwitch.GetCurSel())
	{
	case 0:
		JumptoBase((CBase*)m_routes.GetItemData(pNMListView->iItem));
		break;
	case 1:
		JumptoBase((CBase*)_ttoi(m_routes.GetItemText(pNMListView->iItem, 7)));
		break;
	}
	*pResult = 0;
}

void CFLCompanionDlg::OnItemactivateWaypoints(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem == m_waypoints.GetItemCount()-1)
		JumptoBase((CBase*) m_waypoints.GetItemData(pNMListView->iItem));
	*pResult = 0;
}

void CFLCompanionDlg::OnBack() 
{
	if (m_history.GetCount() < 2) return;
	m_history.RemoveTail();
	JumptoBase(m_history.RemoveTail());
}


void CFLCompanionDlg::OnItemchangedRoutes(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;
	if ((pNMListView->uNewState & LVIS_SELECTED) == 0)
		return;
	SelectedItem = pNMListView->iItem;
	CBase *from;
	CBase *destination;
	switch (m_SrcDestSwitch.GetCurSel())
	{
	case 0:
		from = (CBase*)_ttoi(m_routes.GetItemText(pNMListView->iItem, 7));
		destination = (CBase*)m_routes.GetItemData(pNMListView->iItem);
		break;
	case 1:
		destination = (CBase*)_ttoi(m_routes.GetItemText(pNMListView->iItem, 7));
		from = (CBase*)m_routes.GetItemData(pNMListView->iItem);
		break;
	}

#ifdef ALL_TRADING_ROUTES
	if (m_showAllSolutions)
	{
		SelComboByData(m_baseCombo, from);
		SetDlgItemText(IDC_FACTION, from->m_faction ? (m_displayNicknames ? from->m_faction->m_nickname : from->m_faction->m_caption) : L"");
		ResetMapZoom();
	}
#endif
	if (from && destination)
	{
		int goodIndex = _ttoi(m_routes.GetItemText(pNMListView->iItem, 6));
		UINT units = m_cargoSize == 1 ? 1 : UINT(m_cargoSize/g_goods[goodIndex].m_volume);
		if ((m_maxInvestment > 0) && (max(from->m_sell[goodIndex], 1)*units > m_maxInvestment))
			units = UINT(m_maxInvestment/ max(from->m_sell[goodIndex], 1));
		BOOL perishable = (g_goods[goodIndex].m_decay_time != 0);
		SetDlgItemText(IDC_PERISHABLE, perishable ? L"*PERISHABLE*" : L"");
		UINT distance = from->m_distanceToBase[destination-g_bases];
		CString buymsg;
		CString sellmsg;
		UINT decay_units = perishable ? distance/(g_goods[goodIndex].m_decay_time) : 0;
		if (m_cargoSize == 1)
		{
			buymsg.FormatMessage(L"Buy one unit for $%1!d!", (int)(m_SrcDestSwitch.GetCurSel() == 0 ? max(from->m_sell[goodIndex] == FLT_MAX ? 0 : from->m_sell[goodIndex], 0) : max(destination->m_sell[goodIndex] == FLT_MAX ? 0 : destination->m_sell[goodIndex], 0)));
			if (perishable)
			{
				double unit = 1.0 - decay_units / 100.0;
				sellmsg.Format(L"Sell %.3f unit for $%d", unit, (int)(m_SrcDestSwitch.GetCurSel() == 0 ? max(destination->m_buy[goodIndex] == FLT_MAX ? 0 : destination->m_buy[goodIndex], 0) : max(from->m_buy[goodIndex] == FLT_MAX ? 0 : from->m_buy[goodIndex], 0)));
			}
			else
				sellmsg.FormatMessage(L"Sell one unit for $%1!d!", (int)(m_SrcDestSwitch.GetCurSel() == 0 ? max(destination->m_buy[goodIndex] == FLT_MAX ? 0 : destination->m_buy[goodIndex], 0) : max(from->m_buy[goodIndex] == FLT_MAX ? 0 : from->m_buy[goodIndex], 0)));
		}
		else
		{
			buymsg.FormatMessage(L"Buy %1!d! units for $%2!d! each", units, (int)(m_SrcDestSwitch.GetCurSel() == 0 ? max(from->m_sell[goodIndex] == FLT_MAX ? 0 : from->m_sell[goodIndex], 0) : max(destination->m_sell[goodIndex] == FLT_MAX ? 0 : destination->m_sell[goodIndex], 0)));
			sellmsg.FormatMessage(L"Sell %1!d! units for $%2!d! each", units-decay_units, (int)(m_SrcDestSwitch.GetCurSel() == 0 ? max(destination->m_buy[goodIndex] == FLT_MAX ? 0 : destination->m_buy[goodIndex], 0) : max(from->m_buy[goodIndex] == FLT_MAX ? 0 : from->m_buy[goodIndex], 0)));
		}
		SetDlgItemText(IDC_BUY_PRICE, buymsg);
		SetDlgItemText(IDC_SELL_PRICE, sellmsg);
	}

	SelComboByData(m_destsystemCombo, destination->m_system);
	OnSelchangeDestsystemCombo();
	SelComboByData(m_destbaseCombo, destination);
}

void CFLCompanionDlg::OnTRItemchangedRoutes(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;
	if ((pNMListView->uNewState & LVIS_SELECTED) == 0)
		return;
	SelectedItem = pNMListView->iItem;
	CBase *from;
	switch (m_SrcDestSwitch.GetCurSel())
	{
	case 0:
		from = (CBase*)_ttoi(m_routes.GetItemText(pNMListView->iItem, 7));
		break;
	case 1:
		from = (CBase*)m_routes.GetItemData(pNMListView->iItem);
		break;
	}
#ifdef ALL_TRADING_ROUTES
	if (m_showAllSolutions)
	{
		SelComboByData(m_baseCombo, from);
		SetDlgItemText(IDC_FACTION, from->m_faction ? (m_displayNicknames ? from->m_faction->m_nickname : from->m_faction->m_caption) : L"");
		ResetMapZoom();
	}
#endif
	CBase *destination = (CBase*)m_traderoute.GetItemData(pNMListView->iItem);
	if (from && destination)
	{
		int goodIndex = _ttoi(m_traderoute.GetItemText(pNMListView->iItem, 6));
		UINT units = m_cargoSize == 1 ? 1 : UINT(m_cargoSize / g_goods[goodIndex].m_volume);
		if ((m_maxInvestment > 0) && (max(from->m_sell[goodIndex], 1) * units > m_maxInvestment))
			units = UINT(m_maxInvestment / max(from->m_sell[goodIndex], 1));
		BOOL perishable = (g_goods[goodIndex].m_decay_time != 0);
		SetDlgItemText(IDC_PERISHABLE, perishable ? L"*PERISHABLE*" : L"");
		UINT distance = from->m_distanceToBase[destination - g_bases];
		CString msg;
		if (m_cargoSize == 1)
			msg.FormatMessage(L"Buy one unit for $%1!d!", (int)max(from->m_sell[goodIndex], 1));
		else
			msg.FormatMessage(L"Buy %1!d! units for $%2!d! each", units, (int)max(from->m_sell[goodIndex], 1));
		SetDlgItemText(IDC_BUY_PRICE, msg);
		UINT decay_units = perishable ? distance / (g_goods[goodIndex].m_decay_time) : 0;
		if (m_cargoSize == 1)
			if (perishable)
			{
				double unit = 1.0 - decay_units / 100.0;
				msg.Format(L"Sell %.3f unit for $%d", unit, (int)destination->m_buy[goodIndex]);
			}
			else
				msg.FormatMessage(L"Sell one unit for $%1!d!", (int)destination->m_buy[goodIndex]);
		else
		{
			msg.FormatMessage(L"Sell %1!d! units for $%2!d! each", units - decay_units, (int)destination->m_buy[goodIndex]);
		}
		SetDlgItemText(IDC_SELL_PRICE, msg);
		SelComboByData(m_systemCombo, from->m_system);
		OnSelchangeSystemCombo();
		SelComboByData(m_baseCombo, from);
		SelComboByData(m_destsystemCombo, destination->m_system);
		OnSelchangeDestsystemCombo();
		SelComboByData(m_destbaseCombo, destination);
	}
}

void CFLCompanionDlg::OnSelchangeDestsystemCombo() 
{
	int nIndex = m_destsystemCombo.GetCurSel();
	if (nIndex == CB_ERR)
		nIndex = m_destsystemCombo.SelectString(0, L" (All systems)");
	CSystem *system = reinterpret_cast<CSystem*>(m_destsystemCombo.GetItemDataPtr(nIndex));
	
	nIndex = m_destbaseCombo.GetCurSel();
	CBase *base = (nIndex == CB_ERR) ? NULL : (CBase*)m_destbaseCombo.GetItemDataPtr(nIndex);
	nIndex = m_destbaseCombo.GetCurSel();

	CString nickname;
	m_destbaseCombo.ResetContent();
	for (UINT index = 0; index < BASES_COUNT; index++)
	{
		CBase &base = g_bases[index];
		//if (!base.m_hasSell) continue;
		if (system == NULL)
			if (m_displayNicknames)
				nIndex = m_destbaseCombo.AddString(base.m_caption+L" ("+base.m_nickname+L')');
			else 
				nIndex = m_destbaseCombo.AddString(base.m_caption+L" ("+base.m_system->m_caption+L')');
		else if (base.m_system == system)
			if (m_displayNicknames)
				nIndex = m_destbaseCombo.AddString(base.m_caption+L" ("+base.m_nickname+L')');
			else 
				nIndex = m_destbaseCombo.AddString(m_displayNicknames ? base.m_nickname : base.m_caption);
		else
			continue;
		m_destbaseCombo.SetItemDataPtr(nIndex, &base);
	}

	if (SelComboByData(m_destbaseCombo, base) == CB_ERR)
		m_destbaseCombo.SetCurSel(0);
	PostMessage(WM_COMMAND, MAKELONG(IDC_DESTBASE_COMBO, CBN_SELCHANGE), (LPARAM) m_destbaseCombo.m_hWnd);
	OnSize(0, gcx, gcy);
}

void CFLCompanionDlg::OnSelchangeDestbaseCombo() 
{
	int nIndex = m_destbaseCombo.GetCurSel();
	if (nIndex == CB_ERR) return;
	CBase *destination;
	CBase *fromBase;
	switch (m_SrcDestSwitch.GetCurSel())
	{
	case 0:
		destination = (CBase*)m_destbaseCombo.GetItemDataPtr(nIndex);
		nIndex = m_baseCombo.GetCurSel();
		fromBase = (CBase*)m_baseCombo.GetItemDataPtr(nIndex);
		break;
	case 1:
		fromBase = (CBase*)m_destbaseCombo.GetItemDataPtr(nIndex);
		nIndex = m_baseCombo.GetCurSel();
		destination = (CBase*)m_baseCombo.GetItemDataPtr(nIndex); 
		break;
	}

	if (nIndex == CB_ERR) return;
	SetDlgItemText(IDC_DESTFACTION, m_SrcDestSwitch.GetCurSel()==0 ? (destination->m_faction ? (m_displayNicknames ? destination->m_faction->m_nickname : destination->m_faction->m_caption) : L"") : (fromBase->m_faction ? (m_displayNicknames ? fromBase->m_faction->m_nickname : fromBase->m_faction->m_caption) : L""));
	CDockable *from = fromBase;
	int baseIndex = destination-g_bases;
	m_waypoints.SetRedraw(FALSE);
	m_waypoints.DeleteAllItems();
	int nItem = m_waypoints.InsertItem(MAXLONG, from->m_system->m_caption);
	m_waypoints.SetItemData(nItem, (DWORD) from);
	m_waypoints.SetItemText(nItem, 1, m_displayNicknames ? from->m_nickname : from->m_caption);
	m_waypoints.SetItemText(nItem, 2, MinuteSeconds(fromBase->GetDockingDelay()));
	m_waypoints.SetItemText(nItem, 4, from->LetterPos());
	while (from != destination)
	{
		CDockable *next = from->m_shortestPath[baseIndex];
		if (next == NULL)
		{
			m_waypoints.InsertItem(MAXLONG, L"No route to destination !");
			break;
		}
		nItem = m_waypoints.InsertItem(MAXLONG, m_displayNicknames ? from->m_system->m_nickname : from->m_system->m_caption);
		m_waypoints.SetItemData(nItem, (DWORD) next);
		m_waypoints.SetItemText(nItem, 1, m_displayNicknames ? next->m_nickname : next->m_caption);
		m_waypoints.SetItemText(nItem, 4, next->LetterPos());
		UINT dist = Distance(from->m_pos,next->m_pos);
		m_waypoints.SetItemText(nItem, 3, IntToString((dist*1000/(from->m_distanceToBase[baseIndex]-next->m_distanceToBase[baseIndex]))/20*20));
		if (next != destination)
			next = next->m_shortestPath[baseIndex];
		m_waypoints.SetItemText(nItem, 2, MinuteSeconds(from->m_distanceToBase[baseIndex]-next->m_distanceToBase[baseIndex]));
		from = next;
	}
	m_waypoints.SetRedraw();
	m_systemWaypoints.DeleteAllItems();
	CClientDC dc(this);
	DrawMap(dc);
	OnSize(0, gcx, gcy);
}

void CFLCompanionDlg::OnItemchangedWaypoints(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;
	if ((pNMListView->uNewState & LVIS_SELECTED) == 0)
		return;

	m_mapOrigin = 0; m_zoom = 1;
	CClientDC dc(this);
	DrawMap(dc);
	OnSize(0, gcx, gcy);

#ifdef SYSTEM_WAYPOINTS
	int nItem = pNMListView->iItem;
	if (nItem == 0)
		return;

	CDockable *waypoint = (CDockable*) m_waypoints.GetItemData(nItem);
	if (waypoint == NULL) return;
	CDockable *startwaypoint;
	if (nItem == 1)
		startwaypoint = (CDockable*) m_waypoints.GetItemData(0);
	else
		startwaypoint = ((CJump*) m_waypoints.GetItemData(nItem-1))->m_matchingJump;
	CSystem *system = waypoint->m_system;

	m_systemWaypoints.SetRedraw(FALSE);
	m_systemWaypoints.DeleteAllItems();
	CString columnTitle = L"Route steps inside "+system->m_caption;
	LVCOLUMN column;
	column.mask = LVCF_TEXT;
	column.cchTextMax = columnTitle.GetLength();
	column.pszText = (LPTSTR) (LPCTSTR) columnTitle;
	m_systemWaypoints.SetColumn(0, &column);
	m_systemWaypoints.InsertItem(MAXLONG, startwaypoint->m_caption);

	//system->CalcLaneDistances(*waypoint);
	CLane *lane;
	UINT disttime = system->ComputeDistance(*startwaypoint, *waypoint, &lane);
	UINT deltatime;
	POS3D *posfrom = &startwaypoint->m_pos;
	POS3D *posto;
	CString caption;
	while (1)
	{
		if (lane)
		{
			posto = lane;
			CLane *scan = lane;
			while (scan->m_prev) scan = scan->m_prev;
			caption = scan->m_caption;
			scan = lane;
			while (scan->m_next) scan = scan->m_next;
			if (lane->m_closest == lane->m_next)
				caption = g_tradeLaneCaption+L": "+caption+L" -> "+scan->m_caption;
			else
				caption = g_tradeLaneCaption+L": "+scan->m_caption+L" -> "+caption;
			
			nItem = m_systemWaypoints.InsertItem(MAXLONG, caption);
			deltatime = disttime-lane->m_distance;
			disttime = lane->m_distance;

		}
		else 
		{
			posto = &waypoint->m_pos;
			nItem = m_systemWaypoints.InsertItem(MAXLONG, waypoint->m_caption);
			deltatime = disttime;
		}
		UINT deltadist = Distance(*posfrom, *posto);

		m_systemWaypoints.SetItemText(nItem, 2, IntToString(deltadist));
		m_systemWaypoints.SetItemText(nItem, 3, IntToString(deltadist*1000/deltatime));
		m_systemWaypoints.SetItemText(nItem, 4, IntToString(deltatime/1000));
		
		if (!lane) break;
		if (lane->m_closest && ((lane->m_closest == lane->m_prev) || (lane->m_closest == lane->m_next)))
		{
			do
				lane = lane->m_closest;
			while (lane->m_closest && ((lane->m_closest == lane->m_prev) || (lane->m_closest == lane->m_next)));
		}
		else
			lane = lane->m_closest;
		posfrom = posto;
	}

	m_systemWaypoints.SetRedraw();
#endif //SYSTEM_WAYPOINTS
}

void CFLCompanionDlg::DrawMap(CDC &dc)
{
	int nIndex = m_systemCombo.GetCurSel();
	m_drawnSystem = nIndex == CB_ERR ? NULL : reinterpret_cast<CSystem*>(m_systemCombo.GetItemDataPtr(nIndex));
	if ((int(m_drawnSystem) == 1) || (int(m_drawnSystem) == 2)) m_drawnSystem = NULL;
	CBase *base;

	switch (m_SrcDestSwitch.GetCurSel())
	{
	case 0:
		nIndex = m_baseCombo.GetCurSel();
		base = nIndex == CB_ERR ? NULL : (CBase*)m_baseCombo.GetItemDataPtr(nIndex);
		break;
	case 1:
		nIndex = m_destbaseCombo.GetCurSel();
		base = nIndex == CB_ERR ? NULL : (CBase*)m_destbaseCombo.GetItemDataPtr(nIndex);
		break;
	}
	if (base) m_drawnSystem = base->m_system;
	CDockable *waypoint = NULL;
	CDockable *startwaypoint = NULL;

	POSITION pos = m_waypoints.GetFirstSelectedItemPosition();
	if (pos)
	{
		int nItem = m_waypoints.GetNextSelectedItem(pos);
		BOOL sameSystem = true;
		if (nItem > 0)
		{
			waypoint = (CDockable*) m_waypoints.GetItemData(nItem);
			if (waypoint == NULL) return;
			if (nItem == 1)
				startwaypoint = (CDockable*) m_waypoints.GetItemData(0);
			else
				startwaypoint = ((CJump*) m_waypoints.GetItemData(nItem-1))->m_matchingJump;
			sameSystem = waypoint->m_system == m_drawnSystem;
			if (!sameSystem)
				m_drawnSystem = waypoint->m_system;
		}
		if (m_asteroidsCombo.GetCount())
		{
			EnableDlgItem(*this, IDC_MINING, sameSystem);
			EnableDlgItem(*this, IDC_ASTEROIDS_COMBO, sameSystem);
		}
	}

	CRect rect;
	GetDlgItem(IDC_MAP)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	if (dc.IntersectClipRect(&rect) == NULLREGION)
		return;
	dc.FillSolidRect(rect, RGB(16,16,41));
	if (!m_drawnSystem) return;

	CPen pen(PS_SOLID, 1, RGB(16,58,82));
	dc.SelectObject(pen);
	dc.SetBkMode(TRANSPARENT);

	m_mapmax = (int) (MAPMAX/m_drawnSystem->m_navmapscale);
	dc.SetMapMode(MM_TEXT);
	CFont font;
	if (rect.Height() == 0)
		return;
	font.CreatePointFont(80*m_mapmax/m_zoom/rect.Height()*2, L"Agency FB", &dc);
	dc.SetMapMode(MM_ISOTROPIC);
	dc.SetViewportOrg((rect.left+rect.right)/2, (rect.top+rect.bottom)/2);
	dc.SetViewportExt(rect.Width()/2,rect.Height()/2);
	dc.SetWindowOrg(m_mapOrigin);
	dc.SetWindowExt(m_mapmax/m_zoom,m_mapmax/m_zoom);

	dc.SelectObject(font);
	dc.SetTextColor(RGB(148,222,239));

	for (int i = -4; i <= 4; i++)
	{
		dc.MoveTo(m_mapmax*i/4, -m_mapmax);
		dc.LineTo(m_mapmax*i/4, +m_mapmax);
		dc.MoveTo(-m_mapmax, m_mapmax*i/4);
		dc.LineTo(+m_mapmax, m_mapmax*i/4);
	}
	TCHAR ch;
	for (int i = 0; i < 8; i++)
	{
		dc.SetTextAlign(TA_LEFT|TA_BASELINE);
		ch = '1'+i;
		dc.TextOut(-m_mapmax/m_zoom+m_mapOrigin.x, m_mapmax*(i*2-7)/8, &ch, 1);
		dc.SetTextAlign(TA_CENTER|TA_BOTTOM);
		ch = 'A'+i;
		dc.TextOut(m_mapmax*(i*2-7)/8, m_mapmax/m_zoom+m_mapOrigin.y, &ch, 1);
	}

	CFont font2;
	font2.CreatePointFont(m_mapmax*4/m_zoom/10, L"Agency FB", NULL);
	dc.SelectObject(font2);


	SetDlgItemText(IDC_MAPNAME, m_displayNicknames ? m_drawnSystem->m_caption + L" ("+m_drawnSystem->m_nickname+L")" : m_drawnSystem->m_caption);

	CString dummy;
	{ // POINTS OF INTEREST
		CPen pen(PS_COSMETIC,1,RGB(255,0,0));
		dc.SelectObject(pen);
		CBrush brush;
		brush.CreateStockObject(NULL_BRUSH);
		dc.SelectObject(brush);
		pos = m_drawnSystem->m_poiList.GetHeadPosition();
		while (pos)
		{
			POS3D &poi = m_drawnSystem->m_poiList.GetNext(pos);
			dc.Ellipse(poi.x-384, poi.z-384, poi.x+384, poi.z+384);
		}
	}


	{ // ASTEROIDS
		CBrush brush;
		brush.CreateStockObject(NULL_BRUSH);
		dc.SelectObject(brush);
		pos = m_drawnSystem->m_asteroids.GetStartPosition();
		while (pos)
		{
			CAsteroids &asteroids = m_drawnSystem->m_asteroids.GetNextAssoc(pos, dummy);
			CPen pen(PS_COSMETIC,1,asteroids.m_good ? (m_curAsteroids == &asteroids ? RGB(192,192,64) : RGB(128,128,64)) : RGB(64,64,192));
			dc.SelectObject(pen);
			//asteroids.m_rotate.y = 45;
			POINT pts[16];
			for (int i = 0; i < _countof(pts); i++)
			{
				double x = asteroids.m_size_x*cos(i*M_2PI/_countof(pts));
				double y = asteroids.m_size_z*sin(i*M_2PI/_countof(pts));
				pts[i].x = asteroids.m_pos_x+LONG(x*cos(asteroids.m_rotate_y*M_2PI/360.0)+y*sin(asteroids.m_rotate_y*M_2PI/360.0));
				pts[i].y = asteroids.m_pos_z+LONG(y*cos(asteroids.m_rotate_y*M_2PI/360.0)-x*sin(asteroids.m_rotate_y*M_2PI/360.0));
			}
			dc.Polygon(pts, _countof(pts));
			//dc.Ellipse(asteroids.m_pos.x-1384, asteroids.m_pos.z-1384, asteroids.m_pos.x+1384, asteroids.m_pos.z+1384);
			//dc.Ellipse(asteroids.m_pos.x-asteroids.m_size.x, asteroids.m_pos.z-asteroids.m_size.z, asteroids.m_pos.x+asteroids.m_size.x, asteroids.m_pos.z+asteroids.m_size.z);

		}
	}
	{ // TRADELANES
		CPen pen(PS_GEOMETRIC, 500, RGB(90,123,148));
		dc.SelectObject(pen);
		pos = m_drawnSystem->m_lanesByNick.GetStartPosition();
		while (pos)
		{
			CLane &lane = m_drawnSystem->m_lanesByNick.GetNextAssoc(pos, dummy);
			if (lane.m_next)
			{
				dc.MoveTo(lane.x, lane.z);
				dc.LineTo(lane.m_next->x, lane.m_next->z);
			}
		}
	}
	{ // JUMPS
		pos = m_drawnSystem->m_jumpsByNick.GetStartPosition();
		while (pos)
		{
			CJump &jump = m_drawnSystem->m_jumpsByNick.GetNextAssoc(pos, dummy);
			jump.Draw(dc, (g_avoidLockedGates && jump.m_islocked) ? RGB(148,0,0) : RGB(148,222,239), m_displayNicknames);
		}
	}
	{ // BASES
		pos = m_drawnSystem->m_bases.GetHeadPosition();
		while (pos)
		{
			CBase &base = *m_drawnSystem->m_bases.GetNext(pos);
			base.Draw(dc, RGB(255,255,255), m_displayNicknames);
		}
	}

	// WAYPOINTS
	if (waypoint)
	{
		dc.OffsetViewportOrg(1,1);
		waypoint->Draw(dc, RGB(0,0,0), m_displayNicknames);
		dc.OffsetViewportOrg(-1,-1);
		waypoint->Draw(dc, RGB(255,255,0), m_displayNicknames);
		if (startwaypoint)
		{
			startwaypoint->Draw(dc, RGB(192,0,192), m_displayNicknames);
			m_drawnSystem->CalcLaneDistances(*waypoint);
			CLane *lane;
			UINT disttime = m_drawnSystem->ComputeDistance(*startwaypoint, *waypoint, &lane);
			CPen pen3(PS_SOLID, 1, RGB(255,0,255));
			dc.SelectObject(pen3);
			POS3D *posfrom = &startwaypoint->m_pos;
			POS3D *posto;
			while (1)
			{
				if (lane) posto = lane; else posto = &waypoint->m_pos;
				//OutputDebugFormat(L"distance : %d\n", Distance(*posfrom, *posto));
				dc.MoveTo(posfrom->x, posfrom->z);
				dc.LineTo(posto->x, posto->z);
				if (!lane) break;
				lane = lane->m_closest;
				posfrom = posto;
			}
		}
	}
	else if (base)
	{
		dc.OffsetViewportOrg(1,1);
		base->Draw(dc, RGB(0,0,0), m_displayNicknames);
		dc.OffsetViewportOrg(-1,-1);
		base->Draw(dc, RGB(255,255,0), m_displayNicknames);
	}

}

void CFLCompanionDlg::OnLimitations() 
{
	int nIndex = m_baseCombo.GetCurSel();
	CBase *base = (nIndex == CB_ERR) ? NULL : (CBase*) m_baseCombo.GetItemDataPtr(nIndex);

	CLimitationsDlg dlg;
	dlg.m_cargoSize = m_cargoSize;
	dlg.m_avoidLockedGates = g_avoidLockedGates;
	dlg.m_jumptrade = g_jumptrade;
	dlg.m_avoidHoles = g_avoidHoles;
	dlg.m_avoidUnstableHoles = g_avoidUnstableHoles;
	dlg.m_avoidGates = g_avoidGates;
	dlg.m_avoidLanes = g_avoidLanes;
	dlg.m_isTransport= g_isTransport;
	
	dlg.m_maxInvestment = m_maxInvestment;
	dlg.m_maxDistance = m_maxDistance;
	dlg.m_jumptradeTime = m_jumptradeTime;
	dlg.m_minCSU = m_minCSU;

	if (dlg.DoModal() == IDOK)
	{
		theApp.WriteProfileInt(L"Settings", L"AvoidLockedGates", dlg.m_avoidLockedGates);
		theApp.WriteProfileInt(L"Settings", L"JumpTrade", dlg.m_jumptrade);
		theApp.WriteProfileInt(L"Settings", L"AvoidHoles", dlg.m_avoidHoles);
		theApp.WriteProfileInt(L"Settings", L"AvoidUnstableHoles", dlg.m_avoidUnstableHoles);
		theApp.WriteProfileInt(L"Settings", L"AvoidGates", dlg.m_avoidGates);
		theApp.WriteProfileInt(L"Settings", L"AvoidLanes", dlg.m_avoidLanes);
		theApp.WriteProfileInt(L"Settings", L"IsTransport", dlg.m_isTransport);
		g_avoidLockedGates = dlg.m_avoidLockedGates;
		g_jumptrade = dlg.m_jumptrade;
		g_avoidHoles = dlg.m_avoidHoles;
		g_avoidUnstableHoles = dlg.m_avoidUnstableHoles;
		g_avoidGates = dlg.m_avoidGates;
		g_avoidLanes = dlg.m_avoidLanes;
		g_isTransport = dlg.m_isTransport;
		SetMaxInvestment(dlg.m_maxInvestment);
		SetMaxDistance(dlg.m_maxDistance);
		SetJumpTradeTime(dlg.m_jumptradeTime);
		SetMinCSU(dlg.m_minCSU);
		SetCargoSize(dlg.m_cargoSize);
		Recalc(RECALC_PATHS);
	}
}

void CFLCompanionDlg::OnSpeedDelays() 
{
	int nIndex = m_baseCombo.GetCurSel();
	CBase *base = (nIndex == CB_ERR) ? NULL : (CBase*) m_baseCombo.GetItemDataPtr(nIndex);

	CSpeedDelaysDlg dlg;
	dlg.m_engineSpeed = ENGINE_SPEED;
	dlg.m_laneSpeed = LANE_SPEED;
	dlg.m_jumpDelay = JUMP_DELAY/1000;
	dlg.m_holeDelay = HOLE_DELAY/1000;
	dlg.m_laneDelay = LANE_DELAY/1000;
	dlg.m_baseDelay = BASE_DELAY/1000;
	if (dlg.DoModal() == IDOK)
	{
		theApp.WriteProfileInt(L"Settings", L"EngineSpeed", dlg.m_engineSpeed);
		theApp.WriteProfileInt(L"Settings", L"LaneSpeed", dlg.m_laneSpeed);
		theApp.WriteProfileInt(L"Settings", L"JumpDelay", dlg.m_jumpDelay);
		theApp.WriteProfileInt(L"Settings", L"HoleDelay", dlg.m_holeDelay);
		theApp.WriteProfileInt(L"Settings", L"LaneDelay", dlg.m_laneDelay);
		theApp.WriteProfileInt(L"Settings", L"BaseDelay", dlg.m_baseDelay);
		ENGINE_SPEED = dlg.m_engineSpeed;
		LANE_SPEED = dlg.m_laneSpeed;
		JUMP_DELAY = dlg.m_jumpDelay * 1000;
		HOLE_DELAY = dlg.m_holeDelay * 1000;
		LANE_DELAY = dlg.m_laneDelay * 1000;
		BASE_DELAY = dlg.m_baseDelay * 1000;
		Recalc(RECALC_PATHS);
	}
}

void CFLCompanionDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CRect rect;
	::GetWindowRect(::GetDlgItem(*this, IDC_MAP), rect);
	ScreenToClient(rect);
	if (rect.PtInRect(point))
	{
		//if (m_curAsteroids && ::IsWindowEnabled(::GetDlgItem(*this, IDC_MINING)))
		{
			CMiningBaseDlg dlg(m_curAsteroids, this);
			if (dlg.DoModal() == IDOK)
			{
				int x = (point.x-(rect.left+rect.right)/2)*m_mapmax*2/rect.Width()/m_zoom+m_mapOrigin.x;
				int z = (point.y-(rect.top+rect.bottom)/2)*m_mapmax*2/rect.Height()/m_zoom+m_mapOrigin.y;
				BeginWaitCursor();
				CBase* base = MakeMiningBase(m_drawnSystem, x, 0, z, dlg.m_goodIndex, dlg.m_lootStat*dlg.m_miningSpeed, dlg.m_goodsPrice);
				g_miningbase = base;
				OnSelchangeSystemCombo();
				m_baseCombo.SetCurSel(0);
				OnSelchangeBaseCombo();
				Recalc(RECALC_PATHS);
			}
		}
	}
	else
		CDialog::OnLButtonDown(nFlags, point);
}

void CFLCompanionDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rect;
	::GetWindowRect(::GetDlgItem(*this, IDC_MAP), rect);
	ScreenToClient(rect);
	if (rect.PtInRect(point))
	{
		::SetFocus(::GetDlgItem(*this, IDC_MAP));
		SetCapture();
		m_mapMouseCoords = point;
	}
	else
		CDialog::OnLButtonDown(nFlags, point);
}

void CFLCompanionDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
#if 0
	// TODO: Add your message handler code here
	// Load the desired menu
	CMenu mnuPopupSubmit;
	mnuPopupSubmit.LoadMenu(IDR_CUSTOMBASEMENU);

	// Get a pointer to the button
	CButton *pButton;
	pButton = reinterpret_cast<CButton *>(GetDlgItem(IDC_MAP));

	// Find the rectangle around the button
	CRect rectSubmitButton;
	pButton->GetWindowRect(&rectSubmitButton);

	// Get a pointer to the first item of the menu
	CMenu *mnuPopupMenu = mnuPopupSubmit.GetSubMenu(0);
	ASSERT(mnuPopupMenu);

	// Find out if the user right-clicked the button
	// because we are interested only in the button
	//if (rectSubmitButton.PtInRect(point)) // Since the user right-clicked the button, display the context menu
		mnuPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);

	/*
	CRect rect;
	::GetWindowRect(::GetDlgItem(*this, IDC_MAP), rect);
	ScreenToClient(rect);
	if (rect.PtInRect(point))
	{
		if (nFlags & MK_CONTROL)
		{
			//int x = (point.x - (rect.left + rect.right) / 2)*m_mapmax * 2 / rect.Width() / m_zoom + m_mapOrigin.x;
			//int z = (point.y - (rect.top + rect.bottom) / 2)*m_mapmax * 2 / rect.Height() / m_zoom + m_mapOrigin.y;
			//::ShowWindow(::GetDlgItem(*this, IDC_MAPINFO), SW_SHOW);
		}
		TrackPopupMenu(IDR_CUSTOMBASEMENU, nFlags, point.x, point.y, 0, *this, NULL);
	}
	*/
#endif
}
void CFLCompanionDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_mapMouseCoords.x)
	{
		::ShowWindow(::GetDlgItem(*this, IDC_MAPINFO), SW_HIDE);
		CRect rect;
		::GetWindowRect(::GetDlgItem(*this, IDC_MAP), rect);
		m_mapOrigin.Offset(
			(m_mapMouseCoords.x-point.x)*m_mapmax*2/m_zoom/rect.Width(),
			(m_mapMouseCoords.y-point.y)*m_mapmax*2/m_zoom/rect.Height());
		CClientDC dc(this);
		DrawMap(dc);
		m_mapMouseCoords = point;
	}
	else
	{
		CRect rect;
		::GetWindowRect(::GetDlgItem(*this, IDC_MAP), rect);
		ScreenToClient(rect);
		if (rect.PtInRect(point))
		{
			CString coords;
			if (nFlags & MK_CONTROL)
			{
				int x = (point.x-(rect.left+rect.right)/2)*m_mapmax*2/rect.Width()/m_zoom+m_mapOrigin.x;
				int z = (point.y-(rect.top+rect.bottom)/2)*m_mapmax*2/rect.Height()/m_zoom+m_mapOrigin.y;
				coords.Format(L"X = %d | Z = %d", x, z);
				SetCursor(LoadCursor(NULL,IDC_CROSS));
				SetDlgItemText(IDC_MAPINFO, coords);
				::ShowWindow(::GetDlgItem(*this, IDC_MAPINFO), SW_SHOW);
			}
		}
		else
			CDialog::OnMouseMove(nFlags, point);
	}
}

BOOL CFLCompanionDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	CRect rect;
	::GetWindowRect(::GetDlgItem(*this, IDC_MAP), rect);
	if (rect.PtInRect(pt))
	{
		::ShowWindow(::GetDlgItem(*this, IDC_MAPINFO), SW_HIDE);
		int delta;
		if (zDelta > 0)
			delta = 1;
		else
		{
			if (m_zoom == 1) return 0;
			delta = -1;
		}
		if (m_zoom+delta == 1)
			m_mapOrigin = 0;
		else
		{

			m_mapOrigin = CPoint(m_mapOrigin.x+
				(pt.x-rect.left)*m_mapmax*2/rect.Width()/m_zoom
				-(pt.x-rect.left)*m_mapmax*2/rect.Width()/(m_zoom+delta)
				+m_mapmax/(m_zoom+delta)-m_mapmax/m_zoom,

				m_mapOrigin.y+
				(pt.y-rect.top)*m_mapmax*2/rect.Height()/m_zoom
				-(pt.y-rect.top)*m_mapmax*2/rect.Height()/(m_zoom+delta)
				+m_mapmax/(m_zoom+delta)-m_mapmax/m_zoom);
		}
		m_zoom += delta;
		CClientDC dc(this);
		DrawMap(dc);
		return 0;
	}
	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void CFLCompanionDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_mapMouseCoords.x = 0;
	ReleaseCapture();
	
	CDialog::OnLButtonUp(nFlags, point);
}

BOOL CFLCompanionDlg::SetMaxInvestment(UINT investment)
{
	if (m_maxInvestment == investment)
		return false;
	m_maxInvestment = investment;
	theApp.WriteProfileInt(L"Settings", L"MaxInvestment", m_maxInvestment);
	Recalc(RECALC_SOLUTIONS);
	return true;
}

BOOL CFLCompanionDlg::SetMaxDistance(UINT distance)
{
	if (m_maxDistance == distance)
		return false;
	m_maxDistance = distance;
	theApp.WriteProfileInt(L"Settings", L"MaxDistance", m_maxDistance);
	Recalc(RECALC_SOLUTIONS);
	return true;
}

BOOL CFLCompanionDlg::SetJumpTradeTime(UINT jumptradeTIme)
{
	if (m_jumptradeTime == jumptradeTIme)
		return false;
	m_jumptradeTime = jumptradeTIme;
	theApp.WriteProfileInt(L"Settings", L"JumpTradeTime", m_jumptradeTime);
	Recalc(RECALC_SOLUTIONS);
	return true;
}

BOOL CFLCompanionDlg::SetMinCSU(UINT minCSU)
{
	if (m_minCSU == minCSU)
		return false;
	m_minCSU = minCSU;
	theApp.WriteProfileInt(L"Settings", L"MinCSU", m_minCSU);
	Recalc(RECALC_SOLUTIONS);
	return true;
}

BOOL CFLCompanionDlg::SetCargoSize(UINT cargoSize)
{
	if (m_cargoSize != cargoSize)
	{
		m_cargoSize = cargoSize;
		theApp.WriteProfileInt(L"Settings", L"CargoSize", m_cargoSize);
		Recalc(RECALC_SOLUTIONS);
		return true;
	}
	return false;
}

void CFLCompanionDlg::Recalc(DWORD flags)
{
	if ((m_recalcFlags & flags) == flags) return;
	for(LONG curFlags; curFlags = m_recalcFlags, InterlockedCompareExchange(&m_recalcFlags, curFlags|flags, curFlags) != curFlags ; Sleep(1));
	PostMessage(WM_RECALC);
}

LRESULT CFLCompanionDlg::OnRecalc(WPARAM, LPARAM)
{
	LONG flags = InterlockedExchange(&m_recalcFlags, 0);
	if (flags == 0) return 0;
	BeginWaitCursor();
	if (flags & RECALC_PATHS)
	{
		CalculateDirectRoutes();
		while (PropagateRoutes());
		flags |= RECALC_SOLUTIONS;
	}
	if (flags & RECALC_SOLUTIONS)
	{
		OnSelchangeBaseCombo();
	}
	OnSelchangeSrcDestCombo();
	EndWaitCursor();
	return 0;
}

void CFLCompanionDlg::OnBegintrackRoutes(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	*pResult = 0;
	if (phdn->pitem->mask & HDI_WIDTH) // simple system to prevent revealing 0-width empty-titled columns
	{
		HDITEM hdi;
		TCHAR firstChars[2];
		hdi.mask = HDI_WIDTH|HDI_TEXT;
		hdi.pszText = firstChars;
		hdi.cchTextMax = 2;
		Header_GetItem(phdn->hdr.hwndFrom, phdn->iItem, &hdi);
		if ((firstChars[0] == '\0') && (hdi.cxy == 0))
			*pResult = 1;
	}
}

void CFLCompanionDlg::OnDumpBaseTimes() 
{
	CFileDialog fileDialog(FALSE, L"csv", L"traveltimes.csv", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, L"Comma Separated Values (*.csv)|*.csv||", this);
	if (fileDialog.DoModal() == IDOK)
	{
		CString path = fileDialog.GetPathName();
		CStdioFile file(path,CFile::modeCreate|CFile::modeWrite);
		for (UINT fromIndex = 0; fromIndex < BASES_COUNT; fromIndex++)
		{
			CBase &fromBase = g_bases[fromIndex];
			for (UINT toIndex = 0; toIndex < BASES_COUNT; toIndex++)
			{
				CBase &toBase = g_bases[toIndex];
				file.WriteString(L"\"");
				file.WriteString(fromBase.m_nickname);
				file.WriteString(L"\",\"");
				file.WriteString(toBase.m_nickname);
				file.WriteString(L"\",");
				file.WriteString(IntToString(fromBase.m_distanceToBase[toIndex]));
				file.WriteString(L"\n");
			}
		}
		
	}	
}

void CFLCompanionDlg::OnDumpSolutions()
{
	if (AfxMessageBox(L"Saving all trading solutions can take some time to calculate!\r\n\r\nAre you sure?", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
	{
		CFileDialog fileDialog(FALSE, L"csv", L"solutions.csv", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"Comma Separated Values (*.csv)|*.csv||", this);
		if (fileDialog.DoModal() == IDOK)
		{
			CString path = fileDialog.GetPathName();
			CStdioFile file(path, CFile::modeCreate | CFile::modeWrite);
			blnMultiBaseSolution = true;
			for (int index = 0; index < BASES_COUNT; index++)
			{
				CBase &base = g_bases[index];
				if (!base.m_hasSell || (g_isTransport && base.m_isfreighteronly) || base.m_system->m_avoid) continue;

				// build up the buying price array
				float *sell = base.m_sell;
				if (!blnMultiBaseSolution)
					solutionCount = 0;
				for (int index = 0; index < SYSTEMS_COUNT; index++)
				{
					CSystem *system = &g_systems[index];
					if (system->m_avoid)
						continue;
					// scan all bases in this system
					POSITION pos = system->m_bases.GetHeadPosition();
					while (pos)
					{
						CBase *destbase = system->m_bases.GetNext(pos);
						if (!destbase->m_hasSell)
							continue; // not interesting to go to bases that doesn't sell anything
						if (base.m_shortestPath[destbase - g_bases] == NULL)
							continue; // no route to destination;
						if (m_maxDistance && (base.m_distanceToBase[destbase - g_bases] >= m_maxDistance))
							continue; // route too distant
						if (m_SrcDestSwitch.GetCurSel() == 1 && destbase != (CBase*)m_baseCombo.GetItemDataPtr(m_baseCombo.GetCurSel()))
							continue;
						// scan the buying prices for this destination base
						for (UINT goodIndex = 0; goodIndex < GOODS_COUNT; goodIndex++)
						{
							if (g_goods[goodIndex].m_avoid)
								continue;
							double destbuy = max(destbase->m_buy[goodIndex], 1);
							double srcsell = max(sell[goodIndex], 1);
							if (destbuy > srcsell) // destination buying price is higher than initial sell price
							{
								//AddSolution(int goodIndex, double destbuy, double srcsell, CBase *srcbase, CBase *destbase, LONG distance)
								//(goodIndex, destbase->m_buy[goodIndex], sell[goodIndex], base, destbase,base->m_distanceToBase[destbase - g_bases] + base->GetDockingDelay());

								//CBase *srcbase;
								//CBase *destbase;
								LONG distance = base.m_distanceToBase[destbase - g_bases] + base.GetDockingDelay();
								LONG profit;

								UINT units = m_cargoSize == 1 ? 1 : UINT(m_cargoSize / g_goods[goodIndex].m_volume);
								if (units == 0)
									continue;
								if ((m_maxInvestment > 0) && (srcsell*units > m_maxInvestment))
									units = UINT(m_maxInvestment / srcsell);
								if (g_goods[goodIndex].m_decay_time == 0)
									profit = UINT(destbuy - srcsell)*units;
								else
								{ // lets compute the effect of decaying goods:
									UINT decay_units = distance / g_goods[goodIndex].m_decay_time;
									if (m_cargoSize != 1)
									{
										if (decay_units > units)
											continue; // all cargo would have decayed, so drop this solution
										profit = INT(destbuy*(units - decay_units) - srcsell * units);
									}
									else // user wants a price per cargo unit => we evaluate an average profit per unit, based on a cargo of 100 units
									{
										if (decay_units > 100)
											continue;
										profit = INT(destbuy - srcsell - destbuy * decay_units / 100.0);
									}
									if (profit < 0)
										continue; // negative profit if decay is taken in account, drop it
								}
								if (((profit / units) * 100000 / distance) < m_minCSU && destbase != g_miningbase)
									continue;
								file.WriteString(L"\"");
								file.WriteString(m_displayNicknames ? g_goods[goodIndex].m_nickname : g_goods[goodIndex].m_caption);
								file.WriteString(L"\",\"");
								file.WriteString(m_displayNicknames ? base.m_system->m_nickname + ": " + base.m_nickname : base.m_system->m_caption + ": " + base.m_caption);
								file.WriteString(L"\",\"");
								file.WriteString(m_displayNicknames ? destbase->m_system->m_nickname + ": " + destbase->m_nickname : destbase->m_system->m_caption + ": " + destbase->m_caption);
								file.WriteString(L"\",");
								file.WriteString(IntToString(profit));
								file.WriteString(L",\"");
								file.WriteString(MinuteSeconds(distance, true));
								file.WriteString(L",");

								TCHAR buf[32];
								if (m_cargoSize == 1)
								{
									CString ratio = DoubleToString((profit / g_goods[goodIndex].m_volume) * 100000 / distance);
									int index = ratio.Find('.');
									if (index > 0) ratio = ratio.Left(index + 0);
									file.WriteString(LPCTSTR(ratio));
								}
								else
								{
									CString ratio = DoubleToString(profit*1000.0 / distance);
									int index = ratio.Find('.');
									if (index > 0) ratio = ratio.Left(index + 0);
									file.WriteString(LPCTSTR(ratio));
								}
								file.WriteString(L",");
								CString ratio = DoubleToString((profit / units / g_goods[goodIndex].m_volume) * 100000 / distance);
								int index = ratio.Find('.');
								if (index > 0) ratio = ratio.Left(index + 0);
								file.WriteString(LPCTSTR(ratio));
								file.WriteString(L"\n");
								solutionCount++;
							}
							//base.m_system->m_distances[destbase->m_system-g_systems]);
						}
					}
				}
			}
			blnMultiBaseSolution = false;
		}
		Log(L"All solutions have been saved to file.");
		AfxMessageBox(L"All solutions complete.", MB_OK | MB_ICONEXCLAMATION);
	}
}

void CFLCompanionDlg::OnBaseInfo() 
{
	int nIndex = m_baseCombo.GetCurSel();
	if (nIndex != CB_ERR)
	{
		CBase *base = (CBase*) m_baseCombo.GetItemDataPtr(nIndex);
		CBaseInfoDlg dlg(base, this);
		dlg.DoModal();
	}
}

void CFLCompanionDlg::OnGoodsRepo() 
{
	CGoodsRepoDlg dlg(this);
	if (dlg.DoModal() == IDOK)
	{
		JumptoBase(dlg.m_selectedBase);
	}
}

void CFLCompanionDlg::ImportFromGame()
{
	if (m_importFromGame == 0)
			return;
	CGameInspect gameInspect;
	g_triggeredImport = true;
	gameInspect.DoTask(m_importFromGame);
	g_triggeredImport = false;
}

void CFLCompanionDlg::OnActivateApp(BOOL bActive, DWORD hTask)
{
	CDialog::OnActivateApp(bActive, hTask);
}

void CFLCompanionDlg::OnRefreshClient()
{
	g_triggeredImport = true;
	ImportFromGame();
	g_triggeredImport = false;
}

void CFLCompanionDlg::OnGameImportAbout() 
{
	MessageBox(L"This feature will monitor the game program and import datas from it.\n"
		L"The game must be running and you must have the adequate rights.\n"
		L"\n"
		L"To collect datas from a multi-player game, you must already be\n"
		L"connected to the server as the character of your choice.",
		L"Import from running game",
		MB_ICONINFORMATION|MB_OK);
}

void CFLCompanionDlg::OnUpdateGameImportCheckall(CCmdUI* pCmdUI) 
{
	pCmdUI->SetText(m_importFromGame ? L"Stop importing data" : L"Import all data");
	
}

void CFLCompanionDlg::OnGameImportCheckall() 
{
	if (m_importFromGame)
		m_importFromGame = 0;
	else
		m_importFromGame = (1<<(ID_GAME_IMPORT_CARGOHOLD-ID_GAME_IMPORT_PRICES+1))-1;
	theApp.WriteProfileInt(L"Settings", L"ImportFromGame", m_importFromGame);
	g_triggeredImport = true;
	ImportFromGame();
	g_triggeredImport = false;
}

void CFLCompanionDlg::OnUpdateGameImport(CCmdUI* pCmdUI) 
{
	UINT nID = pCmdUI->m_nID-ID_GAME_IMPORT_PRICES;
	pCmdUI->SetCheck((m_importFromGame & (1 << nID)) != 0);
}


void CFLCompanionDlg::OnGameImport(UINT nID) 
{
	nID -= ID_GAME_IMPORT_PRICES;
	m_importFromGame ^= (1 << nID);
	theApp.WriteProfileInt(L"Settings", L"ImportFromGame", m_importFromGame);
	g_triggeredImport = true;
	ImportFromGame();
	g_triggeredImport = false;
}

void CFLCompanionDlg::OnAbout() 
{
	CAboutDlg dlg(this);
	dlg.DoModal();
}

BOOL CFLCompanionDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (m_hAccel && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
		return(TRUE);
	return CDialog::PreTranslateMessage(pMsg);
}

void CFLCompanionDlg::OnVisitForum() 
{
	ShellExecute(*this, L"open", L"http://wiz0u.free.fr/forum/viewforum.php?f=10", NULL, NULL, SW_SHOWNORMAL);
}

void CFLCompanionDlg::WinHelp(DWORD dwData, UINT nCmd) 
{
	OnVisitForum();
}

void CFLCompanionDlg::OnVisitWebsite() 
{
	ShellExecute(*this, L"open", L"http://wiz0u.free.fr/prog/flc", NULL, NULL, SW_SHOWNORMAL);
}

void CFLCompanionDlg::OnVersionHistory() 
{
	ShellExecute(*this, L"open", L"http://wiz0u.free.fr/prog/check4update.php?app=FLCompanion", NULL, NULL, SW_SHOWNORMAL);
}

void CFLCompanionDlg::OnSetFLDir() 
{
	if (BrowseFLDir(*this))
	{
		AfxMessageBox(L"For this setting to take effect, you will need to exit and restart the application", 
			MB_OK|MB_ICONINFORMATION);
	}
}

void CFLCompanionDlg::OnUpdateNicknames(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_displayNicknames);
}

void CFLCompanionDlg::OnSwitchNicknames() 
{
	int nIndex = m_baseCombo.GetCurSel();
	CBase *base = (nIndex == CB_ERR) ? NULL : (CBase*) m_baseCombo.GetItemDataPtr(nIndex);
	m_displayNicknames = !m_displayNicknames;
	InitSystemCombos();
	if (base) JumptoBase(base);
}

void CFLCompanionDlg::OnMining() 
{
	CString msg;
	msg.Format(
		L"Place a virtual mining operation base by double-clicking the map where you will mine.\n"
		L"(The currently selected mining field is shown in bright yellow)\n"
		L"Hold the CTRL key for a precise location.\n\n"
		L"Please note that the loot probability might be altered by the server in multi-player game");
	MessageBox(msg, L"Mining operation", MB_ICONINFORMATION|MB_OK);
}

void CFLCompanionDlg::OnSelchangeAsteroidsCombo() 
{
	m_curAsteroids = reinterpret_cast<CAsteroids*>(m_asteroidsCombo.GetItemDataPtr(m_asteroidsCombo.GetCurSel()));
	EnableDlgItem(*this, IDC_MINING, !!m_curAsteroids);
	ResetMapZoom();
}

void CFLCompanionDlg::OnUpdateGameLaunch(CCmdUI* pCmdUI) 
{
	HANDLE hMutex = OpenMutex(SYNCHRONIZE, FALSE, L"FreelancerClient");
	if (hMutex)
		CloseHandle(hMutex);
	pCmdUI->Enable(!hMutex);
}

void CFLCompanionDlg::OnGameLaunch() 
{
	ShellExecute(*this, L"open", g_flAppPath+L"\\EXE\\Freelancer.exe", NULL, NULL, SW_SHOWNORMAL);
}

void CFLCompanionDlg::OnModInfo() 
{
	CModInfoDlg dlg(this);
	dlg.DoModal();
	if (!g_modInfo.FLC_info.IsEmpty())
	{
		DWORD hash = FLHash(g_modInfo.FLC_info);
		theApp.WriteProfileInt(L"Settings", L"DisplayModInfo", hash);
	}
}

void CFLCompanionDlg::OnUpdateModInfo(CCmdUI* pCmdUI) 
{
	if (g_modInfo.largeMod)
		pCmdUI->SetText(g_modInfo.name+L" info...");
	else
		pCmdUI->Enable(FALSE);
}

void CFLCompanionDlg::OnMap()
{
	blnMap = !blnMap;
	OnSize(0, gcx, gcy);
}

void CFLCompanionDlg::OnJumps() 
{
	CMenu menu;
	{
		CMap<CSystem*,CSystem*,UINT,UINT> outSystems;
		CString dummy;
		POSITION pos = m_drawnSystem->m_jumpsByNick.GetStartPosition();
		while (pos)
		{
			CJump &dock = m_drawnSystem->m_jumpsByNick.GetNextAssoc(pos, dummy);
			outSystems[dock.m_matchingJump->m_system] |= dock.m_isgate ? 0x1 : 0x2;
		}
		menu.CreatePopupMenu();
		pos = outSystems.GetStartPosition();
		while (pos)
		{
			CSystem* system;
			UINT flags;
			outSystems.GetNextAssoc(pos, system, flags);
			static const LPCTSTR suffixes[] = { NULL, L" (Jump Gate)", L" (Jump Hole)", L" (Jump Gate + Jump Hole)" };
			menu.AppendMenu(MF_STRING, (UINT) system, system->m_caption+suffixes[flags]);
		}
	}
	CPoint mousePos;
	GetCursorPos(&mousePos);
	UINT result = menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,mousePos.x, mousePos.y, this, NULL);
	if (result)
	{
		CSystem* system = reinterpret_cast<CSystem*>(result);
		SelComboByData(m_systemCombo, system);
		OnSelchangeSystemCombo();
	}
}


void CFLCompanionDlg::OnSwitch() 
{
	int nIndex = m_baseCombo.GetCurSel();
	CBase *base1 = (nIndex == CB_ERR) ? NULL : (CBase*) m_baseCombo.GetItemDataPtr(nIndex);
	nIndex = m_destbaseCombo.GetCurSel();
	CBase *base2 = (nIndex == CB_ERR) ? NULL : (CBase*) m_destbaseCombo.GetItemDataPtr(nIndex);
	if (base1 && base2)
	{
		SelComboByData(m_destsystemCombo, base1->m_system);
		OnSelchangeDestsystemCombo();
		SelComboByData(m_destbaseCombo, base1);
		JumptoBase(base2);
	}
}

//TO DO:  Make the TradeRoute Add/Remove buttons function properly.
void CFLCompanionDlg::OnTR_Add()
{
	if (g_traderouteTotal != 0)
	{
		m_traderoute.DeleteItem(g_traderouteTotalHour);
		m_traderoute.DeleteItem(g_traderouteTotal);
		g_traderouteTotalHour = 0;
		g_traderouteTotal = 0;
	}

	int goodIndex = _ttoi(m_routes.GetItemText(SelectedItem, 6));

	int nItem = m_traderoute.InsertItem(MAXLONG, m_displayNicknames ? g_goods[goodIndex].m_nickname : g_goods[goodIndex].m_caption);
	for (size_t i = 0; i <= 8; i++)
	{
		m_traderoute.SetItemText(nItem, i, m_routes.GetItemText(SelectedItem, i));
	}
	m_traderoute.SetItemData(nItem, (DWORD)m_routes.GetItemData(SelectedItem));
	if (m_traderoute.GetItemCount() != 0 && m_traderoute.GetItemText(0, 1) == m_traderoute.GetItemText(m_traderoute.GetItemCount() - 1, 2))
	{
		Calc_TotalRow();
	}
}

void CFLCompanionDlg::OnTR_Rem()
{
	if (g_traderouteTotal != 0)
	{
		if ((int)m_traderoute.GetFirstSelectedItemPosition() - 1 == g_traderouteTotal || (int)m_traderoute.GetFirstSelectedItemPosition() - 1 == g_traderouteTotalHour)
		{
			m_traderoute.DeleteAllItems();
		}
		else
		{
			m_traderoute.DeleteItem(g_traderouteTotalHour);
			m_traderoute.DeleteItem(g_traderouteTotal);
			g_traderouteTotal = 0;
			g_traderouteTotalHour = 0;
		}
	}
	m_traderoute.DeleteItem((int)m_traderoute.GetFirstSelectedItemPosition() - 1);
	if (m_traderoute.GetItemCount() != 0 && m_traderoute.GetItemText(0, 1) == m_traderoute.GetItemText(m_traderoute.GetItemCount() - 1, 2))
	{
		Calc_TotalRow();
	}
}
void CFLCompanionDlg::Calc_TotalRow()
{

	LONG t_profit = 0;
	UINT t_units = 0;
	LONG t_distance = 0;
	for (size_t i = 0; i <= m_traderoute.GetItemCount()-1; i++)
	{
		CBase *from = (CBase*)_ttoi(m_traderoute.GetItemText(i, 7));
		int goodIndex = 0;
		goodIndex = _ttoi(m_traderoute.GetItemText(i, 6));
		CBase *to = (CBase*)m_traderoute.GetItemData(i);
		LONG distance = goodIndex != 0 && g_jumptrade ? m_jumptradeTime : to->m_distanceToBase[from - g_bases] + to->GetDockingDelay();
		UINT units = m_cargoSize == 1 ? 1 : UINT(m_cargoSize / g_goods[goodIndex].m_volume);
		LONG profit = 0;

		if (units == 0)
			return;
		if ((m_maxInvestment > 0) && (max(from->m_sell[goodIndex], 1)*units > m_maxInvestment))
			units = UINT(m_maxInvestment / max(from->m_sell[goodIndex], 1));
		if (g_goods[goodIndex].m_decay_time == 0)
			profit = UINT(to->m_buy[goodIndex] - max(from->m_sell[goodIndex],1))*units;
		else
		{ // lets compute the effect of decaying goods:
			UINT decay_units = distance / g_goods[goodIndex].m_decay_time;
			if (m_cargoSize != 1)
			{
				if (decay_units > units)
					continue; // all cargo would have decayed, so drop this solution
				profit = INT(to->m_buy[goodIndex] *(units - decay_units) - max(from->m_sell[goodIndex], 1) *units);
			}
			else // user wants a price per cargo unit => we evaluate an average profit per unit, based on a cargo of 100 units
			{
				if (decay_units > 100)
					continue;
				profit = INT(to->m_buy[goodIndex] - max(from->m_sell[goodIndex], 1) - to->m_buy[goodIndex] *decay_units / 100.0);
			}
			if (profit < 0)
				continue; // negative profit if decay is taken in account, drop it
		}

		//_stprintf(buf, L"$%d", profit);
		//m_routes.SetItemText(nItem, 3, buf);
		//m_routes.SetItemText(nItem, 4, MinuteSeconds(distance, true));
		if (m_cargoSize == 1)
		{
			//_stprintf(buf, L"%d ¢/sec", profit * 100000 / distance);
		}
		else
		{
			CString ratio = DoubleToString(profit*1000.0 / distance);
			int index = ratio.Find('.');
			if (index > 0) ratio = ratio.Left(index + 3);
		}

		t_profit += profit;
		t_distance += distance;
		t_units = units;
	}

	TCHAR buf[32];
	g_traderouteTotal = m_traderoute.InsertItem(MAXLONG, L"Total : ");

	_stprintf(buf, L"$%d", t_profit);
	m_traderoute.SetItemText(g_traderouteTotal, 3, buf);
	m_traderoute.SetItemText(g_traderouteTotal, 4, MinuteSeconds(t_distance, true));
	if (m_cargoSize == 1)
	{
		_stprintf(buf, L"%d ¢/sec", t_profit * 100000 / t_distance);
	}
	else
	{
		CString ratio = DoubleToString(t_profit*1000.0 / t_distance);
		int index = ratio.Find('.');
		if (index > 0) ratio = ratio.Left(index + 3);
		_stprintf(buf, L"%s $/sec", LPCTSTR(ratio));
	}
	m_traderoute.SetItemText(g_traderouteTotal, 5, buf);
	CString ratio = DoubleToString((t_profit / m_cargoSize) * 100000.00 / t_distance);
	int index = ratio.Find('.');
	if (index > 0) ratio = ratio.Left(index + 3);
	_stprintf(buf, L"%s ¢/sec", LPCTSTR(ratio));
	m_traderoute.SetItemText(g_traderouteTotal, 8, buf);
	g_traderouteTotalHour = m_traderoute.InsertItem(MAXLONG, L"Approx. per Hour : ");

	ratio = DoubleToString(((60.0 / ((t_distance + 500) / 1000 / 60))) * t_profit);
	index = ratio.Find('.');
	if (index > 0) ratio = ratio.Left(index);
	_stprintf(buf, L"$%s", LPCTSTR(ratio));
	m_traderoute.SetItemText(g_traderouteTotalHour, 3, buf);
}