// Dockable.cpp: implementation of the CDockable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FLCompanion.h"
#include "Dockable.h"
#include "System.h"
#include "Datas.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

UINT BASES_COUNT;
UINT GOODS_COUNT;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CDockable::ResetPath()
{
	memset(m_distanceToBase, 0xFF, sizeof(m_distanceToBase));
	memset(m_shortestPath, 0, sizeof(m_shortestPath));
}

void CDockable::Init(const CString &nickname, const CString &caption, CSystem *system)
{
	m_nickname = nickname;
	m_caption = caption;
	m_system = system;
}

void CDockable::Draw(CDC &dc, COLORREF color, BOOL displayNicknames)
{
	
	CPen pen(PS_COSMETIC,1,color);
	dc.SelectObject(pen);
	CBrush brush;
	brush.CreateStockObject(NULL_BRUSH);
	dc.SelectObject(brush);
	dc.Ellipse(m_pos.x-384, m_pos.z-384, m_pos.x+384, m_pos.z+384);
	
//	dc.SetPixel(m_pos.x  , m_pos.z  , color);
	/*
	CPen pen(PS_NULL,0,RGB(0,0,0));
	dc.SelectObject(pen);
	CBrush brush(color);
	dc.SelectObject(brush);
	dc.Ellipse(m_pos.x-750, m_pos.z-750, m_pos.x+750, m_pos.z+750);
	*/
	dc.SetTextAlign(TA_CENTER|TA_TOP);
	dc.SetTextColor(color);
	dc.TextOut(m_pos.x, m_pos.z, displayNicknames ? m_nickname : m_caption);
	
}

CString CDockable::LetterPos()
{
	int mapmax = (int) (MAPMAX/m_system->m_navmapscale);
	int x = (m_pos.x)*40/mapmax+40;
	int z = (m_pos.z)*40/mapmax+40;
	CString letters = CString(char('A'+x/10))+L","+CString(char('1'+z/10),1);
	if (m_pos.y < -1024)
		letters += L" (below horizon)";
	else if (m_pos.y > 1024)
		letters += L" (above horizon)";
	return letters;
}



//|              x              |
//0  1  2  3  4  5  6  7  8  9  0