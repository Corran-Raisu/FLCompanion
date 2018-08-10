// Faction.h: interface for the CFaction class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FACTION_H__05038F85_2503_4BC3_B274_7AEA157DA550__INCLUDED_)
#define AFX_FACTION_H__05038F85_2503_4BC3_B274_7AEA157DA550__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFaction
{
public:
	CFaction() : m_avoid(false) {}
	~CFaction() {}
	void Init(const CString &nickname, const CString &caption);
public:
	CString m_caption;
	CString m_nickname;
	BOOL	m_avoid;
};

#define FACTIONS_MAX	256
extern UINT FACTIONS_COUNT;
extern CFaction g_factions[FACTIONS_MAX];
extern CSortedMap<CString,LPCTSTR,CFaction*,CFaction*> g_factionsByNick;


#endif // !defined(AFX_FACTION_H__05038F85_2503_4BC3_B274_7AEA157DA550__INCLUDED_)
