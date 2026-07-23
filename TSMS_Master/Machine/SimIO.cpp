#include "SimIO.h"
#include <Windows.h>

#include <atomic>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <queue>

#include "../const.h"
#include "../common/IniFile.h"
#include "../common/Logger.h"
#include <chrono>


namespace io
{
	bool g_isSimRunning = false;
	int g_SimParts = 100;
	int g_SimClock = 200;

	std::thread simThread;
	static std::atomic_bool g_StopSim = false;
	static std::atomic<int> g_SimResultCounter = 0;

	static std::atomic<char> data_in[BITS_TO_READ] = { 0 };
	static std::atomic<char> data_in_cached[BITS_TO_READ] = { 0 };
	static std::atomic<char> data_out_cached[BITS_TO_WRITE] = { 0 };
	static func_t registered_functions[BITS_TO_READ] = { 0 };

	void SimulateInputPin(enum InputPins i, bool value)
	{
		data_in[i] = value;
		if (data_in[i] != data_in_cached[i])
			if (registered_functions[i] != 0)
				registered_functions[i]();		// call change event function

		data_in_cached[i] = (char)data_in[i];
	}

	void SimLOT()
	{
		std::cout << "Simulation Lot started.\n" << std::flush;
		Sleep(100);

		SimulateInputPin(IN00_MACHINE_EMPTY, 0);
		SimulateInputPin(IN07_E_DUMMY, 0);
		SimulateInputPin(IN01_SYSTEM_READY, 1);
		SimulateInputPin(IN02_LOT_START, 1);
		SimulateInputPin(IN03_LOT_END, 0);

		Sleep(100);

		std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
		while (!data_out_cached[OUT41_LOT_IN_PROGRESS])
		{
			std::chrono::duration<double, std::milli> diff = std::chrono::high_resolution_clock::now() - start_time;
			if ((int)diff.count() > 5000) // timeout ?
			{
				std::cout << "Simulation: Getting no LOT_IN_PROGRESS signal. Simulation terminated.\n" << std::flush;
				return;
			}
			Sleep(10);
		}

		Sleep(100);

		SimulateInputPin(IN02_LOT_START, 0);
		SimulateInputPin(IN20_LOT_IN_PROGRESS, 1);

		SimulateInputPin(IN22_SHIFT_TRIGGER, 1);
		Sleep(100);
		SimulateInputPin(IN22_SHIFT_TRIGGER, 0);
		Sleep(100);

		if (data_out_cached[OUT41_LOT_IN_PROGRESS])
		{
			g_SimResultCounter = 0;
			int expectedResults = 0;
			unsigned int cnt = 0;
			unsigned int num_loops = g_SimParts + tsms_config::NUM_STATIONS - 1;
			while (!g_StopSim && cnt < num_loops)
			{
				// machine start up
				if (cnt == 0)
				{
					SimulateInputPin(InputPins::IN11_ISOLATION_TRIGGER, 1);
					expectedResults = 1;
				}
				else if (cnt == 1)
				{
					SimulateInputPin(InputPins::IN11_ISOLATION_TRIGGER, 1);
					SimulateInputPin(InputPins::IN12_IMPED_INDUCT_TRIGGER, 1);
					expectedResults = 3;
				}
				else if (cnt == 2)
				{
					SimulateInputPin(InputPins::IN11_ISOLATION_TRIGGER, 1);
					SimulateInputPin(InputPins::IN12_IMPED_INDUCT_TRIGGER, 1);
#if WITH_C	
					SimulateInputPin(InputPins::IN14_CAPACITY_TRIGGER, 1);
					expectedResults = 4;
#else
					expectedResults = 3;
#endif
				}
				else if (cnt == 3)
				{
					SimulateInputPin(InputPins::IN11_ISOLATION_TRIGGER, 1);
					SimulateInputPin(InputPins::IN12_IMPED_INDUCT_TRIGGER, 1);
#if WITH_C	
					SimulateInputPin(InputPins::IN14_CAPACITY_TRIGGER, 1);
					expectedResults = 4;
#else
					expectedResults = 3;
#endif
				}

				else if (cnt == 4)
				{
					SimulateInputPin(InputPins::IN11_ISOLATION_TRIGGER, 1);
					SimulateInputPin(InputPins::IN12_IMPED_INDUCT_TRIGGER, 1);
#if WITH_C	
					SimulateInputPin(InputPins::IN14_CAPACITY_TRIGGER, 1);
					expectedResults = 4;
#else
					expectedResults = 3;
#endif
				}
				// machine runs empty
				else if (num_loops - 5 == cnt)
				{
					SimulateInputPin(IN03_LOT_END, 1);

					SimulateInputPin(InputPins::IN12_IMPED_INDUCT_TRIGGER, 1);
#if WITH_C	
					SimulateInputPin(InputPins::IN14_CAPACITY_TRIGGER, 1);
					SimulateInputPin(InputPins::IN15_RESISTANCE_TRIGGER, 1);
					expectedResults = 5;
#else
					SimulateInputPin(InputPins::IN15_RESISTANCE_TRIGGER, 1);
					expectedResults = 4;
#endif

				}
				else if (num_loops - 4 == cnt)
				{
#if WITH_C
					SimulateInputPin(InputPins::IN14_CAPACITY_TRIGGER, 1);
					SimulateInputPin(InputPins::IN15_RESISTANCE_TRIGGER, 1);	
					expectedResults = 3;
#else
					SimulateInputPin(InputPins::IN15_RESISTANCE_TRIGGER, 1);
					expectedResults = 2;
#endif
				}
				else if (num_loops - 3 == cnt)
				{
					SimulateInputPin(InputPins::IN15_RESISTANCE_TRIGGER, 1);
					expectedResults = 2;
				}
				else if (num_loops - 2 == cnt)
				{
					SimulateInputPin(InputPins::IN15_RESISTANCE_TRIGGER, 1);
					expectedResults = 2;
				}
				else if (num_loops - 1 == cnt)
				{
					SimulateInputPin(InputPins::IN15_RESISTANCE_TRIGGER, 1);
					expectedResults = 2;
				}
				// all triggers
				else
				{
				/*	if (cnt == 10)					
					{ 						
						SimulateInputPin(InputPins::IN12_IMPED_INDUCT_TRIGGER, 1);
						SimulateInputPin(InputPins::IN14_CAPACITY_TRIGGER, 1);
						SimulateInputPin(InputPins::IN15_RESISTANCE_TRIGGER, 1);
						expectedResults = 5;
					}
					else*/
					{
#if WITH_C
						SimulateInputPin(InputPins::IN11_ISOLATION_TRIGGER, 1);
						SimulateInputPin(InputPins::IN12_IMPED_INDUCT_TRIGGER, 1);
						SimulateInputPin(InputPins::IN14_CAPACITY_TRIGGER, 1);
						SimulateInputPin(InputPins::IN15_RESISTANCE_TRIGGER, 1);
						expectedResults = 6;
#else
						SimulateInputPin(InputPins::IN11_ISOLATION_TRIGGER, 1);
						SimulateInputPin(InputPins::IN12_IMPED_INDUCT_TRIGGER, 1);
						SimulateInputPin(InputPins::IN15_RESISTANCE_TRIGGER, 1);
						expectedResults = 5;
#endif
					}
				}

				while (!g_StopSim)
				{
					if (g_SimResultCounter == expectedResults)
					{
						g_SimResultCounter = 0;
						Sleep(10);		// wait a while for the main loop is ready

						SimulateInputPin(IN22_SHIFT_TRIGGER, 1);
						Sleep(100);
						SimulateInputPin(IN22_SHIFT_TRIGGER, 0);
						Sleep(100);

						cnt++;
						break;
					}
					Sleep(g_SimClock);
				}
			}
		}

		Sleep(1000);

		SimulateInputPin(IN00_MACHINE_EMPTY, 1);

		Sleep(3000);
	}

