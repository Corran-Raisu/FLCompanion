// BaseInfo.cpp : implementation file
//

#include "stdafx.h"
#include "flcompanion.h"
#include "BaseInfoDlg.h"
#include "Datas.h"
#include "IniFile.h"
#include "System.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBaseInfoDlg dialog

CBaseInfoDlg *CBaseInfoDlg::g_baseInfoDlg;


CBaseInfoDlg::CBaseInfoDlg(CBase *base, CWnd* pParent /*=NULL*/)
	: CDialog(CBaseInfoDlg::IDD, pParent), m_base(base)
{
	//{{AFX_DATA_INIT(CBaseInfoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CBaseInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBaseInfoDlg)
	DDX_Control(pDX, IDC_EQUIP_LIST, m_equipList);
	DDX_Control(pDX, IDC_GOODS_LIST, m_goodList);
	DDX_Control(pDX, IDC_SHIP_LIST, m_shipList);
	DDX_Control(pDX, IDC_BASE_COMBO, m_baseCombo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBaseInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CBaseInfoDlg)
	ON_CBN_SELCHANGE(IDC_BASE_COMBO, OnSelchangeBaseCombo)
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnItemclickGoodsList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBaseInfoDlg message handlers

CString RDLtoText(CString html)
{
	if (html.IsEmpty()) return html;
	if (html[0] == 0xFEFF) html.Delete(0);
	if (html.Left(6) == L"<?xml ") html.Delete(0, html.Find('>')+1);
	html.TrimRight(L"\r\n\t ");
	if (html.Left(5) == L"<RDL>") html.Delete(0, 5), html.Delete(html.GetLength()-6,6);
	int index;
	while ((index = html.Find(L"<TRA ")) >= 0) html.Delete(index, html.Find('>', index)-index+1);
	while ((index = html.Find(L"<JUST ")) >= 0) html.Delete(index, html.Find('>', index)-index+1);
	html.Replace(L"<PARA/>", L"\r\n");
	html.Replace(L"<PUSH/>", L"");
	html.Replace(L"<POP/>", L"");
	html.Replace(L"<TEXT>", L"");
	html.Replace(L"</TEXT>", L"");
	html.Replace(L"&lt;", L"<");
	html.Replace(L"&gt;", L">");
	html.TrimRight();
	return html;
}

BOOL CBaseInfoDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_baseInfoDlg = this;

	m_goodList.InsertColumn(0, L"Name");
	m_goodList.InsertColumn(1, L"Price", LVCFMT_RIGHT);
	m_goodList.InsertColumn(2, L"Volume");
	m_goodList.InsertColumn(3, L"Decay");
	m_equipList.InsertColumn(0, L"Name");
	m_equipList.InsertColumn(1, L"Price", LVCFMT_RIGHT);
	m_shipList.InsertColumn(0, L"Name");
	m_shipList.InsertColumn(1, L"Price (without equipment)", LVCFMT_RIGHT);
	m_shipList.InsertColumn(2, L"Cargo hold", LVCFMT_RIGHT);

	
	for (UINT index = 0; index < BASES_COUNT; index++)
	{
		CBase &base = g_bases[index];
		int nIndex = m_baseCombo.AddString(base.m_caption+L", "+base.m_system->m_caption+L" System");
		m_baseCombo.SetItemDataPtr(nIndex, &base);
	}
	// m_baseCombo est tri�e par ordre alphabetique
	int nIndex = 0;
	void *data;
	while ((data = m_baseCombo.GetItemDataPtr(nIndex)) != (void*) CB_ERR)
	{
		if (data == m_base)
		{
			m_baseCombo.SetCurSel(nIndex);
			OnSelchangeBaseCombo();
			break;
		}
		nIndex++;
	}

	m_baseCombo.SetFocus();
	return FALSE;  // return TRUE unless you set the focus to a control
}

