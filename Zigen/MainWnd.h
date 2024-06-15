#pragma once
#include <list>
#include <map>
#include <vector>
#include <set>
#include "..\ui\SHostWndEx.hpp"


struct ZigenInfo
{
	SStringT file;			// 
	SStringT code;		// 
	IBitmap* bmp;
};

struct ZigenCount
{
	int index;
	int count;
};

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
public:
	void OnBtnNext();
	//bool OnTextChange(EventArgs* e);
protected:
	//事件处理映射表  频繁的 操作可以写到前面来 减少 判断 最后都用 绑定 
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
		EVENT_NAME_COMMAND(L"btn_next", OnBtnNext)
	EVENT_MAP_END()
	
	//窗口消息处理映射表
	BEGIN_MSG_MAP_EX(CMainWnd)
		MSG_WM_INITDIALOG(OnInitDialog)
		
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_CLOSE(OnBtnClose)
		CHAIN_MSG_MAP(SHostWnd)
	END_MSG_MAP()


	void ShowZigen();
protected:
	SImageWnd* m_pImage;
	SStatic* m_pTxtCode;
	SStatic* m_pTxtFirst;
	SStatic* m_pTxtSecond;
protected:	
	// 全部字根
	std::vector<ZigenInfo*> m_arrZigen;


	// 依次展示的 队列
	std::list<ZigenCount*> m_list;

	unsigned int m_progress = 0;		// 进度
	
	unsigned int m_points = 0;			// 总分
	unsigned int m_ccc = 0;				// 连击
};

