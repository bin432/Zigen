#pragma once

#include <string>
#include <functional>
#include <list>

namespace common
{
	static std::wstring toW(const char* utf8, int len)
	{
		if (-1 == len) {
			len = static_cast<int>(strlen(utf8));
		}

		if ((nullptr == utf8) || (0 == len))
			return std::wstring();

		// cbMultiChar 如果传 -1  那么返回长度 包括 '\0'  也就是 比 wcslen 多 1;
		// 所以 还是 传 len 取好了 
		int outSize = MultiByteToWideChar(CP_UTF8, 0, utf8, len, NULL, 0);

		std::wstring sOut;
		sOut.resize(outSize);

		MultiByteToWideChar(CP_UTF8, 0, utf8, len, (wchar_t*)sOut.data(), outSize);

		return sOut;
	}
	static std::wstring toW(const std::string& utf8)
	{
		return toW(utf8.c_str(), utf8.length());
	}
	static std::string toUtf8(const wchar_t* w, int len=-1)
	{
		if (-1 == len) {
			len = static_cast<int>(wcslen(w));
		}

		if ((nullptr == w) || (0 == len))
			return std::string();

		// cchWideChar 如果传 -1  那么返回长度 包括 '\0'  也就是 比 wcslen 多 1;
		// 所以 还是 传 len 取好了 
		int outSize = WideCharToMultiByte(CP_UTF8, 0, w, len, NULL, 0, NULL, NULL);

		std::string sOut;			// string 不能包含 '\0' 会发生很多意外的事情
		sOut.resize(outSize);

		WideCharToMultiByte(CP_UTF8, 0, w, len, (char*)sOut.data(), outSize, NULL, NULL);
		return sOut;
	}
	static std::string toUtf8(const std::wstring& w)
	{
		return toUtf8(w.c_str(), w.length());
	}

	static int GetIniFileInt(const wchar_t* lpApp, const wchar_t* lpKey, const wchar_t* inifile, int def = 0)
	{
		return GetPrivateProfileIntW(lpApp, lpKey, def, inifile);
	}
	static bool SetIniFileInt(const wchar_t* lpApp, const wchar_t* lpKey, const wchar_t* inifile, int v)
	{
		return TRUE == WritePrivateProfileStringW(lpApp, lpKey, std::to_wstring(v).c_str(), inifile);
	}

	static std::wstring GetIniFileString(const wchar_t* lpApp, const wchar_t* lpKey, const wchar_t* inifile, const wchar_t* def = NULL)
	{
		WCHAR cBuf[1024] = {0};
		DWORD uSize = GetPrivateProfileStringW(lpApp, lpKey, def, cBuf, 1024, inifile);
		if (uSize > 0) {
			cBuf[uSize] = '\0';
		}

		return std::wstring(cBuf);
	}
	static bool SetIniFileString(const wchar_t* lpApp, const wchar_t* lpKey, const wchar_t* inifile, const wchar_t* v)
	{
		return TRUE == WritePrivateProfileStringW(lpApp, lpKey, v, inifile);
	}


	static void Split(const wchar_t* str, wchar_t sep, std::function<bool(const wchar_t*,int)> fn)
	{
		size_t nValueBegin = 0;
		int len = lstrlenW(str);

		for (size_t i = 0; i <= len; ++i) {
			if (L':' == str[i] && nValueBegin < i) {
				
				if (!fn(str + nValueBegin, i - nValueBegin)) {
					break;
				}

				nValueBegin = i + 1;			
			}
		}
		if (nValueBegin < len) {
			fn(str + nValueBegin, len - nValueBegin);
		}
	}
}