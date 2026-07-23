#include "TestTable.h"
#include "../const.h"

bool TestTable::Create()
{
	logger::WriteLog("Create new table '" + _tableName + "' ...");
	DropTable();
	std::string statement = "CREATE TABLE " + _tableName + "([Name] CHAR(20) NOT NULL, [LaserCode] CHAR(2),";

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

	statement += "PRIMARY KEY ([Name]))";

	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
	{
		InsertSampleData();
		return true;
	}
	else
		return false;
}

bool TestTable::InsertConfig(std::string name, TestConfig& data)
{
	std::string statement = "INSERT INTO " + _tableName + "([Name], [LaserCode]";

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
	statement += ") values('" + name + "','" + data.laser_code + "'";
	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		statement += "," + to_string_with_precision(data.configs[i].min, 12);
		statement += "," + to_string_with_precision(data.configs[i].nom, 12);
		statement += "," + to_string_with_precision(data.configs[i].max, 12);
		statement += "," + to_string_with_precision(data.configs[i].offset, 12);
		statement += ",'" + prefix_to_string(data.configs[i].prefix) + "'";
		statement += "," + to_string_with_precision(data.configs[i].error_rate, 12);
		statement += "," + std::to_string(data.configs[i].frequency);
		statement += "," + to_string_with_precision(data.configs[i].stimuli_level, 12);
	}
	statement += ")";

	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (success)
		return true;
	else
		return false;
}

bool TestTable::GetConfig(std::string name, TestConfig& data)
{
	std::string statement = "SELECT * FROM " + _tableName + " WHERE NAME = '" + name + "'";
	auto [success, SQLStatementHandle] = ExecuteSQL(statement);
	if (!success)
		return false;

	bool result = false;
	unsigned int col = 2; // start with second colum, we don't need the name
	if (SQLFetch(SQLStatementHandle) == SQL_SUCCESS)
	{
		char buffer[3] = { 0 };
		SQLGetData(SQLStatementHandle, col++, SQL_C_CHAR, &buffer, 3, NULL);
		data.laser_code = buffer_to_string(buffer);

		for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
		{
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &data.configs[i].min, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &data.configs[i].nom, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &data.configs[i].max, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &data.configs[i].offset, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_CHAR, &data.configs[i].prefix, 2, NULL);
			data.configs[i].scale = prefix_to_scale(data.configs[i].prefix);
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &data.configs[i].error_rate, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_LONG, &data.configs[i].frequency, 0, NULL);
			SQLGetData(SQLStatementHandle, col++, SQL_C_DOUBLE, &data.configs[i].stimuli_level, 0, NULL);
		};

		result = true;
	}
	SQLCloseCursor(SQLStatementHandle);

	return result;
}

#pragma region TestData