BOOL CBaseInfoDlg::FindEquipNames(const CString &iniFilename)
{
	IniSection section;
	CString name;
	{
		LVFINDINFO fi;
		fi.flags = LVFI_STRING;
		IniFile iniFile(iniFilename);
		section = NULL;
		while (iniFile.GetNextSection(section, name))
		{
			name = iniFile.GetValue0(section, "nickname");
			fi.psz = name;
			int nItem = m_equipList.FindItem(&fi);
			if (nItem >= 0)
			{
				DWORD ids_name = iniFile.GetValueInt0(section, "ids_name");
				if (ids_name != 0)
					m_equipList.SetItemText(nItem, 0, g_resourceProvider.GetStringFromID(ids_name));
				double velocity = iniFile.GetValueFloat0(section, "muzzle_velocity");
				if (velocity != 0.0)
					m_equipList.SetItemText(nItem, 3, DoubleToString(velocity));
			}
		}
	}
	return TRUE;
}

BOOL CBaseInfoDlg::LoadBaseMarkets(const CString &iniFilename)
{
	IniSection section;
	CString name, dummy;
	ShipInfo *shipInfo;
	int nItem;
	long price;
	float priceF;
	TCHAR buf[32];
	{
		IniFile iniFile(iniFilename);
		section = NULL;
		while (iniFile.GetNextSection(section, name))
		{
			CHECK(name.CompareNoCase(L"BaseGood") == 0);
			name = iniFile.GetValue0(section, "base");
			name.MakeLower();
			if (name != m_base->m_nickname) // look for our base
				continue;
			IniEntry entry;
			for (UINT entriesCount = iniFile.EnumEntries(section, entry); entriesCount; entriesCount--)
			{
				UINT valuesCount;
				IniValue *values;
				if (iniFile.GetNextEntry(entry, valuesCount, values).CompareNoCase(L"MarketGood") != 0)
					continue;
				name = iniFile.GetValue(values,0);
				if (valuesCount < 7)
					continue;
				// iniFile.GetValueInt(values,1) : Minimum character level to purchase
				//
				// iniFile.GetValueInt(values,2) : Minimum faction level to purchase (can be decimal)
				//		-1 = even if very hostile, 0 = must be at least neutral, 1 = need to be very friendly
				UINT min = iniFile.GetValueInt(values, 3);
				UINT stock = iniFile.GetValueInt(values,4);
				if (!stock && !min)
					continue; // we want to list only goods that are sold here
				if (g_goodsByNick.Contains(name)) // is it a commodity ?
				{
					CGood &good = *g_goodsByNick[name];
					//price = (INT) (good.m_defaultPrice*iniFile.GetValueFloat(values,6));
					price = long(m_base->m_sell[good]); // use value from memory rather than from file (it might have been loaded from DynEcon)
					nItem = m_goodList.InsertItem(MAXLONG, good.m_caption);
					_stprintf(buf, L"$%d", price);
					//FormatNumber(buf, price); // don't use FormatNumber because it breaks ListView sorting
					m_goodList.SetItemText(nItem, 1, buf);
					if (good.m_volume != 1.0)
						m_goodList.SetItemText(nItem, 2, IntToString(UINT(good.m_volume)));
					if (good.m_decay_time != 0)
						m_goodList.SetItemText(nItem, 3, "1 unit every "+IntToString(good.m_decay_time/1000)+" sec");
				}
				else if (g_shippackToInfos.Lookup(name, shipInfo)) // is it a ship ?
				{
					price = long(shipInfo->m_defaultPrice*iniFile.GetValueFloat(values,6));
					nItem = m_shipList.InsertItem(MAXLONG,g_resourceProvider.GetStringFromID(shipInfo->m_ids_name));
					//_stprintf(buf, L"$%d", price);
					FormatNumber(buf, price);
					m_shipList.SetItemText(nItem, 1, buf);
					_stprintf(buf, L"%d units", shipInfo->m_hold_size);
					m_shipList.SetItemText(nItem, 2, buf);
				}
				else if (g_equipPrices.Lookup(name, priceF)) // is it an equipment ?
				{
					price = long(priceF*iniFile.GetValueFloat(values,6));
					nItem = m_equipList.InsertItem(MAXLONG,name);
					_stprintf(buf, L"$%d", price);
					//FormatNumber(buf, price); // don't use FormatNumber because it breaks ListView sorting
					m_equipList.SetItemText(nItem, 1, buf);
				}
			}
		}
	}
	LoadFiles(_FindEquipNames, L"equipment");
	return TRUE;
}