	void SimLot2()
	{
		std::cout << "Simulation Lot started.\n" << std::flush;
		Sleep(100);

		SimulateInputPin(IN00_MACHINE_EMPTY, 0);
		SimulateInputPin(IN07_E_DUMMY, 0);
		SimulateInputPin(IN01_SYSTEM_READY, 1);
		SimulateInputPin(IN02_LOT_START, 1);
		SimulateInputPin(IN03_LOT_END, 0);

		Sleep(100);

		std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
		while (!data_out_cached[OUT41_LOT_IN_PROGRESS])
		{
			std::chrono::duration<double, std::milli> diff = std::chrono::high_resolution_clock::now() - start_time;
			if ((int)diff.count() > 5000) // timeout ?
			{
				std::cout << "Simulation: Getting no LOT_IN_PROGRESS signal. Simulation terminated.\n" << std::flush;
				return;
			}
			Sleep(10);
		}

		Sleep(100);

		SimulateInputPin(IN02_LOT_START, 0);
		SimulateInputPin(IN20_LOT_IN_PROGRESS, 1);

		Sleep(100);

		if (data_out_cached[OUT41_LOT_IN_PROGRESS])
		{


			std::queue<std::string> parts;
			//  ILC--R	
			parts.push("111001");	// 6 Stations, 1 is trigger, 0 no trigger
			parts.push("111001");
			parts.push("111001");
			parts.push("000100");
			parts.push("000010");
			parts.push("000010");
			parts.push("100000");
			parts.push("111000");
			parts.push("111001");
			parts.push("111001");
			parts.push("111001");
			parts.push("000000");

			const std::string no_trigger = "000000";

			std::deque<std::string> stations;
			for (int i = 0; i < 6; i++)
				stations.push_back(no_trigger); // fill stations with no trigger parts

			g_SimResultCounter = 0;
			int expectedResults = 0;
			while (!g_StopSim)
			{
				stations.pop_back();	// remove last part
				if (parts.size() > 0)
				{
					auto new_part = parts.front();
					stations.push_front(new_part);
					parts.pop();
				}
				else
				{
					if (stations.back() == no_trigger)
						break;

					stations.push_front(no_trigger); // add no trigger parts
				}

				expectedResults = 0;
				if (stations[0][0] == '1')
				{
					SimulateInputPin(InputPins::IN11_ISOLATION_TRIGGER, 1);
					expectedResults++;
				}
				if (stations[1][1] == '1')
				{
					SimulateInputPin(InputPins::IN12_IMPED_INDUCT_TRIGGER, 1);
					expectedResults += 2;
				}
				if (stations[2][2] == '1')
				{
					SimulateInputPin(InputPins::IN14_CAPACITY_TRIGGER, 1);
					expectedResults++;
				}
				if (stations[5][5] == '1')
				{
					SimulateInputPin(InputPins::IN15_RESISTANCE_TRIGGER, 1);
					expectedResults += 2;
				}

				while (!g_StopSim)
				{
					if (g_SimResultCounter == expectedResults)
					{
						g_SimResultCounter = 0;
						Sleep(10);		// wait a while for the main loop is ready

						SimulateInputPin(IN22_SHIFT_TRIGGER, 1);
						Sleep(100);
						SimulateInputPin(IN22_SHIFT_TRIGGER, 0);
						Sleep(100);

						break;
					}
					Sleep(g_SimClock);
				}

			}
		}
		SimulateInputPin(IN03_LOT_END, 1);
		
		Sleep(1000);

		SimulateInputPin(IN00_MACHINE_EMPTY, 1);

		Sleep(3000);
	}

