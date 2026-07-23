#pragma once
#include "const.h"

struct ValueConfig
{
	// general settings -> LOT table
	double min = 0;
	double nom = 0;
	double max = 0;
	double offset = 0.0;
	double scale = 1.0;
	unsigned char prefix;
	double error_rate = 0.02;
	long long frequency = (long long)1E+6;
	double stimuli_level = 0.1;
	unsigned int type = 0;
	// pat related settings -> PAT table
	bool pat_enabled = false;
	double pat_min = 0;
	double pat_max = 0;

	void Reset();
	void ResetPAT();
};

struct TestConfig
{
	std::string laser_code;
	TestConfig();
	void Reset();
	void ResetPAT();
	ValueConfig configs[tsms_config::NUM_MEASUREMENTS]; // configuration for each test parameter
};
