#include "StateMachine.h"
#include "const.h"
#include "DummyBuffer.h"
#include "ResultBuffer.h"
#include "PAThandler.h"

#include "db/DBHandler.h"
#include "Machine/ComHandler.h"
#include "Meter/DeviceHandler.h"
#include "Meter/CompensationHandler.h"
#include "common/DateTime.h"
#include "common/Logger.h"
#include "common/IniFile.h"
#include "Machine/DIOHandler.h"

#include <iostream>
#include <conio.h>
#include <chrono>
using namespace std::chrono;	//  for tracking time with varying degrees of precision

std::chrono::high_resolution_clock::time_point start_time;
std::chrono::high_resolution_clock::time_point end_time;
int g_last_cycle_time = 0;

void OnTriggerEvent(io::eTrigger trigger);		//	event for triggering measurement instruments

static eStates g_state = eStates::READY_FOR_NEW_LOT;	// state of TSMS application
static eStates g_previous_state = g_state;	// remembers the previous state

static std::vector<MeasuredValue> g_results; // measurement result from meters after one meas cycle
static ResultBuffer part_buffer;
static DummyBuffer dummy_part_buffer;
static lot::LOT_Data g_lot;
static TSMS_Setup g_setup;
static PATConfig g_pat_setup;
static PAThandler pat_handler;

io::DIOHandler g_io(OnTriggerEvent);	// digital I/O handler
device::DeviceHandler g_devices;
com::ComHandler g_com;
db::DBHandler g_db;

CompensationHandler g_comp(&g_io);

std::atomic <time_t> g_last_dummy_time = 0;		// time for the last dummy test
std::atomic <time_t> g_last_comp_time = 0;		// time for the last compensation
std::atomic <int> g_dummy_valid_sec = 24 * 60 * 60;		// dummy test is valif for 24 hours
std::atomic <int> g_comp_valid_sec = 24 * 60 * 60;		// compenstaion is valif for 24 hours

// calculates remaining time to the next dummy test in hour
unsigned short GetRestDummyHour()
{
	int last_dummy_sec = GetTimeDiffToNow(g_last_dummy_time);
	if (last_dummy_sec < g_dummy_valid_sec)
	{
		return (g_dummy_valid_sec - last_dummy_sec) / 3600;
	}
	return 0;
}

// calculates remaining time to the next compensation in hour
unsigned short GetRestCompensationHour()
{
	int last_comp_sec = GetTimeDiffToNow(g_last_comp_time);
	if (last_comp_sec < g_comp_valid_sec)
	{
		return (g_comp_valid_sec - last_comp_sec) / 3600;
	}
	return 0;
}

#if GUI
#include <thread>
#include "UI/UIHandler.h"
ui::UIHandler g_gui;	// separate thread for the cyclical GUI communication

std::thread ui_thread;
std::atomic<bool> ui_thread_exit = false;
std::atomic<bool> ui_cmd_init_device = false;
std::atomic<bool> ui_cmd_init_db = false;
std::atomic<bool> ui_cmd_init_rs232 = false;
std::atomic<bool> ui_cmd_init_io = false;
std::atomic<bool> ui_cmd_restart_pat = false;

// cyclically sends the system status of this Master application to the GUI application
// also receives command form GUI application
// cycle time min 100ms
void SendCyclicalToUI()
{
	while (!ui_thread_exit)
	{
		g_gui.sendMachineSignals(g_io);		//status of the digital io lines
		Sleep(50);
		ui::TSMS_Status status;
		status.StateMachine = (char)g_state;
		status.Measurements = g_devices.GetStatus();
		status.DigitalIO = g_io.GetStatus();
		status.Database = g_db.GetStatus();
		status.RS232 = g_com.GetStatus();
		status.rest_dummy_hours = GetRestDummyHour();
		status.rest_comp_hours = GetRestCompensationHour();
		status.rest_pat_samples = pat_handler.rest_pat_sample;
		g_gui.sendModuleStatus(status); // sends TSMS status
		Sleep(50);
		ui::CmdFromUI cmd;
		if (g_gui.recvUICommands(cmd))		//receives commands from the GUI application (X + command char)
		{
			switch ((tsms_config::eTelFromUI)cmd.Cmd)
			{
			case tsms_config::TEL_UI_IS_STARTED:
				g_gui.sendSetup(g_setup);
				Sleep(50);
				g_gui.sendLotData(g_lot);
				break;
			case tsms_config::TEL_UI_INIT_RS232:	//GUI command to reinitialize RS232
				if (!ui_cmd_init_rs232)
					ui_cmd_init_rs232 = true;
				break;
			case tsms_config::TEL_UI_INIT_DB:	//GUI command to reinitialize database
				if (!ui_cmd_init_db)
					ui_cmd_init_db = true;
				break;
			case tsms_config::TEL_UI_INIT_DEVICES:	//GUI command to reinitialize devices
				if (!ui_cmd_init_device)
					ui_cmd_init_device = true;
				break;
			case tsms_config::TEL_UI_INIT_IO:	//GUI command to reinitialize digital IO
				if (!ui_cmd_init_io)
					ui_cmd_init_io = true;
				break;
			case tsms_config::TEL_UI_RESTART_PAT:	//GUI command to restart PAT sampling
				pat_handler.StartPAT(g_devices.GetLastStation(), g_lot);
				break;
			case 'z':
				g_state = eStates::SHUTDOWN_CMD;	//GUI command to shutdown the Master application (TSMS_Shutdown)
				break;
			default:
				logger::WriteLog("Wrong command received: " + cmd.Cmd);
				break;
			}
		}
	}
}
#endif

