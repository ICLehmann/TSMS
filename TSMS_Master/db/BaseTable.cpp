#include "BaseTable.h"
#include <cassert>
#include <sstream>

#define TEXT_NONE	"none"
#define TEXT_LOW	"low"
#define TEXT_HIGH	"high"
#define TEXT_PASS	"pass"
#define TEXT_FAIL	"fail"
		

BaseTable::BaseTable()
{
	_pSQL = nullptr;
}

void BaseTable::Setup(SQLConnection* pSQL, std::string tableName)
{
	assert(pSQL);
	_pSQL = pSQL;
	_tableName = tableName;
}

std::pair<bool, SQLHANDLE> BaseTable::ExecuteSQL(std::string statement)
{
	if (!_pSQL)
		throw std::runtime_error("Null pointer exception!\n");
	return _pSQL->ExecSQL(statement);
}

void BaseTable::DropTable()
{
	std::string statement = "DROP TABLE IF EXISTS " + _tableName;
	ExecuteSQL(statement);
}

void BaseTable::DeleteTableContent()
{
	std::string statement = "DELETE FROM " + _tableName;
	ExecuteSQL(statement);
}

std::string BaseTable::buffer_to_string(char* buffer)
{
	std::string data(buffer);
	std::string::size_type last = data.find_last_not_of(' ') + 1;
	return std::move(data.substr(0, last));
}

std::string BaseTable::to_string_with_precision(double value, const int n)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << value;
	return std::move(out).str();
}

std::string BaseTable::bool_to_string(bool input)
{
	return input ? "1" : "0";
}

std::string BaseTable::result_to_string(Evaluation res)
{
	std::string result_str;
	switch (res)
	{
	case Evaluation::NONE:
		result_str = TEXT_NONE;
		break;
	case Evaluation::LOW:
		result_str = TEXT_LOW;
		break;
	case Evaluation::HIGH:
		result_str = TEXT_HIGH;
		break;
	case Evaluation::PASS:
		result_str = TEXT_PASS;
		break;
	case Evaluation::FAIL:
		result_str = TEXT_FAIL;
		break;
	default:
		break;
	}
	return std::move(result_str);
}

Evaluation BaseTable::string_to_result(std::string result)
{
	size_t found = result.find(TEXT_NONE);
	if (found != std::string::npos)
		return Evaluation::NONE;

	found = result.find(TEXT_LOW);
	if (found != std::string::npos)
		return Evaluation::LOW;

	found = result.find(TEXT_HIGH);
	if (found != std::string::npos)
		return Evaluation::HIGH;

	found = result.find(TEXT_PASS);
	if (found != std::string::npos)
		return Evaluation::PASS;

	found = result.find(TEXT_FAIL);
	if (found != std::string::npos)
		return Evaluation::FAIL;

	return Evaluation();
}

double BaseTable::prefix_to_scale(char prefix)
{
	double scale = 1.0;
	switch (prefix)
	{
	case 'T':
		scale = 1E-12;
		break;
	case 'G':
		scale = 1E-9;
		break;
	case 'M':
		scale = 1E-6;
		break;
	case 'k':
		scale = 1E-3;
		break;
	case 'm':
		scale = 1E3;
		break;
	case 'µ':
		scale = 1E6;
		break;
	case 'n':
		scale = 1E9;
		break;
	case 'p':
		scale = 1E12;
		break;
	default:
		break;
	}
	return scale;
}

std::string BaseTable::prefix_to_string(char prefix)
{
	if (prefix == 0)
		return std::string("");
	else
		return std::string(1, prefix);
}
