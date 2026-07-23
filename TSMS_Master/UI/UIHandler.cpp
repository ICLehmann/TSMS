#include "UIHandler.h"
#include "DataToSend.h"

#include "../common/IniFile.h"

namespace ui
{
	UIHandler::UIHandler() : Handler ("UI")
	{
	}

	bool UIHandler::Init()
	{
		IniFile ini(tsms_config::INI_FILE.c_str());
		std::string  tcp_addr = ini.ReadString("GUI", "TCP_Address", "127.0.0.1");
		int portSend = ini.ReadInteger("GUI", "SendPort", 8500);
		int portRecv = ini.ReadInteger("GUI", "RecvPort", 8501);

		if (!_sender.init(tcp_addr.c_str(), portSend))
		{
			_handler_status = eHandlerStatus::H_ERROR;
			return false;
		}
		if (!_receiver.init(tcp_addr.c_str(), portRecv))
		{
			_handler_status = eHandlerStatus::H_ERROR;
			return false;
		}
		_handler_status = eHandlerStatus::H_SUCCESS;
		return true;
	}

	void UIHandler::Close()
	{
		_sender.close();
		_receiver.close();
	}

	void UIHandler::sendValue(MeasuredValue& value)
	{
		// sends shorter telegram when pat is disabled
		if (value.pat_enabled)
		{
			MValueTelegram_PAT data(value);
			if (!_sender.send((char*)&data, sizeof(MValueTelegram_PAT)))
				_handler_status = eHandlerStatus::H_ERROR;
		}
		else
		{
			MValueTelegram data(value);
			if (!_sender.send((char*)&data, sizeof(MValueTelegram)))
				_handler_status = eHandlerStatus::H_ERROR;
		}
	}

	void UIHandler::sendLotData(lot::LOT_Data& lot_data)
	{
		LotDataTelegram data(lot_data);
		if (!_sender.send((char*)&data, sizeof(LotDataTelegram)))
			_handler_status = eHandlerStatus::H_ERROR;
	}

	void UIHandler::sendPATUpdate(TestConfig test_config)
	{
		PATUpdateTelegram data(test_config);
		if (!_sender.send((char*)&data, sizeof(PATUpdateTelegram)))
			_handler_status = eHandlerStatus::H_ERROR;
	}

	void UIHandler::sendCounter(TSMSCounter& cnt)
	{
		CounterTelegram data(cnt);
		if (!_sender.send((char*)&data, sizeof(CounterTelegram)))
			_handler_status = eHandlerStatus::H_ERROR;
	}

	void UIHandler::sendCompStatus(Comp_Status& status)
	{
		CompStatusTelegram data(status);
		if(!_sender.send((char*)&data, sizeof(CompStatusTelegram) + (data.num_comps*tsms_config::TELEGRAM_CHAR_LENGTH)))
			_handler_status = eHandlerStatus::H_ERROR;
	}

	void UIHandler::sendSetup(TSMS_Setup& setup)
	{
		SetupTelegram data(setup);
		if (!_sender.send((char*)&data, sizeof(SetupTelegram)))
			_handler_status = eHandlerStatus::H_ERROR;
	}

	void UIHandler::sendMachineSignals(io::DIOHandler & io)
	{
		MachineSignals data(io);
		if (!_sender.send((char*)&data, sizeof(MachineSignals)))
			_handler_status = eHandlerStatus::H_ERROR;
	}

	void UIHandler::sendModuleStatus(TSMS_Status & status)
	{
		if (!_sender.send((char*)&status, sizeof(TSMS_Status)))
			_handler_status = eHandlerStatus::H_ERROR;
	}

	void UIHandler::sendUserMessage(std::string msg, bool msgBox)
	{
		UserMsgTelegram data(msg, msgBox);
		if (!_sender.send((char*)&data, sizeof(data)))
			_handler_status = eHandlerStatus::H_ERROR;
	}

	bool UIHandler::recvUICommands(CmdFromUI & cmd)
	{
		return _receiver.recv((char*)&cmd, sizeof(CmdFromUI));
	}
}


