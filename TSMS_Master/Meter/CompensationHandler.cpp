#include "CompensationHandler.h"
#include "../common/IniFile.h"
#include "../common/Logger.h"

#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>

#include "MeterGPIB.h"
#include "MeterTCP.h"
#include "MeterSim.h"


Meter* g_devices[tsms_config::NUM_DEVICES] = { 0 };

#define NUM_COMP_THREADS 3
enum eCompThreads { COMP_LZ, COMP_C, COMP_R };
std::thread compThreads[NUM_COMP_THREADS];
std::atomic_bool compensation_running[NUM_COMP_THREADS] = { false };

std::atomic_bool g_OpenCompReady_Lz = false;
std::atomic_bool g_OpenCompReady_C = false;

static Comp_Status g_status;

std::vector<std::pair<unsigned int, bool>> g_res_buffer;
std::mutex buffer_mutex;
std::mutex status_mutex;

void results_push_back(std::pair<unsigned int, bool> res)
{
	std::lock_guard<std::mutex> lk(buffer_mutex);
	g_res_buffer.push_back(res);
}

void set_status(eCompensations idx, eCompStatus stat)
{
	std::lock_guard<std::mutex> lk(status_mutex);
	g_status.status[idx] = stat;
}

void Run_Comp_LZ()
{
	compensation_running[COMP_LZ] = true;
	bool res = false;
	if (!g_OpenCompReady_Lz)
	{
		set_status(L_OPEN, COMP_RUN);
		res = g_devices[tsms_config::IM7581_1]->comp_open();
		set_status(L_OPEN, res ? COMP_OK : COMP_NOK);
		g_OpenCompReady_Lz = true;
	}
	else
	{
		set_status(L_SHORT, COMP_RUN);
		res = g_devices[tsms_config::IM7581_1]->comp_short();
		set_status(L_SHORT, res ? COMP_OK : COMP_NOK);
	}

	results_push_back(std::pair(tsms_config::L, res));
	results_push_back(std::pair(tsms_config::Z1, res));

	compensation_running[COMP_LZ] = false;
}

void Run_Comp_C()
{
	compensation_running[COMP_C] = true;
	bool res = false;
	if (!g_OpenCompReady_C)
	{
		set_status(C_OPEN, COMP_RUN);
#if WITH_C
		res = g_devices[tsms_config::IM7581_2]->comp_open();
		set_status(C_OPEN, res ? COMP_OK : COMP_NOK);
#else
		Sleep(10000);
		set_status(C_OPEN, COMP_OK);
		res = true;
#endif
		g_OpenCompReady_C = true;
	}
	else
	{
		set_status(C_SHORT, COMP_RUN);
#if WITH_C
		res = g_devices[tsms_config::IM7581_2]->comp_short();
		set_status(C_SHORT, res ? COMP_OK : COMP_NOK);
#else
		Sleep(10000);
		set_status(C_SHORT, COMP_OK);
		res = true;
#endif
	}

	results_push_back(std::pair(tsms_config::C1, res));

	compensation_running[COMP_C] = false;
}

void Run_Comp_R()
{
	compensation_running[COMP_R] = true;

	bool res = false;
	set_status(R1_SHORT, COMP_RUN);
	res = g_devices[tsms_config::RES2329_1]->comp_short();
	set_status(R1_SHORT, res ? COMP_OK : COMP_NOK);
	results_push_back(std::pair(tsms_config::R1, res));

	res = false;
	set_status(R2_SHORT, COMP_RUN);
	res = g_devices[tsms_config::RES2329_2]->comp_short();
	set_status(R2_SHORT, res ? COMP_OK : COMP_NOK);
	results_push_back(std::pair(tsms_config::R2, res));

	compensation_running[COMP_R] = false;
}

CompensationHandler::CompensationHandler(io::DIOHandler* io) : Handler("Compensation")
{
	_pIo = io;
	_hasError = false;
	g_OpenCompReady_Lz = false;
	g_OpenCompReady_C = false;
}

CompensationHandler::~CompensationHandler()
{
	Close();
}

