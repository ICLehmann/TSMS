#pragma once
#include <atomic>
#include <vector>
#include "MeasuredValue.h"
#include "const.h"
#include "LotData.h"

class PAThandler
{
public:
	std::atomic<bool> pat_started = false;	// true when pat sampling is in progress
	std::atomic<unsigned short> rest_pat_sample = 0;	// counting backwards the parts that will be the base of the PAT calculation

	//Signal to update the tolerances
	bool UpdateRequest = false;
	unsigned int UpdateStation = 0;

	// PAT configuration
	PATConfig Config {0};
	
	// narrowed measurement parameters
	int NumOf_PAT_Measurements=0;
	bool PAT_Measurements[tsms_config::NUM_MEASUREMENTS];

	void StartPAT(unsigned int laststation, lot::LOT_Data& lot);
	bool AddValues(std::vector<MeasuredValue>& values, TestConfig& test_config);
	void CalculateTolerances(TestConfig& test_config);
	bool Shift();

private:
	// array for storing the part's measurement values that will be used for PAT calculation
	std::vector<std::vector<MeasuredValue>> pat_vector;
	std::atomic<bool> pat_adding = false;	// true when pat sample adding is in progress

	unsigned int _laststation;

	void CalculateTolerances();
	void StartUpdate();
};