void DBErrorHandler()
{
	//when error is occurred during db operation
	logger::WriteLog(g_db.GetLastError());
#if GUI
	g_gui.sendUserMessage(g_db.GetLastError(), true);
#endif
	g_io.SetLotInProgress(false);
	g_previous_state = g_state;
	g_state = DB_ERROR;
}

// callback from DIOHandler
// do not call large functions here !
void OnTriggerEvent(io::eTrigger trigger)
{
	//std::cout << "Trigger event rised \n " << std::flush;

	if (trigger == io::eTrigger::SHIFT_TRIG && g_state != DB_ERROR)		// SHIFT-TRIGGER signal from handler -> parts has moved to next station
	{
		part_buffer.AddNewEntry(g_last_cycle_time);		// adds a new part entry to the buffer with the last part index
		if (pat_handler.UpdateRequest)
			if (pat_handler.Shift()) g_gui.sendPATUpdate(g_lot.test_config); // increments station number for signaling limit update request

		if (part_buffer.GetNumEntries() > tsms_config::NUM_STATIONS)	// when buffer was full of parts before adding
		{
			auto entry = part_buffer.GetAndRemoveFirstPart();	//removes a part from the buffer that has just left the last test station (TS #5)
			if (std::get<2>(entry).size() > 0)
			{
				// if PAT process is active then add to the PAT
				if (pat_handler.pat_started && pat_handler.AddValues(std::get<2>(entry), g_lot.test_config))
				{
					// if PAT calculation is finished the PAT table update is also needed
					// saves part and PAT data to the database asynchronously
					// GetLastPartNum() function returns the part index at the first station that will be measured at the measurement next cycle
					// so it will be the first part that uses the new narrowd PAT tolerances
					if (g_db.ValueTable_AddPart_and_UpdatePAT_Async(g_lot.header.lot_nr, entry, part_buffer.GetLastPartNum(), g_lot) != H_SUCCESS)		// saves the part info into the database asynchronously
						DBErrorHandler();
				}
				else
				{
					// saves part data to the database asynchronously
					if (g_db.ValueTable_AddPart_Async(g_lot.header.lot_nr, entry) != H_SUCCESS)		// saves the part info into the database asynchronously
						DBErrorHandler();
				}
			}
		}

		//Updating PAT narrowed tolerances
		if (pat_handler.UpdateRequest) g_devices.UpdatePATLimits(g_lot.test_config, pat_handler.UpdateStation);
		
		return;
	}
	
	if (trigger == io::eTrigger::INIT_TRIGGER)	//INITIALIZING signal from handler
	{
		logger::WriteLog("Signal: Init trigger");
		//part_buffer.ResetParts();	
		return;
	}
	
	// checks the trigger lines and starts each measurements
	if (g_state == eStates::READY_TO_MEASURE ||	g_state == eStates::MEASUREMENT_RUNNING)
	{
		switch (trigger)
		{
		case io::eTrigger::IOS_TRIG:
			if (g_lot.header.is_dummy)
				std::cout << "Trigger ISO\n" << std::flush;
			g_devices.StartMeasurement(tsms_config::SM7110_1);
			break;

		case io::eTrigger::IMPED_INDUCT_TRIG:
			if (g_lot.header.is_dummy)
				std::cout << "Trigger INDUCT\n" << std::flush;
			g_devices.StartMeasurement(tsms_config::IM7581_1);
			break;

		case io::eTrigger::CAP_TRIG:
#if WITH_C
			if (g_lot.header.is_dummy)
				std::cout << "Trigger CAP\n" << std::flush;
			g_devices.StartMeasurement(tsms_config::IM7581_2);
#endif
			break;

		case io::eTrigger::RESISTANCE_TRIG:
			if (g_lot.header.is_dummy)
				std::cout << "Trigger RES\n" << std::flush;
			g_devices.StartMeasurement(tsms_config::RES2329_1);
			g_devices.StartMeasurement(tsms_config::RES2329_2);
			break;		
		}

		if (g_state == eStates::READY_TO_MEASURE) // starting point of the waiting for measurements
		{
			start_time = std::chrono::high_resolution_clock::now();
			g_state = eStates::MEASUREMENT_RUNNING;
		}
	}
	else if (g_state == eStates::COMPENSATION)
	{
		g_comp.Trigger(trigger);
	}
	else
	{
		logger::WriteLog("ERROR: Not ready for new measurement");
	}
}

