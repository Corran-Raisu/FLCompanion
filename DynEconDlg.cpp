// GameInspect.cpp : implementation file
//

#include "stdafx.h"
#include "FLCompanion.h"
#include "FLCompanionDlg.h"
#include "DynEconDlg.h"
#include "Base.h"
#include "IniFile.h"
#include "Datas.h"
#include "System.h"
#include <Psapi.h>
#include <memory>
#include "ScopedPtr.h"
#pragma comment(lib, "psapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGameInspect class

typedef unsigned int uint;

namespace Archetype
{
	struct Ship
	{
		uint iDunno1[2];
		uint iShipArchID;
		uint iDunno2;
		uint iType;
		uint iIDSName;
		uint iIDSInfo;
		float fHitPts;
		float fMass;
		uint iDunno3[2];
		float fRotationIntertia[3];
		uint iDunno4[45];
		uint iIDSInfo1;
		uint iIDSInfo2;
		uint iIDSInfo3;
		uint iShipClass;
		uint iNumExhaustNozzles;
		float fHoldSize;
		float fLinearDrag;
		float fAngularDrag[3];
		float fSteeringTorque[3];
		float fNudgeForce;
		float fStrafeForce;
		uint iDunno5[6];
		uint iMaxNanobots;
		uint iMaxShieldBats;
	};
}
struct FlNode
{
	FlNode*	_Left;
	FlNode*	_Parent;
	FlNode*	_Right;
	DWORD	_id;
};
struct FlTree
{
	DWORD	_allocator;
	FlNode*	_Head;
	FlNode*	_Nil;
	BOOL	_Multi;
	size_t	_Size;
};
struct FlBaseData
{
	DWORD			_base_id;
	DWORD			_start_location;
	float			_price_variance;
	float			_ship_repair_cost;
	FlTree			_market;
};
struct FlBaseDataNode
{
	FlBaseDataNode*	_Next;
	FlBaseDataNode*	_Prev;
	FlBaseData*		_Value;
};
struct FlBaseDataList
{
	DWORD			allocator;
	FlBaseDataNode*	_Head;
	size_t			_Size;
};
enum TransactionType
{
	TransactionType_Sell = 0,
	TransactionType_Buy = 1
};
struct MarketGoodInfo
{
	uint		iGoodID;
	float		fPrice;
	uint		iStock1;
	uint		iStock2;
	TransactionType iTransType;
	float		fMinLvl;			// min player level required
	float		fRep;				// min reputation level required
};
struct FlRep
{
	DWORD	_faction_id;
	float	_rep;
};
struct FlPlayer
{
	DWORD		_faction_id;
	char		_dummy[64];
	WCHAR		_playerName[24];
	DWORD		_dummy2;
	BYTE		_dummy3;
	FlRep*		_repsBegin;
	FlRep*		_repsEnd;
};
struct FlFactionInfo
{
	size_t		_name_len;
	char		_name[16];
	DWORD		_unknown[3];
	FlTree		_reput;
};

#define FLBASEDATALIST_ADDR 0x063FBB70
#define FLSHIPSTREE_ADDR	0x063FCAC0
#define FLCLIENTID_ADDR     0x00673344
#define FLCREDITS_ADDR		0x00673364
#define FLPLAYERSHIP_ADDR	0x0067337C
#define FLFACTIONS_ADDR		0x064018EC
#define FLPLAYERS_ADDR		0x064018C4 //0x064018C4	// FlTree des joueurs
// 0x64018D8 = currentNode
#define READFLMEM(structure,addr)	if (!ReadProcessMemory(m_hflProcess, LPCVOID(addr), &structure, sizeof(structure), NULL)) return 0;
#define READFLPTR(ptr,addr,size)	if (!ReadProcessMemory(m_hflProcess, LPCVOID(addr), LPVOID(ptr), size, NULL)) return 0;

UINT CGameInspect::CollectGoodPrice(DWORD id, LPVOID ptr)
{
	MarketGoodInfo goodData;
	READFLMEM(goodData, ptr);
	int result = 0;
	UINT goodIndex;
	if (m_idGoodMap.Lookup(id, goodIndex))
	{
		result = MAKELONG(1, 0);
		if (m_base->m_buy[goodIndex] != goodData.fPrice)
			result = MAKELONG(1, 1);
		m_base->m_buy[goodIndex] = goodData.fPrice;
		float sellPrice = (goodData.iTransType == TransactionType_Sell) ? goodData.fPrice : FLT_MAX;
		if (m_base->m_sell[goodIndex] != sellPrice)
			result = MAKELONG(1, 1);
		m_base->m_sell[goodIndex] = sellPrice;

		if (sellPrice < 1)
			ProblemFound(L"Client-Imported MarketGood entry (%s, %s) with value less than 1 credit from server", m_base->m_nickname, g_goods[goodIndex].m_nickname);
		if ((goodData.iTransType == TransactionType_Sell) && !m_base->m_hasSell)
		{
			m_base->m_hasSell = true;
			m_base->m_system->m_hasSell = true;
		}
	}
	return result;
}

