#include "LotTable.h"
#include "../const.h"

bool LotTable::Create()
{
	logger::WriteLog("Create new table '" + _tableName + "' ...");
	DropTable();
	std::string statement = "CREATE TABLE " + _tableName +
		"([LOT] BIGINT NOT NULL, [ProductNum] CHAR(20) NOT NULL, [Line] CHAR(20), [Operator] CHAR(20)," +
		"[TestMachine] CHAR(20), [Status] CHAR(20), [LaserMark] CHAR(20), [LotStart] DATETIME, [LotStop] DATETIME, " +
		"[LabelCounter] INT DEFAULT 0, [TotalParts] INT DEFAULT 0, [TotalGoods] INT DEFAULT 0,";

	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		statement += "[" + tsms_config::MeasurementNames[i] + "_Min] DECIMAL(38,12) NULL,";
		statement += "[" + tsms_config::MeasurementNames[i] + "_Nom] DECIMAL(38,12) NULL,";
		statement += "[" + tsms_config::MeasurementNames[i] + "_Max] DECIMAL(38,12) NULL,";
		statement += "[" + tsms_config::MeasurementNames[i] + "_Offset] DECIMAL(38,12) NULL,";
		statement += "[" + tsms_config::MeasurementNames[i] + "_Prefix] CHAR(1),";
		statement += "[" + tsms_config::MeasurementNames[i] + "_FailRate] DECIMAL(38,12) NULL,";
		statement += "[" + tsms_config::MeasurementNames[i] + "_Frequency] BIGINT NULL,";
		statement += "[" + tsms_config::MeasurementNames[i] + "_StimuLevel] DECIMAL(38,12) NULL,";
	}

	// add  colums for tsms counter
	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		statement += "[" + tsms_config::MeasurementNames[i] + "_Pass] INT DEFAULT 0,";
		statement += "[" + tsms_config::MeasurementNames[i] + "_Low] INT DEFAULT 0,";
		statement += "[" + tsms_config::MeasurementNames[i] + "_High] INT DEFAULT 0,";
		statement += "[" + tsms_config::MeasurementNames[i] + "_Fail] INT DEFAULT 0,";
	}

	// add  colums for machine counter
	statement += "[MachineCntTotal] INT DEFAULT 0, [MachineCntGood] INT DEFAULT 0,";
	statement += "[MachineCntGoodBin] INT DEFAULT 0, [MachineCntLastBin] INT DEFAULT 0,";
	for (unsigned int i = 0; i < tsms_config::NUM_COUNTER_TRB; i++)
		statement += "[" + tsms_config::CounterTRB_Names[i] + "] INT DEFAULT 0,";

	for (unsigned int i = 0; i < tsms_config::NUM_COUNTER_VRB; i++)
		statement += "[" + tsms_config::CounterVRB_Names[i] + "] INT DEFAULT 0,";

	//statement += "PRIMARY KEY ([LOT]), INDEX ([LotStart]))";
	statement += "PRIMARY KEY ([LOT]), INDEX ([LotStart]), INDEX IX_lotstart ([LotStart]))";
	
	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
		return true;
	else
		return false;
}

bool LotTable::LotExists(long long lot, bool& res)
{
	std::string statement = "SELECT 1 FROM " + _tableName + " WHERE LOT = " + std::to_string(lot);

	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (!success)
		return false;

	res = false;
	if (SQLFetch(SQLStatementHandle) == SQL_SUCCESS)
	{
		res = true;
	}
	SQLCloseCursor(SQLStatementHandle);
	return true;
}

