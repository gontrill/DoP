#include "gpk_tcpip.h"
#include "gpk_stdsocket.h"
#include "gpk_endpoint_command.h"

#ifndef DOPL_CONNECTION_H_208937498237
#define DOPL_CONNECTION_H_208937498237

namespace dop
{
	struct SConnection {
		::gpk::SIPv4							AddressLocal				= {};
		::gpk::SIPv4							AddressRemote				= {};
		SOCKET									Socket						= INVALID_SOCKET;	// To local address.
	};

	struct SConnectionDuplex {
		::gpk::SIPv4							AddressLocal				= {};
		::gpk::SIPv4							AddressRemote				= {};
		SOCKET									SocketReceive				= INVALID_SOCKET;	// To local address.
		SOCKET									SocketSend					= INVALID_SOCKET;	// To local address.
		::dop::SConnection						ConnectionReceive			= {};
		::dop::SConnection						ConnectionSend				= {};
	};

	::gpk::error_t							commandByteSend				(SOCKET socket, const ::gpk::SIPv4 & addressRemote, const ::gpk::SEndpointCommand & commandToSend);
	::gpk::error_t							commandBytePeek				(SOCKET socket,		  ::gpk::SIPv4 & addressRemote,		  ::gpk::SEndpointCommand & commandPeeked);
} // namespace

#endif // DOPL_CONNECTION_H_208937498237
