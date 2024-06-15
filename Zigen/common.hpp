#pragma once

#include <string>
#include <functional>
#include <list>

namespace common
{

	// 横向  查询 depth 深度 
	static bool WalkH(const wchar_t* lpPath, std::function<bool(const wchar_t*, bool, unsigned __int64)> fn, const wchar_t* lpFilter = L"*", int depth = -1)
	{
		std::list<std::wstring> listFolder;
		std::list<int> listDepth;
		int nCurDepth = 0;

		std::wstring sPath = lpPath;
		auto en = --sPath.end();
		if ('\\' == *en || '/' == *en) {
			sPath.erase(en);
		}

		listFolder.push_back(sPath);
		listDepth.push_back(0);

		while (listFolder.size() > 0) {
			//从队列首取出路径
			std::wstring sFolder = listFolder.front();
			listFolder.pop_front();

			if (depth >= 0) {				// 大于等于 0  才 判断 深度
				nCurDepth = listDepth.front();
				listDepth.pop_front();
				if (nCurDepth >= depth) {
					break;
				}
			}

			std::wstring sFilePath = sFolder + L"\\" + lpFilter;

			WIN32_FIND_DATAW fd;
			HANDLE hFind = FindFirstFileW(sFilePath.c_str(), &fd);
			if (INVALID_HANDLE_VALUE == hFind) {
				return false;
			}

			//枚举文件夹内的 文件信息
			do {
				std::wstring strName = fd.cFileName;
				if (L"." == strName || L".." == strName) {
					continue;
				}

				std::wstring s = sFolder + L"\\" + fd.cFileName;
				if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
					if (!fn(s.c_str(), true, 0)) {
						FindClose(hFind);
						return true;
					}

					//如果是文件夹 就先入列   
					listFolder.push_back(s);
					if (depth >= 0) {
						listDepth.push_back(nCurDepth + 1);
					}
					continue;
				}

				ULARGE_INTEGER liSize;
				liSize.LowPart = fd.nFileSizeLow;
				liSize.HighPart = fd.nFileSizeHigh;
				if (!fn(s.c_str(), false, liSize.QuadPart)) {
					FindClose(hFind);
					return true;
				}
			} while (FindNextFileW(hFind, &fd));		//查找下一个

			FindClose(hFind);
		}

		return true;
	}
	// 计算 文件 大小
	static unsigned __int64 CalcFileSize(const wchar_t* lpFilePath)
	{
		ULARGE_INTEGER liSize;
		liSize.QuadPart = 0;

		WIN32_FIND_DATAW Find32;
		HANDLE hFind = FindFirstFileW(lpFilePath, &Find32);
		if (INVALID_HANDLE_VALUE != hFind) {
			liSize.LowPart = Find32.nFileSizeLow;
			liSize.HighPart = Find32.nFileSizeHigh;
			FindClose(hFind);
		}
		return liSize.QuadPart;
	}
	// 返回值 是 LastWriteTime  0 失败
	static DWORD GetFileTime(const wchar_t* lpFilePath, DWORD* pCT = nullptr, DWORD* pLA = nullptr)
	{
		WIN32_FIND_DATAW ff32;

		HANDLE hFind = FindFirstFileW(lpFilePath, &ff32);
		if (hFind != INVALID_HANDLE_VALUE) {
			FindClose(hFind);
			DWORD dwLWT = 0;
			FILETIME ftLocal;
			FileTimeToLocalFileTime(&(ff32.ftLastWriteTime), &ftLocal);
			FileTimeToDosDateTime(&ftLocal, ((LPWORD)&dwLWT) + 1, ((LPWORD)&dwLWT) + 0);

			if (nullptr != pCT) {
				FILETIME ft1;
				FileTimeToLocalFileTime(&(ff32.ftCreationTime), &ft1);
				FileTimeToDosDateTime(&ft1, ((LPWORD)pCT) + 1, ((LPWORD)pCT) + 0);
			}

			if (nullptr != pLA) {
				FILETIME ft2;
				FileTimeToLocalFileTime(&(ff32.ftLastAccessTime), &ft2);
				FileTimeToDosDateTime(&ft2, ((LPWORD)pLA) + 1, ((LPWORD)pLA) + 0);
			}
			return dwLWT;
		}
		return 0;
	}
	static std::wstring GetFileNameWithoutExt(const wchar_t* lpFilePath)
	{
		LPCWSTR lpFileName = ::PathFindFileNameW(lpFilePath);
		if (nullptr == lpFileName) {
			return lpFilePath;
		}

		LPCWSTR lpExt = PathFindExtensionW(lpFileName);
		if (nullptr == lpExt || 0 == lstrlenW(lpExt)) {
			return std::wstring(lpFileName);
		}

		std::wstring sName(lpFileName, lpExt - lpFileName);
		return sName;
	}


}