bool LotTable::Insert(lot::LOT_Data& lot)
{
	std::string statement = "INSERT INTO " + _tableName
		+ "([LOT], [ProductNum], [Line], [Operator], [TestMachine], [Status], [LaserMark], [LotStart]";
	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		statement += ",[" + tsms_config::MeasurementNames[i] + "_Min]";
		statement += ",[" + tsms_config::MeasurementNames[i] + "_Nom]";
		statement += ",[" + tsms_config::MeasurementNames[i] + "_Max]";
		statement += ",[" + tsms_config::MeasurementNames[i] + "_Offset]";
		statement += ",[" + tsms_config::MeasurementNames[i] + "_Prefix]";
		statement += ",[" + tsms_config::MeasurementNames[i] + "_FailRate]";
		statement += ",[" + tsms_config::MeasurementNames[i] + "_Frequency]";
		statement += ",[" + tsms_config::MeasurementNames[i] + "_StimuLevel]";
	};
	statement += ") values(" + std::to_string(lot.header.lot_nr) + ",'" + lot.header.test_plan + "','" + lot.header.line_id;
	statement += "','" + lot.header.operator_id + "','" + lot.testing_machine + "','" + lot::GetStateAsString(lot.lot_state) + "','" + lot.laser_mark;
	statement += "','" + lot.lot_start + "'";
	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		statement += "," + to_string_with_precision(lot.test_config.configs[i].min, 12);
		statement += "," + to_string_with_precision(lot.test_config.configs[i].nom, 12);
		statement += "," + to_string_with_precision(lot.test_config.configs[i].max, 12);
		statement += "," + to_string_with_precision(lot.test_config.configs[i].offset, 12);
		statement += ",'" + prefix_to_string(lot.test_config.configs[i].prefix) + "'";
		statement += "," + to_string_with_precision(lot.test_config.configs[i].error_rate, 12);
		statement += "," + std::to_string(lot.test_config.configs[i].frequency);
		statement += "," + to_string_with_precision(lot.test_config.configs[i].stimuli_level, 12);
	}
	statement += ")";
	
	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
		return true;
	else
		return false;
}

bool LotTable::Update(lot::LOT_Data& lot)
{
	std::string statement = "UPDATE " + _tableName + " SET";
	statement += " [Status] = '" + lot::GetStateAsString(lot.lot_state) + "'";
	statement += ", [LotStop] = '" + lot.lot_stop + "'";
	statement += ", [TotalParts] = " + std::to_string(lot.tsms_cnt.input_parts);
	statement += ", [TotalGoods] = " + std::to_string(lot.tsms_cnt.good_parts);

	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		statement += ", [" + tsms_config::MeasurementNames[i] + "_Pass] = " + std::to_string(lot.tsms_cnt.res_pass[i]);
		statement += ", [" + tsms_config::MeasurementNames[i] + "_Low] = " + std::to_string(lot.tsms_cnt.res_low[i]);
		statement += ", [" + tsms_config::MeasurementNames[i] + "_High] = " + std::to_string(lot.tsms_cnt.res_high[i]);
		statement += ", [" + tsms_config::MeasurementNames[i] + "_Fail] = " + std::to_string(lot.tsms_cnt.res_fail[i]);
	}

	statement += ", [MachineCntTotal] = " + std::to_string(lot.machine_cnt.total_input);
	statement += ", [MachineCntGood] = " + std::to_string(lot.machine_cnt.total_good);
	statement += ", [MachineCntGoodBin] = " + std::to_string(lot.machine_cnt.good_bin);
	statement += ", [MachineCntLastBin] = " + std::to_string(lot.machine_cnt.last_bin);

	for (unsigned int i = 0; i < tsms_config::NUM_COUNTER_TRB; i++)
		statement += ", [" + tsms_config::CounterTRB_Names[i] + "] = " + std::to_string(lot.machine_cnt.TRB[i]);

	for (unsigned int i = 0; i < tsms_config::NUM_COUNTER_VRB; i++)
		statement += ", [" + tsms_config::CounterVRB_Names[i] + "] = " + std::to_string(lot.machine_cnt.TRV[i]);

	statement += " WHERE [LOT] = " + std::to_string(lot.header.lot_nr);
	
	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
		return true;
	else
		return false;
}

bool LotTable::UpdateStatus(long long lot_nr, std::string status)
{
	std::string statement = "UPDATE " + _tableName + " SET";
	statement += " [Status] = '" + status + "'";
	statement += " WHERE [LOT] = " + std::to_string(lot_nr);
	
	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
		return true;
	else
		return false;
}

bool LotTable::UpdateLabelCounter(long long lot_nr, unsigned int label_cnt)
{
	std::string statement = "UPDATE " + _tableName + " SET";
	statement += " [LabelCounter] = " + std::to_string(label_cnt);
	statement += " WHERE [LOT] = " + std::to_string(lot_nr);

	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
		return true;
	else
		return false;
}

