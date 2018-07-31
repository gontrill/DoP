#include "gpk_stdsocket.h"
#include "gpk_endpoint_command.h"
#include "gpk_array.h"
#include "gpk_ptr.h"
#include <mutex>

#ifndef DOPL_TCPIP_NODE_UDP
#define DOPL_TCPIP_NODE_UDP

namespace dop
{
#pragma pack(push, 1)
	struct STCPIPEndpointMessage {
		::gpk::SEndpointCommand												Command;
		int64_t																TimeSentLocal;
		::gpk::array_pod<byte_t>											Payload;
	};

	enum TCPIP_NODE_MODE : int8_t
		{ TCPIP_NODE_MODE_CLIENT				= 0
		, TCPIP_NODE_MODE_HOST	
		};

	enum TCPIP_NODE_STATE : int8_t
		{ TCPIP_NODE_STATE_DISCONNECTED			= 0
		, TCPIP_NODE_STATE_HANDSHAKE_0
		, TCPIP_NODE_STATE_HANDSHAKE_1
		, TCPIP_NODE_STATE_HANDSHAKE_2
		, TCPIP_NODE_STATE_IDLE
		};

	struct STCPIPNode {
		SOCKET																SocketSend							= INVALID_SOCKET;
		SOCKET																SocketReceive						= INVALID_SOCKET;
		::gpk::array_obj<STCPIPEndpointMessage>								QueueSend							= {};
		::gpk::array_obj<STCPIPEndpointMessage>								QueueSent							= {};
		::gpk::array_obj<STCPIPEndpointMessage>								QueueReceive						= {};
		::gpk::SIPv4														AddressLocal						= {};
		::gpk::SIPv4														AddressRemote						= {};
		::gpk::SIPv4														AddressConnection					= {};
		int64_t																RemoteTime							= 0;
		uint32_t															TimeoutConnect						= (uint32_t)-1;
		uint32_t															TimeoutLink							= (uint32_t)-1;
		uint16_t															PortReceive							= 0;
		::dop::TCPIP_NODE_STATE												State								= ::dop::TCPIP_NODE_STATE_DISCONNECTED;
		::dop::TCPIP_NODE_MODE												Mode								= ::dop::TCPIP_NODE_MODE_CLIENT;
	};

	struct SServer {
		SOCKET																Socket								= INVALID_SOCKET;
		::gpk::array_obj<::gpk::ptr_obj<::dop::STCPIPNode>>					Clients								= {};		
		::std::mutex														LockClients							;
		::gpk::SIPv4														Address								= {{192, 168, 1, 79}, 6667, };
		bool																Listening							= false;
		bool																Running								= false;
	};
#pragma pack(pop)

	::gpk::error_t														serverListen						(::dop::SServer& server);
	::gpk::error_t														serverShutdown						(::dop::SServer& server);
	::gpk::error_t														serverUpdate						(::dop::SServer& server);
	::gpk::error_t														clientUpdate						(::dop::STCPIPNode & client);

	::gpk::error_t														tcpipNodeConnect					(::dop::STCPIPNode & node, const ::gpk::SIPv4 & addressRemote);
	::gpk::error_t														tcpipNodeConnect					(::dop::STCPIPNode & node);
	::gpk::error_t														tcpipNodeUpdate						(::dop::STCPIPNode & node);

}

#endif