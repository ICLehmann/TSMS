#include "Counter.h"

MachineCounter::MachineCounter()
{
	Reset();
}

void MachineCounter::Reset()
{
	total_input = 0;
	total_good = 0;
	good_bin = 0;
	last_bin = 0;

	for (unsigned int i = 0; i < tsms_config::NUM_COUNTER_TRB; i++)
		TRB[i] = 0;
	for (unsigned int i = 0; i < tsms_config::NUM_COUNTER_VRB; i++)
		TRV[i] = 0;
}

void MachineCounter::Add(MachineCounter& newCnt)
{
	total_input += newCnt.total_input;
	total_good += newCnt.total_good;
	good_bin += newCnt.good_bin;
	last_bin += newCnt.last_bin;

	for (unsigned int i = 0; i < tsms_config::NUM_COUNTER_TRB; i++)
		TRB[i] = newCnt.TRB[i];
	for (unsigned int i = 0; i < tsms_config::NUM_COUNTER_VRB; i++)
		TRV[i] = newCnt.TRV[i]; 
}

void TSMSCounter::Reset()
{
	input_parts = 0;
	good_parts = 0;
	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		res_fail[i] = 0;
		res_pass[i] = 0;
		res_low[i] = 0;
		res_high[i] = 0;
	}
}

void TSMSCounter::Copy(TSMSCounter& newCnt)
{
	input_parts = newCnt.input_parts;
	good_parts = newCnt.good_parts;
	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		res_fail[i] = newCnt.res_fail[i];
		res_pass[i] = newCnt.res_pass[i];
		res_low[i] = newCnt.res_low[i]; 
		res_high[i] = newCnt.res_high[i];
	}
}