	void SimDummyTest()
	{
		std::cout << "Simulation Dummy started.\n" << std::flush;
		Sleep(100);

		SimulateInputPin(IN07_E_DUMMY, 1);
		Sleep(100);

		std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
		while (!data_out_cached[OUT43_E_DUMMY_IN_PROGRESS])
		{
			std::chrono::duration<double, std::milli> diff = std::chrono::high_resolution_clock::now() - start_time;
			if ((int)diff.count() > 5000) // timeout ?
			{
				std::cout << "Simulation: Getting no DUMMY_IN_PROGRESS signal. Simulation terminated.\n" << std::flush;
				return;
			}
			Sleep(10);
		}

		SimulateInputPin(IN22_SHIFT_TRIGGER, 1);
		Sleep(100);
		SimulateInputPin(IN22_SHIFT_TRIGGER, 0);
		Sleep(100);

		std::queue<std::string> dummy_parts;
		dummy_parts.push("111001");	// 6 Stations, 1 is trigger, 0 no trigger
		dummy_parts.push("111001");	// 2 good parts
		dummy_parts.push("100000");	// part 3+4 trigger iso only
		dummy_parts.push("100000");
		dummy_parts.push("010000");	// part 5+6 trigger L/Z
		dummy_parts.push("010000");
		dummy_parts.push("001000");	// part 7+8 trigger C
		dummy_parts.push("001000");
		dummy_parts.push("000001");	// part 9+10 trigger R1/R2
		dummy_parts.push("000001");

		const std::string no_trigger = "000000";

		std::deque<std::string> stations;
		for (int i = 0; i < 6; i++)
			stations.push_back(no_trigger); // fill stations with no trigger parts

		g_SimResultCounter = 0;
		int expectedResults = 0;
		while (!g_StopSim)
		{
			stations.pop_back();	// remove last part
			if (dummy_parts.size() > 0)
			{
				auto new_part = dummy_parts.front();
				stations.push_front(new_part);
				dummy_parts.pop();
			}
			else
			{
				if (stations.back() == no_trigger)
					break;

				stations.push_front(no_trigger); // add no trigger parts
			}

			expectedResults = 0;
			if (stations[0][0] == '1')
			{
				SimulateInputPin(InputPins::IN11_ISOLATION_TRIGGER, 1);
				expectedResults++;
			}
			if (stations[1][1] == '1')
			{
				SimulateInputPin(InputPins::IN12_IMPED_INDUCT_TRIGGER, 1);
			//	if (dummy_parts.size() > 6) // with part 5+6 we expect only one result
					expectedResults += 2;
			//	else
			//		expectedResults += 1;
			}
			if (stations[2][2] == '1')
			{
#if WITH_C
				SimulateInputPin(InputPins::IN14_CAPACITY_TRIGGER, 1);
				expectedResults++;
#endif
			}
			if (stations[5][5] == '1')
			{
				SimulateInputPin(InputPins::IN15_RESISTANCE_TRIGGER, 1);
				expectedResults += 2;
			}

			while (!g_StopSim)
			{
				if (g_SimResultCounter == expectedResults)
				{
					g_SimResultCounter = 0;
					Sleep(10);		// wait a while for the main loop is ready

					SimulateInputPin(IN22_SHIFT_TRIGGER, 1);
					Sleep(100);
					SimulateInputPin(IN22_SHIFT_TRIGGER, 0);
					Sleep(100);

					break;
				}
				Sleep(g_SimClock);
			}

		}

		SimulateInputPin(IN00_MACHINE_EMPTY, 1);
		SimulateInputPin(IN07_E_DUMMY, 0);
	}

