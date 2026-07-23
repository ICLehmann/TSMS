#pragma once

#include <string>

enum eHandlerStatus { H_CLOSED, H_SUCCESS, H_ERROR };

class Handler
{
protected:
	eHandlerStatus _handler_status;
	std::string _msg_prefix = "?";
	std::string _last_error;
public:
	Handler(std::string msg_prefix);

	virtual bool Init() = 0;
	virtual void Close() = 0;
	eHandlerStatus GetStatus() const { return _handler_status; }

	std::string GetLastError() { return _msg_prefix + " Error: " + _last_error; }
};