void GetTestData01(TestConfig& data)
{
	data.laser_code = "DH";

	// ISO
	data.configs[tsms_config::ISO].min = 11.0;
	data.configs[tsms_config::ISO].nom = 20.0;
	data.configs[tsms_config::ISO].max = 10000.0;
	data.configs[tsms_config::ISO].prefix = 'M';
	data.configs[tsms_config::ISO].offset = 0;
	data.configs[tsms_config::ISO].stimuli_level = 80;
	data.configs[tsms_config::ISO].type = tsms_config::ISO;

	// L
	data.configs[tsms_config::L].min = 70.0;
	data.configs[tsms_config::L].nom = 100.0;
	data.configs[tsms_config::L].max = 150.0;
	data.configs[tsms_config::L].prefix = 'µ';
	data.configs[tsms_config::L].offset = 0;
	data.configs[tsms_config::L].type = 1;

	// Z1
	data.configs[tsms_config::Z1].min = 5000.0;
	data.configs[tsms_config::Z1].nom = 9000.0;
	data.configs[tsms_config::Z1].max = 15000.0;
	data.configs[tsms_config::Z1].prefix = ' ';
	data.configs[tsms_config::Z1].offset = 0;
	data.configs[tsms_config::Z1].frequency = 100000;
	data.configs[tsms_config::Z1].type = 2;

	// Z2
	data.configs[tsms_config::Z2].min = 5000.0;
	data.configs[tsms_config::Z2].nom = 9000.0;
	data.configs[tsms_config::Z2].max = 15000.0;
	data.configs[tsms_config::Z2].prefix = ' ';
	data.configs[tsms_config::Z2].offset = 0;
	data.configs[tsms_config::Z2].frequency = 10000000;
	data.configs[tsms_config::Z2].type = 3;
	
	// Z3
	data.configs[tsms_config::Z3].min = 5000.0;
	data.configs[tsms_config::Z3].nom = 9000.0;
	data.configs[tsms_config::Z3].max = 15000.0;
	data.configs[tsms_config::Z3].prefix = ' ';
	data.configs[tsms_config::Z3].offset = 0;
	data.configs[tsms_config::Z3].frequency = 20000000;
	data.configs[tsms_config::Z3].type = 4;
	
	// C1
	data.configs[tsms_config::C1].min = 5.0;
	data.configs[tsms_config::C1].nom = 20.0;
	data.configs[tsms_config::C1].max = 50.0;
	data.configs[tsms_config::C1].prefix = 'p';
	data.configs[tsms_config::C1].offset = 0;
	data.configs[tsms_config::C1].frequency = 1000000;
	data.configs[tsms_config::C1].type = tsms_config::C1;

	// C2
	data.configs[tsms_config::C2].min = 5.0;
	data.configs[tsms_config::C2].nom = 20.0;
	data.configs[tsms_config::C2].max = 50.0;
	data.configs[tsms_config::C2].prefix = 'p';
	data.configs[tsms_config::C2].offset = 0;
	data.configs[tsms_config::C2].frequency = 10000000;
	data.configs[tsms_config::C2].type = tsms_config::C2;

	// C3
	data.configs[tsms_config::C3].min = 5.0;
	data.configs[tsms_config::C3].nom = 20.0;
	data.configs[tsms_config::C3].max = 50.0;
	data.configs[tsms_config::C3].prefix = 'p';
	data.configs[tsms_config::C3].offset = 0;
	data.configs[tsms_config::C3].frequency = 20000000;
	data.configs[tsms_config::C3].type = tsms_config::C3;

	// R1
	data.configs[tsms_config::R1].min = 0.5;
	data.configs[tsms_config::R1].nom = 0.9;
	data.configs[tsms_config::R1].max = 1.5;
	data.configs[tsms_config::R1].prefix = ' ';
	data.configs[tsms_config::R1].offset = 0;
	data.configs[tsms_config::R1].type = 6;

	// R2
	data.configs[tsms_config::R2].min = 0.5;
	data.configs[tsms_config::R2].nom = 0.9;
	data.configs[tsms_config::R2].max = 1.5;
	data.configs[tsms_config::R2].prefix = ' ';
	data.configs[tsms_config::R2].offset = 0;
	data.configs[tsms_config::R2].type = 7;
}

void GetTestData02(TestConfig& data)
{
	data.laser_code = "AH";
	
	//ISO
	data.configs[tsms_config::ISO].min = 11.0;
	data.configs[tsms_config::ISO].nom = 0.0;
	data.configs[tsms_config::ISO].max = .0;
	data.configs[tsms_config::ISO].prefix = 'M';
	data.configs[tsms_config::ISO].offset = 0;
	data.configs[tsms_config::ISO].stimuli_level = 80;
	data.configs[tsms_config::ISO].type = tsms_config::ISO;

	// L
	data.configs[tsms_config::L].min = 7770.0;
	data.configs[tsms_config::L].nom = 11100.0;
	data.configs[tsms_config::L].max = 150.0;
	data.configs[tsms_config::L].scale = 1E+06;
	data.configs[tsms_config::L].prefix = 'µ';
	data.configs[tsms_config::L].offset = 0;
	data.configs[tsms_config::L].type = 1;

	// Z1
	data.configs[tsms_config::Z1].min = 5000.0;
	data.configs[tsms_config::Z1].nom = 9000.0;
	data.configs[tsms_config::Z1].max = 15000.0;
	data.configs[tsms_config::Z1].prefix = ' ';
	data.configs[tsms_config::Z1].offset = 0;
	data.configs[tsms_config::Z1].frequency = 100000;
	data.configs[tsms_config::Z1].type = 2;

	// Z2
	data.configs[tsms_config::Z2].min = 5000.0;
	data.configs[tsms_config::Z2].nom = 9000.0;
	data.configs[tsms_config::Z2].max = 15000.0;
	data.configs[tsms_config::Z2].prefix = ' ';
	data.configs[tsms_config::Z2].offset = 0;
	data.configs[tsms_config::Z2].frequency = 10000000;
	data.configs[tsms_config::Z2].type = 3;

	// Z3
	data.configs[tsms_config::Z3].min = 5000.0;
	data.configs[tsms_config::Z3].nom = 9000.0;
	data.configs[tsms_config::Z3].max = 15000.0;
	data.configs[tsms_config::Z3].prefix = ' ';
	data.configs[tsms_config::Z3].offset = 0;
	data.configs[tsms_config::Z3].frequency = 20000000;
	data.configs[tsms_config::Z3].type = 4;

	// C
	data.configs[tsms_config::C1].min = 30.0;
	data.configs[tsms_config::C1].nom = 40.0;
	data.configs[tsms_config::C1].max = 50.0;
	data.configs[tsms_config::C1].prefix = 'p';
	data.configs[tsms_config::C1].offset = 0;
	data.configs[tsms_config::C1].type = 5;
				 
	// R1		 
	data.configs[tsms_config::R1].min = 0.34;
	data.configs[tsms_config::R1].nom = 0.46;
	data.configs[tsms_config::R1].max = 0.57;
	data.configs[tsms_config::R1].scale = 1.0;
	data.configs[tsms_config::R1].offset = 0;
	data.configs[tsms_config::R1].type = 6;
				 			  
	// R2		 			  
	data.configs[tsms_config::R2].min = 0.88;
	data.configs[tsms_config::R2].nom = 1.14;
	data.configs[tsms_config::R2].max = 1.42f;
	data.configs[tsms_config::R2].scale = 1.0;
	data.configs[tsms_config::R2].offset = 0;
	data.configs[tsms_config::R2].type = 7;
}


