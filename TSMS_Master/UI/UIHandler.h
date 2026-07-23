#pragma once
#include "UDPSender.h"
#include "UDPReceiver.h"
#include "DataToSend.h"
#include "../StateMachine.h"
#include "../LotData.h"
#include "../MeasuredValue.h"
#include "../Machine/DIOHandler.h"

namespace ui
{
	class UIHandler : public Handler
	{
	private:
		UDPSender _sender;
		UDPReceiver _receiver;

	public:
		UIHandler();

		bool Init() override;
		void Close() override;

		void sendValue(MeasuredValue& val);
		void sendLotData(lot::LOT_Data& lot_data);
		void sendPATUpdate(TestConfig test_config);
		void sendCounter(TSMSCounter& cnt);
		void sendCompStatus(Comp_Status& status);
		void sendSetup(TSMS_Setup& setup);
		void sendMachineSignals(io::DIOHandler& io);
		void sendModuleStatus(TSMS_Status& status);

		void sendUserMessage(std::string msg, bool MsgBox = false);

		bool recvUICommands(CmdFromUI& cmd);
	};
}
