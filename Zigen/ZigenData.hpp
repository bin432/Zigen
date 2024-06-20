#pragma once
#include <ShlObj.h>
#include <algorithm>
#include <random>

#include <filesystem>
namespace fs = std::filesystem;

#include "common.hpp"

// 根据连击数 计算倍数

struct ZigenItem
{
	SStringT code;		// 
	IBitmap* bmp;
};
class ZigenData
{
public :
	ZigenData()		
	{
		
	}


	void Init(const wchar_t* iniFile)
	{
		m_userIniFile = iniFile;
		m_arrZigen.clear();
		m_list.clear();
		m_mapCount.clear();

		fs::path file(iniFile);
				
		auto dir = file.parent_path() / common::GetIniFileString(L"info", L"dir", iniFile);
				
		m_onePoint = common::GetIniFileInt(L"info", L"score", m_userIniFile, 2);

		std::vector<std::wstring> vec;
		int count = common::GetIniFileInt(L"file", L"count", m_userIniFile);
		if (count == 0) {
			// 还没初始化

			for (const auto& entry : fs::directory_iterator(dir)) {

				if (entry.is_directory()) {

					continue;
				}

				vec.emplace_back(entry.path().filename().wstring());
			}

			// 使用std::shuffle进行随机排序
			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(vec.begin(), vec.end(), g);


			// 输出排序后的结果
			int i = 0;
			for (auto& f : vec) {

				common::SetIniFileString(L"file", std::to_wstring(i).c_str(), m_userIniFile, f.c_str());
				i++;
			}

			common::SetIniFileInt(L"file", L"count", m_userIniFile, i);
		} else {
			for (int i = 0; i < count; i++) {
				auto& f = common::GetIniFileString(L"file", std::to_wstring(i).c_str(), m_userIniFile);
				vec.push_back(f);
			}
		}

		
		for (auto& f : vec) {

			ZigenItem* item = new ZigenItem;
			fs::path fp(f);
			auto fn = fp.stem().wstring();
			int pos = fn.find('_');
			if (pos < 0) {
				item->code = fn.c_str();
			} else {
				item->code = fn.substr(0, pos).c_str();
			}
			item->code.MakeLower();
			GETRENDERFACTORY->CreateBitmap(&item->bmp);

			auto fpath = dir / f;
			item->bmp->LoadFromFile(fpath.wstring().c_str());

			m_arrZigen.push_back(item);
		}

		// 加载 上次 队列
		

		_initUserList();
		if (m_list.empty()) {
			for (int i = 0; i < m_arrZigen.size(); i++) {
				m_list.push_back(i);
				m_mapCount.insert({i, -1});
			}
		}

		m_maxIndex = common::GetIniFileInt(L"config", L"index", m_userIniFile);
		m_points = common::GetIniFileInt(L"config", L"points", m_userIniFile);
		m_ccc = common::GetIniFileInt(L"config", L"ccc", m_userIniFile);
	}
	
	unsigned int GetOnePoint() const
	{
		return m_onePoint;
	}
	unsigned int GetCount() const
	{
		return m_arrZigen.size();
	}
	unsigned int GetMaxIndex() const
	{
		return m_maxIndex;
	}
	unsigned int GetPoints() const
	{
		return m_points;
	}
	unsigned int GetCCC() const
	{
		return m_ccc;
	}
	
	const ZigenItem* GetZigenItem()
	{
		int ii = m_list.front();
		if (ii < 0 || ii >= m_list.size()) {
			return nullptr;
		}
		return m_arrZigen[ii];
	}
	int GetZigenIndex()
	{
		return m_list.front();
	}
	bool IsShowCode() {
		int ii = m_list.front();
		if (ii < 0 || ii >= m_list.size()) {
			return false;
		}

		auto& ite = m_mapCount.find(ii);
		if (ite == m_mapCount.end()) {
			return true;
		}
		
		return ite->second <0;
	}

	int CalcFailedScore()// 错误 
	{
		int ii = m_list.front();
		if (ii < 0 || ii >= m_list.size()) {
			return 0;
		}

		ZigenItem* item = m_arrZigen[ii];
		
		m_ccc = 0;
		m_mapCount[ii] = -1;
		common::SetIniFileInt(L"count", std::to_wstring(ii).c_str(), m_userIniFile, -1);

		//common::SetIniFileInt(L"config", L"points", m_userIniFile, m_points);
		common::SetIniFileInt(L"config", L"ccc", m_userIniFile, m_ccc);
		// 后面 可能 
		return 0;
	}

