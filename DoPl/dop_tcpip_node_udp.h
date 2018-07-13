#include "gpk_stdsocket.h"
#include "gpk_tcpip.h"

#ifndef DOPL_TCPIP_NODE_UDP
#define DOPL_TCPIP_NODE_UDP

namespace dop
{
	struct STCPIPNode {
		::gpk::SIPv4														AddressLocal						= {};
		::gpk::SIPv4														AddressRemote						= {};
		uint16_t															PortReceive							= 0;
		SOCKET																Socket								= INVALID_SOCKET;
		int64_t																Time								= 0;
	};

	::gpk::error_t														tcpipNodeHandshake					(::dop::STCPIPNode & node, const ::gpk::SIPv4 & addressRemote);
}

#endif