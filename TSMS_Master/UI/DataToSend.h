#pragma once
#include "../const.h"
#include "../LotData.h"
#include "../MeasuredValue.h"
#include "../Machine/DIOHandler.h"
#include "../Meter/CompensationHandler.h"
#include "../StateMachine.h"

#pragma pack(push)
#pragma pack(1)

#define MAX_USER_MSG_LENGTH 1024

namespace ui
{
	struct CompStatusTelegram
	{
		const char id = tsms_config::eTelToUI::TEL_COMP_STATUS; 
		unsigned short num_comps = tsms_config::NUM_COMPS;
		char status[tsms_config::NUM_COMPS];
		char comp_names[tsms_config::NUM_COMPS][tsms_config::TELEGRAM_CHAR_LENGTH];
		
		CompStatusTelegram(Comp_Status comp_status);
	};

	struct SetupTelegram
	{
		const char id = tsms_config::eTelToUI::TEL_SETUP;
		char machine[tsms_config::TELEGRAM_CHAR_LENGTH];
		unsigned short num_measurements = tsms_config::NUM_MEASUREMENTS;
		char measurements_names[tsms_config::NUM_MEASUREMENTS][tsms_config::TELEGRAM_CHAR_LENGTH];
		char measurements_units[tsms_config::NUM_MEASUREMENTS][tsms_config::TELEGRAM_CHAR_LENGTH];
		
		SetupTelegram(TSMS_Setup& setup);
	};

	struct ValueConfigTelegram
	{
		double min = 0;
		double nom = 0;
		double max = 0;
		double offset = 0.0;
		char prefix = ' ';
		double error_rate;
		long long frequency;
		double stimuli_level;
		unsigned short type;
		// pat related settings
		char pat_enabled = 0;
		double pat_min = 0;
		double pat_max = 0;
	};

	struct LotDataTelegram
	{
		const char id = tsms_config::eTelToUI::TEL_LOT;
		long long lot_num;
		char product_num[tsms_config::TELEGRAM_CHAR_LENGTH];
		char line_id[tsms_config::TELEGRAM_CHAR_LENGTH];
		char operator_id[tsms_config::TELEGRAM_CHAR_LENGTH];
		unsigned short num_configs = tsms_config::NUM_MEASUREMENTS;
		ValueConfigTelegram config[tsms_config::NUM_MEASUREMENTS];
		
		LotDataTelegram(lot::LOT_Data lot_data);
	};

	struct PATConfigTelegram
	{
		char pat_enabled = 0;
		double pat_min = 0;
		double pat_max = 0;
	};

	struct PATUpdateTelegram
	{
		const char id = tsms_config::eTelToUI::TEL_PAT_UPDATE;
		unsigned short num_configs = tsms_config::NUM_MEASUREMENTS;
		PATConfigTelegram config[tsms_config::NUM_MEASUREMENTS];

		PATUpdateTelegram(TestConfig test_config);
	};

	struct CounterTelegram
	{
		const char id = tsms_config::eTelToUI::TEL_COUNTER; 
		unsigned int cnt_total;
		unsigned int long cnt_good;
		unsigned short num_counter = tsms_config::NUM_MEASUREMENTS;
		unsigned int res_pass[tsms_config::NUM_MEASUREMENTS] = { 0 };
		unsigned int res_low[tsms_config::NUM_MEASUREMENTS] = { 0 };
		unsigned int res_high[tsms_config::NUM_MEASUREMENTS] = { 0 };
		unsigned int res_Fail[tsms_config::NUM_MEASUREMENTS] = { 0 };
		
		CounterTelegram(TSMSCounter counter);
	};

	struct UserMsgTelegram
	{
		const char id = tsms_config::eTelToUI::TEL_USER_MSG;
		unsigned char _msgBox = 0;
		unsigned short len = 0;
		char buffer[1024];

		UserMsgTelegram(std::string& msg, unsigned char msgBox);
	};

	struct MValueTelegram
	{
		const char id = tsms_config::eTelToUI::TEL_MEAS_VALUE;
		unsigned short type = 0;
		double value = 0;
		char pat_enabled = false;

		MValueTelegram(MeasuredValue& value);
	};

	struct MValueTelegram_PAT
	{
		const char id = tsms_config::eTelToUI::TEL_MEAS_VALUE;
		unsigned short type = 0;
		double value = 0;
		char pat_enabled = true;
		double pat_min;
		double pat_max;

		MValueTelegram_PAT(MeasuredValue& value);
	};

	struct MachineSignals
	{
		const char id = tsms_config::eTelToUI::TEL_MACHINE;
		char MachineReady = 0;
		char MachineEmpty = 0;
		char LotInProgress = 0;
		char LotStart = 0;
		char LotEnd = 0;
		char LotPause = 0;
		char LotDiscard = 0;
		char Compensation = 0;
		char EDummy = 0;
		char Initialization = 0;

		MachineSignals(io::DIOHandler& io);
	};

	struct TSMS_Status
	{
		const char id = tsms_config::eTelToUI::TEL_STATUS;
		char StateMachine = 0;
		char Measurements = 0;
		char RS232 =0;
		char Database = 0;
		char DigitalIO = 0;
		unsigned short rest_dummy_hours = 0;
		unsigned short rest_comp_hours = 0;
		unsigned short rest_pat_samples = 0;	// counter for PAT asampling and calculation
	};

	struct CmdFromUI
	{
		const char id = 'X';
		char Cmd;
	};

#pragma pack(pop)

}