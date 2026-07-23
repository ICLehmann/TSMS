#include "LotData.h"
#include <iostream>
#include "LotData.h"
#include "common/LaserData.h"
#include "common/DateTime.h"

namespace lot
{
	LOT_Data::LOT_Data()
	{
		Reset();
	}

	void LOT_Data::Reset()
	{
		header.lot_nr = 0;
		header.test_plan = "";
		header.line_id = "";
		header.operator_id = "";
		header.is_dummy = false;
		
		laser_mark = "";
		lot_start = "";
		lot_stop = "";
		
		test_config.Reset();
		tsms_cnt.Reset();
		machine_cnt.Reset();

		lot_state = UNKNOWN;

		pat_config.mode = 0;
		pat_config.samplecnt = 0;
		pat_config.ll_sigma = 0;
		pat_config.ul_sigma = 0;
	}

	void LOT_Data::SetHeader(LotHeader newHeader)
	{
		header = newHeader;
	}

	void LOT_Data::SetPATConfig(PATConfig newPATConfig)
	{
		pat_config = newPATConfig;
	}

	void LOT_Data::SetStartTimeToNow()
	{
		lot_start = GetCurrentDateTime();
	}

	void LOT_Data::SetStopTimeToNow()
	{
		lot_stop = GetCurrentDateTime();
	}

	void LOT_Data::CreateNewLaserMark()
	{
		char product = test_config.laser_code.c_str()[0];
		char location = test_config.laser_code.c_str()[1];
		LaserData lt(product, location, header.lot_nr);
		laser_mark = lt.Mark;
	}
}