lot::eLotState LotTable::LotStatusFromString(std::string status)
{
	for (int i = 0; i < lot::NUM_STATES; i++)
	{
		if (lot::LotStateNames[i] == status)
			return lot::eLotState(i);
	}
	return lot::eLotState::UNKNOWN;
}

bool LotTable::GetLotStatus(long long lot_nr, lot::eLotState& lot_status)
{
	std::string status_str;
	char buffer[MAX_CHAR_TO_READ];

	std::string statement = "SELECT STATUS FROM " + _tableName + " WHERE LOT = " + std::to_string(lot_nr);
	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
	{
		if (SQLFetch(SQLStatementHandle) == SQL_SUCCESS)
		{
			SQLGetData(SQLStatementHandle, 1, SQL_C_CHAR, &buffer, MAX_CHAR_TO_READ, NULL);
			status_str = buffer_to_string(buffer);
			lot_status = LotStatusFromString(status_str);
		}
		else
			success = false;

		SQLCloseCursor(SQLStatementHandle);	
	}
	return success;
}

bool LotTable::DeleteLot(long long lot_nr)
{
	bool lot_exits = false;
	if (!LotExists(lot_nr, lot_exits))
		return false;

	if (lot_exits)
	{
		std::string statement = "DELETE FROM " + _tableName + " WHERE LOT = " + std::to_string(lot_nr);
		auto [success, SQLStatementHandle] = ExecuteSQL(statement);
		if (!success)
			return false;
	}
	return true;
}

bool LotTable::GetLot(long long lot, lot::LOT_Data& param)
{
	char buffer[MAX_CHAR_TO_READ];

	std::string statement = "SELECT * FROM " + _tableName + " WHERE LOT = " + std::to_string(lot);
	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (!success)
		return false;

	bool result = false;
	unsigned int col = 1;
	if (SQLFetch(SQLStatementHandle) == SQL_SUCCESS)
	{
		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.header.lot_nr, 0, NULL);

		buffer[0] = 0;
		SQLGetData(SQLStatementHandle, col++, SQL_C_CHAR, &buffer, MAX_CHAR_TO_READ, NULL);
		param.header.test_plan = buffer_to_string(buffer);

		buffer[0] = 0;
		SQLGetData(SQLStatementHandle, col++, SQL_C_CHAR, &buffer, MAX_CHAR_TO_READ, NULL);
		param.header.line_id = buffer_to_string(buffer);

		buffer[0] = 0;
		SQLGetData(SQLStatementHandle, col++, SQL_C_CHAR, &buffer, MAX_CHAR_TO_READ, NULL);
		param.header.operator_id = buffer_to_string(buffer);

		buffer[0] = 0;
		SQLGetData(SQLStatementHandle, col++, SQL_C_CHAR, &buffer, MAX_CHAR_TO_READ, NULL);
		param.testing_machine = buffer_to_string(buffer);

		buffer[0] = 0;
		SQLGetData(SQLStatementHandle, col++, SQL_C_CHAR, &buffer, MAX_CHAR_TO_READ, NULL);
		param.lot_state = LotStatusFromString(buffer_to_string(buffer));

		buffer[0] = 0;
		SQLGetData(SQLStatementHandle, col++, SQL_C_CHAR, &buffer, MAX_CHAR_TO_READ, NULL);
		param.laser_mark = buffer_to_string(buffer);

		buffer[0] = 0;
		SQLGetData(SQLStatementHandle, col++, SQL_C_CHAR, &buffer, MAX_CHAR_TO_READ, NULL);
		param.lot_start = buffer_to_string(buffer);

		buffer[0] = 0;
		SQLGetData(SQLStatementHandle, col++, SQL_C_CHAR, &buffer, MAX_CHAR_TO_READ, NULL);
		param.lot_stop = buffer_to_string(buffer);

		int label_counter;	// we dont need it in tsms master 
		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &label_counter, 0, NULL);
		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.tsms_cnt.input_parts, 0, NULL);
		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.tsms_cnt.good_parts, 0, NULL);

		for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
		{
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &param.test_config.configs[i].min, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &param.test_config.configs[i].nom, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &param.test_config.configs[i].max, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &param.test_config.configs[i].offset, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_CHAR, &param.test_config.configs[i].prefix, 2, NULL);
			param.test_config.configs[i].scale = prefix_to_scale(param.test_config.configs[i].prefix);
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &param.test_config.configs[i].error_rate, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.test_config.configs[i].frequency, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &param.test_config.configs[i].stimuli_level, 0, NULL);
		};

		for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
		{
			SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.tsms_cnt.res_pass[i], 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.tsms_cnt.res_low[i], 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.tsms_cnt.res_high[i], 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.tsms_cnt.res_fail[i], 0, NULL);
		}

		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.machine_cnt.total_input, 0, NULL);
		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.machine_cnt.total_good, 0, NULL);
		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.machine_cnt.good_bin, 0, NULL);
		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.machine_cnt.last_bin, 0, NULL);

		for (unsigned int i = 0; i < tsms_config::NUM_COUNTER_TRB; i++)
			SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.machine_cnt.TRB[i], 0, NULL);

		for (unsigned int i = 0; i < tsms_config::NUM_COUNTER_VRB; i++)
			SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.machine_cnt.TRV[i], 0, NULL);

		result = true;
	}

	SQLCloseCursor(SQLStatementHandle);

	return result;
}

