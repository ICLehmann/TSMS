#pragma once
#include "LotData.h"

enum eStates { /* if you change this, you have to change this in ui too */
	READY_FOR_NEW_LOT, WAIT_UNTIL_LOT_IN_PROGRESS, READY_TO_MEASURE, MEASUREMENT_RUNNING, MEASUREMENT_EVALUATE,
	COMPENSATION, SHUTDOWN_CMD, DEVICE_ERROR, RS232_ERROR, IO_ERROR, DB_ERROR, EXIT
};		

struct TSMS_Setup
{
	std::string machine_name;
	int SimMode = 0;		//Simulation mode -> 0 = off, 1 = Lot,  2 = Dummy Test, 3 = Compensation
	bool CheckUnfinshedLot = false;
	bool PrintBufferContent = false;
};

class StateMachine
{
private:
	int _measurement_counter;
	
	void PrintCurrentMeasurement();

	bool UpdateLotTable();
	std::pair<bool, bool> ContinueUnfinishedLot();

	void ContinueAfterError();
	void CheckStartConditions();
	void CheckEndConditions();

	void ProcessingMeasurementResults();
	void SendMeasurementToUI();

	bool StartExistingLot(long long lot_nr);
	bool StartNewLot(lot::LotHeader& lot_header);
	bool LotStart();
	bool LotStop();
	bool LotDiscard();
	
	bool ReadDummyCompTime();

	bool EDummyStart();
	bool EDummyStop();
		
	bool CompensationStart();
	bool CompensationStop();

	void Reset();

public:
	StateMachine();
	~StateMachine();

	eStates Update();
	void ChangeState(eStates newState);

	void Startup();
	void Shutdown();
};