StateMachine::StateMachine()
{
	_measurement_counter = 0;
}

StateMachine::~StateMachine()
{
}

void StateMachine::PrintCurrentMeasurement()
{
	printf("Current: %3d - %3d ms  ", _measurement_counter, g_last_cycle_time);

	const unsigned int n = tsms_config::NUM_MEASUREMENTS;
	std::vector<double> values(n);
	std::vector<int> times(n);
	for (auto& mv : g_results)
	{
		values[mv.GetID()] = mv.GetValue();
		times[mv.GetID()] = mv.time_ms;
	}

	for (int i = 0; i < n; i++)
	{
		if (values[i] == 0.0 && times[i] == 0)
			std::cout << "-----------\t";
		else
			std::cout << values[i] << "(" << times[i] << "ms)\t";
	}
	std::cout << "\n" << std::flush;
}

// adds measurement results after one cycle to the buffer
void StateMachine::ProcessingMeasurementResults()
{
	_measurement_counter++;

	if (!g_lot.header.is_dummy)
	{	
		// check for a new part 
		for (auto& value : g_results)
		{
			if (value.station_nr == 0)		//  only when station 0 was triggered we have a new part
				part_buffer.NewPart();		//	increments the part index but new buffer entry will be added in the digitalIO event
		}
	}
	else
	{
		part_buffer.NewPart();	// dummy has always a new part
	}
	
	for (auto& value : g_results)
	{
		if (g_lot.header.is_dummy)
			dummy_part_buffer.AddValue(value);
		
		part_buffer.AddValue(value, g_lot.header.is_dummy);		
		g_io.SetResultPins(value.GetID(), value.eval);
	}
}

bool StateMachine::UpdateLotTable()
{
	if (g_db.LotTable_Update(g_lot) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}
	return true;
}

void StateMachine::SendMeasurementToUI()
{
#if GUI
	for (auto& value : g_results)
	{
		g_gui.sendValue(value);
	}
	g_gui.sendCounter(part_buffer.GetCounter());
#endif
}

bool StateMachine::ReadDummyCompTime()
{
	g_last_dummy_time = 0;
	time_t time;
	if (g_db.LotTable_GetLastTime(tsms_config::DummyTestDataSet, time) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}
	g_last_dummy_time = time;

	if (g_db.LotTable_GetLastTime(tsms_config::CompensationTestplan, time) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}
	g_last_comp_time = time;
	return true;
}

// continues an unfinished LOT
bool StateMachine::StartExistingLot(long long lot_nr)
{
	logger::WriteLog("Continue lot '" + std::to_string(lot_nr) + "'.");

	if (g_db.LotTable_GetLot(lot_nr, g_lot) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}

	logger::WriteLog("LOT data loaded from database");

	if (g_db.PatTable_GetLast(lot_nr, g_lot, g_pat_setup) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}

	logger::WriteLog("PAT data loaded from database");

	unsigned int LastPartNumber = 0;
	if (g_db.ValueTable_GetLastPartNum(g_lot.header.lot_nr, LastPartNumber) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}
	_measurement_counter = 0;
	part_buffer.SetCurrentPartNum(LastPartNumber + 1);
	part_buffer.SetCounter(g_lot.tsms_cnt);

#if 0
	if (g_db.BufferTable_ReadBuffer(part_buffer) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}
#endif

	g_lot.lot_state = lot::eLotState::STARTED;

	if (g_db.LotTable_UpdateStatus(g_lot.header.lot_nr, GetStateAsString(g_lot.lot_state)) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}

	return true;
}

