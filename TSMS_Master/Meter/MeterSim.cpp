#include "MeterSim.h"

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <random>
#include <chrono>

using namespace std::chrono;

bool MeterSim::comp_short()
{
	Sleep(2000); 
	return true;
}

bool MeterSim::comp_open()
{
	Sleep(2000);
	return true;
}

MeterSim::MeterSim(double dev_percent)
{
	_is_init = false;
	_dev_percent = dev_percent;
}

MeterSim::~MeterSim()
{
}

bool MeterSim::init(DeviceConfig& dev_config)
{
	set_base_config(dev_config);
	Sleep(10);
	logger::WriteLog("Start Simulation " + get_device_name() + " ...");
	_is_init = true;
	return true;
}

void MeterSim::clean_up()
{
	if (_is_init)
	{
		logger::WriteLog("Clean up " + get_device_name() + " ...");
		_is_init = false;
	}
}

std::vector<MeasuredValue> MeterSim::Measure()
{
	Sleep(_device_config.GetTriggerDelay());

	auto start = std::chrono::high_resolution_clock::now();
	std::vector<MeasuredValue> result;
	for (unsigned int i = 0; i < _value_config.size(); i++)
	{
		_value_config[i].scale = 1.0;
		MeasuredValue mv(_value_config[i].type, _device_config.GetStationNum());
		
		double mean = _value_config[i].nom;
		double stddev = 0.001;
		if (_value_config[i].nom != 0 && _dev_percent != 0)
			stddev = abs(_value_config[i].max - _value_config[i].min) / 100 * _dev_percent;

		// for testing that one parameter is not good
		//if (mv.GetID() == tsms_config::eMeasurement::R1) stddev = 100;
		
		// modifies the mean of the isolation test because the nominal value is too near to lower limit
		if (mv.GetID() == tsms_config::eMeasurement::ISO)
		{
			mean = _value_config[i].min + (_value_config[i].max - _value_config[i].min) / 2;
		}

		std::random_device rd;
		std::mt19937 gen(rd());
		std::normal_distribution<double> dist(mean, stddev);

		Sleep(10);

		duration<double, std::milli> diff = std::chrono::high_resolution_clock::now() - start;

		ReceivedValue rv;
		rv.value = dist(gen);
		rv.vaild = true;

		FillMeasuredValue(mv, rv, i);
		mv.time_ms = (int)diff.count();

		result.push_back(mv);
	}
	return result;
}
