#include "DIOHandler.h"
#include <Windows.h>
#include <iostream>

#include "../common/Logger.h"
#include "../common/IniFile.h"
#include "SimIO.h"
#include "DAQmxIO.h"

#define PRINT_LOT_SIGNALS 1
#define PRINT_TRIGGER 0

namespace io
{
	static BaseIO* g_pIO = nullptr;
	
	void Event_MachineEmpty(void)
	{
#if PRINT_LOT_SIGNALS
		std::cout << "Signal 'MACHINE EMPTY' " << g_pIO->GetPin(InputPins::IN00_MACHINE_EMPTY) << "\n";
#endif
	}

	void Event_LotInProgress(void)
	{
#if PRINT_LOT_SIGNALS
		std::cout << "Signal 'LOT IN PROGRESS' " << g_pIO->GetPin(InputPins::IN20_LOT_IN_PROGRESS) << "\n";
#endif
	}

	void Event_LotStart(void)
	{
#if PRINT_LOT_SIGNALS
		std::cout << "Signal 'LOT START' " << g_pIO->GetPin(InputPins::IN02_LOT_START) << "\n";
#endif
	}

	void Event_LotEnd(void)
	{
#if PRINT_LOT_SIGNALS
		std::cout << "Signal 'LOT END' " << g_pIO->GetPin(InputPins::IN03_LOT_END) << "\n";
#endif
	}

	void Event_LotPause(void)
	{
#if PRINT_LOT_SIGNALS
		std::cout << "Signal 'LOT PAUSE' " << g_pIO->GetPin(InputPins::IN04_LOT_PAUSE) << "\n";
#endif
	}

	void Event_LotDiscard(void)
	{
#if PRINT_LOT_SIGNALS
		std::cout << "Signal 'LOT DISCARD' " << g_pIO->GetPin(InputPins::IN05_LOT_DISCARD) << "\n";
#endif
	}

	void Event_IsolationTrigger_Changed(void)
	{
		bool level = g_pIO->GetPin(InputPins::IN11_ISOLATION_TRIGGER);

#if PRINT_TRIGGER
		std::cout << "Signal 'ISOLATION_TRIGGER' " << level << "\n";
#endif
		if (level)
		{
			g_trigger_event_func(eTrigger::IOS_TRIG);
		}
		else
		{
			g_pIO->ResetPin(OutputPins::OUT60_ISOLATION_PASS);
			g_pIO->ResetPin(OutputPins::OUT61_ISOLATION_FAIL_LIMIT_HIGH);
			g_pIO->ResetPin(OutputPins::OUT62_ISOLATION_FAIL_LIMIT_LOW);
			g_pIO->ResetPin(OutputPins::OUT63_ISOLATION_FAIL_SYSTEM);
		}
	}

	void Event_ImpedanceInductanceTrigger_Changed(void)
	{
		bool level = g_pIO->GetPin(InputPins::IN12_IMPED_INDUCT_TRIGGER);

#if PRINT_TRIGGER
		std::cout << "Signal 'Impedance_Inductance_Trigger' " << level << "\n";
#endif
		if (level)
		{
			g_trigger_event_func(eTrigger::IMPED_INDUCT_TRIG);
		}
		else
		{
			g_pIO->ResetPin(OutputPins::OUT50_IMPEDANCE_PASS);
			g_pIO->ResetPin(OutputPins::OUT51_IMPEDANCE_FAIL_LIMIT_HIGH);
			g_pIO->ResetPin(OutputPins::OUT52_IMPEDANCE_FAIL_LIMIT_LOW);
			g_pIO->ResetPin(OutputPins::OUT53_IMPEDANCE_FAIL_SYSTEM);

			g_pIO->ResetPin(OutputPins::OUT64_INDUCTANCE_PASS);
			g_pIO->ResetPin(OutputPins::OUT65_INDUCTANCE_FAIL_LIMIT_HIGH);
			g_pIO->ResetPin(OutputPins::OUT66_INDUCTANCE_FAIL_LIMIT_LOW);
			g_pIO->ResetPin(OutputPins::OUT67_INDUCTANCE_FAIL_SYSTEM);
		}
	}