bool StateMachine::StartNewLot(lot::LotHeader& lot_header)
{
	logger::WriteLog("Start new lot");

	_measurement_counter = 0;
	part_buffer.ResetParts();
	part_buffer.SetCurrentPartNum(1);
	
	g_lot.SetHeader(lot_header);

	if (g_db.TestTable_GetConfig(g_lot.header.test_plan, g_lot.test_config) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}

	g_lot.SetStartTimeToNow();
	g_lot.CreateNewLaserMark();
	g_lot.lot_state = lot::STARTED;

	if (g_db.LotTable_Insert(g_lot) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}

	// setup LOT according the ini file settings
	g_lot.pat_config = g_pat_setup;
	// start PAT narrowing automatically when a new LOT is started
	if (!g_lot.pat_config.uselastlot)
	{
		// starts a new sampling cycle
		pat_handler.StartPAT(g_devices.GetLastStation(), g_lot);
	}
	else
	{
		bool isloaded=false;
		// tries to use pat from last lot with same product number that started in last 24h
		if (g_db.PatTable_Copy_From_LastsameType(g_lot, g_pat_setup, isloaded) != H_SUCCESS)
		{
			ChangeState(eStates::DB_ERROR);
			return false;
		}

		if (isloaded)	//	if successfully loaded last pat from database
		{
			// saves a new entry to the PAT table
			if (g_db.PatTable_Insert(g_lot.header.lot_nr, 1, g_lot.pat_config, g_lot.test_config.configs))
			{
				ChangeState(eStates::DB_ERROR);
				return false;
			}
		}
		else
		{
			// starts a new sampling cycle
			pat_handler.StartPAT(g_devices.GetLastStation(), g_lot);
		}
	}

	return true;
}

bool StateMachine::LotStart()
{
	bool printLabel = false;

	logger::WriteLog("Lot start ...");

	if (GetRestCompensationHour() < 1)
	{
		std::string msg = "Compensation not valid";
		logger::WriteLog(msg);
#if GUI
		g_gui.sendUserMessage(msg, true);
#endif
		return false;
	}

	if (GetRestDummyHour() < 1)
	{
		std::string msg = "Dummy test not valid";
		logger::WriteLog(msg);
#if GUI
		g_gui.sendUserMessage(msg, true);
#endif
		return false;
	}

	lot::LotHeader machine_data;
	if (!g_com.ReadLotStart(machine_data))
	{
		ChangeState(eStates::RS232_ERROR);
		return false;
	}

	logger::WriteLog("Lot data: " + std::to_string(machine_data.lot_nr) + ", " + machine_data.test_plan);

	bool lot_exists = false;
	if (g_db.LotTable_LotExists(machine_data.lot_nr, lot_exists) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}

	if (lot_exists)
	{
		if (g_setup.SimMode == 0)
		{
			lot::eLotState lot_status = lot::eLotState::UNKNOWN;
			if (g_db.LotTable_GetLotStatus(machine_data.lot_nr, lot_status) != H_SUCCESS)
			{
				ChangeState(eStates::DB_ERROR);
				return false;
			}
			if (lot_status == lot::eLotState::FINISHED)
			{
				std::string msg = "Lot '" + std::to_string(machine_data.lot_nr) + "' already finished\n";
				logger::WriteLog(msg);
#if GUI
				g_gui.sendUserMessage(msg, true);
#endif
				return false;
			}
		}
		if (!StartExistingLot(machine_data.lot_nr))
			return false;
	}
	else
	{
		if (!StartNewLot(machine_data))
			return false;
		printLabel = true;
	}

	if (!g_devices.SetTestConfig(g_lot.test_config))
	{
		ChangeState(eStates::DEVICE_ERROR);
		return false;
	}

	g_io.SetLotInProgress(true);		// after this signal the machine waits for laser data

	if (!g_com.WriteLaserData(g_lot.laser_mark))
	{
		ChangeState(eStates::RS232_ERROR);
		return false;
	}

#if GUI
	g_gui.sendLotData(g_lot);
	g_gui.sendCounter(part_buffer.GetCounter());

	if (printLabel)
	{
		Sleep(100);		// lot data should be received before printing a label
		g_gui.sendUserMessage(tsms_config::MessagePrintLabel, false);
	}
#endif

	return true;
}

bool StateMachine::LotStop()
{
	logger::WriteLog("Lot stop");
	g_lot.SetStopTimeToNow();

	MachineCounter newMachineCounter;
	if (!g_com.ReadLotEnd(newMachineCounter))
	{
		ChangeState(eStates::RS232_ERROR);
		return false;
	}

	g_lot.machine_cnt.Add(newMachineCounter);
	g_lot.tsms_cnt.Copy(part_buffer.GetCounter());

	if (g_db.ValueTable_EmptyBuffer(g_lot.header.lot_nr, part_buffer) != H_SUCCESS)
		ChangeState(eStates::DB_ERROR);

	if (!UpdateLotTable())
		return false;
#if GUI
	g_gui.sendUserMessage(tsms_config::MessagePrintReport, false);
#endif

	return true;
}

