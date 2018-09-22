// GoodsRepo.cpp : implementation file
//

#include "stdafx.h"
#include "FLCompanion.h"
#include "FLCompanionDlg.h"
#include "Datas.h"
#include "IniFile.h"
#include "Base.h"
#include "System.h"
#include "GoodsRepoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGoodsRepoDlg dialog

CGoodsRepoDlg *CGoodsRepoDlg::g_goodsRepoDlg;

CGoodsRepoDlg::CGoodsRepoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGoodsRepoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGoodsRepoDlg)
	//}}AFX_DATA_INIT
}


void CGoodsRepoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGoodsRepoDlg)
	DDX_Control(pDX, IDC_GOODS_LIST, m_goodsList);
	DDX_Control(pDX, IDC_BASES_LIST, m_basesList);
	DDX_Control(pDX, IDC_KIND_COMBO, m_kindCombo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGoodsRepoDlg, CDialog)
	//{{AFX_MSG_MAP(CGoodsRepoDlg)
	ON_CBN_SELCHANGE(IDC_KIND_COMBO, OnSelchangeKindCombo)
	ON_WM_DESTROY()
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnItemclickBasesList)
	ON_BN_CLICKED(IDC_SHOWSELL, OnSelchangeGoodsList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_GOODS_LIST, OnItemchangedGoodsList)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_BASES_LIST, OnItemactivateBasesList)
	ON_BN_CLICKED(IDC_SHOWALL, OnSelchangeGoodsList)
	ON_LBN_SELCHANGE(IDC_GOODS_LIST, OnSelchangeGoodsList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGoodsRepoDlg message handlers

	
void CGoodsRepoDlg::OnDestroy() 
{
	EmptyGoods();
	CDialog::OnDestroy();
}

void CGoodsRepoDlg::EmptyGoods()
{
	int nIndex = m_goodsList.GetItemCount();
	while (nIndex)
		free(LPVOID(m_goodsList.GetItemData(--nIndex)));
	m_goodsList.DeleteAllItems();
	ListView_HeaderSortReset(m_goodsList); 
	m_goodsList.DeleteColumn(2);
	m_goodsList.DeleteColumn(1);
}

BOOL CGoodsRepoDlg::FindEquipKind(const CString &iniFilename)
{
	IniSection section;
	CString name;
	{
		IniFile iniFile(iniFilename);
		section = NULL;
		while (iniFile.GetNextSection(section, name))
		{
			DWORD value = iniFile.GetValueInt0(section, "ids_name");
			if (value <= 1) continue;
			if (m_kindCombo.FindStringExact(-1, name) == CB_ERR)
				m_kindCombo.AddString(name);
		}
	}
	return TRUE;
}

BOOL CGoodsRepoDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_goodsRepoDlg = this;

	m_goodsList.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);
	m_goodsList.InsertColumn(0, L"Name", LVCFMT_LEFT, -1);

	m_basesList.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_TWOCLICKACTIVATE|LVS_EX_UNDERLINEHOT|LVS_EX_HEADERDRAGDROP);
	m_basesList.InsertColumn(0, L"Base", LVCFMT_LEFT, 20);
	m_basesList.InsertColumn(1, L"Price", LVCFMT_RIGHT, 20);
	m_basesList.InsertColumn(2, L"System", LVCFMT_LEFT, 20);
	m_basesList.InsertColumn(3, L"Faction", LVCFMT_LEFT, 20);

	LoadFiles(_FindEquipKind, L"equipment");
	m_kindCombo.SelectString(-1, L"Ship");
	OnSelchangeKindCombo();
	CheckDlgButton(IDC_SHOWSELL, BST_CHECKED);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CGoodsRepoDlg::FindEquipNames(const CString &iniFilename)
{
	IniSection section;
	CString name;
	{
		IniFile iniFile(iniFilename);
		section = NULL;
		while (iniFile.GetNextSection(section, name))
		{
			if (name != m_kind) continue;
			CString nickname = iniFile.GetValue0(section, "nickname");
			DWORD value = iniFile.GetValueInt0(section, "ids_name");
			CString caption = g_mainDlg->m_displayNicknames ? nickname : g_resourceProvider.GetStringFromID(value);
			caption.TrimLeft();
			if (caption.IsEmpty()) caption = nickname;
			CGood* good = NULL;
			if ((m_kind == "Commodity") && g_goodsByNick.Lookup(nickname, good))
				caption = '*'+caption;
			LVFINDINFO fi;
			fi.flags = LVFI_STRING;
			fi.psz = caption;
			int nItem = m_goodsList.FindItem(&fi);
			if (nItem != -1)
			{
				LPTSTR otherEntry = reinterpret_cast<LPTSTR>(m_goodsList.GetItemData(nItem));
				if (otherEntry)
				{
					m_goodsList.SetItemData(nItem, NULL);
					CString subtext = m_goodsList.GetItemText(nItem, 1);
					nItem = m_goodsList.InsertItem(MAXLONG, caption+L" ("+otherEntry+L")");
					m_goodsList.SetItemData(nItem, DWORD(otherEntry));
					m_goodsList.SetItemText(nItem, 1, subtext);
				}
				caption += L" ("+nickname+L")";
			}
			nItem = m_goodsList.InsertItem(MAXLONG, caption);
			m_goodsList.SetItemData(nItem, DWORD(_tcsdup(nickname)));
			float defaultPrice;
			if (good)
				m_goodsList.SetItemText(nItem, 1, L'$'+DoubleToString(good->m_defaultPrice));
			else if (g_equipPrices.Lookup(nickname, defaultPrice))
				m_goodsList.SetItemText(nItem, 1, L'$'+DoubleToString(defaultPrice));
		}
	}
	return TRUE;
}

