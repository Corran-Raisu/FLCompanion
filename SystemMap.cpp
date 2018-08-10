// SystemMap.cpp : implementation file
//

#include "stdafx.h"
#include "FLCompanion.h"
#include "FLCompanionDlg.h"
#include "SystemMap.h"
#include "System.h"
#include "Datas.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSystemMap dialog


CSystemMap::CSystemMap(CWnd* pParent /*=NULL*/)
	: CDialog(CSystemMap::IDD, pParent), m_system(NULL), m_destsystem(NULL)
{
	//{{AFX_DATA_INIT(CSystemMap)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSystemMap::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSystemMap)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSystemMap, CDialog)
	//{{AFX_MSG_MAP(CSystemMap)
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSystemMap message handlers
#define ZOOM 40
#define OFFSET_X 30
#define OFFSET_Y 12

void CSystemMap::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CRect rect;
	GetClientRect(&rect);
	int maxY = rect.Height();
	dc.IntersectClipRect(&rect);
	dc.FillSolidRect(rect, RGB(16,16,41));
	CFont font;
	font.CreatePointFont(80, L"Agency FB", &dc);
	dc.SelectObject(font);
	// 
	CString dummy;

	for (int index = 0; index < SYSTEMS_COUNT; index++)
	{
		// JUMP GATES
		CSystem &system = g_systems[index];
		CPen bluepen(PS_SOLID,3,RGB(0,53,128));
		dc.SelectObject(bluepen);
		POSITION pos = system.m_jumpsByNick.GetStartPosition();
		while (pos)
		{
			CJump &jump = system.m_jumpsByNick.GetNextAssoc(pos, dummy);
			if (!jump.m_isgate) continue;
			if (g_avoidLockedGates && jump.m_islocked) continue;
			if (g_avoidHoles && !jump.m_isgate) continue;
			CSystem *jumpsystem = jump.m_matchingJump->m_system;
			if (jumpsystem == NULL) // abnormal
			{
				ProblemFound(L"Jump %s in %s references an undefined destination %s", jump.m_nickname, system.m_nickname, jump.m_matchingJump->m_nickname);
				jumpsystem = &system;
			}
			dc.MoveTo(OFFSET_X+(system.m_x-m_minX)*ZOOM, OFFSET_Y+(system.m_y-m_minY)*ZOOM);
			dc.LineTo(OFFSET_X+(jumpsystem->m_x-m_minX)*ZOOM, OFFSET_Y+(jumpsystem->m_y-m_minY)*ZOOM);
		}
		// JUMP HOLES
		dc.SetROP2(R2_MERGEPEN);
		CPen redpen(PS_SOLID,1,RGB(107,0,53));
		dc.SelectObject(redpen);
		pos = system.m_jumpsByNick.GetStartPosition();
		while (pos)
		{
			CJump &jump = system.m_jumpsByNick.GetNextAssoc(pos, dummy);
			if (jump.m_isgate) continue;
			if (g_avoidLockedGates && jump.m_islocked) continue;
			if (g_avoidHoles && !jump.m_isgate) continue;
			if (!jump.m_matchingJump) // abnormal
				continue;
			CSystem *jumpsystem = jump.m_matchingJump->m_system;
			if (jumpsystem == NULL) // abnormal
				continue;
			dc.MoveTo(OFFSET_X+(system.m_x-m_minX)*ZOOM, OFFSET_Y+(system.m_y-m_minY)*ZOOM);
			dc.LineTo(OFFSET_X+(jumpsystem->m_x-m_minX)*ZOOM, OFFSET_Y+(jumpsystem->m_y-m_minY)*ZOOM);
		}
	}
	
	// BASES
	dc.SetROP2(R2_COPYPEN);
	CBrush brush(RGB(255,255,200));
	CBrush brushSrc(RGB(0,255,0));
	CBrush brushDst(RGB(255,255,0));
	CPen nopen(PS_NULL,0,RGB(0,0,0));
	dc.SelectObject(nopen);
	dc.SetTextAlign(TA_CENTER|TA_TOP);
	dc.SetBkMode(TRANSPARENT);
	for (int index = 0; index < SYSTEMS_COUNT; index++)
	{
		CSystem &system = g_systems[index];
		if (&system == m_system)
		{
			dc.SetTextColor(RGB(0,255,0));
			dc.SelectObject(brushSrc);
		}
		else if (&system == m_destsystem)
		{
			dc.SetTextColor(RGB(255,255,0));
			dc.SelectObject(brushDst);
		}
		else
		{
			dc.SetTextColor(RGB(148,222,239));
			dc.SelectObject(brush);
		}
		dc.Ellipse(OFFSET_X+(system.m_x-m_minX)*ZOOM-3, OFFSET_Y+(system.m_y-m_minY)*ZOOM-3, OFFSET_X+(system.m_x-m_minX)*ZOOM+5, OFFSET_Y+(system.m_y-m_minY)*ZOOM+5);
		dc.TextOut(OFFSET_X+(system.m_x-m_minX)*ZOOM, OFFSET_Y+(system.m_y-m_minY)*ZOOM+1, m_displayNicknames ? system.m_nickname : system.m_caption);
	}

	// WAYPOINTS
	{
		CPen waypen(PS_SOLID,1,RGB(255,128,255));
		dc.SelectObject(waypen);
		int count = g_mainDlg->m_waypoints.GetItemCount();
		CSystem* fromsystem, *tosystem;
		CDockable* tobase = count > 1 ? reinterpret_cast<CDockable*>(g_mainDlg->m_waypoints.GetItemData(count-1)) : NULL;
		if (tobase == NULL)
			count = 1;
		else 
			tosystem = tobase->m_system;
		while (--count > 1)
		{
			fromsystem = tosystem;
			tosystem = reinterpret_cast<CDockable*>(g_mainDlg->m_waypoints.GetItemData(count-1))->m_system;
			dc.MoveTo(OFFSET_X+(fromsystem->m_x-m_minX)*ZOOM, OFFSET_Y+(fromsystem->m_y-m_minY)*ZOOM);
			dc.LineTo(OFFSET_X+(tosystem->m_x-m_minX)*ZOOM, OFFSET_Y+(tosystem->m_y-m_minY)*ZOOM);

		}
	}

	CFont font2;
	font2.CreatePointFont(80, L"Arial", &dc);
	dc.SelectObject(font2);
	GetClientRect(&rect);
	dc.SetTextAlign(TA_CENTER|TA_BOTTOM);
	dc.SetTextCharacterExtra(1);
	dc.SetTextColor(RGB(0,0,0));
	dc.SetBkColor(RGB(192,192,0));
	dc.SetBkMode(OPAQUE);
	dc.TextOut(rect.right/2, rect.bottom, L"Left-click to choose an origin system (green) - Right-click to choose a destination system (yellow)");
}