bool StateMachine::LotDiscard()
{
	logger::WriteLog("Lot discard");
	if (!g_com.ReadLotDiscard())
	{
		ChangeState(eStates::RS232_ERROR);
		return false;
	}
	
	logger::WriteLog("Delete lot '" + std::to_string(g_lot.header.lot_nr) + "' from database.");
	
	if (g_db.LotTable_DeleteLot(g_lot.header.lot_nr) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}
	return true;
}

bool StateMachine::EDummyStart()
{
	logger::WriteLog("Start E-Dummy test");
	
	if (GetRestCompensationHour() < 1)
	{
		std::string msg = "Compensation not valid. First start Compensation.";
		logger::WriteLog(msg);
#if GUI
		g_gui.sendUserMessage(msg, true);
#endif
		return false;
	}
	
	g_lot.Reset();
	dummy_part_buffer.Reset();

	lot::LotHeader dummyHeader;
	dummyHeader.is_dummy = true;
	dummyHeader.line_id = "";
	dummyHeader.test_plan = tsms_config::DummyTestDataSet;
	dummyHeader.lot_nr = tsms_config::FristInternalLotNumber;

	if (g_db.LotTable_GetNextFreeInternalLotNumber(dummyHeader.lot_nr) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}

	g_lot.SetHeader(dummyHeader);
	g_lot.SetStartTimeToNow();
	g_lot.lot_state = lot::STARTED;

	if (g_db.LotTable_Insert(g_lot) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}

	if (g_db.TestTable_GetConfig(g_lot.header.test_plan, g_lot.test_config) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}

	if (!g_devices.SetTestConfig(g_lot.test_config))
	{
		ChangeState(eStates::DEVICE_ERROR);
		return false;
	}

#if GUI
	g_gui.sendLotData(g_lot);
#endif

	g_io.SetEDummyInProgress(true);
	logger::WriteLog("E-Dummy ready.");
	return true;
}

bool StateMachine::EDummyStop()
{
	logger::WriteLog("Stop E-Dummy test");

	if (g_db.ValueTable_EmptyBuffer(g_lot.header.lot_nr, part_buffer) != H_SUCCESS)
		ChangeState(eStates::DB_ERROR);

	g_lot.SetStopTimeToNow();

	bool success = dummy_part_buffer.Evaluation();

	std::string msg;
	if (success)
		msg = "E-Dummy test finished successfully.\n";
	else
		msg = "E-Dummy test finished with error";

	logger::WriteLog(msg);
#if GUI
	g_gui.sendUserMessage(msg);
#endif

	if (success)
	{
		g_lot.lot_state = lot::FINISHED;
		g_lot.tsms_cnt.good_parts = dummy_part_buffer.GetCounter();

		if (!UpdateLotTable())
			return false;
	}

	if (!ReadDummyCompTime())
		return false;

	return true;
}

bool StateMachine::CompensationStart()
{
	logger::WriteLog("Start compensation");
	g_lot.Reset();

	lot::LotHeader compHeader;
	compHeader.is_dummy = false;
	compHeader.line_id = "";
	compHeader.test_plan = tsms_config::CompensationTestplan;
	compHeader.lot_nr = tsms_config::FristInternalLotNumber;

	if (g_db.LotTable_GetNextFreeInternalLotNumber(compHeader.lot_nr) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}

	g_lot.SetHeader(compHeader);
	g_lot.SetStartTimeToNow();
	g_lot.lot_state = lot::STARTED;

	if (g_db.LotTable_Insert(g_lot) != H_SUCCESS)
	{
		ChangeState(eStates::DB_ERROR);
		return false;
	}

	_measurement_counter = 0;
	g_devices.Close(); // compensation create his own meter instances

	if (!g_comp.Init())
	{
		logger::WriteLog(g_comp.GetLastError());
#if GUI
		g_gui.sendUserMessage(g_comp.GetLastError(), true);
#endif
		return false;
	}

#if GUI
	g_gui.sendLotData(g_lot);
#endif

	g_io.SetECompensationInProgress(true);
	return true;
}