	void SimCompensation()
	{
		std::cout << "Simulation Compensation started.\n" << std::flush;
		Sleep(100);
		SimulateInputPin(IN06_E_COMPENSATION, 1);
		Sleep(100);

		std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
		while (!data_out_cached[OUT42_COMPENSATION_IN_PROGRESS])
		{
			std::chrono::duration<double, std::milli> diff = std::chrono::high_resolution_clock::now() - start_time;
			if ((int)diff.count() > 5000) // timeout ?
			{
				std::cout << "Simulation: Getting no COMPENSATION_IN_PROGRESS signal. Simulation terminated.\n" << std::flush;
				return;
			}
			Sleep(10);
		}

		Sleep(1000);

		g_SimResultCounter = 0;
		int expectedResults = 0;
		int cnt = 0;
		while (!g_StopSim && cnt < 4)
		{
			// machine start up
			if (cnt == 0)
			{
				SimulateInputPin(InputPins::IN12_IMPED_INDUCT_TRIGGER, 1);		// open
				expectedResults = 2;
			}
			else if (cnt == 1)
			{
				SimulateInputPin(InputPins::IN12_IMPED_INDUCT_TRIGGER, 1);		// short
				SimulateInputPin(InputPins::IN14_CAPACITY_TRIGGER, 1);			// open
				expectedResults = 3;
			}
			else if (cnt == 2)
			{
				SimulateInputPin(InputPins::IN14_CAPACITY_TRIGGER, 1);			// short
				expectedResults = 1;
			}
			else if (cnt == 3)
			{
				SimulateInputPin(InputPins::IN15_RESISTANCE_TRIGGER, 1);		// only short
				expectedResults = 2;
			}

			while (!g_StopSim)
			{
				if (g_SimResultCounter == expectedResults)
				{
					g_SimResultCounter = 0;
					Sleep(100);		// wait a while for the main loop is ready
					cnt++;
					break;
				}
				Sleep(10);
			}
		}

		Sleep(2000);

		SimulateInputPin(IN00_MACHINE_EMPTY, 1);
		SimulateInputPin(IN06_E_COMPENSATION,  0);

		std::cout << "Simulation Compensation finished.\n" << std::flush;
	}

