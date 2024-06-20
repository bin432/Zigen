//
#include "stdafx.h"
#include "MainWnd.h"

// 这是 静态 使用 soui 界面库 的 例子


#include <commgr2.h>


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	HRESULT hRes = OleInitialize(NULL);
	SASSERT(SUCCEEDED(hRes));

	// 静态 加载 图片解析库 和 绘画引擎

	try
	{
		SComMgr2* pComMgr = new SComMgr2(L"imgdecoder-gdip");
		BOOL bLoaded = FALSE;
		// 图片 解析
		CAutoRefPtr<IImgDecoderFactory> pImgDecoderFactory;
		bLoaded = pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
		//if(!IMGDECODOR_GDIP::SCreateInstance((IObjRef**)&pImgDecoderFactory))
		if(!bLoaded) {
			::MessageBox(NULL, _T("CreateImgDecoder_GDI Error"), _T("错误"), MB_ICONERROR);
			throw 0;
		}

		// 渲染 引擎
		CAutoRefPtr<IRenderFactory> pRenderFactory;
		if (!pComMgr->CreateRender_Skia((IObjRef**)&pRenderFactory))
		{
			::MessageBox(NULL, _T("CreateRender_SKIA Error"), _T("错误"), MB_ICONERROR);
			throw 0;
		}

		pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);

		//定义一个唯一的SApplication对象，SApplication管理整个应用程序的资源
		SApplication* app = new SApplication(pRenderFactory, hInstance);
		
		// 这个 是 加载 系统资源 这个 要 附加到 每个app 的资源里  不然没法 静态
#if 1
		HMODULE hSysResource = hInstance;
		CAutoRefPtr<IResProvider> sysResProvider;
		sysResProvider = new SResProviderPE;
		sysResProvider->Init((WPARAM)hSysResource, 0);
		UINT uSysRet = app->LoadSystemNamedResource(sysResProvider);
		FreeLibrary(hSysResource);
#endif

		// 加载 窗口资源
		CAutoRefPtr<IResProvider>   pResProvider;
#ifdef _DEBUG
		pResProvider = new SResProviderFiles;

		TCHAR lpResPath[MAX_PATH] = { 0 };
		GetModuleFileName(hInstance, lpResPath, MAX_PATH);
		PathRemoveFileSpec(lpResPath);
		PathRemoveFileSpec(lpResPath);
		PathAddBackslash(lpResPath);
		// 这里用自己工程 名
		_tcscat_s(lpResPath, MAX_PATH, _T("Zigen\\uires"));
		if (!pResProvider->Init((LPARAM)lpResPath, 0))
		{
			SASSERT(0);
			throw 0;
		}
#else 
		pResProvider = new SResProviderPE;
		pResProvider->Init((WPARAM)hInstance, 0);
#endif

		app->AddResProvider(pResProvider);

		
		SRunOnUIOp* pRun = new SRunOnUIOp;

		{
			CMainWnd dlg;
			dlg.Create(GetActiveWindow(), 0, 0, 0, 0);
			dlg.GetNative()->SendMessage(WM_INITDIALOG);
			dlg.ShowWindow(SW_SHOWNORMAL);
			app->Run(dlg.m_hWnd);
		}		

		delete pRun;
		delete app;
		delete pComMgr;
	}
	catch(...)
	{

	}

	OleUninitialize();
	return 0;
}
