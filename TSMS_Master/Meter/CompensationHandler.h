#pragma once
#include "Meter.h"
#include "../common/Handler.h"
#include "../Machine/DIOHandler.h"
#include "../LotData.h"

enum eCompensations { L_OPEN, L_SHORT, C_OPEN, C_SHORT, R1_SHORT, R2_SHORT };
enum eCompStatus { COMP_NONE, COMP_RUN, COMP_OK, COMP_NOK};

struct Comp_Status
{
	bool init = false;
	eCompStatus status[tsms_config::NUM_COMPS];
	std::string names[tsms_config::NUM_COMPS];
};


class CompensationHandler : public Handler
{
private:
	io::DIOHandler* _pIo;

	bool _hasError;

	bool ThreadsFinshed();
	void Reset();

public:
	CompensationHandler(io::DIOHandler* io);
	~CompensationHandler();

	bool Init() override;
	void Close() override;

	void Trigger(io::eTrigger trigger);
	Comp_Status Update();

	TSMSCounter tsms_cnt;

	bool Success() { return !_hasError; }
};