	void Event_CapacityTrigger_Changed(void)
	{
		bool level = g_pIO->GetPin(InputPins::IN14_CAPACITY_TRIGGER);

#if PRINT_TRIGGER
		std::cout << "Signal 'Capacity Trigger' " << level << "\n";
#endif
		if (level)
		{
			g_trigger_event_func(eTrigger::CAP_TRIG);
		}
		else
		{
			g_pIO->ResetPin(OutputPins::OUT54_CAPACITY_PASS);
			g_pIO->ResetPin(OutputPins::OUT55_CAPACITY_FAIL_LIMIT_HIGH);
			g_pIO->ResetPin(OutputPins::OUT56_CAPACITY_FAIL_LIMIT_LOW);
			g_pIO->ResetPin(OutputPins::OUT57_CAPACITY_FAIL_SYSTEM);
		}
	}

	void Event_ResistanceTrigger_Changed(void)
	{
		bool level = g_pIO->GetPin(InputPins::IN15_RESISTANCE_TRIGGER);

#if PRINT_TRIGGER
		std::cout << "Signal 'Resistance Trigger' " << level << "\n";
#endif
		if (level)
		{
			g_trigger_event_func(eTrigger::RESISTANCE_TRIG);
		}
		else
		{
			g_pIO->ResetPin(OutputPins::OUT70_RESISTANCE_R1_PASS);
			g_pIO->ResetPin(OutputPins::OUT71_RESISTANCE_R1_FAIL_LIMIT_HIGH);
			g_pIO->ResetPin(OutputPins::OUT72_RESISTANCE_R1_FAIL_LIMIT_LOW);
			g_pIO->ResetPin(OutputPins::OUT73_RESISTANCE_R1_FAIL_SYSTEM);

			g_pIO->ResetPin(OutputPins::OUT74_RESISTANCE_R2_PASS);
			g_pIO->ResetPin(OutputPins::OUT75_RESISTANCE_R2_FAIL_LIMIT_HIGH);
			g_pIO->ResetPin(OutputPins::OUT76_RESISTANCE_R2_FAIL_LIMIT_LOW);
			g_pIO->ResetPin(OutputPins::OUT77_RESISTANCE_R2_FAIL_SYSTEM);
		}
	}

	void Event_ShiftTrigger_Changed(void)
	{
		bool level = g_pIO->GetPin(InputPins::IN22_SHIFT_TRIGGER);
#if PRINT_TRIGGER
		std::cout << "Signal 'Shift Trigger' " << level << "\n";
#endif
		if (level)
		{
			g_trigger_event_func(eTrigger::SHIFT_TRIG);
		}
	}

	void Event_InitTrigger_Changed(void)
	{
		bool level = g_pIO->GetPin(InputPins::IN21_INIT_TRIGGER);
#if PRINT_TRIGGER
		std::cout << "Signal 'Init Trigger' " << level << "\n";
#endif
		if (level)
		{
			g_trigger_event_func(eTrigger::INIT_TRIGGER);
		}
	}

