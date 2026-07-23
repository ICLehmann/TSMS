#include "DBHandler.h"
#include <thread>
#include <atomic>
#include <vector>
#include <iostream>

#include "../common/DateTime.h"
#include "../common/Logger.h"
#include "../common/IniFile.h"

#define DB_ERROR_NONE 0
#define DB_ERROR_PART_EXISTS 1
#define DB_ERROR_INSERT 2

namespace db
{
	static SQLConnection sql_connect;
#pragma region db_thread

	std::atomic<bool> db_thread_running = false;
	std::atomic<int> db_thread_error = 0;
	std::thread db_thread;
	static std::vector<MeasuredValue> db_thread_shared_results;
	static ValueConfig db_pat_thread_shared_results[tsms_config::NUM_MEASUREMENTS];
	static PATConfig db_pat_thread_shared_setup;
	ValueTable* g_pValTable = nullptr;
	PatTable* g_pPatTable = nullptr;
	
	// saves the part's data into the database
	void db_thread_function(long long lot_nr, unsigned int part_num, int last_cycle_time)
	{
		if (g_pValTable->PartExists(lot_nr, part_num))	// checks if the part with same index number is exist in the database
		{
			db_thread_error = DB_ERROR_PART_EXISTS;
		}
		else
		{
			if (!g_pValTable->Insert(lot_nr, part_num, last_cycle_time, db_thread_shared_results))
				db_thread_error = DB_ERROR_INSERT;
		}
		db_thread_running = false;
	}

	// saves the part's data into the database
	void db_thread_function_extPAT(long long lot_nr, unsigned int part_num, int last_cycle_time, unsigned int pat_startpart)
	{
		if (g_pValTable->PartExists(lot_nr, part_num))	// checks if the part with same index number is exist in the database
		{
			db_thread_error = DB_ERROR_PART_EXISTS;
		}
		else
		{
			if (!g_pValTable->Insert(lot_nr, part_num, last_cycle_time, db_thread_shared_results))
				db_thread_error = DB_ERROR_INSERT;
		}
		if (!g_pPatTable->Insert(lot_nr, pat_startpart, db_pat_thread_shared_setup, db_pat_thread_shared_results))
			db_thread_error = DB_ERROR_INSERT;

		db_thread_running = false;
	}

#pragma endregion

	DBHandler::DBHandler() : Handler("DB")
	{
		_connection_string = tsms_config::db_connection_string;
		_value_table.Setup(&sql_connect, tsms_config::db_TableNameValues);
		_lot_table.Setup(&sql_connect, tsms_config::db_TableNameLot);
		_pat_table.Setup(&sql_connect, tsms_config::db_TableNamePat);
		_test_table.Setup(&sql_connect, tsms_config::db_TableNameTests);
		_buffer_table.Setup(&sql_connect, tsms_config::db_TableNameBuffer);
		_config_table.Setup(&sql_connect, tsms_config::db_TableNameConfig);
		g_pValTable = &_value_table;
		g_pPatTable = &_pat_table;
	}

	DBHandler::~DBHandler()
	{
		Close();
	}

	bool DBHandler::Init()
	{
		IniFile ini(tsms_config::INI_FILE.c_str());
		bool createNewValueTab = ini.ReadBoolean("Database", "CreateNewValueTable", 0);
		bool createNewLotTab = ini.ReadBoolean("Database", "CreateNewLotTable", 0);
		bool createNewPatTab = ini.ReadBoolean("Database", "CreateNewPatTable", 0);
		bool createNewTestplanTab = ini.ReadBoolean("Database", "CreateNewTestplanTable", 0);
		bool createNewBufferTab = ini.ReadBoolean("Database", "CreateNewBufferTable", 0);

		db_thread_error = DB_ERROR_NONE;

		if (sql_connect.Connect(_connection_string))
		{
			if (createNewValueTab)
				_value_table.Create();
			if (createNewLotTab)
				_lot_table.Create();
			if (createNewPatTab)
				_pat_table.Create();
			if (createNewTestplanTab)
				_test_table.Create();
			if (createNewBufferTab)
				_buffer_table.Create();

			_config_table.Create();		// recreates the table because it always filled with current config 

			_handler_status = eHandlerStatus::H_SUCCESS;
			logger::WriteLog("Database handler initialized");
			return true;
		}
		else
		{
			_handler_status = eHandlerStatus::H_ERROR;
			logger::WriteLog("ERROR: Database handler initialization failed");
			return false;
		}
	}

	void DBHandler::Close()
	{
		if (db_thread.joinable())
			db_thread.join();
		sql_connect.Disconnect();
		_handler_status = eHandlerStatus::H_CLOSED;
		logger::WriteLog("Database connection closed");
	}

