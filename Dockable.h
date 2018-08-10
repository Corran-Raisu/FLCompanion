// Dockable.h: interface for the CDockable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOCKABLE_H__40002BD2_0BAB_4670_9019_4B66859BF2E4__INCLUDED_)
#define AFX_DOCKABLE_H__40002BD2_0BAB_4670_9019_4B66859BF2E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <math.h>

#define BASES_MAX	3072
extern UINT BASES_COUNT;

#define MAPMAX 131072

class CSystem;

typedef struct tagPOS3D
{
    LONG  x;
    LONG  y;
	LONG  z;
} POS3D;

inline UINT Distance(POS3D &p1, POS3D &p2)
{
	double dx = p2.x-p1.x;
	double dy = p2.y-p1.y;
	double dz = p2.z-p1.z;
	return (UINT) sqrt(dx*dx+dy*dy+dz*dz);
}


class CDockable  
{
public:
	CDockable() : m_system(NULL) {}
	~CDockable() {}
public:
	void ResetPath();
	void Init(const CString &nickname, const CString &caption, CSystem *system);
	void SetPos(int x, int y, int z = 0) { m_pos.x = x; m_pos.y = y; m_pos.z = z; }
	virtual CString LetterPos();
public:
	void Draw(CDC &dc, COLORREF color, BOOL displayNicknames);
	CSystem *m_system;
	CString m_caption;
	CString m_nickname;
	UINT m_distanceToBase[BASES_MAX];
	CDockable *m_shortestPath[BASES_MAX];
	POS3D m_pos;
};

#endif // !defined(AFX_DOCKABLE_H__40002BD2_0BAB_4670_9019_4B66859BF2E4__INCLUDED_)