	void DIOHandler::ResetAllResultPins()
	{
		g_pIO->ResetPin(OutputPins::OUT50_IMPEDANCE_PASS);
		g_pIO->ResetPin(OutputPins::OUT51_IMPEDANCE_FAIL_LIMIT_HIGH);
		g_pIO->ResetPin(OutputPins::OUT52_IMPEDANCE_FAIL_LIMIT_LOW);
		g_pIO->ResetPin(OutputPins::OUT53_IMPEDANCE_FAIL_SYSTEM);

		g_pIO->ResetPin(OutputPins::OUT54_CAPACITY_PASS);
		g_pIO->ResetPin(OutputPins::OUT55_CAPACITY_FAIL_LIMIT_HIGH);
		g_pIO->ResetPin(OutputPins::OUT56_CAPACITY_FAIL_LIMIT_LOW);
		g_pIO->ResetPin(OutputPins::OUT57_CAPACITY_FAIL_SYSTEM);

		g_pIO->ResetPin(OutputPins::OUT60_ISOLATION_PASS);
		g_pIO->ResetPin(OutputPins::OUT61_ISOLATION_FAIL_LIMIT_HIGH);
		g_pIO->ResetPin(OutputPins::OUT62_ISOLATION_FAIL_LIMIT_LOW);
		g_pIO->ResetPin(OutputPins::OUT63_ISOLATION_FAIL_SYSTEM);

		g_pIO->ResetPin(OutputPins::OUT64_INDUCTANCE_PASS);
		g_pIO->ResetPin(OutputPins::OUT65_INDUCTANCE_FAIL_LIMIT_HIGH);
		g_pIO->ResetPin(OutputPins::OUT66_INDUCTANCE_FAIL_LIMIT_LOW);
		g_pIO->ResetPin(OutputPins::OUT67_INDUCTANCE_FAIL_SYSTEM);

		g_pIO->ResetPin(OutputPins::OUT70_RESISTANCE_R1_PASS);
		g_pIO->ResetPin(OutputPins::OUT71_RESISTANCE_R1_FAIL_LIMIT_HIGH);
		g_pIO->ResetPin(OutputPins::OUT72_RESISTANCE_R1_FAIL_LIMIT_LOW);
		g_pIO->ResetPin(OutputPins::OUT73_RESISTANCE_R1_FAIL_SYSTEM);

		g_pIO->ResetPin(OutputPins::OUT74_RESISTANCE_R2_PASS);
		g_pIO->ResetPin(OutputPins::OUT75_RESISTANCE_R2_FAIL_LIMIT_HIGH);
		g_pIO->ResetPin(OutputPins::OUT76_RESISTANCE_R2_FAIL_LIMIT_LOW);
		g_pIO->ResetPin(OutputPins::OUT77_RESISTANCE_R2_FAIL_SYSTEM);
	}

	DIOHandler::DIOHandler(measure_event mesaure_event_callback) : Handler("IO")
	{
		if (mesaure_event_callback == 0)
			throw std::runtime_error("DIOHandler need a callback!\n");

		g_trigger_event_func = mesaure_event_callback;	
	}

	DIOHandler::~DIOHandler()
	{
		Close();
	}

	bool DIOHandler::Init()
	{
		IniFile ini(tsms_config::INI_FILE.c_str());
		bool SimMode = ini.ReadInteger("Simulation", "Mode", 0);
		
		if (g_pIO == nullptr)
		{
			if (SimMode == 0)
				g_pIO = new DAQmxIO();
			else
				g_pIO = new SimIO();
		}

		g_pIO->RegisterFunction(InputPins::IN00_MACHINE_EMPTY, Event_MachineEmpty);
		g_pIO->RegisterFunction(InputPins::IN02_LOT_START, Event_LotStart);
		g_pIO->RegisterFunction(InputPins::IN03_LOT_END, Event_LotEnd);
		g_pIO->RegisterFunction(InputPins::IN04_LOT_PAUSE, Event_LotPause);
		g_pIO->RegisterFunction(InputPins::IN05_LOT_DISCARD, Event_LotDiscard);
		g_pIO->RegisterFunction(InputPins::IN11_ISOLATION_TRIGGER, Event_IsolationTrigger_Changed);
		g_pIO->RegisterFunction(InputPins::IN12_IMPED_INDUCT_TRIGGER, Event_ImpedanceInductanceTrigger_Changed);
		g_pIO->RegisterFunction(InputPins::IN14_CAPACITY_TRIGGER, Event_CapacityTrigger_Changed);
		g_pIO->RegisterFunction(InputPins::IN15_RESISTANCE_TRIGGER, Event_ResistanceTrigger_Changed);
		g_pIO->RegisterFunction(InputPins::IN20_LOT_IN_PROGRESS, Event_LotInProgress);
		g_pIO->RegisterFunction(InputPins::IN22_SHIFT_TRIGGER, Event_ShiftTrigger_Changed);
		g_pIO->RegisterFunction(InputPins::IN21_INIT_TRIGGER, Event_InitTrigger_Changed);
		g_pIO->InitIO();

		_handler_status = eHandlerStatus::H_SUCCESS;
		logger::WriteLog("IO handler initialized");
		return true;
	}

