#pragma once
#include <ShlObj.h>
#include <algorithm>
#include <random>

#include <filesystem>
namespace fs = std::filesystem;

#include "common.hpp"


struct HanziItem
{
	std::wstring hanzi;		// 
	std::wstring pin;
	std::wstring code;
	std::wstring zigen[4];
	IBitmap* bmp[4];
};
class HanziData
{
public :
	HanziData()
	{
		
	}


	void Init(const wchar_t* iniFile)
	{
		m_userIniFile = iniFile;
		m_arrHanzi.clear();
		m_mapZigen.clear();

		fs::path file(iniFile);
				
		auto dir = file.parent_path() / common::GetIniFileString(L"zigen", L"dir", m_userIniFile);

		// 加载 全部 字根。
		for (const auto& entry : fs::directory_iterator(dir)) {

			if (entry.is_directory()) {
				continue;
			}

			auto f = entry.path().wstring();
			std::wstring nn = entry.path().stem().wstring();

			IBitmap* bmp;
			GETRENDERFACTORY->CreateBitmap(&bmp);
			bmp->LoadFromFile(f.c_str());

			m_mapZigen.insert({nn, bmp});
		}

	
		int count = common::GetIniFileInt(L"hanzi", L"count", m_userIniFile);
		for (int i = 1; i <= count; i++) {
			auto app = std::to_wstring(i);
			HanziItem* item = new HanziItem;

			item->hanzi = common::GetIniFileString(app.c_str(), L"name", m_userIniFile);
			if (item->hanzi.empty()) {
				delete item;
				continue;
			}
			item->pin = common::GetIniFileString(app.c_str(), L"pin", m_userIniFile);
			item->code = common::GetIniFileString(app.c_str(), L"code", m_userIniFile);

			auto zigen = common::GetIniFileString(app.c_str(), L"zigen", m_userIniFile);

			int zigenII = 0;
			common::Split(zigen.c_str(), L'|', [&item,&zigenII,this](const wchar_t* p, int l)->bool {
				std::wstring zg(p, l);
				item->bmp[zigenII] = m_mapZigen[zg];
				int pos = zg.find('_');
				if (pos < 0) {					
					item->zigen[zigenII] = std::move(zg);
				} else {
					item->zigen[zigenII] = std::move(zg.substr(0, pos));
				}

				zigenII += 1;

				return true;
			});
			
			m_arrHanzi.push_back(item);
		}
			
		m_index = common::GetIniFileInt(L"config", L"index", m_userIniFile);
	}
	
	unsigned int GetCount() const
	{
		return m_arrHanzi.size();
	}
	unsigned int GetIndex() const
	{
		return m_index;
	}	

	const HanziItem* GetItem()
	{
		if (m_index >= m_arrHanzi.size()) {
			return nullptr;
		}
		return m_arrHanzi[m_index];
	}

	bool Next()
	{
		m_index += 1;

		if (m_index >= m_arrHanzi.size()) {
			return false;
		}

		common::SetIniFileInt(L"config", L"index", m_userIniFile, m_index);
		return true;
	}
	void Reset()
	{
		m_index = 0;
		common::SetIniFileInt(L"config", L"index", m_userIniFile, m_index);
	}
protected:
	
protected:
	SStringT m_userIniFile;
	// 全部字根
	std::vector<HanziItem*> m_arrHanzi;
	std::map<std::wstring, IBitmap*> m_mapZigen;

	unsigned int m_index = 0;		// 进度
};