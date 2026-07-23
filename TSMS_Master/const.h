#pragma once
#include <string>

#define GUI 1	// cooperation with the TSMS_Vis GUI application
#define WITH_C 0

namespace tsms_config
{
	static const std::string INI_FILE = "tsms.ini";
	static unsigned int NUM_STATIONS = 6;		//number of station in the part buffer
	// in case of ACT1210 -> PRECISOR -> TEST SITE #1 -> TEST SITE #2 -> TEST SITE #3 -> PRECISOR -> TEST SITE #4 -> TEST SITE #5 ->

	// devices
#if WITH_C 
	static const unsigned int NUM_DEVICES = 5;	// number of the measurement instuments
	enum eMeasureDevice { SM7110_1, IM7581_1, IM7581_2, RES2329_1, RES2329_2};
	static const std::string DeviceNames[] = { "SM7110_1", "IM7581_1", "IM7581_2", "RES2329_1", "RES2329_2" };
#else
	static const unsigned int NUM_DEVICES = 4;
	enum eMeasureDevice { SM7110_1, IM7581_1, RES2329_1, RES2329_2 };
	static const std::string DeviceNames[] = { "SM7110_1", "IM7581_1", "RES2329_1", "RES2329_2" };
#endif 
	static const unsigned int DEFAULT_DEVICE_TIMEOUT_MS = 150;

	// RS232
	static const unsigned int DEFAULT_RS232_TIMEOUT_MS = 3000;
	static const std::string SIM_START_TELEGRAM = "NEW;924012301;B82786C 104H  2;LA0;1234";
	static const std::string SIM_END_TELEGRAM = "END;000001;000002;000003;000004;000005;000006;000007;000008;000009;000010;000011;000012;000013;000014;000015;000016;000017;000018";
	static const std::string SIM_DISCARD_TELEGRAM = "DISCARD;000000;000000;000000;000000;000000;000000;000000;000000;000000;000000;000000;000000;000000;000000;000000;000000;000000;000000";

	// measurements
	static const unsigned int NUM_MEASUREMENTS = 10;	// number of measurement can be defined
	static const std::string MeasurementNames[NUM_MEASUREMENTS] = { "Iso", "L", "Z1", "Z2", "Z3", "C1", "C2", "C3", "R1", "R2" };
	static const std::string MeasurementUnits[NUM_MEASUREMENTS] = { "Ohm", "H", "Ohm", "Ohm", "Ohm", "F", "F", "F", "Ohm", "Ohm" };
	enum eMeasurement { ISO, L, Z1, Z2, Z3, C1, C2, C3, R1, R2 };
	
	// limit factor to identify measurement system fail
	static const double FACTOR_LIMIT_TO_FAIL = 10.0;
	
	// compensations
	static const unsigned short NUM_COMPS = 6;	// number of compensations  -> in GUI CompWindow is fix !! 

	// machine counter / reject bins
	static const unsigned int NUM_COUNTER_TRB = 8;
	static const std::string CounterTRB_Names[] = { "TRB_Iso", "TRB_Ind", "TRB_Imp", "TRB_Cap", "TRB_Res1", "TRB_Res2", "TRB_System", "TRB_Reserve" };
	static const unsigned int NUM_COUNTER_VRB = 6;
	static const std::string CounterVRB_Names[] = { "VRB_TSide", "VRB_Side", "VRB_Bot", "VRB_Top_4S", "VRB_Reserve1", "VRB_Reserve2" };

	// database
	const std::string db_connection_string = "DRIVER={SQL Server}; SERVER=localhost\\SQLEXPRESS; DATABASE=tsms_db; Trusted_Connection=yes;";
	const std::string db_TableNameLot = "[dbo].[LOT]";
	const std::string db_TableNamePat = "[dbo].[PAT]";
	const std::string db_TableNameValues = "[dbo].[Measurements]";
	const std::string db_TableNameTests = "[dbo].[Testplan]";
	const std::string db_TableNameBuffer = "[dbo].[BufferBackup]";
	const std::string db_TableNameConfig = "[dbo].[Config]";

	// communication with GUI
	const int TELEGRAM_CHAR_LENGTH = 20;	// number of char in telegrams from master (should be minimum like number of characters in database)
	enum eTelToUI {TEL_SETUP = 32, TEL_STATUS, TEL_LOT, TEL_COUNTER, TEL_MEAS_VALUE, 
		TEL_MACHINE, TEL_COMP_STATUS, TEL_USER_MSG, TEL_PAT_UPDATE };	// if you change this, you have to change same in ui too
	enum eTelFromUI { TEL_UI_IS_STARTED = 32, TEL_UI_INIT_RS232, TEL_UI_INIT_DB, TEL_UI_INIT_DEVICES, TEL_UI_INIT_IO, TEL_UI_RESTART_PAT };	// if you change this, you have to change same in ui too

	static const std::string MessagePrintReport = "PRINT_REPORT";		// message to gui to print the report
	static const std::string MessagePrintLabel = "PRINT_LABEL";		// message to gui to print the label

	// E-Dummy
	static const unsigned int FristInternalLotNumber = 1000; // used to store dummy an compensation info in LotTable
	static const std::string DummyTestDataSet = "E-Dummy";
	static const std::string CompensationTestplan = "Compensation";

}