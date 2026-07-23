#include "DataToSend.h"

namespace ui
{
	MValueTelegram::MValueTelegram(MeasuredValue& value)
	{
		this->value = value.GetValue();
		this->type = value.GetID();
	}

	MValueTelegram_PAT::MValueTelegram_PAT(MeasuredValue& value)
	{
		this->value = value.GetValue();
		this->type = value.GetID();
		this->pat_min = value._pat_min;
		this->pat_max = value._pat_max;
	}

	LotDataTelegram::LotDataTelegram(lot::LOT_Data lot_data)
	{
		this->lot_num = lot_data.header.lot_nr;

		memset(this->product_num, 0, sizeof(this->product_num));
		memcpy_s(this->product_num, tsms_config::TELEGRAM_CHAR_LENGTH, lot_data.header.test_plan.c_str(), lot_data.header.test_plan.size());

		memset(this->operator_id, 0, sizeof(this->operator_id));
		memcpy_s(this->operator_id, tsms_config::TELEGRAM_CHAR_LENGTH, lot_data.header.operator_id.c_str(), lot_data.header.operator_id.size());

		memset(this->line_id, 0, sizeof(this->line_id));
		memcpy_s(this->line_id, tsms_config::TELEGRAM_CHAR_LENGTH, lot_data.header.line_id.c_str(), lot_data.header.line_id.size());

		for (int i = 0; i < num_configs; i++)
		{
			this->config[i].min = lot_data.test_config.configs[i].min;
			this->config[i].nom = lot_data.test_config.configs[i].nom;
			this->config[i].max = lot_data.test_config.configs[i].max;
			this->config[i].offset = lot_data.test_config.configs[i].offset;
			this->config[i].prefix = lot_data.test_config.configs[i].prefix;
			this->config[i].error_rate = lot_data.test_config.configs[i].error_rate;
			this->config[i].frequency = lot_data.test_config.configs[i].frequency;
			this->config[i].stimuli_level = lot_data.test_config.configs[i].stimuli_level;
			this->config[i].type = lot_data.test_config.configs[i].type;
			// pat related settings
			this->config[i].pat_enabled = lot_data.test_config.configs[i].pat_enabled;
			this->config[i].pat_min = lot_data.test_config.configs[i].pat_min;
			this->config[i].pat_max = lot_data.test_config.configs[i].pat_max;
		}
	}

	PATUpdateTelegram::PATUpdateTelegram(TestConfig test_config)
	{
		for (int i = 0; i < num_configs; i++)
		{
			this->config[i].pat_enabled = test_config.configs[i].pat_enabled;
			this->config[i].pat_min = test_config.configs[i].pat_min;
			this->config[i].pat_max = test_config.configs[i].pat_max;
		}
	}

	CounterTelegram::CounterTelegram(TSMSCounter counter)
	{
		this->cnt_good = counter.good_parts;
		this->cnt_total = counter.input_parts;
		for (int i = 0; i < num_counter; i++)
		{
			this->res_pass[i] = counter.res_pass[i];
			this->res_low[i] = counter.res_low[i];
			this->res_high[i] = counter.res_high[i];
			this->res_Fail[i] = counter.res_fail[i];
		}
	}

	SetupTelegram::SetupTelegram(TSMS_Setup& setup)
	{
		memset(this->machine, 0, sizeof(this->machine));
		memcpy_s(this->machine, tsms_config::TELEGRAM_CHAR_LENGTH, setup.machine_name.c_str(), setup.machine_name.size());

		num_measurements = tsms_config::NUM_MEASUREMENTS;

		for (int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
		{
			memset(this->measurements_names[i], 0, sizeof(this->measurements_names[i]));
			memcpy_s(this->measurements_names[i], tsms_config::TELEGRAM_CHAR_LENGTH, tsms_config::MeasurementNames[i].c_str(), tsms_config::MeasurementNames[i].size());

			memset(this->measurements_units[i], 0, sizeof(this->measurements_units[i]));
			memcpy_s(this->measurements_units[i], tsms_config::TELEGRAM_CHAR_LENGTH, tsms_config::MeasurementUnits[i].c_str(), tsms_config::MeasurementUnits[i].size());
		}

	}

	CompStatusTelegram::CompStatusTelegram(Comp_Status comp_status)
	{
		num_comps = tsms_config::NUM_COMPS;

		for (int i = 0; i < tsms_config::NUM_COMPS; i++)
		{
			status[i] = comp_status.status[i];
			memset(this->comp_names[i], 0, sizeof(this->comp_names[i]));
			memcpy_s(this->comp_names[i], tsms_config::TELEGRAM_CHAR_LENGTH, comp_status.names[i].c_str(), comp_status.names[i].size());
		}
	}

	MachineSignals::MachineSignals(io::DIOHandler& io)
	{
		if (io.GetStatus() == eHandlerStatus::H_SUCCESS)
		{
			MachineReady = io.IsMachineReady();
			MachineEmpty = io.IsMachineEmpty();
			LotInProgress = io.IsLotInProgress();
			LotStart = io.IsLotStart();
			LotEnd = io.IsLotEnd();
			LotPause = io.IsLotPause();
			LotDiscard = io.IsLotDiscard();
			Compensation = io.IsECompensation();
			EDummy = io.IsEDummy();
			Initialization = io.IsInitialization();
		}
		else
		{
			MachineReady = 0;
			MachineEmpty = 0;
			LotInProgress = 0;
			LotStart = 0;
			LotEnd = 0;
			LotPause = 0;
			LotDiscard = 0;
			Compensation = 0;
			EDummy = 0;
			Initialization = 0;
		}
	}

	UserMsgTelegram::UserMsgTelegram(std::string& msg, unsigned char msgBox)
	{
		_msgBox = msgBox;
		len = (unsigned short)msg.length();
		if (len > MAX_USER_MSG_LENGTH)
			len = MAX_USER_MSG_LENGTH;
		memset(buffer, 0, sizeof(MAX_USER_MSG_LENGTH));
		memcpy_s(buffer, len, msg.c_str(), len);
	}

}