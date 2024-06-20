// at 2017-12-15 by bin432
// 

#include <souistd.h>
//#include <core/SHostDialog.h>
#include <control/SMessageBox.h>
#include <control/souictrls.h>
#include "TipWnd.hpp"

namespace SOUI
{
class SHostWndEx : public SOUI::SHostWnd
{
public:
	SHostWndEx(LPCTSTR pszResName = NULL)
		: SHostWnd(pszResName)
	{
	}
	virtual ~SHostWndEx()
	{

	}
	void OnBtnMax()
	{
		SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
	}
	void OnBtnRestore()
	{
		SendMessage(WM_SYSCOMMAND, SC_RESTORE);
	}
	void OnBtnMin()
	{
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
		//GetNative()->SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
	}
	
public:
	virtual int ShowInfoBox(LPCTSTR lpText, LPCTSTR lpCaption = _T("��ʾ"))
	{
		return SMessageBox(m_hWnd, lpText, lpCaption, MB_ICONINFORMATION);
	}
	virtual int ShowErrorBox(LPCTSTR lpText, LPCTSTR lpCaption = _T("����"))
	{
		return SMessageBox(m_hWnd, lpText, lpCaption, MB_ICONERROR);
	}
	virtual void ShowTip(LPCTSTR lpTip)
	{
		CPoint pt = GetClientRect().CenterPoint();
		ClientToScreen(&pt);
		CTipWnd::ShowTip(lpTip, pt);
	}
	
	template<class T>
	inline bool InitWnd(T*& pWnd, LPCTSTR lpWndName)
	{
		pWnd = FindChildByName2<T>(lpWndName);
		//assert(pWnd);
		if (nullptr == pWnd) {
			SStringT sErrorText;
			sErrorText.Format(_T("û��nameΪ <%s> �Ŀؼ�"), lpWndName);
			ShowErrorBox(sErrorText);
		}
		return nullptr != pWnd;
	}

	void SetChildVisible(LPCTSTR lpChildName, BOOL bShow)
	{
		auto p = FindChildByName(lpChildName);
		if (nullptr != p) {
			p->SetVisible(bShow, TRUE);
		}
	}

	void SetChildVisible(int nId, BOOL bShow)
	{
		auto p = FindChildByID(nId);
		if (nullptr != p) {
			p->SetVisible(bShow, TRUE);
		}
	}
	BOOL IsChildVisible(LPCTSTR lpChildName, BOOL bCheckParent=FALSE)
	{
		auto p = FindChildByName(lpChildName);
		if (nullptr != p) {
			return p->IsVisible(bCheckParent);
		}
		return FALSE;
	}
	void SetChildText(LPCTSTR lpChildName, LPCTSTR lpText)
	{
		auto p = FindChildByName(lpChildName);
		if (nullptr != p) {
			p->SetWindowText(lpText);
		}
	}
	void SetChildFormatText(LPCTSTR lpChildName, LPCTSTR format, ...)
	{				
		auto p = FindChildByName(lpChildName);
		if (nullptr != p) {
			va_list v;
			va_start(v, format);
			int nlen = _vscwprintf(format, v);			// ���ﲻ +1  ��Ϊstring �� ����Ҫ \0 ��β��
			SStringT s(nlen, nlen);
			int n = vswprintf_s((wchar_t*)s.GetBuffer(nlen), (size_t)nlen + 1, format, v);
			va_end(v);
			s.ReleaseBuffer(nlen);
			p->SetWindowText(s);
		}
	}
	SStringT GetChildText(LPCTSTR lpChildName)
	{
		auto p = FindChildByName(lpChildName);
		if (nullptr != p) {
			return p->GetWindowText();
		}
		return _T("");
	}

	BOOL IsChildChecked(LPCTSTR lpChildName)
	{
		auto p = FindChildByName(lpChildName);
		if (nullptr != p) {
			return p->IsChecked();
		}
		return FALSE;
	}
	void SetChildCheck(LPCTSTR lpChildName, BOOL bCheck)
	{
		auto p = FindChildByName(lpChildName);
		if (nullptr != p) {
			p->SetCheck(bCheck);
		}
	}

