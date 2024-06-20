#pragma once
#include <list>
#include <map>
#include <vector>
#include <set>
#include "..\ui\SHostWndEx.hpp"

#include "ZigenData.hpp"
#include "HanziData.hpp"

class CMainWnd : public SHostWndEx	
{
public:
	CMainWnd(void);
	~CMainWnd(void);
	
	void OnBtnClose()
	{
		CloseMain();
	}

	void CloseMain()
	{
		__super::DestroyWindow();
	}

public:
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnTimer(UINT_PTR idEvent);

protected:
	BOOL OnCombSelChange(EventCBSelChange* pEvt);
	BOOL OnEditRENotifyHanzi(EventRENotify* pEvt);
	BOOL OnAnimStopZigen(EventSwndAnimationStop* pEvt);
	BOOL OnAnimStopHanzi(EventSwndAnimationStop* pEvt);

	//�¼�����ӳ���  Ƶ���� ��������д��ǰ���� ���� �ж� ����� �� 
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
	EVENT_MAP_END()
	
	//������Ϣ����ӳ���
	BEGIN_MSG_MAP_EX(CMainWnd)
		MSG_WM_INITDIALOG(OnInitDialog)
		
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_CLOSE(OnBtnClose)
		CHAIN_MSG_MAP(SHostWndEx)
	END_MSG_MAP()

	void DrawProgress();
	void LoadZigen();
	void DoVerify();

	void LoadHanzi();
protected:
	SComboBox* m_pComb;
	SWindow* m_pLayZigen;
	SImageWnd* m_pImage;
	SStatic* m_pTxtCode;
	SStatic* m_pTxtFirst;
	SStatic* m_pTxtSecond;

	SWindow* m_pLayHanzi;
	SStatic* m_pTxtPin;
	SStatic* m_pTxtHanzi;
	SEdit* m_pEditHanzi;

	SStatic* m_pTxtScore;
	SAutoRefPtr<IAnimation> m_aniHide;
	SAutoRefPtr<IAnimation> m_aniShow;
	SAutoRefPtr<IAnimation> m_aniScore;
protected:
	ZigenData m_dataZigen;
	HanziData m_dataHanzi;
};

