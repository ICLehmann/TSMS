#include "ComHandler.h"
#include "../const.h"
#include "../common/Logger.h"
#include "../common/IniFile.h"

#include <chrono>
#include <sstream>
#include <iostream>
#include <vector>


using namespace std::chrono;

namespace com
{
	ComHandler::ComHandler() : Handler("RS232")
	{
		_sim = false;
		_timeout_ms = tsms_config::DEFAULT_RS232_TIMEOUT_MS;
		_sim_start_telegram = tsms_config::SIM_START_TELEGRAM;
		_sim_end_telegram = tsms_config::SIM_END_TELEGRAM;
	}

	ComHandler::~ComHandler()
	{
		Close();
	}

	bool ComHandler::Init()
	{
		IniFile ini(tsms_config::INI_FILE.c_str());
		int Port = ini.ReadInteger("RS232", "Port", 1);
		int Baud = ini.ReadInteger("RS232", "Baud", 9600);
		_timeout_ms = ini.ReadInteger("RS232", "TimeoutMS", tsms_config::DEFAULT_RS232_TIMEOUT_MS);
		_sim_start_telegram = ini.ReadString("Simulation", "LotStartTelegram", tsms_config::SIM_START_TELEGRAM.c_str());
		_sim_end_telegram = ini.ReadString("Simulation", "LotEndTelegram", tsms_config::SIM_END_TELEGRAM.c_str());
		int SimMode = ini.ReadInteger("Simulation", "Mode", 0);
		if (SimMode > 0)
			_sim = true;

		bool ret = false; 
		if (_sim)
		{
			ret = true;
			logger::WriteLog( _msg_prefix + ": Simulation mode");
		}
		else
			ret = rs232.Open(Port, Baud);

		if (ret)
			_handler_status = eHandlerStatus::H_SUCCESS;
		else
			_handler_status = eHandlerStatus::H_ERROR;

		return ret;
	}

	void ComHandler::Close()
	{
		rs232.Close();
		_handler_status = eHandlerStatus::H_CLOSED;
	}

	bool ComHandler::ReadLoop(std::string& read_data)
	{
		bool ret = false;
		std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
		while (true)
		{
			ret = rs232.Read(read_data);
			if (ret)
				break;

			duration<double, std::milli> diff = std::chrono::high_resolution_clock::now() - start_time;
			if ((unsigned int)diff.count() > _timeout_ms) // timeout ?
				break;

			Sleep(10);	// wait a moment and try again
		}
		return ret;
	}

	bool com::ComHandler::ReadLotStart(lot::LotHeader& data)
	{
		if (_handler_status != eHandlerStatus::H_SUCCESS)
			return false;

		bool success = false;
		std::string result;

		logger::WriteLog(_msg_prefix + ": Wait for lot start telegram ... ");

		if (_sim)
		{
			result = _sim_start_telegram;
			success = true;
		}
		else
			success = ReadLoop(result);


		if (success && result.size() > 0)
		{
			logger::WriteLog(_msg_prefix + ": LOT START Telegram -> " + result);

			std::istringstream f(result);
			std::string s;
			getline(f, s, ';');
			if (s == "NEW")
			{
				getline(f, s, ';');
				data.lot_nr = std::strtoll(s.c_str(), 0, 10);

				getline(f, s, ';');
				data.test_plan = s;

				getline(f, s, ';');
				data.line_id = s;

				getline(f, s, ';');
				data.operator_id = s;
			}
			else
				success = false;
		}

		if (!success)
		{
			logger::WriteLog(_msg_prefix + " ERROR: No lot data received");
			_handler_status = eHandlerStatus::H_ERROR;
		}

		return success;
	}

	bool ComHandler::ReadLotEnd(MachineCounter& counter)
	{
		if (_handler_status != eHandlerStatus::H_SUCCESS)
			return false;

		bool success = false;
		std::string result;

		logger::WriteLog(_msg_prefix + ": Wait for lot end telegram ... ");

		if (_sim)
		{
			result = _sim_end_telegram;
			success = true;
		}
		else
			success = ReadLoop(result);

		if (success)
		{
			logger::WriteLog(_msg_prefix + ": LOT END Telegram -> " + result);

			std::istringstream f(result);
			std::string s;
			getline(f, s, ';');
			if (s == "END" || s == "PAUSED")
			{
				if (getline(f, s, ';'))
					counter.total_input = std::stoi(s.c_str());

				if (getline(f, s, ';'))
					counter.total_good = std::stoi(s.c_str());

				for (unsigned int i = 0; i < tsms_config::NUM_COUNTER_TRB; i++)
				{
					if (getline(f, s, ';') && s != "EOF")
						counter.TRB[i] = std::stoi(s.c_str());
				}

				for (unsigned int i = 0; i < tsms_config::NUM_COUNTER_VRB; i++)
				{
					if (getline(f, s, ';') && s != "EOF")
						counter.TRV[i] = std::stoi(s.c_str());
				}

				if (getline(f, s, ';') && s != "EOF")
					counter.good_bin = std::stoi(s.c_str());

				if (getline(f, s, ';') && s != "EOF")
					counter.last_bin = std::stoi(s.c_str());
			}
			else
			{
				success = false;
			}
		}

		success = WriteAnswer(success);

		if (!success)
		{
			logger::WriteLog("RS232 ERROR: Reading counter failed");
			_handler_status = eHandlerStatus::H_ERROR;
		}

		return success;
	}

	bool ComHandler::ReadLotDiscard()
	{
		if (_handler_status != eHandlerStatus::H_SUCCESS)
			return false;

		bool success = false;
		std::string result;

		logger::WriteLog(_msg_prefix + ": Wait for lot discard telegram ... ");

		if (_sim)
		{
			result = _sim_end_telegram;
			success = true;
		}
		else
			success = ReadLoop(result);

		if (success)
		{
			logger::WriteLog(_msg_prefix + ": LOT DISCARD Telegram -> " + result);

			std::istringstream f(result);
			std::string s;
			getline(f, s, ';');
			if (s != "DISCARD")
			{
				success = false;
			}
		}

		success = WriteAnswer(success);

		if (!success)
		{
			logger::WriteLog("RS232 ERROR: Read lot discard telegram.");
			_handler_status = eHandlerStatus::H_ERROR;
		}

		return success;
	}

	bool ComHandler::WriteAnswer(bool ok)
	{
		bool success = false;
		std::string anwser_string = "END;";
		if (ok)
			anwser_string += "OK";
		else
			anwser_string += "NOK";
		anwser_string += ";EOF";

		logger::WriteLog(_msg_prefix + ": Write answer -> " + anwser_string);

		if (!_sim)
			success = rs232.Write(anwser_string);
		else
			success = true;
		
		if (!success)
			_handler_status = eHandlerStatus::H_ERROR;

		return success;
	}
	
	bool ComHandler::WriteLaserData(std::string& laser_data)
	{
		if (_handler_status != eHandlerStatus::H_SUCCESS)
			return false;

		std::string write_string = "NEW;OK;" + laser_data + ";EOF";
		bool success = false;

		if (_sim)
		{
			logger::WriteLog(_msg_prefix + ": Laser telegramm -> " + write_string);
			success = true;
		}
		else
			success = rs232.Write(write_string);

		if (!success)
		{
			logger::WriteLog(_msg_prefix + " ERROR: Write laser mark.");
			_handler_status = eHandlerStatus::H_ERROR;
		}

		return success;
	}
}