	int CalcOkScore()
	{
		int ii = m_list.front();
		if (ii < 0 || ii >= m_list.size()) {
			return 0;
		}

		ZigenItem* item = m_arrZigen[ii];

		auto& ite = m_mapCount.find(ii);
		if (ite == m_mapCount.end()) {
			m_mapCount.insert({ii, 0});
		} else {
			ite->second += 1;
		}
		common::SetIniFileInt(L"count", std::to_wstring(ii).c_str(), m_userIniFile, ite->second);
		// 倍数 
		int bb = calcScoreTimes(m_ccc);
		m_ccc += 1;
		// 和 分数
		int calc = m_onePoint * bb;
		m_points += calc;
	
		common::SetIniFileInt(L"config", L"points", m_userIniFile, m_points);
		common::SetIniFileInt(L"config", L"ccc", m_userIniFile, m_ccc);

		return calc;
	}
	
	
	void Next()
	{
		int ii = m_list.front();

		if (ii > m_maxIndex) {
			m_maxIndex = ii;
			common::SetIniFileInt(L"config", L"index", m_userIniFile, m_maxIndex);
		}

		//移出 队头
		m_list.pop_front();

		int count = m_mapCount[ii];

		if (count >= 8) {
			// 连续8次后  直接 移动到 队尾		
			m_list.push_back(ii);
		} else {
			// zigenCount count[01234567] 几次后 跳到 列表的多少位
			static const int indexCount[] = {2, 4, 8, 12, 20, 30, 60, 100};
			int after = indexCount[count];

			auto ite = m_list.begin();
			std::advance(ite, after);
			m_list.insert(ite, ii);
		}

		_saveList();
	}

protected:
	void _initUserList()
	{
		wchar_t* cBuf = new wchar_t[10240];
		DWORD uSize = GetPrivateProfileStringW(L"queue", L"list", L"", cBuf, 10240, m_userIniFile);
		if (uSize > 0) {
			cBuf[uSize] = '\0';
		}

		if (uSize == 0) {
			return;
		}

		int nn;
		size_t nValueBegin = 0;

		for (size_t i = 1; i <= uSize; ++i) {
			if (L':' == cBuf[i] && nValueBegin < i) {
				std::wstring vv(cBuf + nValueBegin, i - nValueBegin);
				nValueBegin = i + 1;

				nn = std::stoi(vv);
				m_list.push_back(nn);
				m_mapCount[nn] = common::GetIniFileInt(L"count", vv.c_str(), m_userIniFile, -1);
			}
		}
		if (nValueBegin < uSize) {
			std::wstring vv(cBuf + nValueBegin, uSize - nValueBegin);
			
			nn = std::stoi(vv);
			m_list.push_back(nn);
			m_mapCount[nn] = common::GetIniFileInt(L"count", vv.c_str(), m_userIniFile, -1);
		}

		delete[] cBuf;
	}

	void _saveList()
	{
		std::wstring value;
		value.reserve(4096);
		for (auto& i : m_list) {
			value += std::to_wstring(i);
			value += L':';
		}

		common::SetIniFileString(L"queue", L"list", m_userIniFile, value.c_str());
	}
	int  calcScoreTimes(int ccc)
	{
		if (ccc > 200) {
			// 抄虎码官网的 
			if (ccc > 1e4) {		// 10000
				return 2e4;
			}
			if (ccc > 5e3) {		// 5000
				return 1e4;
			}
			if (ccc > 3e3) {		// 3000
				return 3e3;
			}
			if (ccc > 2e3) {		// 2000
				return 2e3;
			}
			if (ccc > 1e3) {
				return 1e3;
			}
			if (ccc > 500) {
				return 100;
			}
			if (ccc > 300) {
				return 50;
			}
			if (ccc > 200) {
				return 30;
			}
		}

		if (ccc > 100) {
			return 20;
		}
		if (ccc > 50) {
			return 15;
		}
		if (ccc > 40) {
			return 10;
		}
		if (ccc > 30) {
			return 6;
		}
		if (ccc > 20) {
			return 4;
		}
		if (ccc > 10) {
			return 2;
		}

		return 1;
	}

protected:
	SStringT m_userIniFile;
	int m_onePoint = 2;
	// 全部字根
	std::vector<ZigenItem*> m_arrZigen;

	// 依次展示的 队列
	std::list<int> m_list;
	// index 对应的 正确数量
	std::map<int, int> m_mapCount;

	unsigned int m_maxIndex = 0;		// 进度
	unsigned int m_points = 0;			// 总分
	unsigned int m_ccc = 0;				// 连击
};