	void DIOHandler::Close()
	{
		if (g_pIO != nullptr)
		{
			SetSystemReady(false);
			SetLotInProgress(false);

			g_pIO->CloseIO();

			delete g_pIO;
			g_pIO = nullptr;
		}
		_handler_status = eHandlerStatus::H_CLOSED;
		logger::WriteLog("IO handler closed");
	}

	bool DIOHandler::IsMachineEmpty()
	{
		return g_pIO->GetPin(InputPins::IN00_MACHINE_EMPTY);
	}

	bool DIOHandler::IsMachineReady()
	{
		return g_pIO->GetPin(InputPins::IN01_SYSTEM_READY);
	}

	bool DIOHandler::IsLotInProgress()
	{
		return g_pIO->GetPin(InputPins::IN20_LOT_IN_PROGRESS);
	}

	bool DIOHandler::IsLotStart()
	{
		return g_pIO->GetPin(InputPins::IN02_LOT_START);
	}

	bool DIOHandler::IsLotEnd()
	{
		return g_pIO->GetPin(InputPins::IN03_LOT_END);
	}

	bool DIOHandler::IsLotPause()
	{
		return g_pIO->GetPin(InputPins::IN04_LOT_PAUSE);
	}

	bool DIOHandler::IsLotDiscard()
	{
		return g_pIO->GetPin(InputPins::IN05_LOT_DISCARD);
	}

	bool DIOHandler::IsECompensation()
	{
		return g_pIO->GetPin(InputPins::IN06_E_COMPENSATION);
	}

	bool DIOHandler::IsEDummy()
	{
		return g_pIO->GetPin(InputPins::IN07_E_DUMMY);
	}

	bool DIOHandler::IsInitialization()
	{
		return false;
	}

	void DIOHandler::SetSystemReady(bool flag)
	{
		if (flag)
			g_pIO->SetPin(OutputPins::OUT40_SYSTEM_READY);
		else
		{
			g_pIO->ResetPin(OutputPins::OUT40_SYSTEM_READY);
			g_pIO->StopSimulation();
		}
	}

	void DIOHandler::SetLotInProgress(bool flag)
	{
		if (flag)
			g_pIO->SetPin(OutputPins::OUT41_LOT_IN_PROGRESS);
		else
		{
			g_pIO->ResetPin(OutputPins::OUT41_LOT_IN_PROGRESS);
			g_pIO->StopSimulation();
		}
	}

	void DIOHandler::SetInitPin(bool flag)
	{
		if (flag)
			g_pIO->SetPin(OutputPins::OUT47_INIT);
		else
			g_pIO->ResetPin(OutputPins::OUT47_INIT);
	}

