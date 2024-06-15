#pragma once
#ifndef __TIP_WND_H___
#define __TIP_WND_H___
namespace SOUI
{
class CTipWnd : public SHostWnd
{
public:
	CTipWnd(LPCTSTR lpLayout)
		: SHostWnd(lpLayout)
	{

	}
	~CTipWnd()
	{

	}
	enum AnchorType{
		AT_Center,
		AT_LeftTop,
		AT_LeftBottom,
		AT_RightTop,
		AT_RightBottom,
	};
protected:
	void OnFinalMessage(HWND hWnd)
	{
		__super::OnFinalMessage(hWnd);
		delete this;
	}

	void SetTip(LPCTSTR lpTip)
	{
		SWindow* pText = FindChildByName(L"txt_tip");
		pText->SetWindowText(lpTip);
	}

	void OnWindowPosChanging(LPWINDOWPOS lpWndPos)
	{
		if (!(lpWndPos->flags&SWP_NOSIZE))
		{//窗口大小改变，根据设置的anchor移动位置
			switch (m_anchorType)
			{
			case AT_Center:
			default:
				lpWndPos->x = m_ptAnchor.x - (lpWndPos->cx)/2;
				lpWndPos->y = m_ptAnchor.y - (lpWndPos->cy) / 2;
				break;
			case AT_LeftTop:
				lpWndPos->x = m_ptAnchor.x;
				lpWndPos->y = m_ptAnchor.y;
				break;
			case AT_LeftBottom:
				lpWndPos->x = m_ptAnchor.x;
				lpWndPos->y = m_ptAnchor.y - lpWndPos->cy;
				break;
			case AT_RightTop:
				lpWndPos->x = m_ptAnchor.x - lpWndPos->cx;
				lpWndPos->y = m_ptAnchor.y;
				break;
			case AT_RightBottom:
				lpWndPos->x = m_ptAnchor.x - lpWndPos->cx;
				lpWndPos->y = m_ptAnchor.y - lpWndPos->cy;
				break;
			}
			lpWndPos->flags &= ~SWP_NOMOVE;
		}
	}
		
	void OnTimer(UINT_PTR timerID)
	{
		SetMsgHandled(FALSE);
		if (100 == timerID)
		{
			__super::DestroyWindow();
		}
	}
	BEGIN_MSG_MAP_EX(CTipWnd)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_WINDOWPOSCHANGING(OnWindowPosChanging)
		CHAIN_MSG_MAP(SHostWnd)
	END_MSG_MAP()
		
	CPoint m_ptAnchor;
	AnchorType	m_anchorType;
	//static TCHAR* ms_lpTipLayout;
public:	
	static void ShowTip(LPCTSTR lpTipText, const CPoint& pt, AnchorType at= AT_Center)
	{
		CTipWnd *pTipWnd = new CTipWnd(_T("layout:wnd_tip"));
		pTipWnd->m_ptAnchor.x = pt.x;
		pTipWnd->m_ptAnchor.y = pt.y;
		pTipWnd->m_anchorType = at;
		pTipWnd->Create(NULL);
		pTipWnd->ShowWindow(SW_SHOWNOACTIVATE);
		pTipWnd->SetTip(lpTipText);
		pTipWnd->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		pTipWnd->SetTimer(100, 3000);
	}
};
//TCHAR* CTipWnd::ms_lpTipLayout = 0;
}

#endif