#include "gpk_stdsocket.h"
#include "gpk_endpoint_command.h"
#include "gpk_array.h"

#ifndef DOPL_TCPIP_NODE_UDP
#define DOPL_TCPIP_NODE_UDP

namespace dop
{
	struct STCPIPEndpointMessage {
		::gpk::SEndpointCommand												Command;
		::gpk::array_pod<byte_t>											Payload;
	};

	struct STCPIPNode {
		::gpk::SIPv4														AddressLocal						= {};
		::gpk::SIPv4														AddressRemote						= {};
		uint16_t															PortReceive							= 0;
		SOCKET																SocketSend							= INVALID_SOCKET;
		SOCKET																SocketReceive						= INVALID_SOCKET;
		int64_t																RemoteTime							= 0;
		uint32_t															TimeoutConnect						= (uint32_t)-1;
		uint32_t															TimeoutLink							= (uint32_t)-1;
		::gpk::array_obj<STCPIPEndpointMessage>								QueueSend							= {};
		::gpk::array_obj<STCPIPEndpointMessage>								QueueReceive						= {};
	};

	::gpk::error_t														tcpipNodeHandshake					(::dop::STCPIPNode & node, const ::gpk::SIPv4 & addressRemote);
	::gpk::error_t														tcpipNodeAccept						(::dop::STCPIPNode & node, const ::gpk::SIPv4 & addressRemote);
}

#endif