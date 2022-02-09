// Datas.cpp: implementation of the Datas class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FLCompanion.h"
#include "Datas.h"
#include "ResourceProvider.h"
#include "IniFile.h"
#include "System.h"
#include <ShObjIdl.h>
#include <math.h>
#include "Faction.h"

_COM_SMARTPTR_TYPEDEF(IFileOpenDialog, IID_IFileOpenDialog);

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BOOL g_avoidLockedGates;
BOOL g_jumptrade;
BOOL g_avoidHoles;
BOOL g_avoidUnstableHoles;
BOOL g_avoidGates;
BOOL g_avoidLanes;
BOOL g_isTransport;
BOOL g_triggeredImport;
ModInfo g_modInfo;

UINT BASE_DELAY = 20*1000;
UINT LANE_DELAY = 0;
UINT JUMP_DELAY = 15*1000; // the jumpgate sequence takes 15 seconds
UINT HOLE_DELAY = 15*1000; // the jumphole sequence takes 15 seconds
UINT LANE_SPEED = 2000;		// speed in lane is 2000 per second
UINT ENGINE_SPEED = 300;	// full engine speed is 300 per second

CString g_flAppPath;
CString g_tradeLaneCaption;
CString g_tempExcludeList;
//CString g_jumpHoleCaption;
//CString g_jumpGateCaption;
ResourceProvider g_resourceProvider;
enum SolarArchType
{
	ARCH_STATION,
//	ARCH_STATION_NODOCK,
	ARCH_JUMP_GATE,
	ARCH_JUMP_HOLE,
	ARCH_TRADELANE,
	ARCH_POI,
};
CMap<CString,LPCTSTR,SolarArchType,SolarArchType> g_solarArch;
CList<DWORD,DWORD> g_lockedGates;
CMapEx<CString,LPCTSTR,ShipInfo,ShipInfo&> g_shipInfos;
CMap<CString,LPCTSTR,ShipInfo*,ShipInfo*> g_hullToInfos;
CMap<CString,LPCTSTR,ShipInfo*,ShipInfo*> g_shippackToInfos;
CMap<CString,LPCTSTR,float,float> g_equipPrices;

DWORD FLHash(LPCTSTR str)
{
	DWORD crc = 0;
	while (BYTE ch = BYTE(*str))
	{
		str++;
		ch ^= crc;
		crc = (crc>>8) ^ (ch<<7)^(ch<<20)^(ch<<22);
	}
	return (_byteswap_ulong(crc) >> 2) | 0x80000000ul;
}

WORD FactionIDHashTable[256];

WORD FLFactionHash(LPCTSTR str)
{
    WORD hash = 0xFFFF;
    while (BYTE ch = BYTE(*str))
    {
		str++;
        hash = (hash>>8) ^ FactionIDHashTable[BYTE(ch^hash)];
    }
	return hash;
}

void InitializeHashTable()
{
    for (int index = 0; index < 256; index++)
    {
        DWORD hash = index << 8;
        for (int j = 0; j < 8; j++)
        {
			hash <<= 1;
			if (hash & 0x10000)
				hash ^= 0x1021;
        }
        FactionIDHashTable[index] = WORD(hash);
    }
}


BOOL LoadInitialWorld(const CString &iniFilename)
{
	IniSection section;
	CString name, systemname;
	Log(L"\nParsing Initial World...");
	{
		CFaction &faction = g_factions[FACTIONS_COUNT++];
		faction.Init("fc_player_grp", "The Player");
		g_factionsByNick["fc_player_grp"] = &faction;
	}
	{
		IniFile iniFile(iniFilename);
		section = NULL;
		while (iniFile.GetNextSection(section, name))
		{
			if (name.CompareNoCase(L"locked_gates") == 0)
			{
				IniEntry entry;
				for (UINT entriesCount = iniFile.EnumEntries(section, entry); entriesCount; entriesCount--)
				{
					UINT valuesCount;
					IniValue *values;
					if (iniFile.GetNextEntry(entry, valuesCount, values).CompareNoCase(L"locked_gate") != 0)
						continue;
					g_lockedGates.AddTail(iniFile.GetValueDWORD(values,0));
				}
			}
			else if (name.CompareNoCase(L"group") == 0)
			{
				name = iniFile.GetValue(section, "nickname");
				name.MakeLower();
				CFaction &faction = g_factions[FACTIONS_COUNT++];
				faction.Init(name, g_resourceProvider.GetStringFromID(iniFile.GetValueInt(section, "ids_name")));
				if (g_tempExcludeList.Find(L','+name+L',') >= 0)
					faction.m_avoid = true;
				g_factionsByNick[name] = &faction;	
				IniEntry entry;
				UINT entrynumber = 0;
				for (UINT entriesCount = iniFile.EnumEntries(section, entry); entriesCount; entriesCount--)
				{
					UINT valuesCount;
					IniValue *values;
					FLOAT rep;
					if (iniFile.GetNextEntry(entry, valuesCount, values).CompareNoCase(L"rep") != 0)
						continue;
					name = iniFile.GetValue(values, 1);
					rep = iniFile.GetValueFloat(values, 0)*100;
					faction.m_reputations[entrynumber].Init(name, rep);
					faction.m_reputationCount++;
					faction.repsByNick[name] = &faction.m_reputations[entrynumber];
					entrynumber++;
					
				}
			}
		}
	}
	return TRUE;
}

