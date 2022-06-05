#include <chrono>
#include <iosfwd>
#include "SpeedTest.h"
#include "Clock.h"

SpeedTestRecord g_speedtestrecord;

SpeedTestData* SpeedTestRecord::Reg(const SpeedTestPosition& testpos)
{
	// 记录 name值有效才记录
	if (!brecord || !testpos.name)
	{
		return NULL;
	}
	// 先用共享锁 如果存在直接修改
	{
		read_lock lock(mutex);
		auto it = ((const SpeedTestPositionMap&)records).find(testpos); // 显示的调用const的find
		if (it != records.cend())
		{
			return it->second;
		}
	}

	// 不存在构造一个
	SpeedTestData* p = new SpeedTestData;
	// 使用写锁
	{
		write_lock lock(mutex);
		records.insert(std::make_pair(testpos, p));
	}
	return p;
}

std::string SpeedTestRecord::Snapshot(SnapshotType type, const std::string& metricsprefix, const std::map<std::string, std::string>& tags)
{
	SpeedTestPositionMap lastdata;
	{
		read_lock lock(mutex);
		lastdata = records;
	}
	static const int metirs_num = 3;
	static const char* metirs[metirs_num] = { "speedtest_calltimes", "speedtest_elapse", "speedtest_maxelapse" };

	std::ostringstream ss;
	if (type == Json)
	{
		ss << "[";
		int index = 0;
		for (const auto& it : lastdata)
		{
			int64_t value[metirs_num] = { it.second->calltimes, it.second->elapsedTSC / TSCPerUS(), it.second->elapsedMaxTSC / TSCPerUS() };
			ss << ((++index) == 1 ? "[" : ",[");
			for (int i = 0; i < metirs_num; ++i)
			{
				ss << (i == 0 ? "{" : ",{");
				ss << "\"metrics\":\"" << metricsprefix << metirs[i] << "\",";
				ss << "\"name\":\"" << it.first.name << "\",";
				ss << "\"num\":" << it.first.num << ",";
				for (const auto& it : tags)
				{
					ss << "\"" << it.first << "\":\"" << it.second << "\",";
				}
				ss << "\"value\":" << value[i] << "";
				ss << "}";
			}
			ss << "]";
		}
		ss << "]";
	}
	else if (type == Influx)
	{
		std::string tag;
		for (const auto& t : tags)
		{
			tag += ("," + t.first + "=" + t.second);
		}
		for (const auto& it : lastdata)
		{
			int64_t value[metirs_num] = { it.second->calltimes, it.second->elapsedTSC / TSCPerUS(), it.second->elapsedMaxTSC / TSCPerUS() };
			for (int i = 0; i < metirs_num; ++i)
			{
				ss << metricsprefix << metirs[i];
				ss << ",name=" << it.first.name << ",num=" << it.first.num << tag;
				ss << " value=" << value[i] << "i\n";
			}
		}
	}
	else if (type == Prometheus)
	{
		std::string tag;
		for (const auto& t : tags)
		{
			tag += ("," + t.first + "=\"" + t.second + "\"");
		}
		for (const auto& it : lastdata)
		{
			int64_t value[metirs_num] = { it.second->calltimes, it.second->elapsedTSC / TSCPerUS(), it.second->elapsedMaxTSC / TSCPerUS() };
			for (int i = 0; i < metirs_num; ++i)
			{
				ss << metricsprefix << metirs[i];
				ss << "{name=\"" << it.first.name << "\",num=\"" << it.first.num << "\"" << tag << "}";
				ss << " " << value[i] << "\n";
			}
		}
	}
	return ss.str();
}

SpeedTest::SpeedTest(const char* _name_, int _index_)
	: pspeedtestdata(g_speedtestrecord.Reg(SpeedTestPosition(_name_, _index_)))
	, begin_tsc(TSC())
{
}

SpeedTest::SpeedTest(SpeedTestData* p)
	: pspeedtestdata(p)
	, begin_tsc(TSC())
{
}

SpeedTest::~SpeedTest()
{
	if (pspeedtestdata)
	{
		int64_t tsc = TSC() - begin_tsc;
		pspeedtestdata->calltimes++;
		pspeedtestdata->elapsedTSC += tsc;
		if (pspeedtestdata->elapsedMaxTSC < tsc)
		{
			pspeedtestdata->elapsedMaxTSC = tsc;
		}
	}
}

