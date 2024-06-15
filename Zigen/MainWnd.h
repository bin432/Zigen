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
	//�¼�����ӳ���  Ƶ���� ��������д��ǰ���� ���� �ж� ����� �� 
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
		EVENT_NAME_COMMAND(L"btn_next", OnBtnNext)
	EVENT_MAP_END()
	
	//������Ϣ����ӳ���
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
	// ȫ���ָ�
	std::vector<ZigenInfo*> m_arrZigen;


	// ����չʾ�� ����
	std::list<ZigenCount*> m_list;

	unsigned int m_progress = 0;		// ����
	
	unsigned int m_points = 0;			// �ܷ�
	unsigned int m_ccc = 0;				// ����
};