BOOL LoadStationSolarArch(const CString &iniFilename)
{
	IniSection section;
	CString name, type, systemname;
	Log(L"Parsing Solars...");
	{
		IniFile iniFile(iniFilename);
		section = NULL;
		while (iniFile.GetNextSection(section, name))
		{
			if (name.CompareNoCase(L"Solar") == 0)
			{
				name = iniFile.GetValue(section, "nickname");

				name.MakeLower();
				type = iniFile.GetValue(section, "type");
				type.MakeUpper();
				if ((type.CompareNoCase(L"STATION") == 0) || (type.CompareNoCase(L"DOCKING_RING") == 0))
				{
					IniValue *value;
					if (iniFile.GetValues(section, "docking_sphere", value))
					{
						CString dock = iniFile.GetValue(value,0);
						BOOL dockable = (dock.CompareNoCase(L"berth") == 0) || (dock.CompareNoCase(L"ring") == 0) || (dock.CompareNoCase(L"jump") == 0);
						if (dockable)
							g_solarArch[name] = ARCH_STATION;
					}
				}
				else if (type.CompareNoCase(L"JUMP_GATE") == 0)
					g_solarArch[name] = ARCH_JUMP_GATE;
				else if ((type.CompareNoCase(L"JUMP_HOLE") == 0) || (type.CompareNoCase(L"AIRLOCK_GATE") == 0))
					g_solarArch[name] = ARCH_JUMP_HOLE;
				else if (type.CompareNoCase(L"TRADELANE_RING") == 0)
					g_solarArch[name] = ARCH_TRADELANE;
				else if ((type.CompareNoCase(L"MISSION_SATELLITE") == 0) || (type.CompareNoCase(L"DESTROYABLE_DEPOT") == 0))
					g_solarArch[name] = ARCH_POI;
			}
		}
	}
	return TRUE;
}

BOOL LoadSystems(const CString &iniFilename)
{
	IniSection section;
	CString name, systemname;
	Log(L"Parsing Systems...");
	{
		IniFile iniFile(iniFilename);
		section = NULL;
		while (iniFile.GetNextSection(section, name))
		{
			if (name.CompareNoCase(L"system") == 0)
			{
				name = iniFile.GetValue(section, "nickname");
				name.MakeLower();

				if (SYSTEMS_COUNT == SYSTEMS_MAX)
				{
					ProblemFound(L"This mod has too many systems for FLCompanion !");
					continue;
				}		
				CString file = iniFile.GetValue0(section, "file");
				if (!file.IsEmpty())
				{
					CSystem &system = g_systems[SYSTEMS_COUNT++];
					system.Init(name, g_resourceProvider.GetStringFromID(iniFile.GetValueInt(section, "strid_name")), file);
					if (g_tempExcludeList.Find(L','+name+L',') >= 0)
						system.m_avoid = true;
					IniValue *value;
					if (iniFile.GetValues(section, "NavMapScale", value))
						system.SetMapScale(iniFile.GetValueFloat(value,0));
					if (iniFile.GetValues(section, "pos", value))
						system.SetPos(iniFile.GetValueInt(value,0),iniFile.GetValueInt(value,1));
					g_systemsByNick[name] = &system;
				}
			}
		}
	}
	return TRUE;
}

BOOL LoadBases(const CString &iniFilename)
{		
	IniSection section;
	CString name, systemname;
	Log(L"Parsing Bases...");
	{
		IniFile iniFile(iniFilename);
		section = NULL;
		while (iniFile.GetNextSection(section, name))
		{
			if (name.CompareNoCase(L"Base") == 0)
			{
				name = iniFile.GetValue(section, "nickname");
				name.MakeLower();
				
				DWORD strid = iniFile.GetValueInt(section, "strid_name");
				CString caption = g_resourceProvider.GetStringFromID(strid);
				if (!g_basesByNick.Contains(name)) // ignore unwanted bases
					continue;
				CBase &base = *g_basesByNick[name];
				if (wcsncmp(name, L"intro", 5) == 0) // ignore intro bases
					continue;
				base.m_caption = caption;
			}
		}
	}
	return TRUE;
}

BOOL LoadGoods(const CString &iniFilename)
{
	IniSection section;
	CString name;
	Log(L"Parsing Goods...");
	{
		//Add a blank item at ID 0 for returns with no commodity.
		CGood &good = g_goods[GOODS_COUNT++];
		good.InitPrice(0);
		good.Init("return_novalue", "Return with No Commodity", 1, 0);
	}
	{ // fetch standard goods id & price
		IniFile iniFile(iniFilename);
		section = NULL;
		while (iniFile.GetNextSection(section, name))
		{
			if (name.CompareNoCase(L"Good") == 0)
			{
				CString category = iniFile.GetValue0(section, "category");
				if (category.CompareNoCase(L"commodity") == 0)
				{
					name = iniFile.GetValue(section, "nickname");
					//if ((name == L"commodity_vips") || (name == L"commodity_prisoners")) continue;
					if (GOODS_COUNT == GOODS_MAX)
					{
						ProblemFound(L"This mod has too many goods for FLCompanion !");
						continue;
					}
					CGood &good = g_goods[GOODS_COUNT++];
					good.InitPrice(iniFile.GetValueFloat(section, "price"));
					if (g_tempExcludeList.Find(L','+name+L',') >= 0)
						good.m_avoid = true;
					g_goodsByNick[name] = &good;
				}
				else if (category.CompareNoCase(L"shiphull") == 0)
				{
					name = iniFile.GetValue(section, "nickname");
					name.MakeLower();
					ShipInfo *info;
					if (g_hullToInfos.Lookup(name, info))
					{
						ProblemFound(L"Goods: Shiphull %s defined twice", name);
						continue;
					}
					CString ship = iniFile.GetValue(section, "ship"); 
					info = &g_shipInfos[ship];
					info->m_defaultPrice = iniFile.GetValueFloat(section, "price");
					g_hullToInfos[name] = info;
				}
				else if (category.CompareNoCase(L"ship") == 0)
				{
					name = iniFile.GetValue(section, "nickname");
					name.MakeLower();
					ShipInfo *info;
					if (g_shippackToInfos.Lookup(name, info))
					{
						ProblemFound(L"Goods: Ship %s defined twice", name);
						continue;
					}
					CString hull = iniFile.GetValue(section, "hull");
					hull.MakeLower();
					if (!g_hullToInfos.Lookup(hull, info))
					{
						ProblemFound(L"Goods: Ship %s references an unknown shiphull %s", name, hull);
						continue;
					}
					g_shippackToInfos[name] = info;
				}
				else if (category.CompareNoCase(L"equipment") == 0)
				{
					name = iniFile.GetValue(section, "nickname");
					name.MakeLower();
					float price;
					if (g_equipPrices.Lookup(name, price))
					{
						ProblemFound(L"Goods: Equipment %s defined twice", name);
						continue;
					}
					CString equipName = iniFile.GetValue(section, "equipment");
					equipName.MakeLower();
					if (equipName != name)
						ProblemFound(L"Goods: nickname & equipment differs for %s", name);
					g_equipPrices[name] = iniFile.GetValueFloat(section, "price");
				}
			}
		}
	}
	return TRUE;
}