int InfocardMap(int ids_info)
{					
	IniFile iniFile(L"Interface\\InfocardMap.ini");
	IniSection section = NULL;
	CString name;
	while (iniFile.GetNextSection(section, name))
	{
		if (name.CompareNoCase(L"InfocardMapTable") == 0)
		{
			IniEntry entry;
			for (UINT entriesCount = iniFile.EnumEntries(section, entry); entriesCount; entriesCount--)
			{
				UINT valuesCount;
				IniValue *values;
				if (iniFile.GetNextEntry(entry, valuesCount, values).CompareNoCase(L"Map") != 0)
					continue;
				int ids = iniFile.GetValueInt(values,0);
				if (ids == ids_info)
					return iniFile.GetValueInt(values, 1);
			}
		}
	}
	return 0;
}

void CBaseInfoDlg::OnSelchangeBaseCombo() 
{
	UpdateData();
	int nIndex = m_baseCombo.GetCurSel();
	if (nIndex == CB_ERR) return;
	m_base = (CBase*) m_baseCombo.GetItemDataPtr(nIndex);
	CBase &base = *m_base;

	SetRedraw(FALSE);
	SetDlgItemText(IDC_FACTION, base.m_faction ? base.m_faction->m_caption : L"");
	m_goodList.DeleteAllItems();
	m_equipList.DeleteAllItems();
	m_shipList.DeleteAllItems();

	g_resourceProvider.Init(g_flAppPath);
	try
	{
		IniFile iniFile(L"Universe\\"+base.m_system->m_file);
		IniSection section = NULL;
		CString name;
		while (iniFile.GetNextSection(section, name))
		{
			if (name.CompareNoCase(L"Object") != 0)
				continue;
			IniValue *value;
			if (iniFile.GetValues(section, "base", value)) // base
			{
				name = iniFile.GetValue(value,0);
				if (name.CompareNoCase(base.m_nickname) == 0)
				{
					int ids_info = iniFile.GetValueInt0(section, "ids_info");
					SetDlgItemText(IDC_DESCR,	RDLtoText(g_resourceProvider.GetHTMLFromID(ids_info))+
												"\r\n\r\n"+
												RDLtoText(g_resourceProvider.GetHTMLFromID(InfocardMap(ids_info))));
					break;
				}
			}
		}
		LoadFiles(_LoadBaseMarkets, L"markets");// parse bases price

	}
	catch(...)
	{
		g_resourceProvider.Free();
		throw;
	}
	g_resourceProvider.Free();

	ListView_HeaderSort(m_goodList, 0);
	ListView_HeaderSort(m_shipList, 0);

	m_goodList.SetColumnWidth(0, LVSCW_AUTOSIZE);
	m_goodList.SetColumnWidth(1, LVSCW_AUTOSIZE);
	m_goodList.SetColumnWidth(2, LVSCW_AUTOSIZE);
	m_goodList.SetColumnWidth(3, LVSCW_AUTOSIZE);
	m_equipList.SetColumnWidth(0, LVSCW_AUTOSIZE);
	m_equipList.SetColumnWidth(1, LVSCW_AUTOSIZE);
	m_shipList.SetColumnWidth(0, LVSCW_AUTOSIZE);
	m_shipList.SetColumnWidth(1, LVSCW_AUTOSIZE);
	m_shipList.SetColumnWidth(2, LVSCW_AUTOSIZE);
	SetRedraw(TRUE);
	Invalidate();
}

void CBaseInfoDlg::OnItemclickGoodsList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ListView_HeaderSort(pNMHDR, pResult);
}