bool CompensationHandler::Init()
{
	Close();
	Reset();

	IniFile ini(tsms_config::INI_FILE.c_str());
	bool simMeter = ini.ReadBoolean("Meter", "SimulateMeter", 0);
	int simDev = ini.ReadInteger("Meter", "SimulatedDeviation", 5);

	for (unsigned int i = 0; i < tsms_config::NUM_DEVICES; i++)
	{
		if (i == tsms_config::SM7110_1)		// we don't need Iso in moment
			continue;

		DeviceConfig dev_config;
		if (!dev_config.ReadConfigFile(tsms_config::DeviceNames[i]))
		{
			_handler_status = eHandlerStatus::H_ERROR;
			_last_error = "Read config file failed";
			return false;
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

		g_devices[i]->comp_init();
	}

	_handler_status = eHandlerStatus::H_SUCCESS;
	logger::WriteLog(_msg_prefix + " initialized");
	return true;
}

void CompensationHandler::Close()
{
	if (_handler_status == eHandlerStatus::H_CLOSED)
		return;

	for (unsigned int i = 0; i < tsms_config::NUM_DEVICES; i++)
	{
		if (g_devices[i] != 0)
		{
			g_devices[i]->clean_up();
			delete g_devices[i];
			g_devices[i] = 0;
		}
	}
	_handler_status = eHandlerStatus::H_CLOSED;
	logger::WriteLog(_msg_prefix + " closed");
}

bool CompensationHandler::ThreadsFinshed()
{
	bool comp_finished = true;
	for (int i = 0; i < NUM_COMP_THREADS; i++)
		if (compensation_running[i])
			comp_finished = false;
	return comp_finished;
}

void CompensationHandler::Reset()
{
	for (int i = 0; i < NUM_COMP_THREADS; i++)
		compensation_running[i] = false;

	tsms_cnt.Reset();
	g_res_buffer.clear();
	g_OpenCompReady_Lz = false;
	g_OpenCompReady_C = false;
	_hasError = false;

	for (unsigned short i = 0; i < tsms_config::NUM_COMPS; i++)
		g_status.status[i] = COMP_NONE;

	g_status.names[L_SHORT] = "Short L/Z (IM7581 1)";
	g_status.names[L_OPEN] = "Open L/Z (IM7581 1)";
	g_status.names[C_SHORT] = "Short C (IM7581 2)";
	g_status.names[C_OPEN] = "Open C (IM7581 2)";
	g_status.names[R1_SHORT] = "Short R1 (RES2329 1)";
	g_status.names[R2_SHORT] = "Short R2 (RES2329 2)";
}

Comp_Status CompensationHandler::Update()
{
	g_status.init = _handler_status == eHandlerStatus::H_SUCCESS;

	if (ThreadsFinshed() && g_res_buffer.size() > 0)
	{
		for (int i = 0; i < NUM_COMP_THREADS; i++)
			if (compThreads[i].joinable())
				compThreads[i].join();

		for (auto& res : g_res_buffer)
		{
			Evaluation eval;
			if (res.second)
			{
				eval = Evaluation::PASS;
				tsms_cnt.res_pass[res.first]++;
			}
			else
			{
				eval = Evaluation::FAIL;
				tsms_cnt.res_fail[res.first]++;
				_hasError = true;
			}

			_pIo->SetResultPins(res.first, eval);
		}
		g_res_buffer.clear();
	}

	return g_status;
}

void CompensationHandler::Trigger(io::eTrigger trigger)
{
	switch (trigger)
	{
	case io::eTrigger::IOS_TRIG:
		std::cout << "Comp Trigger Iso\n" << std::flush;
		// nothing to do in moment
		break;

	case io::eTrigger::IMPED_INDUCT_TRIG:
		std::cout << "Comp Trigger L+Z\n" << std::flush;
		if (!compensation_running[eCompThreads::COMP_LZ])
			compThreads[eCompThreads::COMP_LZ] = std::thread(Run_Comp_LZ);	
		break;

	case io::eTrigger::CAP_TRIG:
		std::cout << "Comp Trigger C\n" << std::flush;
		if (!compensation_running[eCompThreads::COMP_C])
			compThreads[eCompThreads::COMP_C] = std::thread(Run_Comp_C);
		break;

	case io::eTrigger::RESISTANCE_TRIG:
		std::cout << "Comp Trigger R1+R2\n" << std::flush;
		if (!compensation_running[eCompThreads::COMP_R])
			compThreads[eCompThreads::COMP_R] = std::thread(Run_Comp_R);
		break;
	}
}


