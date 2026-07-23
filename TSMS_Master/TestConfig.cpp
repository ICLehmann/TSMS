#include "TestConfig.h"

void ValueConfig::Reset()
{
	min = 0;
	nom = 0;
	max = 0;
	scale = 1.0;
	offset = 0.0;
	prefix = ' ';
	error_rate = 0.02;
	frequency = (unsigned long)1E+6;
	stimuli_level = 0.1;
	unsigned int type = 0;
	// pat related settings
	pat_enabled = false;
	pat_min = 0;
	pat_max = 0;
}

void ValueConfig::ResetPAT()
{
	pat_enabled = false;
	pat_min = 0;
	pat_max = 0;
}

TestConfig::TestConfig()
{
	Reset();
}

void TestConfig::Reset()
{
	laser_code = "??";
	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		configs[i].Reset();
		configs[i].type = i;
	}
}

void TestConfig::ResetPAT()
{
	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
		configs[i].ResetPAT();
	}
}