BOOL LoadEquip(const CString &iniFilename)
{
	IniSection section;
	CString name;
	{ // fetch caption & decay for those goods
		IniFile iniFile(iniFilename);
		section = NULL;
		while (iniFile.GetNextSection(section, name))
		{
			if (name.CompareNoCase(L"Commodity") == 0)
			{
				name = iniFile.GetValue(section, "nickname");
				if (!g_goodsByNick.Contains(name)) // only the goods we added previously
					continue;
				CGood &good = *g_goodsByNick[name];
				float decay_per_second = iniFile.GetValueFloat0(section, "decay_per_second");
				good.Init(name, g_resourceProvider.GetStringFromID(iniFile.GetValueInt(section, "ids_name")), 
					iniFile.GetValueFloat(section, "volume"),
					decay_per_second == 0.0 ? 0 : UINT((1000*iniFile.GetValueFloat(section, "hit_pts"))/decay_per_second));
			}
			if (name.CompareNoCase(L"Tractor") == 0)
			{
				name = iniFile.GetValue(section, "nickname");
				if (name.Find(L"license") > 0)
				{
					CGood &equip = g_equip[EQUIP_COUNT++];
					equip.Init(name, g_resourceProvider.GetStringFromID(iniFile.GetValueInt(section, "ids_name")), 1, 0);
					g_equipByNick[name] = &equip;
				}
			}
		}
	}
	return TRUE;
}

BOOL LoadMarketPrices(const CString &iniFilename)
{
	IniSection section;
	CString name;
	Log(L"Parsing Market prices...");
	{
		IniFile iniFile(iniFilename);
		section = NULL;
		while (iniFile.GetNextSection(section, name))
		{
			CHECK(name.CompareNoCase(L"BaseGood") == 0);
			name = iniFile.GetValue0(section, "base");
			if (name.IsEmpty())
			{
				ProblemFound(L"[BaseGood] section missing base= entry in %s", iniFilename);
				continue;
			}
			name.MakeLower();
			if (!g_basesByNick.Contains(name)) // ignore unwanted bases
				continue;
			CBase &base = *g_basesByNick[name];
			IniEntry entry;
			for (UINT entriesCount = iniFile.EnumEntries(section, entry); entriesCount; entriesCount--)
			{
				UINT valuesCount;
				IniValue *values;
				if (iniFile.GetNextEntry(entry, valuesCount, values).CompareNoCase(L"MarketGood") != 0)
					continue;
				name = iniFile.GetValue(values,0);
				if (valuesCount < 7)
				{
					ProblemFound(L"MarketGood entry (%s) with less than 7 parameters in %s", name, iniFilename);
					continue;
				}
				if (!g_goodsByNick.Contains(name)) // only the goods we added previously
					continue;
				CGood &good = *g_goodsByNick[name];
				// iniFile.GetValueInt(values,1) : Minimum character level to purchase
				//
				// iniFile.GetValueInt(values,2) : Minimum faction level to purchase (can be decimal)
				//		-1 = even if very hostile, 0 = must be at least neutral, 1 = need to be very friendly
				UINT buyonly = iniFile.GetValueInt(values,5);
				if ((buyonly != 0) && (buyonly != 1)) ProblemFound(L"MarketGood entry (%s on %s) with 6th value (buy only) different than 0 or 1 (%d)", name, base.m_nickname, buyonly);
				float price = (good.m_defaultPrice*iniFile.GetValueFloat(values,6));
				if(price < 1)
					ProblemFound(L"MarketGood entry (%s, %s) with value less than 1 credit in %s", base.m_nickname, name, iniFilename);
				//if (base.m_buy[good] != 0) ProblemFound(L"MarketGood entry (%s on %s) is defined twice", name, base.m_nickname);
				
				if (buyonly)
				{
					if ((iniFile.GetValueInt(values,3) != 0) || (iniFile.GetValueInt(values,4) != 0)) ProblemFound(L"MarketGood buy-only entry (%s on %s) with 4th or 5th value different than 0", name, base.m_nickname);
					base.m_buy[good] = price;
					base.m_sell[good] = FLT_MAX;
				}
				else
				{
					//CHECK((iniFile.GetValueInt(values,3) == 150) && (iniFile.GetValueInt(values,4) == 500));
					//	supposedly: number of items in stock (but not implemented in game)
					base.m_buy[good] = price;
					base.m_sell[good] = price;
					if (!base.m_hasSell)
					{
						base.m_hasSell = true;
						base.m_system->m_hasSell = true;
					}
				}
			}
		}
	}
	return TRUE;
}

BOOL LoadShips(const CString &iniFilename)
{
	IniSection section;
	CString name;
	{ // fetch ids_name for those ships
		IniFile iniFile(iniFilename);
		section = NULL;
		while (iniFile.GetNextSection(section, name))
		{
			if (name.CompareNoCase(L"Ship") == 0)
			{
				name = iniFile.GetValue(section, "nickname");
				if (!g_shipInfos.Contains(name)) // only the ships we added previously (available on the market)
					continue;
				ShipInfo &info = g_shipInfos[name];
				info.m_hold_size = iniFile.GetValueInt(section, "hold_size");
				info.m_ids_name = iniFile.GetValueInt(section, "ids_name");
			}
		}
	}
	return TRUE;
}

BOOL LoadIDHacks()
{
	//WIP
	Log(L"Parsing ID Rep Hacks");
	IniSection section;
	CString sectionname;
	CString entryname;
	CString name;
	{
		IniFile iniFile(L"res\\IDHacks.ini");
		section = NULL;
		while (iniFile.GetNextSection(section, sectionname))
		{
			CString IDName = g_equipByNick[sectionname]->m_caption;
			CFaction &ID = g_ID[ID_COUNT++];
			ID.Init(sectionname, g_equipByNick[sectionname]->m_caption);
			IniEntry entry;
			UINT entrynumber = 0;
			for (UINT entriesCount = iniFile.EnumEntries(section, entry); entriesCount; entriesCount--)
			{
				UINT valuesCount;
				IniValue *values;
				FLOAT rep;
				name = iniFile.GetNextEntry(entry, valuesCount, values);
				if (valuesCount == 0)
					rep = 0;
				else
					rep = iniFile.GetValueFloat(values, 0) * 100;
				ID.m_reputations[entrynumber].Init(name, rep);
				ID.m_reputationCount++;
				ID.repsByNick[name] = &ID.m_reputations[entrynumber];
				entrynumber++;

			}
			g_IDByNick[name] = &ID;
		}
	}
	return TRUE;
}

