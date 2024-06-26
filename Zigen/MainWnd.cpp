#include "StdAfx.h"
#include "MainWnd.h"

#include <thread>

#include "common.hpp"


auto configFile =L".\\config.ini";

const int TIMER_NEXT = 101;
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
	InitWnd(m_pComb, L"cb_zz");
	InitWnd(m_pLayZigen, L"lay_zigen");
	InitWnd(m_pImage, L"img_zi");
	InitWnd(m_pTxtCode, L"text_code");
	InitWnd(m_pTxtFirst, L"text_first");
	InitWnd(m_pTxtSecond, L"text_second");
	InitWnd(m_pTxtScore, L"text_score");

	m_pLayZigen->GetEventSet()->subscribeEvent(&CMainWnd::OnAnimStopZigen, this);


	InitWnd(m_pLayHanzi, L"lay_hanzi");
	InitWnd(m_pTxtPin, L"text_pin");
	InitWnd(m_pTxtHanzi, L"text_hanzi");
	InitWnd(m_pEditHanzi, L"edit_hanzi");

	m_pEditHanzi->GetEventSet()->subscribeEvent(&CMainWnd::OnEditRENotifyHanzi, this);
	m_pLayHanzi->GetEventSet()->subscribeEvent(&CMainWnd::OnAnimStopHanzi, this);

	
	m_aniHide = SApplication::getSingletonPtr()->LoadAnimation(_T("anim:hide"));
	m_aniShow = SApplication::getSingletonPtr()->LoadAnimation(_T("anim:show"));
	m_aniScore = SApplication::getSingletonPtr()->LoadAnimation(_T("anim:score"));

	
	int count = common::GetIniFileInt(L"list", L"count", configFile);	
	for(int i=1; i<=count; i++) {
		auto app = std::to_wstring(i);
		auto name = common::GetIniFileString(app.c_str(), L"name", configFile);
		auto& ini = common::GetIniFileString(app.c_str(), L"ini", configFile);
		int type = common::GetIniFileInt(app.c_str(), L"type", configFile);
		std::wstring* pd = new std::wstring(std::move(ini));
		m_pComb->InsertItem(i-1, name.c_str(), type, (LPARAM)pd);
	}
	
	if (m_pComb->GetCount() == 0) {
		return TRUE;
	}

	m_pComb->GetEventSet()->subscribeEvent(&CMainWnd::OnCombSelChange, this);

	int sel = common::GetIniFileInt(L"config", L"comb_sel", configFile, 0);
	m_pComb->SetCurSel(sel);

	return TRUE;
}
BOOL CMainWnd::OnCombSelChange(EventCBSelChange* pEvt)
{
	LPARAM p = m_pComb->GetItemData(pEvt->nCurSel);
	if (p == 0) {
		ShowInfoBox(L"无效的ItemData");
		return TRUE;
	}

	int type = m_pComb->GetListBox()->GetItemImage(pEvt->nCurSel);

	common::SetIniFileInt(L"config", L"comb_sel", configFile, pEvt->nCurSel);

	std::wstring iniFile;
    std::wstring userIni;

#ifdef _DEBUG
        iniFile = L"C:\\Tool\\zigen\\data\\";
        userIni = L"C:\\Tool\\zigen\\user\\";
#else
        iniFile = L".\\data\\";
        userIni = L".\\user\\";
#endif  // DEBUG
        iniFile += ((std::wstring*)p)->c_str();
        userIni += ((std::wstring*)p)->c_str();
	
	
	STabCtrl* pTab;
	InitWnd(pTab, L"tab_main");

	if (type == 0) {
		pTab->SetCurSel(L"zigen", FALSE);
        m_dataZigen.Init(iniFile.c_str(), userIni.c_str());

		SetChildParentVisible(L"text_points", TRUE);
		SetChildParentVisible(L"text_ccc", TRUE);

		SetChildText(L"text_points", SStringT().Format(L"%d", m_dataZigen.GetPoints()));
		SetChildFormatText(L"text_ccc", SStringT().Format(L"%d", m_dataZigen.GetCCC()));

		LoadZigen();
	} else {
		pTab->SetCurSel(L"hanzi", FALSE);
		m_pImage->SetImage(nullptr);
		m_dataHanzi.Init(iniFile.c_str());

		SetChildParentVisible(L"text_points", FALSE);
		SetChildParentVisible(L"text_ccc", FALSE);
		
		m_pEditHanzi->SetFocus();
		LoadHanzi();
	}
	
	DrawProgress();
	return TRUE;
}
void CMainWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nullptr == m_pImage->GetImage()) {
		SetMsgHandled(FALSE);
		return;
	}


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

	// 空格 用来 补充 单编码的字根第二位
	// 或 用来提示，连击清零
	if (nChar == VK_SPACE) {
          DoVerify();
		// 
		return;
	}

	if (nChar > 90 || nChar < 65) {
		return;
	}

	bool ok = false;
	SStringT sChar;
	sChar.Format(L"%C", nChar);
	sChar.MakeLower();

	// 单 编码
	if (!m_pTxtSecond->IsVisible() ) {
		
		m_pTxtFirst->SetAttribute(L"colorText", L"#AAAAAA");
		m_pTxtFirst->SetWindowText(sChar);

		// 只有一个 编码 立即 验证
		DoVerify();
		return;
	}

	// 双编码 
	SStringT txtFirst = m_pTxtFirst->GetWindowText();
	if (txtFirst.IsEmpty()) {		
		m_pTxtFirst->SetAttribute(L"colorText", L"#AAAAAA");
		m_pTxtFirst->SetWindowText(sChar);

		return;
	}
	// 判断是否有出错的
	if (!m_pTxtSecond->GetWindowText().IsEmpty()) {
		// 出错了 才近来
		m_pTxtFirst->SetWindowText(sChar);
		m_pTxtSecond->SetWindowText(L"");
		m_pTxtFirst->SetAttribute(L"colorText", L"#AAAAAA");
		m_pTxtSecond->SetAttribute(L"colorText", L"#AAAAAA");

		return;
	}

	m_pTxtSecond->SetAttribute(L"colorText", L"#AAAAAA");
	m_pTxtSecond->SetWindowText(sChar);

	DoVerify();
}
void CMainWnd::OnTimer(UINT_PTR idEvent)
{
	if (TIMER_NEXT == idEvent) {
		KillTimer(TIMER_NEXT);


	} else {
		__super::OnTimer(idEvent);
	}
}