void CGoodsRepoDlg::OnSelchangeKindCombo() 
{
	int nIndex = m_kindCombo.GetCurSel();
	if (nIndex == CB_ERR) return;
	m_kindCombo.GetLBText(nIndex, m_kind);
	EmptyGoods();
	g_resourceProvider.Init(g_flAppPath);
	try
	{
		if (m_kind == L"Ship")
		{
			m_goodsList.InsertColumn(1, L"Price", LVCFMT_RIGHT, 20);
			m_goodsList.InsertColumn(2, L"Hold", LVCFMT_RIGHT, 20);
			POSITION pos = g_shippackToInfos.GetStartPosition();
			CString nickname;
			ShipInfo* shipInfo;
			while (pos)
			{
				POSITION savedPos = pos;
				g_shippackToInfos.GetNextAssoc(pos, nickname, shipInfo);
				int nItem = m_goodsList.InsertItem(MAXLONG, g_mainDlg->m_displayNicknames ? nickname : g_resourceProvider.GetStringFromID(shipInfo->m_ids_name));
				m_goodsList.SetItemText(nItem, 1, L'$'+DoubleToString(shipInfo->m_defaultPrice));
				m_goodsList.SetItemText(nItem, 2, IntToString(shipInfo->m_hold_size));
				m_goodsList.SetItemData(nItem, DWORD(_tcsdup(nickname)));
			}
		}
		else
		{
			m_goodsList.InsertColumn(1, L"Price (default)", LVCFMT_RIGHT, 20);
			LoadFiles(_FindEquipNames, L"equipment");
			int nIndex = m_goodsList.GetItemCount();
			while (nIndex--)
			{
				if (m_goodsList.GetItemData(nIndex) == NULL)
					m_goodsList.DeleteItem(nIndex);
			}
		}
	}
	catch(...)
	{
		g_resourceProvider.Free();
		throw;
	}
	g_resourceProvider.Free();
	m_goodsList.SetColumnWidth(0, LVSCW_AUTOSIZE);
	m_goodsList.SetColumnWidth(1, LVSCW_AUTOSIZE);
	m_goodsList.SetColumnWidth(2, LVSCW_AUTOSIZE);
	OnSelchangeGoodsList();
}

BOOL CGoodsRepoDlg::FindItemRefs(const CString &iniFilename)
{
	IniSection section;
	CString name;
	float price;
	TCHAR buf[32];
	{
		IniFile iniFile(iniFilename);
		section = NULL;
		while (iniFile.GetNextSection(section, name))
		{
			CHECK(name.CompareNoCase(L"BaseGood") == 0);
			CBase* base;
			name = iniFile.GetValue0(section, "base");
			name.MakeLower();
			if (g_basesByNick.Lookup(name, base))
			{
				IniEntry entry;
				for (UINT entriesCount = iniFile.EnumEntries(section, entry); entriesCount; entriesCount--)
				{
					UINT valuesCount;
					IniValue *values;
					if (iniFile.GetNextEntry(entry, valuesCount, values).CompareNoCase(L"MarketGood") != 0)
						continue;
					name = iniFile.GetValue(values,0);
					if (name != m_item)
						continue;
					if (valuesCount < 7)
						continue;
					// iniFile.GetValueInt(values,1) : Minimum character level to purchase
					//
					// iniFile.GetValueInt(values,2) : Minimum faction level to purchase (can be decimal)
					//		-1 = even if very hostile, 0 = must be at least neutral, 1 = need to be very friendly
					UINT buyonly = iniFile.GetValueInt(values,5);
					if (buyonly && !m_showAllBases)
						continue; // we want to list only goods that are sold here
					price = m_defaultPrice*iniFile.GetValueFloat(values,6);
					if (m_kind == "Commodity")
					{
						CGood* good;
						if (g_goodsByNick.Lookup(name, good))
							price = base->m_buy[*good]; // use value from memory rather than from file (it might have been loaded from DynEcon)
					}
					int nItem = m_basesList.InsertItem(LVIF_TEXT|LVIF_PARAM, MAXLONG, base->m_caption, 0, 0, 0, long(base));
					_stprintf(buf, L"$%d", INT(price));
					//FormatNumber(buf, long(price)); // don't use FormatNumber because it breaks ListView sorting
					m_basesList.SetItemText(nItem, 1, buf);
					m_basesList.SetItemText(nItem, 2, base->m_system->m_caption);
					m_basesList.SetItemText(nItem, 3, base->m_faction->m_caption);
				}
			}
		}
	}
	return TRUE;
}