UINT CGameInspect::CollectFactions(DWORD id, LPVOID ptr)
{
	FlFactionInfo factionData;
	READFLMEM(factionData, ptr);
	LPVOID playerReputPtr = TreeFind(factionData._reput, 1);
	if (playerReputPtr)
	{
		float playerReput;
		READFLMEM(playerReput, playerReputPtr);
	}
	return 0;
}
UINT CGameInspect::TreeForEachRecurse(FlNode* nodePtr, FlNode* nodeNil, UINT (CGameInspect::*callback)(DWORD id, LPVOID ptr))
{
	FlNode node;
	READFLMEM(node, nodePtr);
	int result = (this->*callback)(node._id, nodePtr+1);
	if (node._Left != nodeNil)
		result += TreeForEachRecurse(node._Left, nodeNil, callback);
	if (node._Right != nodeNil)
		result += TreeForEachRecurse(node._Right, nodeNil, callback);
	return result;
}

UINT CGameInspect::TreeForEach(FlTree& tree, UINT (CGameInspect::*callback)(DWORD id, LPVOID ptr))
{
	FlNode* nodeNil = tree._Nil;
	FlNode node;
	READFLMEM(node, tree._Head);
	// marketNode._Left  = feuille la plus à gauche
	// marketNode._Parent= sommet de l'arbre
	// marketNode._Right = feuille la plus à droite
	if (node._Parent != nodeNil)
		return TreeForEachRecurse(node._Parent, nodeNil, callback);
	return 0;
}

LPVOID CGameInspect::TreeFindRecurse(FlNode* nodePtr, DWORD id, FlNode* nodeNil)
{
	FlNode node;
	READFLMEM(node, nodePtr);
	if (node._id == id)
		return nodePtr+1;
	else if (node._id > id)
	{
		if (node._Left == nodeNil) return NULL;
		return TreeFindRecurse(node._Left, id, nodeNil);
	}
	else 
	{
		if (node._Right == nodeNil) return NULL;
		return TreeFindRecurse(node._Right, id, nodeNil);
	}
}

LPVOID CGameInspect::TreeFind(FlTree& tree, DWORD id)
{
	FlNode* nodeNil = tree._Nil;
	FlNode node;
	READFLMEM(node, tree._Head);
	if (node._Parent == nodeNil) return NULL;
	return TreeFindRecurse(node._Parent, id, nodeNil);
}
DWORD cb = 0;
HANDLE OpenGameProcess()
{
	if (cb == 0)
	{
		while (true)
		{
			DWORD lpcbNeeded_;
			std::unique_ptr<DWORD[]> _dwProcessIDs(new DWORD[cb]);
			if (EnumProcesses(_dwProcessIDs.get(), cb, &lpcbNeeded_) == FALSE)
				return NULL;
			if (cb == lpcbNeeded_)
			{
				Log(L"Searching additional processes: %d", cb);
				cb += 256;
			}
			else
			{
				break;
			}
		}
	}
	DWORD lpcbNeeded;
	std::unique_ptr<DWORD[]> dwProcessIDs(new DWORD[cb]);
	if (EnumProcesses(dwProcessIDs.get(), cb, &lpcbNeeded) == FALSE)
		return NULL;
	lpcbNeeded /= sizeof(DWORD);
	for (size_t idx = 0; idx < lpcbNeeded; idx++)
	{
		DWORD dwProcessID = dwProcessIDs[idx];

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|PROCESS_SET_INFORMATION, FALSE, dwProcessID);

		if (!hProcess) hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, dwProcessID);
		if (hProcess)
		{
			TCHAR filename[MAX_PATH];
			if (GetModuleBaseName(hProcess, NULL, filename, _countof(filename)))
				if (lstrcmpi(filename, L"Freelancer.exe") == 0)
					return hProcess;
			CloseHandle(hProcess);
		}
	}
	return NULL;
}

