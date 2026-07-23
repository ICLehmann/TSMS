#include "ValueTable.h"
#include "../const.h"

#include <algorithm>  

bool ValueTable::Create()
{
	logger::WriteLog("Create new table '" + _tableName + "' ...");
	DropTable();
	std::string statement = "CREATE TABLE " + _tableName + "([LOT] BIGINT NOT NULL, [PART] INT NOT NULL, [LastCycleTime] INT, ";
	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		statement += "[" + tsms_config::MeasurementNames[i] + "] DECIMAL(38,12) NULL,";
		statement += "[" + tsms_config::MeasurementNames[i] + "_Result] CHAR(20),";
		statement += "[" + tsms_config::MeasurementNames[i] + "_Time] SMALLINT NULL,";
	}
	statement += "PRIMARY KEY CLUSTERED([LOT] ASC, [PART] ASC))";

	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
		return true;
	else
		return false;
}

bool ValueTable::Insert(long long LOTnumber, unsigned int PartNumber, int LastCycleTime, std::vector<MeasuredValue>& data)
{
	if (data.size() == 0)
		return true;		// insert no empty data sets

	std::string statement = "INSERT INTO " + _tableName + "([LOT], [PART], [LastCycleTime] ";
	for (auto& value : data)
	{
		statement += ",[" + tsms_config::MeasurementNames[value.GetID()] + "],["
			+ tsms_config::MeasurementNames[value.GetID()] + "_Result],["
			+ tsms_config::MeasurementNames[value.GetID()] + "_Time]";
	};
	statement += ") values(" + std::to_string(LOTnumber) + "," + std::to_string(PartNumber) + "," + std::to_string(LastCycleTime);
	for (auto& value : data)
	{
		statement += "," + to_string_with_precision(value.GetValue(), 12);
		statement += ",'" + result_to_string(value.eval);
		statement += "'," + std::to_string(value.time_ms);
	}
	statement += ")";

	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
		return true;
	else
		return false;
}

bool ValueTable::GetLastPartNum(long long LOTnumber, unsigned int& PartNumber)
{
	std::string statement = "SELECT PART FROM " + _tableName + " WHERE LOT = " + std::to_string(LOTnumber) + " ORDER BY PART DESC";
	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
	{
		if (SQLFetch(SQLStatementHandle) == SQL_SUCCESS)
		{
			SQLGetData(SQLStatementHandle, 1, SQL_C_LONG, &PartNumber, 0, NULL);
		}
		SQLCloseCursor(SQLStatementHandle);
	}
	return success;
}

bool ValueTable::PartExists(long long LOTnumber, unsigned int PartNumber)
{
	bool result = false;
	std::string statement = "SELECT 1 FROM " + _tableName + " WHERE LOT = " +
		std::to_string(LOTnumber) + " AND PART = " + std::to_string(PartNumber);
	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
	{
		if (SQLFetch(SQLStatementHandle) == SQL_SUCCESS)
		{
			result = true;
		}
		SQLCloseCursor(SQLStatementHandle);
	}
	return result;
}

long ValueTable::CountOneResult(long long LOTnumber, std::string measurement, std::string result)
{
	long counter = -1;
	std::string statement = "SELECT COUNT(" + measurement + "_Result) FROM " + _tableName + " WHERE LOT = "
		+ std::to_string(LOTnumber) + " AND " + measurement + "_Result = '" + result + "'";
	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
	{
		if (SQLFetch(SQLStatementHandle) == SQL_SUCCESS)
		{
			SQLGetData(SQLStatementHandle, 1, SQL_C_LONG, &counter, 0, NULL);
		}
		SQLCloseCursor(SQLStatementHandle);
	}
	return counter;
}

bool ValueTable::CountResults(long long LOTnumber, TSMSCounter& cnt)
{
	cnt.input_parts = 0;
	if (!GetLastPartNum(LOTnumber, cnt.input_parts))
		return false;

	std::vector<long> all_pass;
	
	long tmp_counter = -1;
	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		tmp_counter = CountOneResult(LOTnumber, tsms_config::MeasurementNames[i], result_to_string(Evaluation::PASS));
		if (tmp_counter > -1)
		{
			cnt.res_pass[i] = tmp_counter;
			all_pass.push_back(tmp_counter);
		}
		else
			return false;
		
		tmp_counter = CountOneResult(LOTnumber, tsms_config::MeasurementNames[i], result_to_string(Evaluation::LOW));
		if (tmp_counter > -1)
			cnt.res_low[i] = tmp_counter;
		else
			return false;

		tmp_counter = CountOneResult(LOTnumber, tsms_config::MeasurementNames[i], result_to_string(Evaluation::HIGH));
		if (tmp_counter > -1)
			cnt.res_high[i] = tmp_counter;
		else
			return false;

		tmp_counter = CountOneResult(LOTnumber, tsms_config::MeasurementNames[i], result_to_string(Evaluation::FAIL));
		if (tmp_counter > -1)
			cnt.res_fail[i] = tmp_counter;
		else
			return false;
	}

	std::sort(all_pass.begin(), all_pass.end());
	cnt.good_parts = all_pass[0];
	
	return true;
}