bool StateMachine::CompensationStop()
{
	logger::WriteLog("Stop compensation");
	g_lot.SetStopTimeToNow();
	g_comp.Close();
	if (g_comp.Success())
		g_lot.lot_state = lot::FINISHED;

	g_lot.tsms_cnt = g_comp.tsms_cnt;

	if (!UpdateLotTable())
		return false;

	if (!ReadDummyCompTime())
		return false;

	return true;
}

void StateMachine::Reset()
{
	logger::WriteLog("Reset");

	g_io.SetLotInProgress(false);
	g_io.SetEDummyInProgress(false);
	g_io.SetECompensationInProgress(false);

	_measurement_counter = 0;
	g_lot.Reset();
	part_buffer.ResetCounter();
	part_buffer.ResetParts();
	dummy_part_buffer.Reset();
#if GUI
	g_gui.sendLotData(g_lot);
	g_gui.sendCounter(part_buffer.GetCounter());
#endif
}

std::pair<bool, bool> StateMachine::ContinueUnfinishedLot()
{
	bool result = false;
	bool error = false;

	if (g_setup.SimMode == 0 && !g_io.IsLotInProgress())
		return { result, error };

	long long unfinshed_lot_num = 0;
	if (g_db.LotTable_GetLastStartedLot(unfinshed_lot_num) == H_SUCCESS && unfinshed_lot_num > 0)
	{
		logger::WriteLog("Last unfinshed lot in database: " + std::to_string(unfinshed_lot_num));
		
		if (StartExistingLot(unfinshed_lot_num))
		{
			if (!g_devices.SetTestConfig(g_lot.test_config))
			{
				ChangeState(eStates::DEVICE_ERROR);
				error = true;
			}
			else
			{
				g_io.SetInitPin(true);
				Sleep(100);
				g_io.SetLotInProgress(true);
#if GUI
				g_gui.sendLotData(g_lot);
				g_gui.sendCounter(part_buffer.GetCounter());
#endif
				result = true;
			}
		}
		else
			error = true;
	}
	return { result, error };
}

void StateMachine::Startup()
{
	// reads settings from ini file
	IniFile ini(tsms_config::INI_FILE.c_str());
	// reads general settings
	g_setup.machine_name = ini.ReadString("Common", "MachineName", "ACT1210-PA5");
	g_setup.CheckUnfinshedLot = ini.ReadBoolean("Common", "CheckUnfinishedLot", 0);
	g_setup.PrintBufferContent = ini.ReadBoolean("Common", "PrintBufferContent", 0);
	g_dummy_valid_sec = 60 * 60 * ini.ReadInteger("DummyTest", "ValidTimeHours", 24);
	g_comp_valid_sec = 60 * 60 * ini.ReadInteger("Compensation", "ValidTimeHours", 24);
	g_setup.SimMode = ini.ReadInteger("Simulation", "Mode", 0);
	// reads PAT related settings
	g_pat_setup.mode = ini.ReadInteger("PAT", "Mode", 0);
	g_pat_setup.samplecnt = ini.ReadInteger("PAT", "SampleCnt", 50);
	g_pat_setup.ll_sigma = ini.ReadInteger("PAT", "UL_Sigma", 6);
	g_pat_setup.ul_sigma = ini.ReadInteger("PAT", "LL_Sigma", 6);
	g_pat_setup.uselastlot = ini.ReadBoolean("PAT", "UseLastLot", false);
	// reads whether measurement parameters enabled for narrowing
	pat_handler.NumOf_PAT_Measurements = 0;
	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++) {
		pat_handler.PAT_Measurements[i] = ini.ReadBoolean("PAT", tsms_config::MeasurementNames[i].c_str(), false);
		if (pat_handler.PAT_Measurements[i]) pat_handler.NumOf_PAT_Measurements++; // counts params for narrowing
	}

	std::string log_path = ini.ReadString("Common", "LogPath", "./");
	logger::setPath(log_path);
	logger::WriteLog("----------------------------------------------------------------------------------------------", false);
	logger::WriteLog("TSMS start up ...");

	g_lot.testing_machine = g_setup.machine_name;

#if GUI
	if (!g_gui.Init())	// reads settings and initialize UPD communication
	{
		logger::WriteLog("Init UI connection failed!");
		ChangeState(eStates::EXIT);
		return;
	}
	g_gui.sendSetup(g_setup);	// sends TSMS setup to GUI application
	ui_thread = std::thread(SendCyclicalToUI);	// separate thread for the cyclical GUI communication