void CGoodsRepoDlg::DisplayGoodPrices(CGood* good)
{
	UINT goodIndex = *good;
	TCHAR buf[32];
	for (UINT index = 0; index < BASES_COUNT; index++)
	{
		CBase* base = &g_bases[index];
		if ((base->m_sell[goodIndex] == FLT_MAX) && !m_showAllBases)
			continue; // we want to list only goods that are sold here
		if (base->m_buy[goodIndex] == good->m_defaultPrice)
			continue;
		int nItem = m_basesList.InsertItem(LVIF_TEXT|LVIF_PARAM, MAXLONG, base->m_caption, 0, 0, 0, long(base));
		_stprintf(buf, L"$%d", INT(base->m_buy[goodIndex]));
		//FormatNumber(buf, long(price)); // don't use FormatNumber because it breaks ListView sorting
		m_basesList.SetItemText(nItem, 1, buf);
		m_basesList.SetItemText(nItem, 2, base->m_system->m_caption);
		//m_basesList.SetItemText(nItem, 3, base->m_faction->m_caption);
	}
}

void CGoodsRepoDlg::OnItemchangedGoodsList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;
	if ((pNMListView->uNewState & LVIS_SELECTED) == 0)
		return;
	OnSelchangeGoodsList();
}

void CGoodsRepoDlg::OnSelchangeGoodsList() 
{
	POSITION pos = m_goodsList.GetFirstSelectedItemPosition();
	if (pos == NULL) return;
	int nIndex = m_goodsList.GetNextSelectedItem(pos);
	m_basesList.DeleteAllItems();
	m_selectedBase = NULL;
	m_showAllBases = IsDlgButtonChecked(IDC_SHOWALL) == BST_CHECKED;
	m_item = LPCTSTR(m_goodsList.GetItemData(nIndex));
	CGood* good = NULL;
	CString str;
	TCHAR buf[32];
	if (m_kind == L"Ship")
	{
		ShipInfo* shipInfo = g_shippackToInfos[m_item];
		m_defaultPrice = shipInfo->m_defaultPrice;
		FormatNumber(buf, long(m_defaultPrice));
		str.Format(L"Default price: %s \x2014 Cargo hold size: %d units", buf, shipInfo->m_hold_size);
	}
	else if (g_equipPrices.Lookup(m_item, m_defaultPrice))
	{
		FormatNumber(buf, long(m_defaultPrice));
		str.Format(L"Default price: %s", buf);
	}
	else if (g_goodsByNick.Lookup(m_item, good))
	{
		m_defaultPrice = good->m_defaultPrice;
		FormatNumber(buf, long(m_defaultPrice));
		str.Format(L"Default price: %s", buf);
	}
	else
		m_defaultPrice = 0;
	SetDlgItemText(IDC_DEFAULT_PRICE, str);
	SetRedraw(FALSE);
	if (good)
		DisplayGoodPrices(good);
	else
		LoadFiles(_FindItemRefs, L"markets");// parse bases price
	ListView_HeaderSort(m_basesList, 1, m_showAllBases); 
	m_basesList.SetColumnWidth(0, LVSCW_AUTOSIZE);
	m_basesList.SetColumnWidth(1, LVSCW_AUTOSIZE);
	m_basesList.SetColumnWidth(2, LVSCW_AUTOSIZE);
	m_basesList.SetColumnWidth(3, LVSCW_AUTOSIZE);
	SetRedraw(TRUE);
	Invalidate();
}


void CGoodsRepoDlg::OnItemclickBasesList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ListView_HeaderSort(pNMHDR, pResult);
}

/*
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_BASES_LIST, OnCustomDrawBasesList)
void CGoodsRepoDlg::OnCustomDrawBasesList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMLVCUSTOMDRAW lplvcd = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	*pResult = CDRF_DODEFAULT;
	switch (lplvcd->nmcd.dwDrawStage)
	{
    case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
    case CDDS_ITEMPREPAINT:
		{
			TCHAR price[32];
			m_basesList.GetItemText(lplvcd->nmcd.dwItemSpec,1,price,_countof(price));
			int delta = _ttoi(price+1)-m_defaultPrice;
			if (delta > 0)
				lplvcd->clrText = RGB(255,0,0);
			else if (delta < 0)
				lplvcd->clrText = RGB(0,255,0);
			*pResult = CDRF_NEWFONT;
			break;
		}
	}
}
*/


void CGoodsRepoDlg::OnItemactivateBasesList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	m_selectedBase = reinterpret_cast<CBase*>(m_basesList.GetItemData(pNMListView->iItem));
	*pResult = 0;
	EndDialog(IDOK);
}

