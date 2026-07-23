#include "PAThandler.h"
#include "const.h"

// starts PAT data collection
void PAThandler::StartPAT(unsigned int laststation, lot::LOT_Data& lot)
{
	Config = lot.pat_config; // copies pat config from lot data

	// mode: 0 -> pat is not enabled
	if (Config.mode != 0 && pat_started == false)
	{
		// initializes variables for starting
		rest_pat_sample = Config.samplecnt;
		_laststation = laststation;
		pat_started = true;
		pat_vector.clear();
	}
}

// adds one part's data to the collection, returns true if PAT calculation is finished
bool PAThandler::AddValues(std::vector<MeasuredValue>& values, TestConfig& test_config)
{
	if (pat_adding) return false;
	pat_adding = true;

	// checks whether the part is ok for PAT
	bool finished=false;
	bool isAllParamsGood = true;
	int numof_measurements = 0;

	for (auto& value : values)
	{
		auto id = value.GetID();
		// checks all measurements were passed
		// however, it uses the original tolerance limits!!!
		if (value.eval_nopat != Evaluation::PASS) isAllParamsGood = false;
		// check necessary measurements
		if (PAT_Measurements[id]) numof_measurements++;
	}

	if (isAllParamsGood && numof_measurements == NumOf_PAT_Measurements)
	{
		// decrementing the sample counter
		rest_pat_sample--;

		pat_vector.push_back(values);

		// when PAT sampling is finished
		if (rest_pat_sample == 0)
		{
			pat_started = false;
			CalculateTolerances(test_config);
			StartUpdate();
			finished = true;
		}
	}

	pat_adding = false;
	return finished;
}

// eventually, calculates the new narrowed upper and lower limits
void PAThandler::CalculateTolerances(TestConfig& test_config)
{
	int i, id;
	double sum[tsms_config::NUM_MEASUREMENTS] = { 0.0 };
	double avg[tsms_config::NUM_MEASUREMENTS] = { 0.0 };
	double totvar[tsms_config::NUM_MEASUREMENTS] = { 0.0 };
	double stdev[tsms_config::NUM_MEASUREMENTS] = { 0.0 };
	double tmp_limit;

	// calculates mean
	for (i = 0; i < Config.samplecnt; i++)
	{
		for (auto& value : pat_vector[i])
		{
			id = value.GetID();
			if (PAT_Measurements[id]) sum[id] += value.GetValue();
		}
	}
	
	// calculates averages
	for (i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		if (PAT_Measurements[i]) avg[i] = sum[i] / Config.samplecnt;
	}

	// calculates total variance
	for (i = 0; i < Config.samplecnt; i++)
	{
		for (auto& value : pat_vector[i])
		{
			id = value.GetID();
			if (PAT_Measurements[id]) totvar[id] += pow(value.GetValue() - avg[id], 2);
		}
	}

	// calculates standard deviation
	for (i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		if (PAT_Measurements[i]) stdev[i] = sqrt(totvar[i] / Config.samplecnt);
	}

	// calculates UL and LL
	for (i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		if (PAT_Measurements[i]) {
			// min limit
			tmp_limit = avg[i] - (stdev[i] * Config.ll_sigma);
			if (tmp_limit < test_config.configs[i].min) tmp_limit = test_config.configs[i].min;
			test_config.configs[i].pat_min = tmp_limit;
			// max limit
			tmp_limit = avg[i] + (stdev[i] * Config.ll_sigma);
			if (tmp_limit > test_config.configs[i].max) tmp_limit = test_config.configs[i].max;
			test_config.configs[i].pat_max = tmp_limit;
			// enables pat narrowing
			test_config.configs[i].pat_enabled = true;
		}
		else
		{
			test_config.configs[i].pat_enabled = false;
		}
	}
}

void PAThandler::StartUpdate()
{
	UpdateStation = 0;
	UpdateRequest = true;
}

// increments station number when tray shifts
bool PAThandler::Shift()
{
	UpdateStation++;
	if (UpdateStation > _laststation) {
		UpdateRequest = false; // stops updating when last station is reached
		UpdateStation = 0;
		return true;
	}
	else
		return false;

}