// Gate.cpp: implementation of the CGate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FLCompanion.h"
#include "Jump.h"
#include "Datas.h"
#include "System.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CJump::SetMatchingJump(CJump *matchingJump)
{ 
	CHECK(matchingJump);
	m_matchingJump = matchingJump; 
}
extern DWORD FLHash (LPCTSTR string);

void CJump::Init(const CString &nickname, BOOL isgate, BOOL islocked, const CString &caption, CSystem *system)
{
	//CString sCaption = isgate ? g_jumpGateCaption : g_jumpHoleCaption;
	//sCaption += L": "+caption;
	m_isgate = isgate;
	m_islocked = islocked;
	CDockable::Init(nickname, caption, system);
}


CString CJump::LetterPos()
{
	if (m_isgate) return CDockable::LetterPos();
	// jump holes requires more precise position :
	int mapmax = (int) (MAPMAX/m_system->m_navmapscale);
	int x = (m_pos.x)*40/mapmax+40;
	int z = (m_pos.z)*40/mapmax+40;
	CString letters = CString(char('A'+x/10));
	x = x%10;
	if (x < 5)
		letters += L"-"+CString(char('0'+5-x));
	else if (x > 5)
		letters += L"+"+CString(char('0'-5+x));
	letters += L",";
	letters += CString(char('1'+z/10),1);
	z = z%10;
	if (z < 5)
		letters += L"-"+CString(char('0'+5-z));
	else if (z > 5)
		letters += L"+"+CString(char('0'-5+z));
	if (m_pos.y < -1024)
		letters += L" (below horizon)";
	else if (m_pos.y > 1024)
		letters += L" (above horizon)";
	return letters;
}