void CMainWnd::DrawProgress()
{
	int hadC;
	int total;
	int cent;
	if (nullptr == m_pImage->GetImage()) {
		hadC = m_dataHanzi.GetIndex() + 1;
		total = m_dataHanzi.GetCount();
		cent = hadC * 100 / (max(total, 1));
	} else {
		hadC = m_dataZigen.GetMaxIndex() + 1;
		total = m_dataZigen.GetCount();
		cent = hadC * 100 / (max(total, 1));
	}
	SetChildFormatText(L"text_cent", L"%d%%", cent);
	SetChildFormatText(L"text_progress", L"%d/%d", hadC, total);
}
void CMainWnd::LoadZigen()
{
	
	const ZigenItem* item = m_dataZigen.GetZigenItem();

	m_pImage->SetImage(item->bmp);
	
	int maxIndex = m_dataZigen.GetMaxIndex();
	if (m_dataZigen.GetZigenIndex() >= maxIndex) {
		DrawProgress();
	} 

	m_pTxtCode->SetWindowText(item->code);

	if (m_dataZigen.IsShowCode()) {
		m_pTxtCode->SetVisible(TRUE);
	} else {
		m_pTxtCode->SetVisible(FALSE);
	}
		
	m_pTxtFirst->SetWindowText(L"");
	m_pTxtSecond->SetWindowText(L"");

	if (item->code.GetLength() > 1) {
		m_pTxtSecond->SetVisible(TRUE);
	} else {
		m_pTxtSecond->SetVisible(FALSE);
	}
}