#endif

	if (!g_db.Init() || !g_devices.Init() || !g_com.Init() || !g_io.Init())		// when start is not possibele because of critical errors
	{
		logger::WriteLog("Start up failed!");
		ChangeState(eStates::EXIT);
		return;
	}

	if (!g_devices.WriteConfigToDatabase(g_db))		// always recreate the a config table and writes measurement device configuration to the database
	{
		logger::WriteLog(g_db.GetLastError());
		logger::WriteLog("Start up failed!");
		ChangeState(eStates::EXIT);
		return;
	}

	if (!ReadDummyCompTime())	// reads last dummy and compensation time from the LOT table
	{
		logger::WriteLog(g_db.GetLastError());
		logger::WriteLog("Start up failed!");
		ChangeState(eStates::EXIT);
		return;
	}

	g_io.SetSystemReady(true);
	logger::WriteLog("TSMS started successfully.");

	bool continueUnfinshedLot = false;
	if (g_setup.CheckUnfinshedLot)
	{
		bool error = false;
		std::tie(continueUnfinshedLot, error) = ContinueUnfinishedLot();
		if (error)
			return;  // don't change state if we have an error
	}

	if (continueUnfinshedLot)
	{
		ChangeState(eStates::WAIT_UNTIL_LOT_IN_PROGRESS);
		if (g_setup.SimMode > 0)
			g_io.SimulateNewLOT();
	}
	else
		ChangeState(eStates::READY_FOR_NEW_LOT);
}

void StateMachine::Shutdown()
{
	logger::WriteLog("TSMS shutdown ...");

	g_db.Close();
	g_com.Close();
	g_io.Close();
	g_devices.Close();

#if GUI
	ui_thread_exit = true;
	if (ui_thread.joinable())
		ui_thread.join();
	g_gui.Close();
#endif

	logger::WriteLog("TSMS stopped");
}

void StateMachine::ContinueAfterError()
{
	g_io.SetLotInProgress(true);
	if (g_setup.SimMode > 0)
	{
		ChangeState(eStates::READY_FOR_NEW_LOT);
	}
	ChangeState(g_previous_state);
}

void StateMachine::CheckStartConditions()
{
	bool wait = false;
	if (g_io.IsLotStart())
	{
		if (LotStart())
			ChangeState(eStates::WAIT_UNTIL_LOT_IN_PROGRESS);
		else
			wait = true;
	}
	else if (g_io.IsEDummy())
	{
		if (EDummyStart())
			ChangeState(eStates::READY_TO_MEASURE);
		else
			wait = true;
	}
	else if (g_io.IsECompensation())
	{
		if (CompensationStart())
			ChangeState(eStates::COMPENSATION);
		else
			wait = true;
	}

	if (wait)
		Sleep(5000);	// not possible to start -> wait for the machine to reset the bit
}

void StateMachine::CheckEndConditions()
{
	// check machine status ?
	if (!g_io.IsMachineReady())
	{
		//todo: What should I do?
	}

	if (g_io.IsMachineEmpty())
	{
		if (g_io.IsLotDiscard())
		{
			if (LotDiscard())
				ChangeState(eStates::READY_FOR_NEW_LOT);
			return;
		}

		if (g_io.IsLotEnd() || g_io.IsLotPause())
		{
			if (g_io.IsLotPause())
				g_lot.lot_state = lot::PAUSED;
			else
				g_lot.lot_state = lot::FINISHED;

			if (LotStop())
				if (g_setup.SimMode == 0)
					ChangeState(eStates::READY_FOR_NEW_LOT);
				else
					ChangeState(eStates::EXIT);		// exit program in simulation mode
		}

		if (g_lot.header.is_dummy && !g_io.IsEDummy())
		{
			if (EDummyStop())
			{
				if (g_setup.SimMode == 0)
					ChangeState(eStates::READY_FOR_NEW_LOT);
				else
					ChangeState(eStates::EXIT);		// exit program in simulation mode
			}
		}

		if (g_state == eStates::COMPENSATION && !g_io.IsECompensation())
		{
			if (CompensationStop())
			{
				if (g_setup.SimMode == 0)
					ChangeState(eStates::READY_FOR_NEW_LOT);
				else
					ChangeState(eStates::EXIT);		// exit program in simulation mode
			}
		}
	}
}

