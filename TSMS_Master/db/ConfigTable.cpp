#include "../const.h"
#include "ConfigTable.h"

bool ConfigTable::Create()
{
	logger::WriteLog("Create new table '" + _tableName + "' ...");
	DropTable();
	std::string statement = "CREATE TABLE " + _tableName + "([ID] INT NOT NULL, [NAME] CHAR(20), [UNIT] CHAR(20), [DEVICE] CHAR(20), [STATION] INT PRIMARY KEY CLUSTERED([ID] ASC))";

	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
		return true;
	else
		return false;
}

bool ConfigTable::Insert(MeasurementValueConfig& config, std::string device, int station)
{
	std::string statement = "INSERT INTO " + _tableName + "([ID], [NAME], [UNIT], [DEVICE], [STATION]) VALUES (" ;
	statement += std::to_string(config.id) + ",'" + config.name + "','" + config.unit + "','" + device + "'," +  std::to_string(station) + ")";

	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
		return true;
	else
		return false;
}
