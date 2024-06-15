// at 2018-8-21 by bin432
// soui 里的 runonui 在vs2010 不能编译 
// 这个头文件是 给 界面工程单独使用的  如果 你 soui 是大于 2010  就不用这个 头文件了 直接使用 NotifyCenter了

#include <functional>
namespace SOUI
{

////////////////////////////////-宏- - 将fn封装的代码 传递到UI线程去 执行 -//////////////////////////
// 这个是用SendMessage 到UI线程
// [&] 中的 & 是指 fn里调用的变量 都是 引用拷贝的
//#define SRUNONUISYNC(fn)		SRunOnUIOp::RunOnUISync([&](){fn})
#define SRUNONUISYNC		SRunOnUIOp::getSingleton() + [&]()

// 这个是用PostMessage 到UI线程  [=] 中的 = 是指 fn里调用的变量 都是 值拷贝的
//#define SRUNONUI(fn)		SRunOnUIOp::RunOnUIAsync([=](){fn})
#define SRUNONUI		SRunOnUIOp::getSingleton() - [=]()

// 运行在UI线程
class SRunOnUIOp : public SSingleton<SRunOnUIOp>
{
public:
	SRunOnUIOp()
	{
		m_pUIWnd = new SRunOnUIWnd(this);
		m_pUIWnd->CreateNative(_T("RunOnUI"), WS_POPUP, 0, 0, 0, 0, 0, HWND_MESSAGE, 0);
		SASSERT(m_pUIWnd->IsWindow());
	}
	~SRunOnUIOp()
	{
		m_pUIWnd->DestroyWindow();
		delete m_pUIWnd;
		m_pUIWnd = nullptr;
	}

	
	void operator+(std::function<void()> fn)
	{
		m_pUIWnd->SendMessage(SRunOnUIWnd::UM_RUNUI, 1, (LPARAM)&fn);
	}

	void operator-(std::function<void()> fn)
	{
		auto f = new std::function<void()>(std::move(fn));
		m_pUIWnd->PostMessage(SRunOnUIWnd::UM_RUNUI, 2, (LPARAM)f);
	}
	
	class SRunOnUIWnd : public SNativeWnd
	{
	public:
		enum{UM_RUNUI = (WM_USER + 1204) };
		SRunOnUIWnd(SRunOnUIOp* pOwner){}
		~SRunOnUIWnd(){ }
		LRESULT OnRunUI(UINT uMsg, WPARAM wParam, LPARAM lParam)
		{		
			std::function<void()>* fn= (std::function<void()>*)lParam;
			(*fn)();

			if (2 == wParam) {
				delete fn;
			}
			return 0;
		}

		BEGIN_MSG_MAP_EX(SRunOnUIWnd)
			MESSAGE_HANDLER_EX(UM_RUNUI, OnRunUI)
		END_MSG_MAP()
	};
protected:
	SRunOnUIWnd* m_pUIWnd;
public:
	//static void RunOnUISync(std::function<void()> fn)
	//{
	//	ms_Singleton->m_pUIWnd->SendMessage(SRunOnUIWnd::UM_RUNUI, 1, (LPARAM)&fn);
	//}
	//static void RunOnUIAsync(std::function<void()> fn)
	//{
	//	auto f = new std::function<void()>(std::move(fn));
	//	ms_Singleton->m_pUIWnd->PostMessage(SRunOnUIWnd::UM_RUNUI, 2, (LPARAM)f);
	//}
};

template<>
SRunOnUIOp* SSingleton<SRunOnUIOp>::ms_Singleton = 0;



}