	void DIOHandler::SetResultPins(unsigned int measure_id, Evaluation res)
	{
		switch (measure_id)
		{
		case tsms_config::ISO:
			if (res == Evaluation::PASS)
				g_pIO->SetPin(OutputPins::OUT60_ISOLATION_PASS);
			else if (res == Evaluation::HIGH)
				g_pIO->SetPin(OutputPins::OUT61_ISOLATION_FAIL_LIMIT_HIGH);
			else if (res == Evaluation::LOW)
				g_pIO->SetPin(OutputPins::OUT62_ISOLATION_FAIL_LIMIT_LOW);
			else if (res == Evaluation::FAIL)
				g_pIO->SetPin(OutputPins::OUT63_ISOLATION_FAIL_SYSTEM);
			break;

		case tsms_config::L:
			if (res == Evaluation::PASS)
				g_pIO->SetPin(OutputPins::OUT64_INDUCTANCE_PASS);
			else if (res == Evaluation::HIGH)
				g_pIO->SetPin(OutputPins::OUT65_INDUCTANCE_FAIL_LIMIT_HIGH);
			else if (res == Evaluation::LOW)
				g_pIO->SetPin(OutputPins::OUT66_INDUCTANCE_FAIL_LIMIT_LOW);
			else if (res == Evaluation::FAIL)
				g_pIO->SetPin(OutputPins::OUT67_INDUCTANCE_FAIL_SYSTEM);
			break;

		case tsms_config::Z1:			//todo: handle Z2+Z3+Zn
			if (res == Evaluation::PASS)
				g_pIO->SetPin(OutputPins::OUT50_IMPEDANCE_PASS);
			else if (res == Evaluation::HIGH)
				g_pIO->SetPin(OutputPins::OUT51_IMPEDANCE_FAIL_LIMIT_HIGH);
			else if (res == Evaluation::LOW)
				g_pIO->SetPin(OutputPins::OUT52_IMPEDANCE_FAIL_LIMIT_LOW);
			else if (res == Evaluation::FAIL)
				g_pIO->SetPin(OutputPins::OUT53_IMPEDANCE_FAIL_SYSTEM);
			break;

		case tsms_config::C1:	//todo: handle C2+C3
			if (res == Evaluation::PASS)
				g_pIO->SetPin(OutputPins::OUT54_CAPACITY_PASS);
			else if (res == Evaluation::HIGH)
				g_pIO->SetPin(OutputPins::OUT55_CAPACITY_FAIL_LIMIT_HIGH);
			else if (res == Evaluation::LOW)
				g_pIO->SetPin(OutputPins::OUT56_CAPACITY_FAIL_LIMIT_LOW);
			else if (res == Evaluation::FAIL)
				g_pIO->SetPin(OutputPins::OUT57_CAPACITY_FAIL_SYSTEM);
			break;

		case tsms_config::R1:
			if (res == Evaluation::PASS)
				g_pIO->SetPin(OutputPins::OUT70_RESISTANCE_R1_PASS);
			else if (res == Evaluation::HIGH)
				g_pIO->SetPin(OutputPins::OUT71_RESISTANCE_R1_FAIL_LIMIT_HIGH);
			else if (res == Evaluation::LOW)
				g_pIO->SetPin(OutputPins::OUT72_RESISTANCE_R1_FAIL_LIMIT_LOW);
			else if (res == Evaluation::FAIL)
				g_pIO->SetPin(OutputPins::OUT73_RESISTANCE_R1_FAIL_SYSTEM);
			break;

		case tsms_config::R2:
			if (res == Evaluation::PASS)
				g_pIO->SetPin(OutputPins::OUT74_RESISTANCE_R2_PASS);
			else if (res == Evaluation::HIGH)
				g_pIO->SetPin(OutputPins::OUT75_RESISTANCE_R2_FAIL_LIMIT_HIGH);
			else if (res == Evaluation::LOW)
				g_pIO->SetPin(OutputPins::OUT76_RESISTANCE_R2_FAIL_LIMIT_LOW);
			else if (res == Evaluation::FAIL)
				g_pIO->SetPin(OutputPins::OUT77_RESISTANCE_R2_FAIL_SYSTEM);
			break;
		}

#if PRINT_LOT_SIGNALS
	//	g_io.PrintDebugOutputPins();
#endif
	//	std::cout << "Result pins setted\n " << std::flush;

	}

	void DIOHandler::SetECompensationInProgress(bool flag)
	{
		if (flag)
			g_pIO->SetPin(OutputPins::OUT42_COMPENSATION_IN_PROGRESS);
		else
			g_pIO->ResetPin(OutputPins::OUT42_COMPENSATION_IN_PROGRESS);
	}

	void DIOHandler::SetEDummyInProgress(bool flag)
	{
		if (flag)
			g_pIO->SetPin(OutputPins::OUT43_E_DUMMY_IN_PROGRESS);
		else
			g_pIO->ResetPin(OutputPins::OUT43_E_DUMMY_IN_PROGRESS);
	}

	void DIOHandler::SimulateCompensation()
	{
		logger::WriteLog("Start Simulation Comp");
		g_pIO->SimulateCompensation();
	}

	void DIOHandler::SimulateDummyTest()
	{
		logger::WriteLog("Start Simulation Dummy");
		g_pIO->SimulateDummyTest();
	}

	void DIOHandler::SimulateNewLOT()
	{
		logger::WriteLog("Start Simulation Lot");
		g_pIO->SimulateNewLOT();
	}

}