	SimIO::SimIO()
	{
		isInit = false;
	}

	SimIO::~SimIO()
	{
		CloseIO();
	}

	void SimIO::RegisterFunction(InputPins pin, func_t func)
	{
		registered_functions[pin] = func;
	}

	bool SimIO::GetPin(InputPins pin)
	{
		return data_in[pin];
	}

	void SimIO::SetPin(OutputPins pin)
	{
		if (!isInit)
			return;

		if (pin == OutputPins::OUT60_ISOLATION_PASS ||
			pin == OutputPins::OUT61_ISOLATION_FAIL_LIMIT_HIGH ||
			pin == OutputPins::OUT62_ISOLATION_FAIL_LIMIT_LOW ||
			pin == OutputPins::OUT63_ISOLATION_FAIL_SYSTEM)
		{
			g_SimResultCounter++;
			//std::cout << "Result Iso\n";
			SimulateInputPin(IN11_ISOLATION_TRIGGER, 0);
		}
		else if (pin == OutputPins::OUT64_INDUCTANCE_PASS ||
			pin == OutputPins::OUT65_INDUCTANCE_FAIL_LIMIT_HIGH ||
			pin == OutputPins::OUT66_INDUCTANCE_FAIL_LIMIT_LOW ||
			pin == OutputPins::OUT67_INDUCTANCE_FAIL_SYSTEM)
		{
			g_SimResultCounter++;
			//std::cout << "Result L\n";
			SimulateInputPin(IN12_IMPED_INDUCT_TRIGGER, 0);
		}
		else if (pin == OutputPins::OUT50_IMPEDANCE_PASS ||
			pin == OutputPins::OUT51_IMPEDANCE_FAIL_LIMIT_HIGH ||
			pin == OutputPins::OUT52_IMPEDANCE_FAIL_LIMIT_LOW ||
			pin == OutputPins::OUT53_IMPEDANCE_FAIL_SYSTEM)
		{
			g_SimResultCounter++;
			//std::cout << "Result Z\n";
			SimulateInputPin(IN12_IMPED_INDUCT_TRIGGER, 0);
		}
		else if (pin == OutputPins::OUT54_CAPACITY_PASS ||
			pin == OutputPins::OUT55_CAPACITY_FAIL_LIMIT_HIGH ||
			pin == OutputPins::OUT56_CAPACITY_FAIL_LIMIT_LOW ||
			pin == OutputPins::OUT57_CAPACITY_FAIL_SYSTEM)
		{
			g_SimResultCounter++;
			//std::cout << "Result C\n";
			SimulateInputPin(IN14_CAPACITY_TRIGGER, 0);
		}
		else if (pin == OutputPins::OUT70_RESISTANCE_R1_PASS ||
			pin == OutputPins::OUT71_RESISTANCE_R1_FAIL_LIMIT_HIGH ||
			pin == OutputPins::OUT72_RESISTANCE_R1_FAIL_LIMIT_LOW ||
			pin == OutputPins::OUT73_RESISTANCE_R1_FAIL_SYSTEM)
		{
			g_SimResultCounter++;
			//std::cout << "Result R1\n";
			SimulateInputPin(IN15_RESISTANCE_TRIGGER, 0);
		}
		else if (pin == OutputPins::OUT74_RESISTANCE_R2_PASS ||
			pin == OutputPins::OUT75_RESISTANCE_R2_FAIL_LIMIT_HIGH ||
			pin == OutputPins::OUT76_RESISTANCE_R2_FAIL_LIMIT_LOW ||
			pin == OutputPins::OUT77_RESISTANCE_R2_FAIL_SYSTEM)
		{
			g_SimResultCounter++;
			//std::cout << "Result R2\n";
			SimulateInputPin(IN15_RESISTANCE_TRIGGER, 0);
		}
		else
			data_out_cached[pin] = 1; // the other bits pass trough
	}