int CGameInspect::DoTask(DWORD flags)
{
	if (g_triggeredImport)
	{
		m_hflProcess = OpenGameProcess();
		static BOOL signalAccessDenied = true;
		if (!m_hflProcess)
		{
			if (signalAccessDenied)
			{
				Log(L"Could not find or access \"Freelancer.exe\" process! You may need to Run FLC as Administrator");
				signalAccessDenied = false;
			}
			return 0;
		}
		HANDLE hflProcess = m_hflProcess; // Leak! closehandle on function return
		signalAccessDenied = true;
		DWORD dwPriorityClass = GetPriorityClass(m_hflProcess);
		SetPriorityClass(m_hflProcess, IDLE_PRIORITY_CLASS);

		if (flags & IMPORT_PRICES)
		{
			CMap<DWORD, DWORD, CBase*, CBase*> idBaseMap;
			UINT i;
			for (i = 0; i < BASES_COUNT; i++)
				idBaseMap[FLHash(g_bases[i].m_nickname)] = &g_bases[i];
			for (i = 0; i < GOODS_COUNT; i++)
				m_idGoodMap[FLHash(g_goods[i].m_nickname)] = i;
			FlBaseDataList* baseDataListPtr;
			READFLMEM(baseDataListPtr, FLBASEDATALIST_ADDR);
			FlBaseDataList baseDataList;
			READFLMEM(baseDataList, baseDataListPtr);
			FlBaseDataNode* baseDataNodePtr = baseDataList._Head;
			UINT basesFound = 0;
			UINT pricesChanged = 0;
			for (i = 0; i < baseDataList._Size; i++)
			{
				FlBaseDataNode baseDataNode;
				READFLMEM(baseDataNode, baseDataNodePtr);
				if (baseDataNode._Value)
				{
					FlBaseData baseData;
					READFLMEM(baseData, baseDataNode._Value);
					if (idBaseMap.Lookup(baseData._base_id, m_base))
					{
						basesFound++;
						pricesChanged += TreeForEach(baseData._market, &CGameInspect::CollectGoodPrice);
					}
				}
				baseDataNodePtr = baseDataNode._Next;
			}
			if (basesFound != BASES_COUNT)
			{
				static BOOL signalBadBases = true;
				if (signalBadBases)
				{
					signalBadBases = false;
					Log(L"Only %d bases out of %d were recognized in game", basesFound, BASES_COUNT);
				}
			}
			//if (HIWORD(pricesChanged))
			{
				Log(L"Imported from game: %d changed prices", HIWORD(pricesChanged));
				g_mainDlg->Recalc(g_mainDlg->RECALC_SOLUTIONS);
			}
		}
		if (flags & IMPORT_CREDITS)
		{
			DWORD credits;
			READFLMEM(credits, FLCREDITS_ADDR);
			if (credits && g_mainDlg->SetMaxInvestment(credits))
				Log(L"Imported from game: Investment set to %d credits", credits);
		}
		if (flags & IMPORT_CARGOHOLD)
		{
			DWORD playerShipId;
			READFLMEM(playerShipId, FLPLAYERSHIP_ADDR);
			FlTree shipsTree;
			READFLMEM(shipsTree, FLSHIPSTREE_ADDR);
			LPVOID ptr = TreeFind(shipsTree, playerShipId);
			if (ptr)
			{
				Archetype::Ship* shipPtr;
				READFLMEM(shipPtr, ptr);
				if (shipPtr)
				{
					Archetype::Ship ship;
					READFLMEM(ship, shipPtr);
					UINT cargoSize = UINT(ship.fHoldSize);
					if (g_mainDlg->SetCargoSize(cargoSize))
						Log(L"Imported from game: Cargo hold size set to %d units", cargoSize);
				}
			}
		}
#if 0
		
		if (true) //(flags & IMPORT_FACTIONS)
		{
			UINT changed = 0;
			CMap<DWORD, DWORD, CFaction*, CFaction*> idFactionMap;
			UINT i;
			for (i = 0; i < FACTIONS_COUNT; i++)
				idFactionMap[FLFactionHash(g_factions[i].m_nickname)] = &g_factions[i];

			FlTree factionsTree;
			READFLMEM(factionsTree, FLFACTIONS_ADDR);

					LPBYTE ptr;
					READFLMEM(ptr, 0x61e0260);
					DWORD player_id;
					int oset = 0;
					player_id = 0;
					while (player_id==0)
					{
						READFLMEM(player_id, ptr + (4+ oset));
						oset += 4;
					}

					FlTree playersTree;
					READFLMEM(playersTree, FLPLAYERS_ADDR);
					FlNode playerNode;
					READFLMEM(playerNode, playersTree._Head);
					READFLMEM(playerNode, playerNode._Parent);
					LPVOID playerPtr = TreeFind(playersTree, 1);
					if (playerPtr)
					{
						FlPlayer player;
						READFLMEM(player, playerPtr);
						int count = player._repsEnd-player._repsBegin;
						//CScopedArray<FlRep> reps = new FlRep[count];
						//READFLPTR(reps,player._repsBegin,  count*sizeof(FlRep));
						for (int index = 0; index < count; index++)
							//if (reps[index]._rep <= -0.6)
							{
								//CFaction *faction = idFactionMap[reps[index]._faction_id];
								////if (!faction->m_avoid)
								{
									changed++;
									//faction->m_avoid = true;
								}
							}
					}
					if (changed)
					{
						Log(L"Imported from game: %d factions to avoid", changed);
						g_mainDlg->Recalc(g_mainDlg->RECALC_PATHS);
					}

		}
		
#endif
		SetPriorityClass(m_hflProcess, dwPriorityClass);
	}
	return 1;
}

// v10			v11
//				0x0673364 : money
// 0x639F39C	0x639F3CC : tradelane speed


// ship* +100=holdsize
// shipid du joueur en 67337c
// 63fcac0:map  des ship*

// 0x64018f0
// 0x64018C4



// 673320 IdsTree (mélange de FactionID et FLHash) { BYTE flags; }
// 61226C

// 64018EC: tree des groups?