void CMainWnd::DoVerify()
{
	SStringT code = m_pTxtFirst->GetWindowText();
	if (m_pTxtSecond->IsVisible()) {
		code += m_pTxtSecond->GetWindowText();
	} else {
		m_pTxtSecond->SetWindowText(L"");
	}

	SStringT realCode = m_pTxtCode->GetWindowText();

	if (realCode.CompareNoCase(code) != 0) {
		int scroe = m_dataZigen.CalcFailedScore();
		m_pTxtCode->SetVisible(TRUE, TRUE);

		m_pTxtFirst->SetAttribute(L"colorText", L"#FF0000");
		m_pTxtSecond->SetAttribute(L"colorText", L"#FF0000");

		SetChildText(L"text_ccc", L"0");
		SetChildText(L"text_bb", L"*1倍");
		return;
	}
	
	int calc = m_dataZigen.CalcOkScore();
	
	m_dataZigen.Next();


	SStringT ss;
	ss.Format(L"+%d", calc);
	m_pTxtScore->SetWindowText(ss);
	m_pTxtScore->StartAnimation(m_aniScore);

	
	SetChildText(L"text_bb", SStringT().Format(L"*%d倍", calc/m_dataZigen.GetOnePoint()));
	SetChildText(L"text_points", SStringT().Format(L"%d", m_dataZigen.GetPoints()));
	SetChildText(L"text_ccc", SStringT().Format(L"%d", m_dataZigen.GetCCC()));
	/*
	// 倍数 
	int bb = calcScoreTimes(m_ccc);
	// 和 分数
	int star = 2 * bb;
	m_points += star;
	SetConfigInt(L"points", (int)m_points);
	SetConfigInt(L"ccc", (int)m_ccc);



	// 动画
	m_pTxtScore->StartAnimation(m_aniScore);
	

	*/
	// 动画 下一个
	m_pLayZigen->StartAnimation(m_aniHide);
}

BOOL CMainWnd::OnAnimStopZigen(EventSwndAnimationStop* pEvt)
{
	// 加分的动画结束后 才 加载 下一个 
	//pEvt->get
	if (m_pLayZigen->GetAnimation() == m_aniHide) {
		// hide 动画 结束


		LoadZigen();
		DrawProgress();
		m_pLayZigen->StartAnimation(m_aniShow);
	} else if(m_pLayZigen->GetAnimation() == m_aniShow){
		// 显示 动画 结束

	}
	
	return TRUE;
}

void CMainWnd::LoadHanzi()
{
	const HanziItem* item = m_dataHanzi.GetItem();
	if (item == nullptr) {
		ShowInfoBox(L"nullptr");
		return;
	}
	m_pTxtPin->SetWindowText(item->pin.c_str());
	m_pTxtHanzi->SetWindowText(item->hanzi.c_str());

	m_pEditHanzi->SetUserData((ULONG_PTR)item->hanzi.c_str());
	//m_pTxtFirst->SetWindowText(L"");
	//m_pTxtSecond->SetWindowText(L"");
}
BOOL CMainWnd::OnEditRENotifyHanzi(EventRENotify* pEvt)
{	
	if (EN_CHANGE != pEvt->iNotify) {
		return TRUE;
	}

	SStringT sHanzi = m_pEditHanzi->GetWindowText();
	if (sHanzi.IsEmpty()) {
		return TRUE;
	}

	auto realHanzi = (const wchar_t*)m_pEditHanzi->GetUserData();
	if (realHanzi == nullptr) {
		ShowErrorBox(L"m_pEditHanzi error");
		return TRUE;
	}

	if (sHanzi == realHanzi) {
		m_pEditHanzi->SetWindowText(L"");
		if (!m_dataHanzi.Next()) {
			ShowInfoBox(L"己完成一遍");

			m_dataHanzi.Reset();	
		}

		m_pLayHanzi->StartAnimation(m_aniHide);		
	}

	return TRUE;
}
BOOL CMainWnd::OnAnimStopHanzi(EventSwndAnimationStop* pEvt)
{
	
	if (m_pLayHanzi->GetAnimation() == m_aniHide) {
		// hide 动画 结束

		LoadHanzi();
		DrawProgress();
		m_pLayHanzi->StartAnimation(m_aniShow);
	} else if (m_pLayHanzi->GetAnimation() == m_aniShow) {
		// 显示 动画 结束

	}

	return TRUE;	
}