	void SimIO::ResetPin(OutputPins pin)
	{
		if (!isInit)
			return;

		data_out_cached[pin] = 0;
	}

	void SimIO::ResetAllPins()
	{
		if (!isInit)
			return;

		memset(&data_out_cached, 0, BITS_TO_WRITE * sizeof(char));
	}

	void SimIO::PrintDebugOutputPins()
	{
		char buff[256];
		char* buffPtr = buff;
		for (int i = 0; i < BITS_TO_WRITE; ++i)
		{
			if (i % 8 == 0)
			{
				sprintf(buffPtr, " ");
				buffPtr++;
			}
			sprintf(buffPtr, "%c", data_out_cached[i] ? '|' : '_');
			buffPtr++;
		}
		puts(buff);
		fflush(stdout);
	}

	void SimIO::InitIO()
	{
		if (isInit)
			CloseIO();

		IniFile ini(tsms_config::INI_FILE.c_str());
		g_SimParts = ini.ReadInteger("Simulation", "PartsPerLot", 100);
		g_SimClock = ini.ReadInteger("Simulation", "ClockMS", 200);

		for (int i = 0; i < BITS_TO_READ; ++i)
		{
			data_in[i] = 0;
			data_in_cached[i] = 0;
		}

		for (int i = 0; i < BITS_TO_WRITE; ++i)
		{
			data_out_cached[i] = 0;
		}

		isInit = true;

		logger::WriteLog("SimIO initialized");
	}

	void SimIO::CloseIO()
	{
		if (isInit)
		{
			StopSimulation();
			SimulateInputPin(IN01_SYSTEM_READY, 0);
			SimulateInputPin(IN20_LOT_IN_PROGRESS, 0);
			isInit = false;
		}
	}

	void SimIO::SimulateCompensation()
	{
		StopSimulation();
		g_isSimRunning = true;
		g_StopSim = false;
		simThread = std::thread(SimCompensation);
	}

	void SimIO::SimulateDummyTest()
	{
		StopSimulation();
		g_isSimRunning = true;
		g_StopSim = false;
		simThread = std::thread(SimDummyTest);
	}

	void SimIO::SimulateNewLOT()
	{
		StopSimulation();
		g_isSimRunning = true;
		g_StopSim = false;
		simThread = std::thread(SimLOT);
	}

	void SimIO::StopSimulation()
	{
		for (int i = 0; i < BITS_TO_READ; ++i)
		{
			data_in[i] = 0;
			data_in_cached[i] = 0;
		}

		g_isSimRunning = false;
		g_StopSim = true;
		if (simThread.joinable())
			simThread.join();
	}
}