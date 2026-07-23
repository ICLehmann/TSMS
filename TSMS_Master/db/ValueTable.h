#pragma once
#include "BaseTable.h"
#include "../MeasuredValue.h"
#include "../Counter.h"

class ValueTable : public BaseTable
{
private:
	long CountOneResult(long long LOTnumber, std::string measurement, std::string result);
public:
	bool Create();
	bool Insert(long long LOTnumber, unsigned int PartNumber, int LastCycleTime, std::vector<MeasuredValue>& data);
	bool GetLastPartNum(long long LotNumber, unsigned int& PartNumber);
	bool PartExists(long long LOTnumber, unsigned int PartNumber);
	bool CountResults(long long LOTnumber, TSMSCounter& cnt);
};