bool LotTable::GetLastStartedLot(long long& last_started_lot)
{
	last_started_lot = 0;
	std::string statement = "SELECT LOT FROM " + _tableName + " WHERE STATUS = '" + lot::GetStateAsString(lot::eLotState::STARTED) 
		+ "' AND ProductNum != 'Compensation' AND ProductNum != 'E-Dummy'ORDER BY LotStart DESC";
	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
	{
		if (SQLFetch(SQLStatementHandle) == SQL_SUCCESS)
		{
			SQLGetData(SQLStatementHandle, 1, SQL_C_LONG, &last_started_lot, 0, NULL);
		}		
		SQLCloseCursor(SQLStatementHandle);
	}
	return success;
}

bool LotTable::GetLastInternalLotNumber(long long& last_dummy_lot)
{
	last_dummy_lot = last_dummy_lot = tsms_config::FristInternalLotNumber;;
	std::string statement = "SELECT LOT FROM " + _tableName + " WHERE ProductNum = '" + tsms_config::DummyTestDataSet + 
		"' OR ProductNum = '" + tsms_config::CompensationTestplan + "' ORDER BY LOT DESC";
	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
	{
		if (SQLFetch(SQLStatementHandle) == SQL_SUCCESS)
		{
			SQLGetData(SQLStatementHandle, 1, SQL_C_LONG, &last_dummy_lot, 0, NULL);
		}
		SQLCloseCursor(SQLStatementHandle);
	}
	return success;
}

bool LotTable::GetLastTime(std::string product_num, std::string& time_str)
{
	//SELECT LotStart, STATUS FROM[dbo].[LOT] WHERE ProductNum = 'E-Dummy' ORDER BY LotStart DESC

	std::string statement = "SELECT LotStart, STATUS FROM " + _tableName + " WHERE ProductNum = '" + product_num + "' ORDER BY LotStart DESC";
	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
	{
		lot::eLotState lot_state = lot::eLotState::UNKNOWN;

		if (SQLFetch(SQLStatementHandle) == SQL_SUCCESS)
		{
			char buffer[MAX_CHAR_TO_READ];
			buffer[0] = 0;
			unsigned int col = 1;
			SQLGetData(SQLStatementHandle, col++, SQL_C_CHAR, &buffer, MAX_CHAR_TO_READ, NULL);
			time_str = buffer_to_string(buffer);

			buffer[0] = 0;
			SQLGetData(SQLStatementHandle, col++, SQL_C_CHAR, &buffer, MAX_CHAR_TO_READ, NULL);
			lot_state = LotStatusFromString(buffer_to_string(buffer));
		}
		else 
			time_str = "";
		SQLCloseCursor(SQLStatementHandle);


		if (lot_state != lot::eLotState::FINISHED)
			time_str = "2000-01-01 00:00:00";		// give a unvalid date back
	}
	return success;
}