	void EnableChild(LPCTSTR lpChildName, BOOL bEnable, BOOL bUpdate=FALSE)
	{
		auto p = FindChildByName(lpChildName);
		if (nullptr != p) {
			p->EnableWindow(bEnable, bUpdate);
		}
	}

	void SetChildBgColor(LPCTSTR lpChildName, COLORREF cr, BOOL bUpdate = FALSE)
	{
		auto p = FindChildByName(lpChildName);
		if (nullptr != p) {
			p->GetStyle().m_crBg = cr;
			if (bUpdate) {
				p->Invalidate();
			}
		}
	}
	ULONG_PTR SetChildData(LPCTSTR lpChildName, ULONG_PTR uData)
	{
		auto p = FindChildByName(lpChildName);
		if (nullptr != p) {
			return p->SetUserData(uData);
		}
		return 0;
	}

	ULONG_PTR GetChildData(LPCTSTR lpChildName)
	{
		auto p = FindChildByName(lpChildName);
		if (nullptr != p) {
			return p->GetUserData();
		}
		return 0;
	}

	void SetChildParentVisible(LPCTSTR lpChildName, BOOL bShow)
	{
		auto p = FindChildByName(lpChildName);
		if (nullptr != p) {
			p = p->GetParent();
			if (nullptr != p) {
				p->SetVisible(bShow, TRUE);
			}
		}
	}

	HRESULT SetChildAttribute(LPCTSTR lpChildName, LPCTSTR lpAttribName, LPCTSTR lpValue, BOOL bLoading=FALSE)
	{
		auto p = FindChildByName(lpChildName);
		if (nullptr != p) {
			return p->SetAttribute(lpAttribName, lpValue, bLoading);
		}
		return E_FAIL;
	}

	template<typename T, typename A>
	bool subEvent(LPCTSTR lpName, bool (T::* pFn)(A *), T* pObject)
	{
		auto p = FindChildByName(lpName);
		if (nullptr == p) {
			return false;
		}
		return p->GetEventSet()->subscribeEvent(A::EventID, Subscriber(pFn, pObject));
	}

	template<typename T, typename A>
	bool subEvent(int nId, bool (T::* pFn)(A *), T* pObject)
	{
		auto p = FindChildByID(nId);
		if (nullptr == p) {
			return false;
		}
		return p->GetEventSet()->subscribeEvent(A::EventID, Subscriber(pFn, pObject));
	}

	void FlashWindow(DWORD dwFlags = FLASHW_TRAY | FLASHW_TIMERNOFG)
	{
		FLASHWINFO fwi;
		fwi.cbSize = sizeof(fwi);
		fwi.hwnd = m_hWnd;
		fwi.dwFlags = dwFlags;
		fwi.uCount = 0;
		fwi.dwTimeout = 0;

		::FlashWindowEx(&fwi);
	}

	// nType 0-�м�; 1-����; 2-����; 3- ����; 4-����
	// 11-����; 12-����
	void MoveWnd(int nType=0, int nPadding = 10)
	{
		HMONITOR hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
		if (hMonitor == NULL) {
			return;
		}
		MONITORINFO minfo;
		minfo.cbSize = sizeof(MONITORINFO);
		::GetMonitorInfo(hMonitor, &minfo);

		CRect rc = GetClientRect();
		switch (nType) {
		case 1:	// ����
			rc.OffsetRect(minfo.rcWork.right - rc.Width() - nPadding, minfo.rcWork.bottom - rc.Height() - nPadding);
			break;
		case 2: // ����
			rc.OffsetRect(minfo.rcWork.right - rc.Width() - nPadding, nPadding);
			break;
		case 3:	// ����
			rc.OffsetRect(nPadding, nPadding);
			break;
		case 4:	// ����
			rc.OffsetRect(nPadding, minfo.rcWork.bottom - rc.Height() - nPadding);
			break;
		case 11: // ����
			rc.OffsetRect((minfo.rcWork.right-rc.right)/2, nPadding);
			break;
		case 12: // ����
			rc.OffsetRect((minfo.rcWork.right - rc.right) / 2, minfo.rcWork.bottom - rc.Height() - nPadding);
			break;
		default:
			return;
		}
		__super::MoveWindow2(&rc);
	}
};

}
