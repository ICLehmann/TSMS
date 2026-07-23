#include "BufferTable.h"
#include "../const.h"

bool BufferTable::Create()
{
	// todo: add [LastCycleTime]

	logger::WriteLog("Create new table '" + _tableName + "' ...");
	DropTable();
	std::string statement = "CREATE TABLE " + _tableName + "([PART] INT NOT NULL,";
	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		statement += "[" + tsms_config::MeasurementNames[i] + "] DECIMAL(38,12) NULL,";
		statement += "[" + tsms_config::MeasurementNames[i] + "_Result] CHAR(5),";
		statement += "[" + tsms_config::MeasurementNames[i] + "_Time] SMALLINT NULL,";
	}
	statement += "PRIMARY KEY CLUSTERED([PART] ASC))";

	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
		return true;
	else
		return false;
}

bool BufferTable::Insert(long long PartNumber, int last_cycle_time, std::vector<MeasuredValue>& data)
{
	// todo: write time

	std::string statement = "INSERT INTO " + _tableName + "([PART]";
	for (auto& value : data)
	{
		statement += ",[" + tsms_config::MeasurementNames[value.GetID()] + "],["
			+ tsms_config::MeasurementNames[value.GetID()] + "_Result],["
			+ tsms_config::MeasurementNames[value.GetID()] + "_Time]";
	};
	statement += ") values(" + std::to_string(PartNumber);
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


bool BufferTable::PartExists(long long PartNumber)
{
	bool result = false;
	std::string statement = "SELECT 1 FROM " + _tableName + " WHERE PART = " + std::to_string(PartNumber);
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

bool BufferTable::GetBuffer(ResultBuffer& result_buffer)
{
	result_buffer.ResetParts();

	std::string statement = "SELECT * FROM " + _tableName;
	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (!success)
		return false;

	char buffer[MAX_CHAR_TO_READ];

	SQLSMALLINT fieldCount = 0;
	SQLNumResultCols(SQLStatementHandle, &fieldCount);
	if (fieldCount > 0)
	{
		RETCODE rc = SQLFetch(SQLStatementHandle);
		while (SQL_SUCCEEDED(rc))
		{
			long part_num = 0;
			double value = 0;
			std::string result_str;
			unsigned int read_time = 0;

			unsigned int col = 1;

			rc = SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &part_num, 0, NULL);
			if (SQL_SUCCEEDED(rc) == FALSE)
				break;

			std::vector<MeasuredValue> values;
			for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
			{
				rc = SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &value, 0, NULL);
				if (SQL_SUCCEEDED(rc) == FALSE)
					break;

				buffer[0] = 0;
				rc = SQLGetData(SQLStatementHandle, col++, SQL_C_CHAR, &buffer, MAX_CHAR_TO_READ, NULL);
				if (SQL_SUCCEEDED(rc) == FALSE)
					break;
				result_str = buffer_to_string(buffer);

				SQLGetData(SQLStatementHandle, col++, SQL_INTEGER, &read_time, 0, NULL);
				if (SQL_SUCCEEDED(rc) == FALSE)
					break;

				MeasuredValue mv(i, 0);
				mv.SetValue(value);
				mv.eval = string_to_result(result_str);
				mv.time_ms = read_time;
				values.push_back(mv);
			}

			result_buffer.PushBack(part_num, 0, values);		//todo: read time back

			rc = SQLFetch(SQLStatementHandle);
		};
	}

	SQLCloseCursor(SQLStatementHandle);

	return true;
}

