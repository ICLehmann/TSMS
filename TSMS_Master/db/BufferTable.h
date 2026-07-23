#pragma once
#include "BaseTable.h"
#include "../MeasuredValue.h"
#include "../ResultBuffer.h"

class BufferTable : public BaseTable
{
public:
	bool Create();
	bool Insert(long long PartNumber, int last_cycle_time, std::vector<MeasuredValue>& data);
	bool PartExists(long long PartNumber);

	bool GetBuffer(ResultBuffer& result_buffer);
};

