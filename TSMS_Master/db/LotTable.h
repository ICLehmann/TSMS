#pragma once
#include "BaseTable.h"
#include "../LotData.h"

class LotTable : public BaseTable
{
private:
	lot::eLotState LotStatusFromString(std::string status);
public:

	bool Create();
	bool LotExists(long long lot, bool& res);
	bool Insert(lot::LOT_Data& param);
	bool Update(lot::LOT_Data& param);
	bool UpdateStatus(long long lot_nr, std::string status);
	bool UpdateLabelCounter(long long lot_nr, unsigned int label_cnt);
	bool GetLotStatus(long long lot_nr, lot::eLotState& lot_status);
	bool DeleteLot(long long lot_nr);
	bool GetLot(long long lot, lot::LOT_Data& param);
	
	bool GetLastStartedLot(long long& last_started_lot);
	bool GetLastInternalLotNumber(long long& last_dummy_lot);
	
	bool GetLastTime(std::string product_num, std::string& time_str);
};

