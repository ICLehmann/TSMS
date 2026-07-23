#include "Handler.h"

Handler::Handler(std::string msg_prefix)
{
	_handler_status = eHandlerStatus::H_CLOSED;
	_msg_prefix = msg_prefix;
}

