#include "StdAfx.h"
#include "MainWnd.h"
#include <thread>
#include <ShlObj.h>
#include <algorithm>

#include "common.hpp"


auto configFile =L".\\config.ini";
static int GetConfigInt(const wchar_t* lpKey, int def=0)
{
	return GetPrivateProfileIntW(L"info", lpKey, def, configFile);
}
static void SetConfigInt(const wchar_t* lpKey, int value)
{
	WritePrivateProfileStringW(L"info", lpKey, std::to_wstring(value).c_str(), configFile);
}
static std::wstring GetConfigString(const wchar_t* lpKey, const wchar_t* def = NULL)
{
	WCHAR cBuf[1024] = {0};
	DWORD uSize = GetPrivateProfileStringW(L"info", lpKey, def, cBuf, 1024, configFile);
	if (uSize > 0) {
		cBuf[uSize] = '\0';
	}

	return std::wstring(cBuf);
}
static void InitConfigListS(std::list<ZigenCount*>& lst)
{
	wchar_t* cBuf = new wchar_t[10240];
	DWORD uSize = GetPrivateProfileStringW(L"list", L"value", L"", cBuf, 10240, configFile);
	if (uSize > 0) {
		cBuf[uSize] = '\0';
	}

	if (uSize == 0) {
		return;
	}

	int nKeyBegin = 0;
	if (';' == cBuf[0]) {
		nKeyBegin = 1;
	}

	int nKeyEnd = 0;
	int nValueBegin = 0;

	for (int i = 1; i <= uSize; ++i) {
		if (':' == cBuf[i])			// 有 等于 
		{
			nKeyEnd = i;
			nValueBegin = i + 1;
		} else if (';' == cBuf[i] || '\0' == cBuf[i] || i == uSize) {
			if (nKeyBegin == nKeyEnd) {
				continue;
			}
			
			std::wstring sKey(cBuf + nKeyBegin, nKeyEnd - nKeyBegin);
			std::wstring sValue(cBuf + nValueBegin, i - nValueBegin);

			int index = std::stoi(sKey);
			int count = std::stoi(sValue);
			lst.push_back(new ZigenCount{index, count});

			nKeyBegin = i + 1;
			nKeyEnd = nKeyBegin;
			nValueBegin = nKeyBegin;
		}
	}

	delete[] cBuf;
}
static void SaveConfigZigenCountList(const std::list<ZigenCount*>& lst)
{
	std::wstring value;
	value.reserve(4096);
	for (auto& i : lst) {
		value += std::to_wstring(i->index);
		value += L':';
		value += std::to_wstring(i->count);
		value += L';';
	}

	WritePrivateProfileStringW(L"list", L"value", value.c_str(), configFile);
}


// 连击对应
const int bbArr[] = {10, 30, 60, 100,  200, 300, 400, 500, 600,   800, 1000, 1500, 2000, 2500};
const int scoreArr[] = {2, 4, 8, 16, 40,   80,160,200,250,300,   360, 400, 480, 600, 800};
int static calcScore(int ccc)
{	
	int index = 0;
	for (int c : bbArr) {
		if (ccc < c) {
			break;
		}
		index += 1;
	}
	
	return scoreArr[index];
}

CMainWnd::CMainWnd(void)
	: SHostWndEx(_T("layout:wnd_main"))//这里定义主界面需要使用的布局文件 在uires.idx文件中定义的
{
	
}

CMainWnd::~CMainWnd(void)
{
	
}

///////////////////////////////////界面 事件  函数///////////////////////////////////////
BOOL CMainWnd::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
	InitWnd(m_pImage, L"img_zi");
	InitWnd(m_pTxtCode, L"text_code");
	InitWnd(m_pTxtFirst, L"text_first");
	InitWnd(m_pTxtSecond, L"text_second");

#ifdef _DEBUG
	SetChildVisible(L"btn_next", TRUE);
#endif // _DEBUG

	common::WalkH(
#ifdef _DEBUG	
		L"C:\\Tool\\真码字根练习\\zm"
#else
		L".\\zm"
#endif // DEBUG
		, [this](const wchar_t* f, bool dir, unsigned __int64 s)->bool {

		if (dir) {
			return true;
		}

		ZigenInfo* info = new ZigenInfo;

		info->file = f;
		auto fn = common::GetFileNameWithoutExt(f);
		int pos = fn.find('_');
		if (pos < 0) {
			info->code = fn.c_str();
		} else {
			info->code = fn.substr(0, pos).c_str();
		}
		info->code.MakeLower();
		GETRENDERFACTORY->CreateBitmap(&info->bmp);
		info->bmp->LoadFromFile(info->file);

		m_arrZigen.push_back(info);
		return true;
	});

	if (m_arrZigen.empty()) {
		ShowInfoBox(L"请放置zm文件夹");
		OnBtnClose();
		return TRUE;
	}

	// 固定的 排序
	std::sort(m_arrZigen.begin(), m_arrZigen.end(), [](const ZigenInfo* a, const ZigenInfo* b)->bool {
		return common::CalcFileSize(a->file) < common::CalcFileSize(b->file);
	});

	//SStringT sss;
	// 加载 上次保存的 列表
	InitConfigListS(m_list);
	if (m_list.empty()) {
		for (int i = 0; i < m_arrZigen.size(); i++) {

			//sss += common::GetFileNameWithoutExt(m_arrZigen.at(i)->file).c_str();
			//sss += L".png\r\n";
			m_list.push_back(new ZigenCount{i, -1});
		}
	}
	
	m_progress = GetConfigInt(L"progress");
	m_points = GetConfigInt(L"points");
	m_ccc = GetConfigInt(L"ccc");
	SetChildText(L"text_points", SStringT().Format(L"%d", m_points));
	SetChildFormatText(L"text_ccc", SStringT().Format(L"%d", m_ccc));

	ShowZigen();
	return TRUE;
}

void CMainWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_BACK) {
		SStringT txtSecond = m_pTxtSecond->GetWindowText();
		if (!txtSecond.IsEmpty()) {
			m_pTxtSecond->SetWindowText(L"");
		} else {
			m_pTxtFirst->SetWindowText(L"");
		}
		m_pTxtFirst->SetAttribute(L"colorText", L"#AAAAAA");
		m_pTxtSecond->SetAttribute(L"colorText", L"#AAAAAA");
		return;
	}

	if (nChar > 90 || nChar < 65) {
		return;
	}

	ZigenCount* zcount = m_list.front();
	ZigenInfo* info = m_arrZigen.at(zcount->index);
	if (info == nullptr) {
		return;
	}

	bool ok = false;
	SStringT sChar;
	sChar.Format(L"%C", nChar);
	sChar.MakeLower();

	SStringT txtFirst = m_pTxtFirst->GetWindowText();
	if (txtFirst.IsEmpty()) {		
		m_pTxtFirst->SetWindowText(sChar);
		if (m_pTxtSecond->IsVisible()) {
			// 双编码  
			return;
		}
		ok = sChar == info->code;
	} else if (!m_pTxtSecond->GetWindowText().IsEmpty()) {
		// 出错了 才近来
		m_pTxtFirst->SetWindowText(sChar);
		m_pTxtSecond->SetWindowText(L"");
		m_pTxtFirst->SetAttribute(L"colorText", L"#AAAAAA");
		m_pTxtSecond->SetAttribute(L"colorText", L"#AAAAAA");

		if (m_pTxtSecond->IsVisible()) {
			// 双编码  
			return;
		}
		ok = sChar == info->code;
	}

	m_pTxtSecond->SetWindowText(sChar);
	ok = (txtFirst+sChar) == info->code;
	
	if (!ok) {
		zcount->count = -1;
		m_ccc = 0;
		SetConfigInt(L"ccc", (int)m_ccc);
		SaveConfigZigenCountList(m_list);

		m_pTxtCode->SetWindowText(info->code);

		m_pTxtFirst->SetAttribute(L"colorText", L"#FF0000");
		m_pTxtSecond->SetAttribute(L"colorText", L"#FF0000");
				
		SetChildText(L"text_ccc", SStringT().Format(L"%d", m_ccc));
		SetChildText(L"text_bb", L"*1倍");			

		return;
	}

	zcount->count += 1;
	// 连击数
	m_ccc += 1;
	SetChildText(L"text_ccc", SStringT().Format(L"%d", m_ccc));

	// 倍数 和 分数
	int star = calcScore(m_ccc);
	int bb = star / 2;
	m_points += star;
	SetConfigInt(L"points", (int)m_points);
	SetConfigInt(L"ccc", (int)m_ccc);

	SetChildText(L"text_bb", SStringT().Format(L"*%d倍", bb));
	SetChildText(L"text_points", SStringT().Format(L"%d", m_points));
	
	// 正确后，移出 队头
	m_list.pop_front();

	if (zcount->count >= 8) {
		// 连续8次后  直接 移动到 队尾		
		m_list.push_back(zcount);
	} else {
		// zigenCount count[01234567] 几次后 跳到 列表的多少位
		static const int indexCount[] = {2, 4, 8, 12, 20, 30, 60, 100};
		int after = indexCount[zcount->count];

		auto ite = m_list.begin();
		std::advance(ite, after);
		m_list.insert(ite, zcount);
	}
	SaveConfigZigenCountList(m_list);
	
	ShowZigen();
}


void CMainWnd::ShowZigen()
{
	ZigenCount* zcount = m_list.front();
	ZigenInfo* info = m_arrZigen[zcount->index];
	
	if (info == nullptr) {
		ShowInfoBox(L"结束到头了");
		return;
	}

	m_pImage->SetImage(info->bmp);

	if (zcount->index >= m_progress) {
		m_progress = zcount->index;

		int hadC = m_progress + 1;
		int cent = hadC * 100 / m_arrZigen.size();

		SetChildFormatText(L"text_cent", L"%d%%", cent);
		SetChildFormatText(L"text_progress", L"%d/%d", hadC, m_arrZigen.size());

		SetConfigInt(L"progress", m_progress);
	}


	if (zcount->count < 0) {
		// 第一次出现  显示 编码
		m_pTxtCode->SetWindowText(info->code);
	} else {
		m_pTxtCode->SetWindowText(L"");
	}
	
	m_pTxtFirst->SetWindowText(L"");
	m_pTxtSecond->SetWindowText(L"");

	if (info->code.GetLength() == 1) {
		// 单编码
		m_pTxtSecond->SetVisible(FALSE, TRUE);
	} else {
		// 双编码
		m_pTxtSecond->SetVisible(TRUE, TRUE);
	}
}

void CMainWnd::OnBtnNext()
{
	
	
}

