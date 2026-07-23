#pragma once
#include <string>
#include <vector>
#include <chrono>
#include "../const.h"
#include "../common/Handler.h"
#include "../db/DBHandler.h"
#include "../MeasuredValue.h"
#include "../TestConfig.h"

namespace device
{
	using namespace std::chrono;

	class DeviceHandler : public Handler
	{
	private:
		std::chrono::high_resolution_clock::time_point _start_time;
		unsigned int _last_cycle_time_ms;
		unsigned int _last_station;
		unsigned int _timeout_ms;

		bool _measure_active;
		void StartThreads();
		void StopThreads();

	public:
		DeviceHandler();
		~DeviceHandler();

		bool Init() override;
		void Close() override;
		
		bool SetTestConfig(TestConfig& test_config); // sets complete test configuration for each measurement device
		void UpdatePATLimits(TestConfig& test_config, unsigned int station); // updates only the PAT narrowed tolerance limits
		void StartMeasurement(tsms_config::eMeasureDevice dev);
		bool IsFinished();
		void GetResults(std::vector<MeasuredValue>& data);

		unsigned int GetLastCycleTime() { return _last_cycle_time_ms; }
		unsigned int GetLastStation() { return _last_station; }

		bool WriteConfigToDatabase(db::DBHandler& db);

	};

}