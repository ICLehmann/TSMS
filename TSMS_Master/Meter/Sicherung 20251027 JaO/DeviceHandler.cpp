#include "DeviceHandler.h"
#include "../const.h"
#include "../common/Logger.h"
#include "../common/IniFile.h"

#include <iostream>
#include <mutex>
#include <utility>

#include "MeterGPIB.h"
#include "MeterTCP.h"
#include "MeterSim.h"

namespace device
{
	std::vector<MeasuredValue> g_thread_results;
	std::mutex values_mutex;

	Meter* g_devices[tsms_config::NUM_DEVICES] = { 0 };
	std::thread measureThreads[tsms_config::NUM_DEVICES];
	std::atomic_bool do_measurement[tsms_config::NUM_DEVICES] = { false };
	std::atomic_bool stop_all_threads = false;

	void results_push_back(MeasuredValue& v)
	{
		std::lock_guard<std::mutex> lk(values_mutex);
		g_thread_results.push_back(v);
	}

	void measure_function(tsms_config::eMeasureDevice device)
	{
		while (!stop_all_threads)
		{
			if (do_measurement[device])
			{
				auto m = g_devices[device]->Measure();
				for (auto& value : m)
					results_push_back(value);

				do_measurement[device] = false;
			}
			Sleep(1);
		}
	}

	DeviceHandler::DeviceHandler() : Handler("Devices")
	{
		_measure_active = false;
		_last_cycle_time_ms = 0;
		_last_station = 0;
		_timeout_ms = tsms_config::DEFAULT_DEVICE_TIMEOUT_MS;
	}

	DeviceHandler::~DeviceHandler()
	{
		Close();
	}

	void DeviceHandler::StartThreads()
	{
		g_thread_results.clear();
		stop_all_threads = false;
		for (unsigned int i = 0; i < tsms_config::NUM_DEVICES; i++)
		{
			measureThreads[i] = std::thread(measure_function, static_cast<tsms_config::eMeasureDevice>(i));
		}
	}

	void DeviceHandler::StopThreads()
	{
		stop_all_threads = true;
		for (unsigned int i = 0; i < tsms_config::NUM_DEVICES; i++)
		{
			if (measureThreads[i].joinable())
				measureThreads[i].join();
		}
	}

	bool DeviceHandler::Init()
	{
		Close();

		IniFile ini(tsms_config::INI_FILE.c_str());
		_timeout_ms = ini.ReadInteger("Meter", "TimeoutMS", tsms_config::DEFAULT_DEVICE_TIMEOUT_MS);
		bool simMeter = ini.ReadBoolean("Meter", "SimulateMeter", 0);
		int simDev = ini.ReadInteger("Meter", "SimulatedDeviation", 5);

		_last_station = 0;

		unsigned int num_measurements = 0;
		for (unsigned int i = 0; i < tsms_config::NUM_DEVICES; i++)
		{
			DeviceConfig dev_config;
			if (!dev_config.ReadConfigFile(tsms_config::DeviceNames[i]))
			{
				_handler_status = eHandlerStatus::H_ERROR;
				_last_error = "Read config file failed";
				return false;
			}

			for (auto& m : dev_config.measurements)
			{
				m.id = num_measurements++;
				/*	tsms_config::MeasurementNames[num_measurements] = m.name;		//todo
					tsms_config::MeasurementUnits[num_measurements] = m.unit;*/
			}

			if (simMeter)
			{
				g_devices[i] = new MeterSim(simDev);
			}
			else
			{
				if (dev_config.IsGPIB())
					g_devices[i] = new MeterGPIB();
				else
					g_devices[i] = new MeterTCP();
			}

			if (!g_devices[i]->init(dev_config))
			{
				_handler_status = eHandlerStatus::H_ERROR;
				_last_error = "Initialization '" + g_devices[i]->get_device_name() + "' failed";
				return false;
			}

			if (dev_config.GetStationNum() > _last_station)
				_last_station = dev_config.GetStationNum();
		}

		StartThreads();

		_handler_status = eHandlerStatus::H_SUCCESS;
		logger::WriteLog(_msg_prefix + " initialized");
		return true;
	}

	void DeviceHandler::Close()
	{
		if (_handler_status == eHandlerStatus::H_CLOSED)
			return;

		StopThreads();
		for (unsigned int i = 0; i < tsms_config::NUM_DEVICES; i++)
		{
			if (g_devices[i] != 0)
			{
				g_devices[i]->clean_up();
				delete g_devices[i];
				g_devices[i] = 0;
			}
		}
		g_thread_results.clear();
		_handler_status = eHandlerStatus::H_CLOSED;
		logger::WriteLog(_msg_prefix + " closed");
	}

