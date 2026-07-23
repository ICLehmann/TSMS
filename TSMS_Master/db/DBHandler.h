#pragma once
#include "../const.h"
#include "../common/Handler.h"
#include "../ResultBuffer.h"

#include "SQLConnection.h"
#include "ValueTable.h"
#include "LotTable.h"
#include "PatTable.h"
#include "TestTable.h"
#include "BufferTable.h"
#include "ConfigTable.h"

namespace db
{
	class DBHandler : public Handler
	{

	private:
		std::string _connection_string;
		BufferTable _buffer_table;
		ValueTable _value_table;
		LotTable _lot_table;
		PatTable _pat_table;
		TestTable _test_table;
		ConfigTable _config_table;

	public:
		DBHandler();
		~DBHandler();

		bool Init() override;
		void Close() override;

		eHandlerStatus TestTable_GetConfig(std::string name, TestConfig& data);

		eHandlerStatus LotTable_LotExists(long long lot, bool& res);
		eHandlerStatus LotTable_Insert(lot::LOT_Data& lot);
		eHandlerStatus LotTable_Update(lot::LOT_Data& lot);
		eHandlerStatus LotTable_UpdateStatus(long long lot_nr, std::string status);
		eHandlerStatus LotTable_GetLotStatus(long long lot_nr, lot::eLotState& lot_status);
		eHandlerStatus LotTable_DeleteLot(long long lot_nr);
		eHandlerStatus LotTable_GetLot(long long lot, lot::LOT_Data& param);
		eHandlerStatus LotTable_GetLastStartedLot(long long &last_started_lot);
		eHandlerStatus LotTable_GetNextFreeInternalLotNumber(long long& next_free_lot_number);
		eHandlerStatus LotTable_GetLastTime(std::string product_num, time_t& time);
			
		eHandlerStatus ValueTable_GetLastPartNum(long long LotNumber, unsigned int& partNumber);
		eHandlerStatus ValueTable_CountResults(long long LOTnumber, TSMSCounter& cnt);
		eHandlerStatus ValueTable_AddPart_Async(long long lot_nr, std::tuple<long, int, std::vector<MeasuredValue>>& entry);
		eHandlerStatus ValueTable_AddPart_and_UpdatePAT_Async(long long lot_nr, std::tuple<long, int, std::vector<MeasuredValue>>& entry, unsigned int pat_startpart, lot::LOT_Data& lot);
		eHandlerStatus ValueTable_EmptyBuffer(long long lot_nr, ResultBuffer& buffer);

		eHandlerStatus PatTable_Insert(long long lot_nr, unsigned int startpartnum, PATConfig pat_config, ValueConfig valueconfig[]);
		eHandlerStatus PatTable_GetLast(long long lot, lot::LOT_Data& param, PATConfig& ini_pat_config);
		eHandlerStatus PatTable_Copy_From_LastsameType(lot::LOT_Data& param, PATConfig& ini_pat_config, bool &isloaded);

		// these buffer functions are not in use
		eHandlerStatus BufferTable_WriteBuffer(ResultBuffer& buffer);
		eHandlerStatus BufferTable_ReadBuffer(ResultBuffer& buffer);

		eHandlerStatus ConfigTable_Insert(DeviceConfig& dev_config);
	};

}