#include "Meter.h"

#include "../common/Logger.h"

#include <windows.h>
#include <iostream>

#include <chrono>
using namespace std::chrono;


Meter::Meter()
{
	_is_init = false;
	_value_config.clear();
}

bool Meter::set_value_config(std::vector<ValueConfig>& config)
{
	_value_config = config;
	if (!_device_config.CreateSetupSeq(_value_config))
		return false;
	if (!send_setup())
		return false;
	return true;
}

void Meter::update_PAT(std::vector<ValueConfig>& config)
{
	for (unsigned int i = 0; i < config.size(); i++)
	{
		_value_config[i].pat_enabled = config[i].pat_enabled;
		_value_config[i].pat_max = config[i].pat_max;
		_value_config[i].pat_min = config[i].pat_min;
	}
}

void Meter::set_base_config(DeviceConfig& dev_config)
{
	_device_config = dev_config;
	logger::WriteLog("Initialize " + _device_config.GetDeviceName() + " (" + std::to_string(_device_config.GetStationNum()) + ") ... ");
}

bool Meter::send_setup()
{
	if (!_is_init)
		return false;

	for (auto& line : _device_config.setup_seq_filled)
	{
		if (!Send(line.c_str(), true))
			return false;
	}
	return true;
}

void Meter::FillMeasuredValue(MeasuredValue& m_val, ReceivedValue raw_value, unsigned int config_index)
{
	if (config_index >= _value_config.size())
		return;

	m_val.station_nr = _device_config.GetStationNum();

	// pat related data
	m_val.pat_enabled = _value_config[config_index].pat_enabled;
	m_val._pat_min = _value_config[config_index].pat_min;
	m_val._pat_max = _value_config[config_index].pat_max;

	if (raw_value.vaild)
	{
		double value = raw_value.value * _value_config[config_index].scale + _value_config[config_index].offset;

		m_val.SetValue(value);

		// evaluation by original tolerance limits
		if (_value_config[config_index].max != 0 && value > _value_config[config_index].max)
		{
			m_val.eval = Evaluation::HIGH;
			if (value > _value_config[config_index].max * tsms_config::FACTOR_LIMIT_TO_FAIL)
				m_val.eval = Evaluation::FAIL;
		}
		else if (_value_config[config_index].min != 0 && value < _value_config[config_index].min)
		{
			m_val.eval = Evaluation::LOW;
			if (value < _value_config[config_index].min / tsms_config::FACTOR_LIMIT_TO_FAIL)
				m_val.eval = Evaluation::FAIL;
		}
		else
		{
			m_val.eval = Evaluation::PASS;
		}

		m_val.eval_nopat = m_val.eval;

		// evaluation by narrowed PAT tolerance limits
		if (_value_config[config_index].pat_enabled && m_val.eval != Evaluation::FAIL)
		{
			if (_value_config[config_index].pat_max != 0 && value > _value_config[config_index].pat_max)
				m_val.eval = Evaluation::HIGH;
			else if (_value_config[config_index].pat_min != 0 && value < _value_config[config_index].pat_min)
				m_val.eval = Evaluation::LOW;
			else
				m_val.eval = Evaluation::PASS;
		}
	}
	else
	{
		m_val.eval = Evaluation::FAIL;
	}
}

std::vector<MeasuredValue> Meter::Measure()
{
	std::vector<MeasuredValue> result_vec;

	// check condition
	bool device_error = false;
	if (!_is_init || _value_config.empty() || _value_config.size() > _device_config.measurements.size())
	{
		logger::WriteLog("Config error device '" + _device_config.GetDeviceName() + "': No Config");
		device_error = true;
	}

	Sleep(_device_config.GetTriggerDelay());

	auto start = std::chrono::high_resolution_clock::now();

	if (!device_error)
	{
		// trigger measurement

		for (auto& line : _device_config.start_measure_seq)
		{
			if (!Send(line, false))
			{
				device_error = true;
				break;
			}
		}
	}

	if (!device_error)
	{
		Sleep(_device_config.GetWaitAfterTrigger());

		// ask device for answer telegram

		for (auto& line : _device_config.ask_for_answer)
		{
			size_t found = line.find("FETC");		// RES2329 only !!!
			if (found != std::string::npos)
			{
				if (!Wait_Until_RES2329_Finished())
				{
					device_error = true;
					break;
				}
			}

			if (!Send(line, false))
			{
				device_error = true;
				break;
			}
		}
	}

	if (!device_error)
	{
		std::vector<ReceivedValue> values;
		if (!ReceiveValues(values))
			device_error = true;
	
		if (!device_error)
		{
			duration<double, std::milli> diff = std::chrono::high_resolution_clock::now() - start;

			for (unsigned int i = 0; i < _value_config.size(); i++)
			{
				MeasuredValue result(_value_config[i].type, _device_config.GetStationNum());
				unsigned int value_pos = _device_config.measurements[i].answer_pos;
				if (value_pos >= values.size())
				{
					logger::WriteLog("Config error device '" + _device_config.GetDeviceName() + "' : the configured position is greater than the number of received values");
					device_error = true;
					break;
				}

				FillMeasuredValue(result, values[value_pos], i);
				result.time_ms = (int)diff.count();
				result_vec.push_back(result);
			}
		}
	}

	if (device_error)
	{
		MeasuredValue error_res(0, _device_config.GetStationNum());
		error_res.eval = NONE;
		duration<double, std::milli> diff = std::chrono::high_resolution_clock::now() - start;
		error_res.time_ms = (int)diff.count();
		result_vec.push_back(error_res);
	}

	return result_vec;
}