	eHandlerStatus DBHandler::TestTable_GetConfig(std::string name, TestConfig& data)
	{
		if (!_test_table.GetConfig(name, data))
		{
			_last_error = "Can not read test plan '" + name + "'.";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		return _handler_status;
	}

	eHandlerStatus DBHandler::LotTable_LotExists(long long lot, bool& res)
	{
		if (!_lot_table.LotExists(lot, res))
		{
			_last_error = "Can not check if lot exists.";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		return _handler_status;
	}

	eHandlerStatus DBHandler::LotTable_Insert(lot::LOT_Data& lot)
	{
		if (!_lot_table.Insert(lot))
		{
			_last_error = "Can not insert lot '" + std::to_string(lot.header.lot_nr) + "'.";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		return _handler_status;
	}

	eHandlerStatus DBHandler::LotTable_Update(lot::LOT_Data& lot)
	{
		if (!_lot_table.Update(lot))
		{
			_last_error = "Can not update lot '" + std::to_string(lot.header.lot_nr) + ".";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		return _handler_status;
	}

	eHandlerStatus DBHandler::LotTable_DeleteLot(long long lot_nr)
	{
		if (!_lot_table.DeleteLot(lot_nr))
		{
			_last_error = "Can not delete lot '" + std::to_string(lot_nr) + ".";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		return _handler_status;
	}

	eHandlerStatus DBHandler::LotTable_GetLot(long long lot, lot::LOT_Data& param)
	{
		if (!_lot_table.GetLot(lot, param))
		{
			_last_error = "Can not read lot '" + std::to_string(lot) + ".";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		return _handler_status;
	}

	eHandlerStatus DBHandler::LotTable_GetLastStartedLot(long long& last_started_lot)
	{
		if (!_lot_table.GetLastStartedLot(last_started_lot))
		{
			_last_error = "Can not read last started lot'.";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		return _handler_status;
	}

	eHandlerStatus DBHandler::LotTable_GetNextFreeInternalLotNumber(long long& next_free_lot_number)
	{
		if (!_lot_table.GetLastInternalLotNumber(next_free_lot_number))
		{
			_last_error = "Can not read last compensation or dummy lot number.";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		else
			next_free_lot_number++;
		return _handler_status;
	}

	eHandlerStatus DBHandler::LotTable_UpdateStatus(long long lot_nr, std::string status)
	{
		if (!_lot_table.UpdateStatus(lot_nr, status))
		{
			_last_error = "Can not update status from lot '" + std::to_string(lot_nr) + ".";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		return _handler_status;
	}

	eHandlerStatus DBHandler::LotTable_GetLotStatus(long long lot_nr, lot::eLotState& lot_status)
	{
		if (!_lot_table.GetLotStatus(lot_nr, lot_status))
		{
			_last_error = "Can not get status from lot '" + std::to_string(lot_nr) + ".";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		return _handler_status;
	}

	eHandlerStatus DBHandler::LotTable_GetLastTime(std::string product_num, time_t& time)
	{
		time = 0;
		std::string time_str;
		if (!_lot_table.GetLastTime(product_num, time_str))
		{
			_last_error = "Can not read last dummy or compensation time.";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		else if (time_str.length() > 0)
		{
			time = StrTimeToTime(time_str);
		}
		return _handler_status;
	}

	eHandlerStatus DBHandler::ValueTable_GetLastPartNum(long long LotNumber, unsigned int& partNumber)
	{
		if (!_value_table.GetLastPartNum(LotNumber, partNumber))
		{
			_last_error = "Can not read last part number.";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		return _handler_status;
	}

	eHandlerStatus DBHandler::ValueTable_CountResults(long long LOTnumber, TSMSCounter& cnt)
	{
		if (!_value_table.CountResults(LOTnumber, cnt))
		{
			_last_error = "Can not count results.\n";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		return _handler_status;
	}


	// checks db errors and starts a thread that save the part's data
	eHandlerStatus DBHandler::ValueTable_AddPart_Async(long long lot_nr, std::tuple<long, int, std::vector<MeasuredValue>>& entry)
	{
		if (db_thread_running)
		{
			_last_error = "Database not ready after one measurement cycle.";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		else if (db_thread_error == DB_ERROR_PART_EXISTS)
		{
			_last_error = "Last insert into value table failed: Part exists!";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		else if (db_thread_error == DB_ERROR_INSERT)
		{
			_last_error = "Last insert into value table failed: Insert not possible!";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		else
		{
			if (std::get<2>(entry).size() > 0)
			{
				db_thread_shared_results = std::get<2>(entry);		// copy for database
				long part_num = std::get<0>(entry);

				if (db_thread.joinable())
					db_thread.join();
				db_thread_running = true;	// flag that sign the saving thread is running
				db_thread = std::thread(db_thread_function, lot_nr, part_num, std::get<1>(entry));
			}
		}

		return _handler_status;
	}

	// checks db errors and starts a thread that save the part's data
	eHandlerStatus DBHandler::ValueTable_AddPart_and_UpdatePAT_Async(long long lot_nr, std::tuple<long, int, std::vector<MeasuredValue>>& entry, unsigned int pat_startpart, lot::LOT_Data& lot)
	{
		if (db_thread_running)
		{
			_last_error = "Database not ready after one measurement cycle.";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		else if (db_thread_error == DB_ERROR_PART_EXISTS)
		{
			_last_error = "Last insert into value table failed: Part exists!";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		else if (db_thread_error == DB_ERROR_INSERT)
		{
			_last_error = "Last insert into value table failed: Insert not possible!";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		else
		{
			if (std::get<2>(entry).size() > 0)
			{
				// copy for database
				db_thread_shared_results = std::get<2>(entry);		
				for (int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++) {
					db_pat_thread_shared_results[i] = lot.test_config.configs[i];
				}
				// copy for database
				db_pat_thread_shared_setup= lot.pat_config;

				long part_num = std::get<0>(entry);

				if (db_thread.joinable())
					db_thread.join();
				db_thread_running = true;	// flag that sign the saving thread is running
				db_thread = std::thread(db_thread_function_extPAT, lot_nr, part_num, std::get<1>(entry), pat_startpart);
			}
		}

		return _handler_status;
	}

	eHandlerStatus DBHandler::PatTable_Insert(long long lot_nr, unsigned int startpartnum, PATConfig pat_config, ValueConfig valueconfig[])
	{
		if (!_pat_table.Insert(lot_nr, startpartnum, pat_config, valueconfig))
		{
			_last_error = "Can not insert pat '" + std::to_string(lot_nr) + ".";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		return _handler_status;
	}

	eHandlerStatus DBHandler::PatTable_GetLast(long long lot, lot::LOT_Data& param, PATConfig& ini_pat_config)
	{
		if (!_pat_table.GetLast(lot, param, ini_pat_config))
		{
			_last_error = "Can not read pat '" + std::to_string(lot) + ".";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		return _handler_status;
	}

	eHandlerStatus DBHandler::PatTable_Copy_From_LastsameType(lot::LOT_Data& param, PATConfig& ini_pat_config, bool &isloaded)
	{
		if (!_pat_table.Copy_From_LastsameType(param, ini_pat_config, isloaded))
		{
			_last_error = "Can not read last pat for '" + param.header.test_plan + ".";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		return _handler_status;
	}

	// writes part data into the value 
	eHandlerStatus DBHandler::ValueTable_EmptyBuffer(long long lot_nr, ResultBuffer& buffer)
	{
		int error = DB_ERROR_NONE;
		while (buffer.GetNumEntries() > 0)
		{
			auto entry = buffer.GetAndRemoveFirstPart();
			long part_num = std::get<0>(entry);
			if (_value_table.PartExists(lot_nr, part_num))
			{
				error = DB_ERROR_PART_EXISTS;
				break;
			}
			else if (!_value_table.Insert(lot_nr, part_num, std::get<1>(entry), std::get<2>(entry)))
			{
				error = DB_ERROR_INSERT;
				break;
			}
		}

		if (error == DB_ERROR_INSERT)
		{
			_last_error = "Insert into value table failed: Insert not possible!";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		else if (error == DB_ERROR_INSERT)
		{
			_last_error = "Insert into value table failed: Part exists!";
			_handler_status = eHandlerStatus::H_ERROR;
		}

		return _handler_status;
	}

	// this buffer function is not in use
	eHandlerStatus DBHandler::BufferTable_WriteBuffer(ResultBuffer& buffer)
	{
		logger::WriteLog("Write buffer to db");
		_buffer_table.DeleteTableContent();
		Sleep(10);
		while (buffer.GetNumEntries() > 0)
		{
			auto entry = buffer.GetAndRemoveFirstPart();
			long part_num = std::get<0>(entry);
			if (_buffer_table.PartExists(part_num))
			{
				_last_error = "Part number " + std::to_string(part_num) + " allready exists in buffer table.";
				_handler_status = eHandlerStatus::H_ERROR;
				break;
			}
			else if (!_buffer_table.Insert(part_num, std::get<1>(entry), std::get<2>(entry)))
			{
				_last_error = "Can not insert part " + std::to_string(part_num) + " into buffer table.";
				_handler_status = eHandlerStatus::H_ERROR;
				break;
			}

		}

		return _handler_status;
	}

	// this buffer function is not in use
	eHandlerStatus DBHandler::BufferTable_ReadBuffer(ResultBuffer& buffer)
	{
		if (!_buffer_table.GetBuffer(buffer))
		{
			_last_error = "Can not read buffer from buffer table.";
			_handler_status = eHandlerStatus::H_ERROR;
		}
		return _handler_status;
	}

	eHandlerStatus DBHandler::ConfigTable_Insert(DeviceConfig& dev_config)
	{
		for (auto meas_config : dev_config.measurements)
		{
			if (!_config_table.Insert(meas_config, dev_config.GetDeviceName(), dev_config.GetStationNum()))
			{
				_last_error = "Can not insert into config table.";
				_handler_status = eHandlerStatus::H_ERROR;
				break;
			}
		}
		return _handler_status;
	}

}