#pragma once
#include "DIOHandler.h"
#include "../const.h"
#include "../common/Handler.h"
#include "../MeasuredValue.h"

namespace io
{
	enum eTrigger { IOS_TRIG, IMPED_INDUCT_TRIG, CAP_TRIG, RESISTANCE_TRIG, SHIFT_TRIG, INIT_TRIGGER };
	typedef void(*measure_event)(eTrigger);
	static measure_event g_trigger_event_func = 0;
	
	class DIOHandler : public Handler
	{
	private:
		void ResetAllResultPins();		// test only

	public:
		DIOHandler(measure_event mesaure_event_callback);
		~DIOHandler();

		bool Init() override;
		void Close() override;

		bool IsMachineEmpty();
		bool IsMachineReady();
		bool IsLotInProgress();
		bool IsLotStart();
		bool IsLotEnd();
		bool IsLotPause();
		bool IsLotDiscard();

		bool IsECompensation();
		bool IsEDummy();
		bool IsInitialization();

		void SetSystemReady(bool flag);
		void SetLotInProgress(bool flag);
		void SetInitPin(bool flag);
		void SetResultPins(unsigned int measure_id, Evaluation res);
		
		void SetECompensationInProgress(bool flag);
		void SetEDummyInProgress(bool flag);

		void SimulateCompensation();
		void SimulateDummyTest();
		void SimulateNewLOT();
	
	};
}
