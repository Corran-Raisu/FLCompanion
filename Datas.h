// Datas.h: interface for the Datas class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAS_H__E8E3DB26_F51B_418C_8C6D_D36E4334FC05__INCLUDED_)
#define AFX_DATAS_H__E8E3DB26_F51B_418C_8C6D_D36E4334FC05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LogDlg.h"
#include "ResourceProvider.h"

extern CLogDlg g_logDlg;
extern BOOL g_avoidLockedGates;
extern BOOL g_jumptrade;
extern BOOL g_avoidHoles;
extern BOOL g_avoidUnstableHoles;
extern BOOL g_avoidGates;
extern BOOL g_avoidLanes;
extern BOOL g_isTransport;
extern BOOL g_triggeredImport;
extern CString g_flAppPath;
extern CString g_tradeLaneCaption;
extern ResourceProvider g_resourceProvider;
struct ShipInfo {
	float m_defaultPrice;
	UINT m_hold_size;
	DWORD m_ids_name;
};
extern CSortedMap<CString,LPCTSTR,ShipInfo*,ShipInfo*> g_hullTofos;
extern CMap<CString,LPCTSTR,ShipInfo*,ShipInfo*> g_shippackToInfos;
extern CMap<CString,LPCTSTR,float,float> g_equipPrices;

//extern CString g_jumpHoleCaption;
//extern CString g_jumpGateCaption;

BOOL LoadAppDatas(CWnd *wnd);
void CalculateDirectRoutes();
BOOL PropagateRoutes();
BOOL BrowseFLDir(HWND hWnd, const CString &initialPath = g_flAppPath, BOOL displayWarning = false);
void LoadFiles(BOOL (*callback)(const CString &), LPCTSTR entriesName, LPCTSTR sectionName = L"Data");
DWORD FLHash(LPCTSTR string);
WORD  FLFactionHash(LPCTSTR str);

struct ModInfo
{
	CString name;
	CString author;
	CString url;
	CString description;
	CString FLC_info;
	CString FLC_info_name;
	int		largeMod;
};
extern ModInfo g_modInfo;

extern UINT BASE_DELAY;		// the base docking delay
extern UINT LANE_DELAY;		// the lane sequence delay
extern UINT JUMP_DELAY;		// the jumpgate sequence delay
extern UINT HOLE_DELAY;		// the jumphole sequence delay
extern UINT LANE_SPEED;		// speed in lane
extern UINT ENGINE_SPEED;	// full engine speed


#endif // !defined(AFX_DATAS_H__E8E3DB26_F51B_418C_8C6D_D36E4334FC05__INCLUDED_)
