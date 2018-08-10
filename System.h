// System.h: interface for the CSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYSTEM_H__E6DD50D3_E7FD_4B12_A41A_0F055556EDB7__INCLUDED_)
#define AFX_SYSTEM_H__E6DD50D3_E7FD_4B12_A41A_0F055556EDB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Base.h"
#include "Jump.h"

class CSystem;

#define SYSTEMS_MAX 512
extern int SYSTEMS_COUNT;

typedef CList<CSystem*, CSystem*> CSystemArray;
extern CSystem g_systems[SYSTEMS_MAX];
extern CSortedMap<CString,LPCTSTR,CSystem*,CSystem*> g_systemsByNick;

class CLane : public POS3D
{
public:
	CLane() : m_prev(NULL), m_next(NULL) {};
	CString m_caption;
	CLane *m_closest;
	UINT m_distance;
	CLane *m_prev;
	CLane *m_next;
};

struct CAsteroids
{
	CGood* m_good;
	double m_lootStat;
	LONG m_pos_x;
	LONG m_pos_z;
	LONG m_size_x;
	LONG m_size_z;
	LONG m_rotate_y;
};

class CSystem  
{
public:
	void ResetPath();
	void CalcLaneDistances(CDockable &to);
	UINT ComputeDistance(CDockable &from, CDockable &to, CLane **pbestRoute = NULL);
	void AddJump(CJump *gate);
	void AddBase(CBase *base);
	void RemoveBase(CBase *base);
	CSystem();
	~CSystem();

public:
	CList<CBase*, CBase*> m_bases;
	CMapEx<CString,LPCTSTR,CJump,CJump&> m_jumpsByNick;
	CMapEx<CString,LPCTSTR,CLane,CLane&> m_lanesByNick;
	CList<CLane*, CLane*> m_laneEnds;
	CList<POS3D, POS3D&> m_poiList;
	CMapEx<CString,LPCTSTR,CAsteroids,CAsteroids&> m_asteroids;
	CString m_caption;
	CString m_nickname;
	CString m_file;
	BOOL m_hasSell;
	BOOL m_avoid;
	double m_navmapscale;
	void SetMapScale(double navmapscale);
	int m_x, m_y;
	void SetPos(int x, int y) { m_x = x; m_y = y; }
	void Init(const CString &nickname, const CString &caption, const CString &m_file);

#ifdef DESACTIVE_POUR_LINSTANT
	void LoadMap(CStdioFile &file, const CMapStringToPtr &systemsByName);
	BOOL PropagateDistances();
	void CalcDistances();
private:
	UINT CalcDistance(CDockable *dock1, CDockable *dock2);
#endif
	operator UINT() { return this-g_systems; }
};


#endif // !defined(AFX_SYSTEM_H__E6DD50D3_E7FD_4B12_A41A_0F055556EDB7__INCLUDED_)
