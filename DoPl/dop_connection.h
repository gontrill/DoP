#include "gpk_tcpip.h"
#include "gpk_stdsocket.h"
#include "gpk_endpoint_command.h"

#ifndef DOPL_CONNECTION_H_208937498237
#define DOPL_CONNECTION_H_208937498237

namespace dop
{
	::gpk::error_t							commandByteSend				(SOCKET socket, const ::gpk::SIPv4 & addressRemote, const ::gpk::SEndpointCommand & commandToSend);
	::gpk::error_t							commandBytePeek				(SOCKET socket,		  ::gpk::SIPv4 & addressRemote,		  ::gpk::SEndpointCommand & commandPeeked);
} // namespace

#endif // DOPL_CONNECTION_H_208937498237
