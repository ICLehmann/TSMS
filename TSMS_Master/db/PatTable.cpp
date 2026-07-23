#include "PatTable.h"
#include "../const.h"
#include "../common/DateTime.h"

bool PatTable::Create()
{
	logger::WriteLog("Create new table '" + _tableName + "' ...");
	DropTable();
	std::string statement = "CREATE TABLE " + _tableName +
		"([LOT] BIGINT NOT NULL, [STARTPART] INT DEFAULT 0, [MODE] INT NOT NULL, [SAMPLECNT] INT NOT NULL, [LL_SIGMA] INT NOT NULL, [UL_SIGMA] INT NOT NULL,";

	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		statement += "[" + tsms_config::MeasurementNames[i] + "_Enabled] BIT NULL,";
		statement += "[" + tsms_config::MeasurementNames[i] + "_Min] DECIMAL(38,12) NULL,";
		statement += "[" + tsms_config::MeasurementNames[i] + "_Max] DECIMAL(38,12) NULL,";
	}

	statement += "PRIMARY KEY CLUSTERED([LOT] ASC, [STARTPART] ASC),";
	statement += "FOREIGN KEY([LOT]) REFERENCES [LOT]([LOT]) ON DELETE CASCADE)";

	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
		return true;
	else
		return false;
}

bool PatTable::Insert(long long lot_nr, unsigned int startpartnum, PATConfig pat_config, ValueConfig valueconfig[])
{
	std::string statement = "INSERT INTO " + _tableName
		+ "([LOT],[STARTPART],[MODE],[SAMPLECNT],[LL_SIGMA],[UL_SIGMA]";
	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		statement += ",[" + tsms_config::MeasurementNames[i] + "_Enabled]";
		statement += ",[" + tsms_config::MeasurementNames[i] + "_Min]";
		statement += ",[" + tsms_config::MeasurementNames[i] + "_Max]";
	};
	statement += ") values(" + std::to_string(lot_nr) + "," + std::to_string(startpartnum) + "," + std::to_string(pat_config.mode) + "," +
		std::to_string(pat_config.samplecnt) + ","  + std::to_string(pat_config.ll_sigma) + "," + std::to_string(pat_config.ul_sigma);
	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		statement += "," + bool_to_string(valueconfig[i].pat_enabled);
		statement += "," + to_string_with_precision(valueconfig[i].pat_min, 12);
		statement += "," + to_string_with_precision(valueconfig[i].pat_max, 12);
	}
	statement += ")";

	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
		return true;
	else
		return false;
}

// gets the last executed / valid PAT narrowing limits where the STARTPART number is the largest
bool PatTable::GetLast(long long lot, lot::LOT_Data& param, PATConfig& ini_pat_config)
{
	std::string statement = "SELECT TOP 1 * FROM " + _tableName + " WHERE LOT = " + std::to_string(lot) + " ORDER BY STARTPART DESC";
	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (!success)
		return false;

	unsigned int col = 3; // skipping the LOT and STARTPART fields
	if (SQLFetch(SQLStatementHandle) == SQL_SUCCESS)
	{
		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.pat_config.mode, 0, NULL);
		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.pat_config.samplecnt, 0, NULL);
		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.pat_config.ll_sigma, 0, NULL);
		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.pat_config.ul_sigma, 0, NULL);
		// if PAT narrowing has already executed once
		for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
		{
			SQLGetData(SQLStatementHandle, col++, SQL_C_BIT, &param.test_config.configs[i].pat_enabled, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &param.test_config.configs[i].pat_min, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &param.test_config.configs[i].pat_max, 0, NULL);
		};
	}
	else
	{
		// if PAT narrowing has not executed yet
		param.pat_config = ini_pat_config; // sets config from ini file if thre is not available setup
		param.test_config.ResetPAT();
	}

	SQLCloseCursor(SQLStatementHandle);

	return success;
}

// gets the last executed / valid PAT narrowing limits where the STARTPART number is the largest
bool PatTable::Copy_From_LastsameType(lot::LOT_Data& param, PATConfig& ini_pat_config, bool &isloaded)
{
	std::string statement = "SELECT TOP 1 " + _tableName + ".* FROM " + tsms_config::db_TableNameLot + 
		" INNER JOIN " + _tableName + " ON (" + tsms_config::db_TableNameLot + ".[LOT] = " + _tableName + ".[LOT])" +
		"WHERE " + tsms_config::db_TableNameLot + ".[ProductNum] = '" + param.header.test_plan + "' AND " +
		"[Status]='FINISHED' AND " +
		tsms_config::db_TableNameLot + ".[LotStart] > '" + GetCurrentDateTime_Minus24h() + "' " +
		"ORDER BY " + tsms_config::db_TableNameLot + ".[LotStart] DESC, " + _tableName + ".[STARTPART] DESC";

	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (!success)
		return false;

	unsigned int col = 3; // skipping the LOT and STARTPART fields
	if (SQLFetch(SQLStatementHandle) == SQL_SUCCESS)
	{
		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.pat_config.mode, 0, NULL);
		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.pat_config.samplecnt, 0, NULL);
		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.pat_config.ll_sigma, 0, NULL);
		SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &param.pat_config.ul_sigma, 0, NULL);
		// if PAT narrowing has already executed once
		for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
		{
			SQLGetData(SQLStatementHandle, col++, SQL_C_BIT, &param.test_config.configs[i].pat_enabled, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &param.test_config.configs[i].pat_min, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &param.test_config.configs[i].pat_max, 0, NULL);
		};
		isloaded = true;
	}

	SQLCloseCursor(SQLStatementHandle);

	return success;
}