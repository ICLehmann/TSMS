#pragma once
#include "BaseTable.h"
#include "../TestConfig.h"

class TestTable : public BaseTable
{
private:
	bool InsertConfig(std::string name, TestConfig& data);
	void InsertSampleData();

public:
	bool Create();
	bool GetConfig(std::string name, TestConfig& data);
};

