#pragma once
#include "SQLConnection.h"
#include "../const.h"
#include "../common/Logger.h"
#include "../MeasuredValue.h"
#include <string>

#define MAX_CHAR_TO_READ 21	// should be one more like on creation

class BaseTable
{
private:
	SQLConnection* _pSQL;
protected:
	
	std::pair<bool, SQLHANDLE> ExecuteSQL(std::string statement);

	std::string _tableName;
	std::string buffer_to_string(char* buffer);
	std::string to_string_with_precision(double value, const int n = 12);
	std::string bool_to_string(bool input);
	std::string result_to_string(Evaluation res);
	Evaluation string_to_result(std::string result);

	double prefix_to_scale(char prefix);
	std::string prefix_to_string(char prefix);

public:
	BaseTable();
	void Setup(SQLConnection* pSQL, std::string tableName);
	void DropTable();
	void DeleteTableContent();
};