BOOL LoadAsteroids(const CString &iniFilename, CAsteroids& asteroids)
{
	IniFile iniFile(iniFilename);
	IniSection section = iniFile.FindFirstSection("LootableZone");
	if (section)
	{
		CString name = iniFile.GetValue(section, "dynamic_loot_commodity");
		if (g_goodsByNick.Lookup(name, asteroids.m_good))
		{
			IniValue *value;
			if (iniFile.GetValues(section, "dynamic_loot_count", value))
			{
				int lootMin = iniFile.GetValueInt(value,0);
				int lootMax = iniFile.GetValueInt(value,1);
				int chance = iniFile.GetValueInt(section, "dynamic_loot_difficulty");
				if ((lootMin <= lootMax) && (lootMax > 0) && (chance > 0))
				{
					asteroids.m_lootStat = (lootMin+lootMax)/(chance*2.0);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

void LoadSystemObjects()
{
	IniSection section;
	CString name;
	for (int sysIndex = 0; sysIndex < SYSTEMS_COUNT; sysIndex++)
	{
		CSystem &system = g_systems[sysIndex];
		if (!PathFileExists(IniFile::g_basePath+L"Universe\\"+system.m_file))
		{
			ProblemFound(L"File Universe\\%s not found for system %s", system.m_file, system.m_nickname);
			continue;
		}
		IniFile iniFile(L"Universe\\"+system.m_file);
		section = NULL;
		while (iniFile.GetNextSection(section, name))
		{
			if (name.CompareNoCase(L"Object") == 0)
			{
				IniValue *value, *value2;
				if (iniFile.GetValues(section, "pos", value) != 3)
				{
					ProblemFound(L"Object %s with invalid or missing pos= in file Universe\\%s", iniFile.GetValue(section, "nickname"), system.m_file);
					continue;
				}
				int x = iniFile.GetValueInt(value,0);
				int y = iniFile.GetValueInt(value,1);
				int z = iniFile.GetValueInt(value,2);
				name = iniFile.GetValue(section, "Archetype");
				name.MakeLower();
				SolarArchType archType;
				if (!g_solarArch.Lookup(name, archType))
				{
					//if (iniFile.GetValues(section, "dock_with", value)) // on devrait tomber sur les stations sans docking_sphere ou de type incompatible
						//ProblemFound(L"Object %s uses an undefined station Archetype= in file Universe\\%s", iniFile.GetValue(section, "nickname"), system.m_file);
					if (iniFile.GetValues(section, "goto", value) == 3) // jump hole or jump gate
						ProblemFound(L"Object %s uses an undefined jump Archetype= in file Universe\\%s", iniFile.GetValue(section, "nickname"), system.m_file);
					if (iniFile.GetValues(section, "prev_ring", value) || iniFile.GetValues(section, "next_ring", value2))
						ProblemFound(L"Object %s uses an undefined tradelane Archetype= in file Universe\\%s", iniFile.GetValue(section, "nickname"), system.m_file);
					continue;
				}
				switch (archType)
				{
				case ARCH_STATION:
					if (!iniFile.GetValues(section, "dock_with", value)) // base dock
						ProblemFound(L"Station %s is missing a dock_with= in file Universe\\%s", iniFile.GetValue(section, "nickname"), system.m_file);
					else
					{
						name = iniFile.GetValue(value,0);
						name.MakeLower();
						if (g_basesByNick.Contains(name) && (g_basesByNick[name] != NULL)) // ignore base multiple docking facilities
							continue;
						DWORD strid = iniFile.GetValueInt0(section, "ids_name");
						if (BASES_COUNT == BASES_MAX)
						{
							ProblemFound(L"This mod has too many bases for FLCompanion !");
							continue;
						}
						CString faction = iniFile.GetValue0(section, "reputation");
						faction.MakeLower();
						CString dock = iniFile.GetValue(section, "archetype");
						CBase &base = g_bases[BASES_COUNT++];
						base.Init(name, g_resourceProvider.GetStringFromID(strid), &system, faction, ((dock.Find(L"_fighter") > 0) || (dock.Find(L"_notransport") > 0) || (dock.CompareNoCase(L"dsy_comsat_planetdock") == 0) ));
						g_basesByNick[name] = &base;
						base.m_system->AddBase(&base);
						base.SetPos(x,y,z);
						//OutputDebugFormat(L"%s\t%d,%d,%d\n", base.m_caption, x,y,z);
					}
					break;
				case ARCH_JUMP_GATE:
				case ARCH_JUMP_HOLE:
					if (iniFile.GetValues(section, "goto", value) != 3) // jump hole or jump gate
						ProblemFound(L"Jump %s is missing a valid goto= in file Universe\\%s", iniFile.GetValue(section, "nickname"), system.m_file);
					else
					{
						name = iniFile.GetValue(section, "nickname");
						name.MakeLower();
						CJump &jump = system.m_jumpsByNick[name];
						CString dock = iniFile.GetValue(section, "archetype");
						jump.Init(name,
							archType == ARCH_JUMP_GATE,
							g_lockedGates.Find(FLHash(name.MakeLower())) != NULL, // true if gate is locked in initial world
							( (dock.Find(L"_fighter") > 0) || (dock.Find(L"_notransport") > 0)),
							g_resourceProvider.GetStringFromID(iniFile.GetValueInt0(section, "ids_name")), // caption
							&system); // system it is in
						jump.SetPos(x,y,z);
						CString sysname = iniFile.GetValue(value,0);
						sysname.MakeLower();
						CSystem *matchingSystem;
						if (!g_systemsByNick.Lookup(sysname, matchingSystem))
						{
							ProblemFound(L"Jump gate/hole %s in system %s points to an undefined system %s in file Universe\\%s", name, system.m_nickname, sysname, system.m_file);
							continue;
						}
						name = iniFile.GetValue(value,1);
						name.MakeLower();
						CJump *matchingjump = &matchingSystem->m_jumpsByNick[name];
						matchingjump->m_nickname = name;
						jump.SetMatchingJump(matchingjump);
					}
					break;
				case ARCH_TRADELANE:
					iniFile.GetValues(section, "prev_ring", value);
					iniFile.GetValues(section, "next_ring", value2);
					if (!value && !value2)
						ProblemFound(L"Tradelane %s is missing a prev_ring= or next_right= in file Universe\\%s", iniFile.GetValue(section, "nickname"), system.m_file);
					else // trade lanes
					{
						name = iniFile.GetValue(section, "nickname");
						CLane &lane = system.m_lanesByNick[name];
						lane.x = x;
						lane.y = y;
						lane.z = z;
						IniValue *value3;
						if (value)
							lane.m_prev = &system.m_lanesByNick[iniFile.GetValue(value,0)];
						else
						{
							iniFile.GetValues(section, "tradelane_space_name", value3);
							UINT n = value3 ? iniFile.GetValueInt(value3,0) : iniFile.GetValueInt(section, "ids_name");
							if (n == 5621)
								n = 196776;
							lane.m_caption = g_resourceProvider.GetStringFromID(n);
						}
		
						if (value2)
							lane.m_next = &system.m_lanesByNick[iniFile.GetValue(value2,0)];
						else
						{
							iniFile.GetValues(section, "tradelane_space_name", value3);
							UINT n = value3 ? iniFile.GetValueInt(value3,0) : iniFile.GetValueInt(section, "ids_name");
							if (n == 5611)
								n = 196766;
							lane.m_caption = g_resourceProvider.GetStringFromID(n);
						}
						if (!value || !value2)
							system.m_laneEnds.AddTail(&lane);
					}
					break;
				case ARCH_POI: // point of interest (wreck or depot)
					{
						POS3D pos = { x, y, z };
						system.m_poiList.AddTail(pos);
					}
					break;
				}
			}
			else if (name.CompareNoCase(L"Asteroids") == 0)
			{
				CAsteroids asteroids;
				memset(&asteroids, 0, sizeof(asteroids));
				name = iniFile.GetValue(section, "zone");
				LoadAsteroids(iniFile.GetValue(section, "file"), asteroids);
				system.m_asteroids[name] = asteroids;
			}
			else if (name.CompareNoCase(L"Zone") == 0)
			{
				CAsteroids* asteroids;
				name = iniFile.GetValue(section, "nickname");
				if (system.m_asteroids.LookupPtr(name, asteroids))
				{
					IniValue *value;
					if (iniFile.GetValues(section, "pos", value) != 3)
					{
						ProblemFound(L"Object %s with invalid or missing pos= in file Universe\\%s", name, system.m_file);
						continue;
					}
					asteroids->m_pos_x = iniFile.GetValueInt(value,0);
					asteroids->m_pos_z = iniFile.GetValueInt(value,2);
					CString shape = iniFile.GetValue(section, "shape");
					if (shape == L"ELLIPSOID")
					{
						if (iniFile.GetValues(section, "size", value) != 3)
						{
							ProblemFound(L"Object %s with invalid or missing size= in file Universe\\%s", name, system.m_file);
							continue;
						}
						asteroids->m_size_x = iniFile.GetValueInt(value,0);
						asteroids->m_size_z = iniFile.GetValueInt(value,2);
						switch (iniFile.GetValues(section, "rotate", value))
						{
						case 3:
							asteroids->m_rotate_y = iniFile.GetValueInt(value,1);
						case 0:
							break;
						default:
							ProblemFound(L"Object %s with invalid or missing rotate= in file Universe\\%s", name, system.m_file);
						}
					}
					else if (shape == L"SPHERE")
					{
						if (iniFile.GetValues(section, "size", value))
							asteroids->m_size_x = asteroids->m_size_z = iniFile.GetValueInt(value,0);
						else
							ProblemFound(L"Object %s with missing size= in file Universe\\%s", name, system.m_file);
					}
					else
						ProblemFound(L"Asteroid field %s uses an undefined shape in file Universe\\%s", name, system.m_file);
				}
			}
		}
	}
}

void CalculateDirectRoutes()
{
	for (int sysIndex = 0; sysIndex < SYSTEMS_COUNT; sysIndex++)
		g_systems[sysIndex].ResetPath();
	
	for (UINT baseIndex = 0; baseIndex < BASES_COUNT; baseIndex++)
	{
		CBase &tobase = g_bases[baseIndex];
		if (tobase.m_faction && tobase.m_faction->m_avoid)
			continue;
		if ( g_isTransport && tobase.m_isfreighteronly)
			continue;
		CSystem &system = *tobase.m_system;
		system.CalcLaneDistances(tobase);
		// in-system initial-base to final-base
		POSITION pos = system.m_bases.GetHeadPosition();
		while (pos)
		{
			CBase &frombase = *system.m_bases.GetNext(pos);
			frombase.m_shortestPath[baseIndex] = &tobase;
			frombase.m_distanceToBase[baseIndex] = system.ComputeDistance(frombase, tobase);
		}
		// in-system middle-jumps to final-base
		CString dummy;
		pos = system.m_jumpsByNick.GetStartPosition();
		while (pos)
		{
			CJump &fromjump = system.m_jumpsByNick.GetNextAssoc(pos, dummy);
			if (g_avoidLockedGates && fromjump.m_islocked) continue;
			if (g_avoidHoles && !fromjump.m_isgate) continue;
			if (g_avoidGates && fromjump.m_isgate) continue;
			if (g_isTransport && fromjump.m_isfreighteronly) continue;
			fromjump.m_shortestPath[baseIndex] = &tobase;
			fromjump.m_distanceToBase[baseIndex] = system.ComputeDistance(fromjump, tobase);
			if (!fromjump.m_matchingJump)
			{
				fromjump.m_matchingJump = &fromjump;
				fromjump.m_system = &system;
				ProblemFound(L"A jump gate/hole references an undefined %s in %s", dummy, system.m_nickname);
			}
			else if (!system.m_avoid)
			{
				fromjump.m_matchingJump->m_shortestPath[baseIndex] = &fromjump;
				fromjump.m_matchingJump->m_distanceToBase[baseIndex] = fromjump.m_distanceToBase[baseIndex]+(fromjump.m_isgate ? JUMP_DELAY : HOLE_DELAY);
			}
		}
	}
}

BOOL PropagateRoutes()
{ // the only routes we need to propagate now is from middle-jumps to middle-jumps & from initial-bases to middle-jumps
	BOOL changeMade = false;
	CString dummy, dummy2;
	for (int sysIndex = 0; sysIndex < SYSTEMS_COUNT; sysIndex++)
	{
		CSystem &system = g_systems[sysIndex];
		POSITION pos2 = system.m_jumpsByNick.GetStartPosition();
		while (pos2)
		{
			CJump &tojump = system.m_jumpsByNick.GetNextAssoc(pos2, dummy2);
			if (g_isTransport && tojump.m_isfreighteronly) continue;
			if (g_avoidLockedGates && tojump.m_islocked) continue;
			if (g_avoidHoles && !tojump.m_isgate) continue;
			if (g_avoidGates && tojump.m_isgate) continue;
			system.CalcLaneDistances(tojump);
			POSITION pos = system.m_avoid ? NULL : pos2;
			while (pos)
			{
				CJump &fromjump = system.m_jumpsByNick.GetNextAssoc(pos, dummy);
				if (g_avoidLockedGates && fromjump.m_islocked) continue;
				if (g_avoidHoles && !fromjump.m_isgate) continue;
				if (g_avoidGates && fromjump.m_isgate) continue;
				if (g_isTransport && fromjump.m_isfreighteronly) continue;
				UINT distance = system.ComputeDistance(fromjump, tojump);
				for (UINT baseIndex = 0; baseIndex < BASES_COUNT; baseIndex++)
				{
					if (~tojump.m_distanceToBase[baseIndex]) // tojump has a known way to final-base
					{
						UINT newdist = distance+tojump.m_distanceToBase[baseIndex];
						if (newdist < fromjump.m_distanceToBase[baseIndex])
						{
							//if (~fromjump.m_distanceToBase[baseIndex])
								//OutputDebugFormat(L"%4d %s\t%4d\t%s %4d\n", fromjump.m_distanceToBase[baseIndex], fromjump.m_caption,distance,tojump.m_caption,tojump.m_distanceToBase[baseIndex]);
							fromjump.m_shortestPath[baseIndex] = &tojump;
							fromjump.m_distanceToBase[baseIndex] = newdist;
							if (!fromjump.m_matchingJump)
							{
								fromjump.m_matchingJump = &fromjump;
								fromjump.m_system = &system;
								ProblemFound(L"A jump gate/hole references an undefined %s in %s", dummy, system.m_nickname);
							}
							newdist += fromjump.m_isgate ? JUMP_DELAY : HOLE_DELAY;
							if (newdist < fromjump.m_matchingJump->m_distanceToBase[baseIndex])
							{
								fromjump.m_matchingJump->m_shortestPath[baseIndex] = &fromjump;
								fromjump.m_matchingJump->m_distanceToBase[baseIndex] = newdist;
							}
							changeMade = true;
						}
					}
					if (~fromjump.m_distanceToBase[baseIndex]) // fromjump has a known way to final-base
					{
						UINT newdist = distance+fromjump.m_distanceToBase[baseIndex];
						if (newdist < tojump.m_distanceToBase[baseIndex])
						{
							//if (~tojump.m_distanceToBase[baseIndex])
								//OutputDebugFormat(L"%d %s\t%d\t%s %d\n", fromjump.m_distanceToBase[baseIndex], fromjump.m_caption,distance,tojump.m_caption,tojump.m_distanceToBase[baseIndex]);
							tojump.m_shortestPath[baseIndex] = &fromjump;
							tojump.m_distanceToBase[baseIndex] = newdist;
							if (!tojump.m_matchingJump)
							{
								tojump.m_system = &system;
								tojump.m_matchingJump = &tojump;
								ProblemFound(L"A jump gate/hole references an undefined %s in %s", dummy2, system.m_nickname);
							}
							newdist += tojump.m_isgate ? JUMP_DELAY : HOLE_DELAY;
							if (newdist < tojump.m_matchingJump->m_distanceToBase[baseIndex])
							{
								tojump.m_matchingJump->m_shortestPath[baseIndex] = &tojump;
								tojump.m_matchingJump->m_distanceToBase[baseIndex] = newdist;
							}
							changeMade = true;
						}
					}
				}
			}
			
			pos = system.m_bases.GetHeadPosition();
			while (pos)
			{
				CBase &frombase = *system.m_bases.GetNext(pos);
				UINT distance = system.ComputeDistance(frombase, tojump);
				for (UINT baseIndex = 0; baseIndex < BASES_COUNT; baseIndex++)
				{
					if (~tojump.m_distanceToBase[baseIndex]) // tojump has a known way to final-base
					{
						if (distance+tojump.m_distanceToBase[baseIndex] < frombase.m_distanceToBase[baseIndex])
						{
							frombase.m_shortestPath[baseIndex] = &tojump;
							frombase.m_distanceToBase[baseIndex] = distance+tojump.m_distanceToBase[baseIndex];
							changeMade = true;
						}
					}
				}
			}
		}
	}
				
	return changeMade;
}



void LoadFiles(BOOL (*callback)(const CString &), LPCTSTR entriesName, LPCTSTR sectionName)
{
	IniFile iniFile(L"..\\EXE\\Freelancer.ini");
	IniSection section = NULL;
	CString name;
	while (iniFile.GetNextSection(section, name))
	{
		if (name.CompareNoCase(sectionName) == 0)
		{
			IniEntry entry;
			for (UINT entriesCount = iniFile.EnumEntries(section, entry); entriesCount; entriesCount--)
			{
				UINT valuesCount;
				IniValue *values;
				name = iniFile.GetNextEntry(entry, valuesCount, values);
				if (name.CompareNoCase(entriesName) == 0)
				{
					name = iniFile.GetValue(values,0);
					name = name.SpanExcluding(L";");
					name.TrimRight();
					if (!callback(name))
						break;
				}
			}
			break;
		}
	}
}

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	return 0;
}


BOOL BrowseForFolder(CString &path, LPCTSTR lpszTitle, HWND hwndOwner, UINT ulFlags)
{
	IFileOpenDialogPtr ofd;
	if (SUCCEEDED(ofd.CreateInstance(__uuidof(FileOpenDialog))))
	{
		ofd->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_NOCHANGEDIR);
		ofd->SetTitle(lpszTitle);
		IShellItem *pItem;
		PWSTR pszFilePath;
		if (SUCCEEDED(SHCreateItemFromParsingName(path.AllocSysString(), NULL, IID_PPV_ARGS(&pItem))))
			ofd->SetFolder(pItem);
		if (SUCCEEDED(ofd->Show(hwndOwner)) &&
			SUCCEEDED(ofd->GetResult(&pItem)) &&
			SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
		{
			path = pszFilePath;
			CoTaskMemFree(pszFilePath);
			return true;
		}
		return false;
	}

	//USES_CONVERSION;
	IMalloc* pMalloc; // smart pointer qui release l'interface quand on sort du scope
	SHGetMalloc(&pMalloc);
	LPTSTR pathBuf = path.GetBuffer(MAX_PATH);
	
	BROWSEINFO bi;
	bi.hwndOwner = hwndOwner;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = lpszTitle;
	bi.ulFlags = ulFlags;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = path.IsEmpty() ? NULL : (LPARAM) pathBuf;
	bi.iImage = NULL;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi); 
	if (pidl == NULL) return false;

	CHECK(SHGetPathFromIDList(pidl, pathBuf));
	path.ReleaseBuffer();
	pMalloc->Free(pidl);
	return true;
}
BOOL BrowseFLDir(HWND hWnd, const CString &initialPath, BOOL displayWarning)
{
	CString path = initialPath;
	do
	{
		if (displayWarning && AfxMessageBox(L"Freelancer was not found on this computer !\n"
				L"Please indicate the directory where you installed Freelancer", MB_OKCANCEL|MB_ICONWARNING) == IDCANCEL)
			return false;
		if (!BrowseForFolder(path, L"Please specify Freelancer directory", hWnd, BIF_USENEWUI|BIF_NONEWFOLDERBUTTON|BIF_VALIDATE))
			return false;
		displayWarning = true;
	}
	while (!PathFileExists(path+L"\\EXE\\Freelancer.ini"));
	g_flAppPath = path;
	theApp.WriteProfileString(L"Settings", L"FLPath", g_flAppPath);
	return true;
}

BOOL XMLCallback(const CString& tag, const CString& attrs, const CString& content, LPVOID param)
{
	struct ModInfo* pModInfo = (struct ModInfo*) param;
	CString name;
	int scan = attrs.Find(L"name=\"");
	if (scan != -1)
		name = attrs.Mid(scan+6, attrs.Find('"',scan+6)-scan-6);
	if (tag == L"header")
	{
		int scan = attrs.Find(L"name=\"");
		if (name.IsEmpty())
			pModInfo->name = L"?";
		else
			pModInfo->name = attrs.Mid(scan+6, attrs.Find('"',scan+6)-scan-6);
		return false;
	}
	else if (tag == L"author")
		pModInfo->author = content;
	else if (tag == L"modurl")
	{
		pModInfo->url = content;
		pModInfo->largeMod++;
	}
	else if (tag == L"description")
		pModInfo->description = content;
	else if (tag == L"FLC_info")
	{
		pModInfo->FLC_info = content;
		pModInfo->FLC_info_name = name;
		pModInfo->largeMod++;
	}
	else if (tag == L"options")
	{
		pModInfo->largeMod++;
	}
	return true;
}

BOOL QuickXMLReader(CStdioFile& file, BOOL (*callback)(const CString&,const CString&,const CString&,LPVOID), LPVOID param)
{
	CString line;
	if (!file.ReadString(line))
		return false;
	line.TrimLeft();
	ASSERT((line[0] == '<') && (line[1] != '/'));
	int scan = line.Find('>', 1);
	CString tag = line.Mid(1, scan-1);
	line.Delete(0, scan+1);
	BOOL nocontent = (tag[tag.GetLength()-1] == '/');
	if (nocontent)
		tag.Delete(tag.GetLength()-1);
	scan = tag.Find(' ');
	CString attrs;
	if (scan != -1)
	{
		attrs = tag.Mid(scan+1);
		tag.Delete(scan, tag.GetLength()-scan);
	}
	CString content;
	if (nocontent)
	{
		if (!callback(tag, attrs, content, param))
			return false;
		file.Seek(-line.GetLength()-2, CFile::current);
		return true;
	}

	for (;;)
	{
		scan = line.Find(L'<', 0);
		if (scan == -1)
			content += line+L"\n";
		else
		{
			content += line.Left(scan);
			line.Delete(0, scan);
			if (line[1] == '/')
			{
				ASSERT((_tcsncmp(LPCTSTR(line)+2, tag, tag.GetLength()) == 0) && (line[tag.GetLength()+2] == '>'));
				content.TrimLeft();
				content.TrimRight();
				if (!callback(tag, attrs, content, param))
					return false;
				file.Seek(tag.GetLength()+1-line.GetLength(), CFile::current);
				return true;
			}
			else
			{
				file.Seek(-line.GetLength()-2, CFile::current);
				if (!QuickXMLReader(file, callback, param))
					return false;
			}
		}
		if (!file.ReadString(line))
			return false;
	}
}

void DetectMod()
{
	HKEY flmmKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Freelancer Mod Manager", 0, KEY_READ, &flmmKey) == ERROR_SUCCESS)
	{
		TCHAR flmmPath[MAX_PATH];
		DWORD cbData = MAX_PATH;
		if (RegQueryValueEx(flmmKey, L"Install_Dir", NULL, NULL, (LPBYTE) flmmPath, &cbData) == ERROR_SUCCESS)
		{
			LPTSTR flmmPathEnd = PathAddBackslash(flmmPath);
			for (DWORD dwIndex = 0; ; dwIndex++)
			{
				DWORD cbValue = flmmPath+MAX_PATH-flmmPathEnd;
				LONG err = RegEnumValue(flmmKey, dwIndex, flmmPathEnd, &cbValue, NULL, NULL, NULL, NULL);
				if (err == ERROR_NO_MORE_ITEMS)
					break;
				else if (err != ERROR_SUCCESS)
					continue;
				if (_tcsnicmp(flmmPathEnd, L"mods\\", 5) != 0) // we want only registry value names starting with "mods\"
					continue;
				PathAppend(flmmPathEnd, L"\\script.xml");
				CStdioFile script;
				if (!script.Open(flmmPath, CFile::modeRead|CFile::shareDenyWrite))
					continue;
				g_modInfo.largeMod = 0;
				QuickXMLReader(script, XMLCallback, &g_modInfo);
				if (g_modInfo.largeMod >= 2)
					break;
				g_modInfo.largeMod = 0;
			}
		}
		RegCloseKey(flmmKey);
	}
}

BOOL LoadAppDatas(CWnd *wnd)
{
	g_flAppPath = theApp.GetProfileString(L"Settings", L"FLPath");
	if (g_flAppPath.IsEmpty() || !PathFileExists(g_flAppPath+L"\\EXE\\Freelancer.exe"))
	{
		g_flAppPath.Empty();
		theApp.WriteProfileString(L"Settings", L"FLPath", NULL);
		CRegKey regKey;
		if (regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Microsoft Games\\Freelancer\\1.0"), KEY_QUERY_VALUE) == ERROR_SUCCESS)
		{
			DWORD dwCount = MAX_PATH;
			regKey.QueryValue(g_flAppPath.GetBuffer(MAX_PATH), _T("AppPath"), &dwCount);
			g_flAppPath.ReleaseBuffer();
		}
		if (g_flAppPath.IsEmpty() || !PathFileExists(g_flAppPath+L"\\EXE\\Freelancer.ini"))
		{
			CString programFiles;
			SHGetSpecialFolderPath(*wnd, programFiles.GetBuffer(MAX_PATH), CSIDL_PROGRAM_FILES, FALSE);
			programFiles.ReleaseBuffer();
			g_flAppPath = programFiles+L"\\Microsoft Games\\Freelancer";
			if (!PathFileExists(g_flAppPath+L"\\EXE\\Freelancer.ini"))
			{
				g_flAppPath = programFiles+L"\\Freelancer";
				if (!PathFileExists(g_flAppPath+L"\\EXE\\Freelancer.ini"))
				{
					if (!BrowseFLDir(*wnd, programFiles, true))
						return false;
				}
			}
		}
	}	
	IniFile::SetBasePath(g_flAppPath+L"\\DATA\\");

	SYSTEMS_COUNT = 0;
	BASES_COUNT = 0;
	GOODS_COUNT = 0;
	EQUIP_COUNT = 0;

	//InitializeHashTable();
	DetectMod();

	g_resourceProvider.Init(g_flAppPath);
	g_tradeLaneCaption	= g_resourceProvider.GetStringFromID(60245);
	//g_jumpHoleCaption	= g_resourceProvider.GetStringFromID(60211);
	//g_jumpGateCaption	= g_resourceProvider.GetStringFromID(60209);
	g_tempExcludeList = L','+theApp.GetProfileString(L"Settings", L"ExcludeFactions")+L',';
	LoadFiles(LoadInitialWorld, L"initial_world", L"Freelancer");	// list of locked gates
	LoadFiles(LoadStationSolarArch, L"solar");	// list of solar archetypes
	g_tempExcludeList = L','+theApp.GetProfileString(L"Settings", L"ExcludeSystems")+L',';
	LoadFiles(LoadSystems, L"universe");	// list of systems
	g_tempExcludeList = L','+theApp.GetProfileString(L"Settings", L"ExcludeGoods")+L',';
	LoadFiles(LoadGoods, L"goods");			// list of commodity goods & default price
	LoadSystemObjects();					// read system object (including list of bases)
	LoadFiles(LoadBases, L"universe");		// read bases names
	LoadFiles(LoadEquip, L"equipment");		// read more info on goods
	LoadFiles(LoadMarketPrices, L"markets");// parse bases price
	LoadFiles(LoadShips, L"ships");			// read more info on ships
	LoadIDHacks();
	g_resourceProvider.Free();
	
	return true;
}


void ProblemFound(LPCTSTR lpFormat, ...)
{
	TCHAR msg[1024];
	static BOOL logDlgInit = false;
	if (logDlgInit)
	{
		logDlgInit = true;
		g_logDlg.Create(g_logDlg.IDD, AfxGetMainWnd());
		if (g_modInfo.largeMod)
			_stprintf(msg,
				L"Some error was found in %s files. Freelancer Companion will try to work around these.\r\n"
				L"But please signal the following problem(s) to %s :\r\n\r\n", 
				LPCTSTR(g_modInfo.name), LPCTSTR(g_modInfo.author));
		else
			_stprintf(msg, 
				L"Some error was found in your game or mods' files. Freelancer Companion will try to work around these.\r\n"
				L"But please signal the following problem(s) to the author of the mods you activated in FLMM :\r\n\r\n");
		LogText(msg);
	}
	va_list arglist;
	va_start(arglist, lpFormat);
	_vstprintf(msg, lpFormat, arglist);
	_tcscat(msg, _T("\r\n"));
	LogText(msg);
}

void FatalError(const CString &msg)
{
	if (g_modInfo.largeMod)
		MessageBox(NULL,	L"An error was found in "+g_modInfo.name+L" files :\r\n\r\n"+
							msg+
							L"\r\n\r\nPlease signal this problem to "+g_modInfo.author+
							L"\r\nPress Ctrl-C to copy this information. Program will terminate now", L"Fatal Error !", MB_ICONERROR|MB_OK);
	else
		MessageBox(NULL,	L"An error was found in your game or mods' files :\r\n\r\n"+
							msg+
							L"\r\n\r\nPlease signal this problem to the author of the mod you activated"
							L"\r\nPress Ctrl-C to copy this information. Program will terminate now", L"Fatal Error !", MB_ICONERROR|MB_OK);
	ExitProcess(1);
}

