#pragma once
#include <string>

#include "RS232.h"
#include "../Counter.h"
#include "../LotData.h"
#include "../common/Handler.h"

namespace com
{
	class ComHandler : public Handler
	{
	private:
		bool _sim;
		unsigned int _timeout_ms;
		std::string _sim_start_telegram; 
		std::string _sim_end_telegram;

		RS232 rs232;
		bool ReadLoop(std::string& read_data);
		bool WriteAnswer(bool ok);

	public:
		ComHandler();
		~ComHandler();

		bool ReadLotStart(lot::LotHeader &data);
		bool WriteLaserData(std::string &data);
		bool ReadLotEnd(MachineCounter &data);
		bool ReadLotDiscard();
		
		bool Init() override;
		void Close() override;
	};
}