	bool DeviceHandler::WriteConfigToDatabase(db::DBHandler& db)
	{
		for (unsigned int i = 0; i < tsms_config::NUM_DEVICES; i++)
		{
			if (g_devices[i] == 0)
				continue;

			DeviceConfig& dev_config = g_devices[i]->get_config();
			db.ConfigTable_Insert(dev_config);
			if (_handler_status != eHandlerStatus::H_SUCCESS)
				return false;
		}
		return true;
	}

	bool DeviceHandler::SetTestConfig(TestConfig& test_config)
	{
		std::string error_message = "Setting testplan config to '";
		std::vector<ValueConfig> val_config;
		val_config.push_back(test_config.configs[tsms_config::ISO]);
		if (!g_devices[tsms_config::SM7110_1]->set_value_config(val_config))
		{
			_last_error = error_message + g_devices[tsms_config::SM7110_1]->get_device_name() + "' failed!";
			_handler_status = eHandlerStatus::H_ERROR;
			return false;
		}

		val_config.clear();
		val_config.push_back(test_config.configs[tsms_config::R1]);
		if (!g_devices[tsms_config::RES2329_1]->set_value_config(val_config))
		{
			_last_error = error_message + g_devices[tsms_config::RES2329_1]->get_device_name() + "' failed!"; 
			_handler_status = eHandlerStatus::H_ERROR;
			return false;
		}

		val_config.clear();
		val_config.push_back(test_config.configs[tsms_config::R2]);
		if (!g_devices[tsms_config::RES2329_2]->set_value_config(val_config))
		{
			_last_error = error_message + g_devices[tsms_config::RES2329_2]->get_device_name() + "' failed!";
			_handler_status = eHandlerStatus::H_ERROR;
			return false;
		}

		val_config.clear();
		val_config.push_back(test_config.configs[tsms_config::L]);
		val_config.push_back(test_config.configs[tsms_config::Z1]);
		val_config.push_back(test_config.configs[tsms_config::Z2]);
		val_config.push_back(test_config.configs[tsms_config::Z3]);
		if (!g_devices[tsms_config::IM7581_1]->set_value_config(val_config))
		{
			_last_error = error_message + g_devices[tsms_config::IM7581_1]->get_device_name() + "' failed!";
			_handler_status = eHandlerStatus::H_ERROR;
			return false;
		}

		val_config.clear();
		val_config.push_back(test_config.configs[tsms_config::C1]);
		val_config.push_back(test_config.configs[tsms_config::C2]);
		val_config.push_back(test_config.configs[tsms_config::C3]);
		if (!g_devices[tsms_config::IM7581_2]->set_value_config(val_config))
		{
			_last_error = error_message + g_devices[tsms_config::IM7581_2]->get_device_name() + "' failed!";
			_handler_status = eHandlerStatus::H_ERROR;
			return false;
		}

		return true;
	}

	void DeviceHandler::StartMeasurement(tsms_config::eMeasureDevice device)
	{
		if (_handler_status == eHandlerStatus::H_ERROR || _handler_status == eHandlerStatus::H_CLOSED)
			return;

		if (!_measure_active)
		{
			_start_time = high_resolution_clock::now();
			_measure_active = true;
		}

		do_measurement[device] = true;
	}

	bool DeviceHandler::IsFinished()
	{
		if (!_measure_active)
			return true;

		bool measure_finished = true;
		for (int i = 0; i < tsms_config::NUM_DEVICES; i++)
			if (do_measurement[i])
				measure_finished = false;

		duration<double, std::milli> diff = high_resolution_clock::now() - _start_time;
		_last_cycle_time_ms = (unsigned int)diff.count();

		if (measure_finished)
		{
			_measure_active = false;
		}
		else
		{
			if (_last_cycle_time_ms > _timeout_ms)
			{
				for (int i = 0; i < tsms_config::NUM_DEVICES; i++)
				{
					if (do_measurement[i])
						_last_error = "Timeout on device '" + g_devices[i]->get_device_name() +"' after " + std::to_string(GetLastCycleTime()) + "ms";
				}
				_handler_status = eHandlerStatus::H_ERROR;
				measure_finished = true;
				_measure_active = false;
			}
		}

		return measure_finished;
	}

	void DeviceHandler::GetResults(std::vector<MeasuredValue>& data)
	{
		data.clear();
		for (auto& value : g_thread_results)
		{
			if (value.eval == Evaluation::NONE)
			{
				logger::WriteLog("Error: No value for '" + tsms_config::MeasurementNames[value.GetID()] + "'!"); 
				_handler_status = eHandlerStatus::H_ERROR;
			}
			data.push_back(value);
		}
		g_thread_results.clear();
	}
}


