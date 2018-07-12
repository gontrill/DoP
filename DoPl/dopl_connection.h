#include "gpk_tcpip.h"
#include "gpk_stdsocket.h"

#ifndef DOPL_CONNECTION_H_208937498237
#define DOPL_CONNECTION_H_208937498237

namespace dop
{
	struct SConnection {
		::gpk::SIPv4							AddressLocal				= {};
		::gpk::SIPv4							AddressRemote				= {};
		SOCKET									Socket						= INVALID_SOCKET;
	};

	struct SConnectionDuplex {
		::dop::SConnection						ConnectionReceive			= {};
		::dop::SConnection						ConnectionSend				= {};
	};
} // namespace

#endif // DOPL_CONNECTION_H_208937498237