void CSystemMap::OnLButtonUp(UINT nFlags, CPoint point) 
{
	for (int index = 0; index < SYSTEMS_COUNT; index++)
	{
		CSystem &system = g_systems[index];
		CRect rect(OFFSET_X+(system.m_x-m_minX)*ZOOM-ZOOM/2, OFFSET_Y+(system.m_y-m_minY)*ZOOM-ZOOM/2, OFFSET_X+(system.m_x-m_minX)*ZOOM+ZOOM/2, OFFSET_Y+(system.m_y-m_minY)*ZOOM+ZOOM/2);
		if (rect.PtInRect(point))
		{
			m_system = &system;
			EndDialog(nFlags & MK_RBUTTON ? 101 : 100);
			return;
		}
	}
	CDialog::OnLButtonUp(nFlags, point);
}

void CSystemMap::OnRButtonUp(UINT nFlags, CPoint point) 
{
	OnLButtonUp(nFlags|MK_RBUTTON, point);
}

BOOL CSystemMap::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	int maxX,maxY;
	m_minX = m_minY = maxX = maxY = 0;
	for (int index = 0; index < SYSTEMS_COUNT; index++)
	{
		CSystem &system = g_systems[index];
		m_minX = min(m_minX, system.m_x);
		maxX = max(maxX, system.m_x);
		m_minY = min(m_minY, system.m_y);
		maxY = max(maxY, system.m_y);
	}
	CRect rect(0,0,OFFSET_X+(maxX-m_minX+1)*ZOOM,(maxY-m_minY+1)*ZOOM+8);
	CalcWindowRect(&rect);
	SetWindowPos(NULL,0,0,rect.Width(), rect.Height(), SWP_NOMOVE|SWP_NOZORDER);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
