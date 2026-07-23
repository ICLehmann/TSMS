#pragma once
#include <string>
#include <vector>

#undef UNICODE
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

class SQLConnection
{

public:
	SQLConnection();
	~SQLConnection();

	bool Connect(std::string connection_string);
	void Disconnect();

	std::pair<bool, SQLHANDLE> ExecSQL(std::string statement);
};

