#pragma once
#include "BaseTable.h"
#include "../LotData.h"

class PatTable : public BaseTable
{
private:
	
public:

	bool Create();
	bool Insert(long long lot_nr, unsigned int startpartnum, PATConfig pat_config, ValueConfig valueconfig[]);
	bool GetLast(long long lot, lot::LOT_Data& param, PATConfig& ini_pat_config);
	bool Copy_From_LastsameType(lot::LOT_Data& param, PATConfig& ini_pat_config, bool &isloaded);
};