void StateMachine::ChangeState(eStates newState)
{
	g_previous_state = g_state;

	switch (newState)
	{
	case eStates::READY_FOR_NEW_LOT:
		Reset();	
		g_devices.Init();

		if (g_setup.SimMode == 1)
			g_io.SimulateNewLOT();
		else if (g_setup.SimMode == 2)
			g_io.SimulateDummyTest();
		else if (g_setup.SimMode == 3)
			g_io.SimulateCompensation();

		logger::WriteLog("Wait for signal 'LOT START' ...");
		break;
	case eStates::WAIT_UNTIL_LOT_IN_PROGRESS:
		logger::WriteLog("Wait for signal 'LOT IN PROGRESS' ...");
		break;
	case eStates::READY_TO_MEASURE:
		logger::WriteLog("Ready to measure ...");
		break;
	case eStates::COMPENSATION:
		logger::WriteLog("Compensation mode started ...");
		break;
	case eStates::DEVICE_ERROR:
		logger::WriteLog(g_devices.GetLastError());
#if GUI
		g_gui.sendUserMessage(g_devices.GetLastError(), true);
#endif
		g_io.SetLotInProgress(false);
		break;
	case eStates::DB_ERROR:
		logger::WriteLog(g_db.GetLastError());
#if GUI
		g_gui.sendUserMessage(g_db.GetLastError(), true);
#endif
		g_io.SetLotInProgress(false);
		break;
	}

	g_state = newState;
}

// it is called continuously from the main loop
eStates StateMachine::Update()
{
	Comp_Status stat;

	switch (g_state)
	{
	case eStates::READY_FOR_NEW_LOT:
		CheckStartConditions();
		break;

	case eStates::WAIT_UNTIL_LOT_IN_PROGRESS:
		if (g_io.IsLotInProgress() && g_io.IsMachineReady())
		{
			g_io.SetInitPin(false);
			ChangeState(eStates::READY_TO_MEASURE);
		}
		break;

	case eStates::READY_TO_MEASURE:

		//
		// now trigger events starts the measurements
		//

		CheckEndConditions();
		break;
	case eStates::MEASUREMENT_RUNNING:
		if (g_devices.IsFinished())
		{
			g_devices.GetResults(g_results);
			g_state = eStates::MEASUREMENT_EVALUATE;
		}
		break;

	case eStates::MEASUREMENT_EVALUATE:
		// calculates cycle time
		duration<double, std::milli> diff = std::chrono::high_resolution_clock::now() - start_time;
		g_last_cycle_time = (int)diff.count();

		ProcessingMeasurementResults(); // adds new part to the part buffer and set the result on the digital I/O pins

		if (g_devices.GetStatus() == eHandlerStatus::H_ERROR)
			ChangeState(eStates::DEVICE_ERROR);
		else
			g_state = eStates::READY_TO_MEASURE;
				
		PrintCurrentMeasurement(); // prints result to the output

		if (g_setup.PrintBufferContent)
			part_buffer.PrintAll();		

		SendMeasurementToUI();
		break;

	case eStates::COMPENSATION:
		stat = g_comp.Update();
#if GUI
		g_gui.sendCompStatus(stat);
#endif
		CheckEndConditions();
		Sleep(20);
		break;

	case eStates::SHUTDOWN_CMD:
		logger::WriteLog("Shutdown command received");
		g_io.SetLotInProgress(false);
		g_io.SetSystemReady(false);

		//g_db.BufferTable_WriteBuffer(part_buffer);	// backup the buffer (there could be incompleted parts)
		g_db.ValueTable_EmptyBuffer(g_lot.header.lot_nr, part_buffer);

		g_lot.tsms_cnt.Copy(part_buffer.GetCounter());	// get current counter from buffer
		g_lot.SetStopTimeToNow();
		g_db.LotTable_Update(g_lot);		// write lot data to database
		g_state = eStates::EXIT;
		break;

	case eStates::DEVICE_ERROR:
#if GUI
		if (ui_cmd_init_device)
		{
			g_devices.Init();
			ui_cmd_init_device = false;
		}
		else if (g_devices.GetStatus() == eHandlerStatus::H_SUCCESS)
		{
			if (!g_devices.SetTestConfig(g_lot.test_config))
			{
			//	ChangeState(eStates::DEVICE_ERROR); // todo: test
			}
			ContinueAfterError();
		}
#endif
		break;

	case eStates::RS232_ERROR:
#if GUI
		if (ui_cmd_init_rs232)
		{
			g_com.Init();
			ui_cmd_init_rs232 = false;
		}
		else if (g_com.GetStatus() == eHandlerStatus::H_SUCCESS)
			ContinueAfterError();
#endif
		break;
	case eStates::IO_ERROR:
		break;
	case eStates::DB_ERROR:
#if GUI
		if (ui_cmd_init_db)
		{
			g_db.Init();
			ui_cmd_init_db = false;
		}
		else if (g_db.GetStatus() == eHandlerStatus::H_SUCCESS)
		{
			ContinueAfterError();
		}
		break;
#endif
	case eStates::EXIT:
		// handled in the main loop
		break;
	default:
		break;
	}
	return g_state;
}