void GetDummyTest(TestConfig& data)
{
	data.laser_code = "";

	// ISO
	data.configs[tsms_config::ISO].min = 11.0;
	data.configs[tsms_config::ISO].nom = 40.0;
	data.configs[tsms_config::ISO].max = 1000.0;
	data.configs[tsms_config::ISO].prefix = 'M';
	data.configs[tsms_config::ISO].offset = 0;
	data.configs[tsms_config::ISO].type = 0;
				
	// L		
	data.configs[tsms_config::L].min = 39.0;
	data.configs[tsms_config::L].nom = 51.0;
	data.configs[tsms_config::L].max = 73.0;
	data.configs[tsms_config::L].prefix = 'µ';
	data.configs[tsms_config::L].offset = 0;
	data.configs[tsms_config::L].type = 1;
				 
	// Z1		 
	data.configs[tsms_config::Z1].min = 1100.0;
	data.configs[tsms_config::Z1].nom = 2600.0;
	data.configs[tsms_config::Z1].max = 3700.0;
	data.configs[tsms_config::Z1].prefix = ' ';
	data.configs[tsms_config::Z1].offset = 0;
	data.configs[tsms_config::Z1].frequency = 100000;
	data.configs[tsms_config::Z1].type = 2;
				 
	// Z2		 
	data.configs[tsms_config::Z2].min = 1100.0;
	data.configs[tsms_config::Z2].nom = 2600.0;
	data.configs[tsms_config::Z2].max = 3700.0;
	data.configs[tsms_config::Z2].prefix = ' ';
	data.configs[tsms_config::Z2].offset = 0;
	data.configs[tsms_config::Z2].frequency = 10000000;
	data.configs[tsms_config::Z2].type = 3;
				 
	// Z3		 
	data.configs[tsms_config::Z3].min = 1100.0;
	data.configs[tsms_config::Z3].nom = 2600.0;
	data.configs[tsms_config::Z3].max = 3700.0;
	data.configs[tsms_config::Z3].prefix = ' ';
	data.configs[tsms_config::Z3].offset = 0;
	data.configs[tsms_config::Z3].frequency = 20000000;
	data.configs[tsms_config::Z3].type = 4;
				 
	// C		 
	data.configs[tsms_config::C1].min = 30.0;
	data.configs[tsms_config::C1].nom = 40.0;
	data.configs[tsms_config::C1].max = 50.0;
	data.configs[tsms_config::C1].prefix = 'p';
	data.configs[tsms_config::C1].offset = 0;
	data.configs[tsms_config::C1].type = 5;
				 
	// R1		 
	data.configs[tsms_config::R1].min = 0.11;
	data.configs[tsms_config::R1].nom = 0.5;
	data.configs[tsms_config::R1].max = 0.68;
	data.configs[tsms_config::R1].prefix = ' ';
	data.configs[tsms_config::R1].offset = 0;
	data.configs[tsms_config::R1].type = 6;
				 
	// R2		 
	data.configs[tsms_config::R2].min = 0.11;
	data.configs[tsms_config::R2].nom = 0.5;
	data.configs[tsms_config::R2].max = 0.68;
	data.configs[tsms_config::R2].prefix = ' ';
	data.configs[tsms_config::R2].offset = 0;
	data.configs[tsms_config::R2].type = 7;
}

void TestTable::InsertSampleData()
{
	TestConfig param;
	GetTestData01(param);
	InsertConfig("B82786C 104H  2", param);
	GetTestData02(param);
	InsertConfig("B82786C 513H  2", param);
	InsertConfig("B82786L 104N  2", param);
	InsertConfig("B82786L 204N  2", param);
	GetDummyTest(param);
	InsertConfig(tsms_config::DummyTestDataSet, param);
}

#pragma endregion