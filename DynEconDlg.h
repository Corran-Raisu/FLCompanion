#if !defined(AFX_GAMEINSPECT_H__9F97255C_A826_4F43_A436_1482185AA628__INCLUDED_)
#define AFX_GAMEINSPECT_H__9F97255C_A826_4F43_A436_1482185AA628__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GameInspect.h : header file
//

#include "Base.h"
#include "Good.h"

struct FlNode;
struct FlTree;

/////////////////////////////////////////////////////////////////////////////
// CGameInspect class

class CGameInspect
{
// Construction
public:
	enum {	// must be in the same order as ID_GAME_IMPORT_*
		IMPORT_PRICES		= 0x01,
		IMPORT_CREDITS		= 0x02,
		IMPORT_CARGOHOLD	= 0x04,
		IMPORT_FACTIONS		= 0x08,
	};
	int DoTask(DWORD flags);
private:
	UINT CollectGoodPrice(DWORD id, LPVOID ptr);
	UINT CollectFactions(DWORD id, LPVOID ptr);
	UINT CGameInspect::TreeForEachRecurse(FlNode* nodePtr, FlNode* nodeNil, UINT (CGameInspect::*callback)(DWORD id, LPVOID ptr));
	UINT CGameInspect::FoundPlayer(DWORD id, LPVOID ptr);
	UINT TreeForEach(FlTree& tree, UINT (CGameInspect::*callback)(DWORD id, LPVOID ptr));
	LPVOID TreeFindRecurse(FlNode* nodePtr, DWORD id, FlNode* nodeNil);
	LPVOID TreeFind(FlTree& tree, DWORD id);
private:
	HANDLE						m_hflProcess;
	CBase*						m_base;
	CMap<DWORD,DWORD,UINT,UINT> m_idGoodMap;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAMEINSPECT_H__9F97255C_A826_4F43_A436_1482185AA628__INCLUDED_)
