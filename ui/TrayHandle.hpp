#pragma once

#include <functional>

#define WM_ICONNOTIFY				(WM_USER + 1111)
#define ID_TIMER_TWINKLE						1234

typedef std::function<void(UINT)> funTrayNotifyCallback;

class TrayHandle : public CSimpleWnd
{
public:
	TrayHandle(void)
	{
		m_funCallback = nullptr;
		m_bTwinking = false;
		ZeroMemory(&m_NotifyIconData, sizeof(m_NotifyIconData));
		m_hTempIcon = NULL;
		WM_TASKBAE_CREATE = ::RegisterWindowMessageW(L"TaskbarCreated");		
	}
	~TrayHandle(void)
	{
		if (nullptr == m_hWnd)
			return;

		Delete();
		DestroyWindow();
	}
public:
	bool Create(HICON hIcon, LPCTSTR lpNotifyText, UINT uFlags, SStringT& sErrText)
	{
		if (0 != m_NotifyIconData.cbSize)
		{
			sErrText = _T("cbSize not zero");
			return false;
		}

		CSimpleWnd::Create(_T("Tray_Virtual_Wnd"), 0, 0, 0, 0, 0, 0, GetActiveWindow(), NULL);
		if (NULL == m_hWnd)
		{
			sErrText.Format(_T("Wnd::Create Fail: %d"), GetLastError());
			return false;
		}

		m_NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
		m_NotifyIconData.hWnd = m_hWnd;
		m_NotifyIconData.uID = 0;
		m_NotifyIconData.uFlags = (uFlags == 0) ? (NIF_ICON | NIF_MESSAGE | NIF_TIP) : uFlags;
		m_NotifyIconData.uCallbackMessage = WM_ICONNOTIFY;
		m_NotifyIconData.hIcon = hIcon;
		_tcscpy_s(m_NotifyIconData.szTip, 128, lpNotifyText);

		Shell_NotifyIcon(NIM_ADD, &m_NotifyIconData);
		// 不判断 成功与否，让程序继续执行，
		// 一般出错 也是 explorer 没启动，等 explorer 启动了  TaskbarCreated 消息 也可以 响应
		//{
		//	sErrText.Format(_T("Shell_NotifyIcon Fail: %d"), GetLastError());
		//	return false;
		//}
		return true;
	}
	void SetNotify_Callback(funTrayNotifyCallback fun)
	{
		m_funCallback = fun;
	}
	bool Delete()
	{
		return (TRUE == Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData));
	}
	void Modify(LPCTSTR lpNotifyText)
	{
		_tcscpy_s(m_NotifyIconData.szTip, 128, lpNotifyText);
		Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
	}
	void Modify(HICON hIcon)
	{
		m_NotifyIconData.hIcon = hIcon;
		Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
	}
	void Modify(LPCTSTR lpNotifyText, HICON hIcon)
	{
		_tcscpy_s(m_NotifyIconData.szTip, 128, lpNotifyText);
		m_NotifyIconData.hIcon = hIcon;
		Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
	}
	void StartTwinke(int nInterval=400)		// 毫秒
	{
		if (m_bTwinking)
			return;

		m_bTwinking = true;
		m_hTempIcon = m_NotifyIconData.hIcon;
		SetTimer(ID_TIMER_TWINKLE, nInterval);
	}
	void StopTwinke()
	{
		m_bTwinking = false;
	}
	bool IsTwinking()
	{
		return m_bTwinking;
	}
	bool ShowBalloon(LPCTSTR lpBalloonTitle, LPCTSTR lpBalloonMsg, DWORD dwIcon = NIIF_NONE, UINT nTimeOut = 10)
	{
		m_NotifyIconData.dwInfoFlags = dwIcon;
		//m_NotifyIconData.uFlags |= NIF_TIP;
		m_NotifyIconData.uFlags |= NIF_INFO;
		m_NotifyIconData.uTimeout = nTimeOut;
		// Set the balloon title
		_tcscpy_s(m_NotifyIconData.szInfoTitle, 64, lpBalloonTitle);

		// Set balloon message
		_tcscpy_s(m_NotifyIconData.szInfo, 256, lpBalloonMsg);

		// Show balloon....
		bool b = TRUE == Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
		m_NotifyIconData.szInfoTitle[0] = 0;
		m_NotifyIconData.szInfo[0] = 0;
		return b;
	}
protected:
	LRESULT OnIconNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bHandled)
	{
		if (nullptr == m_funCallback)
		{
			return 0;
		}

		m_funCallback(LOWORD(lParam));

		return 0;
	}
	LRESULT OnTaskbarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (NULL == m_NotifyIconData.hIcon)
			m_NotifyIconData.hIcon = m_hTempIcon;

		Shell_NotifyIcon(NIM_ADD, &m_NotifyIconData);
		return 0;
	}
	void OnTimer(UINT_PTR nIDEvent)
	{
		if (ID_TIMER_TWINKLE == nIDEvent)
		{
			if (!m_bTwinking)
			{
				KillTimer(nIDEvent);
				m_NotifyIconData.hIcon = m_hTempIcon;
				Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
				m_hTempIcon = NULL;
			}
			else if (NULL == m_NotifyIconData.hIcon)
			{
				m_NotifyIconData.hIcon = m_hTempIcon;
				Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
			}
			else
			{
				m_NotifyIconData.hIcon = NULL;
				Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
			}
		}
	}
	

	BEGIN_MSG_MAP_EX(TrayHandle)
		MESSAGE_HANDLER_EX(WM_TASKBAE_CREATE, OnTaskbarCreated)			// Taskbar 重启
		MSG_WM_TIMER(OnTimer)
		MESSAGE_HANDLER(WM_ICONNOTIFY, OnIconNotify)
		CHAIN_MSG_MAP(CSimpleWnd)
	END_MSG_MAP()

	
protected:
	NOTIFYICONDATA		m_NotifyIconData;
	HICON						m_hTempIcon;
	funTrayNotifyCallback	m_funCallback;
	UINT							WM_TASKBAE_CREATE;
	bool							m_bTwinking;
};


