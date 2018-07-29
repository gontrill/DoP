// http://www.gomorgan89.com 
#include "application.h"
#include "gpk_windows.h"
#include "gpk_safe.h"
#include "gpk_view_stream.h"
#include "gpk_endpoint_command.h"

#include <Ctime>
#include <chrono>
#include <socketapi.h>

#if defined(GPK_WINDOWS)
#	include <process.h>
#endif

		::gpk::error_t					tcpipNodeConnect			(::dop::STCPIPNode& client)										{
	::gpk::auto_socket_close					sdsafeS						= {};
	::gpk::auto_socket_close					sdsafeR						= {};

	::gpk::SEndpointCommand						command						= {::gpk::ENDPOINT_COMMAND_CONNECT, 0, ::gpk::ENDPOINT_MESSAGE_TYPE_REQUEST};

	char										recv_buffer	[256]			= {};					// Host name of this computer */
	SOCKET										& sdSend					= client.SocketSend = ::socket(AF_INET, SOCK_DGRAM, 0);		// Open a datagram socket */
	ree_if(sdSend == INVALID_SOCKET, "Could not create socket.");
	sdsafeS.Handle							= sdSend;
	SOCKET										& sdRecv					= client.SocketReceive = ::socket(AF_INET, SOCK_DGRAM, 0);		// Open a datagram socket */
	ree_if(sdRecv == INVALID_SOCKET, "Could not create socket.");
	sdsafeR.Handle							= sdRecv;

	::gpk::SIPv4								& addrLocal					= client.AddressLocal;
	sockaddr_in									sa_local					; /* Information about the client */

	::gpk::tcpipAddressToSockaddr(addrLocal, sa_local);
	gpk_necall(::bind(sdRecv, (sockaddr*)&sa_local, sizeof(sockaddr_in)), "Cannot bind address to socket.");
	sa_local.sin_port						= 0;
	gpk_necall(::bind(sdSend, (sockaddr*)&sa_local, sizeof(sockaddr_in)), "Cannot bind address to socket.");

	::gpk::SIPv4								& addrConn					= client.AddressConnection;// = {{192, 168, 1, 79}, 6667, };
	sockaddr_in									sa_remote					;					/* Information about the server */
	::gpk::tcpipAddressToSockaddr(addrConn, sa_remote);
	gpk_necall(::sendto(sdRecv, (const char*)&command, (int)sizeof(::gpk::SEndpointCommand), 0, (sockaddr*)&sa_remote, sizeof(sockaddr_in)), "Error transmitting data.");

	client.State							= ::dop::TCPIP_NODE_STATE_HANDSHAKE_0;
	while(client.State != ::dop::TCPIP_NODE_STATE_DISCONNECTED) {
		int											server_length				= sizeof(sockaddr_in);	/* Length of server struct */
		SOCKET										sdRead						= (client.State == ::dop::TCPIP_NODE_STATE_HANDSHAKE_1) ? sdSend : sdRecv;
		if(SOCKET_ERROR == ::recvfrom(sdRead, (char *)&command, (int)sizeof(::gpk::SEndpointCommand), MSG_PEEK, (sockaddr*)&sa_remote, &server_length)) {
#if defined(GPK_WINDOWS)
			ree_if(::WSAGetLastError() != WSAEMSGSIZE, "recvfrom failed with code 0x%X: '%s'.", ::WSAGetLastError(), ::gpk::getWindowsErrorAsString(::WSAGetLastError()).begin());
#endif
		}
		if(command.Type == ::gpk::ENDPOINT_MESSAGE_TYPE_RESPONSE) {
			switch(command.Command) {
			default:
				{
				info_printf("Received unknown response.");
				::gpk::view_stream<char>					inputCommand				= {recv_buffer};
				const uint32_t								sizeToRead					= sizeof(::gpk::SEndpointCommand) + command.Payload;
				if(SOCKET_ERROR == ::recvfrom(sdRecv, inputCommand.begin(), (int)sizeToRead, MSG_PEEK, (sockaddr*)&sa_remote, &server_length)) {
#if defined(GPK_WINDOWS)
					warning_printf("recvfrom failed with code 0x%X: '%s'.", ::WSAGetLastError(), ::gpk::getWindowsErrorAsString(::WSAGetLastError()).begin());
					::WSASetLastError(0);
#endif
				}
				::dop::STCPIPEndpointMessage				msg							= {};
				msg.Payload.resize(command.Payload);
				inputCommand.read_pod(msg.Command);
				inputCommand.read_pod(msg.Payload.begin(), msg.Payload.size());
				client.QueueReceive.push_back(msg);
				}
				break;
			case ::gpk::ENDPOINT_COMMAND_CONNECT:
				{
				info_printf("Received CONNECT response. Stage: %u.", (uint32_t)command.Payload);
				switch(command.Payload) {
				case 0: 
					{ 
					::gpk::tcpipAddressFromSockaddr(sa_remote, client.AddressRemote); 
					client.PortReceive					= client.AddressRemote.Port;
					command								= {::gpk::ENDPOINT_COMMAND_CONNECT, 1, ::gpk::ENDPOINT_MESSAGE_TYPE_REQUEST};
					sa_remote.sin_port					= htons(client.AddressConnection.Port);
					gpk_necall(::sendto(sdSend, (const char*)&command, (int)sizeof(::gpk::SEndpointCommand), 0, (sockaddr*)&sa_remote, sizeof(sockaddr_in)), "Error transmitting data.");
					} 
					client.State						= ::dop::TCPIP_NODE_STATE_HANDSHAKE_1;
					break;
				case 1: 
					::gpk::tcpipAddressFromSockaddr(sa_remote, client.AddressRemote); 
					client.QueueSend.push_back({::gpk::ENDPOINT_COMMAND_CONNECT, 2, ::gpk::ENDPOINT_MESSAGE_TYPE_REQUEST});
					client.State						= ::dop::TCPIP_NODE_STATE_HANDSHAKE_2;
					break;
				case 2: {
					info_printf("Connection successfully established.", (uint32_t)command.Payload);
					int64_t									curTime						= {};
					::dop::STCPIPEndpointMessage			msgToSend					= {{::gpk::ENDPOINT_COMMAND_TIME, 8, ::gpk::ENDPOINT_MESSAGE_TYPE_REQUEST}, };
					msgToSend.Payload.append((const byte_t*)&curTime, sizeof(int64_t));
					client.QueueSend.push_back(msgToSend);
					client.State						= ::dop::TCPIP_NODE_STATE_IDLE;
				} break;
				}
				}
				break;
			case ::gpk::ENDPOINT_COMMAND_PAYLOAD:
				{
				info_printf("Received PAYLOAD response.");
				::gpk::view_stream<char>					inputCommand				= {recv_buffer};
				const uint32_t								sizeToRead					= sizeof(::gpk::SEndpointCommand) + command.Payload;
				if(SOCKET_ERROR == ::recvfrom(sdRecv, inputCommand.begin(), (int)sizeToRead, MSG_PEEK, (sockaddr*)&sa_remote, &server_length)) {
	#if defined(GPK_WINDOWS)
					warning_printf("recvfrom failed with code 0x%X: '%s'.", ::WSAGetLastError(), ::gpk::getWindowsErrorAsString(::WSAGetLastError()).begin());
					::WSASetLastError(0);
	#endif
				}
				::dop::STCPIPEndpointMessage				msg							= {};
				int32_t										payloadSize					= 0;
				inputCommand.read_pod(msg.Command);
				inputCommand.read_pod((ubyte_t*)&payloadSize, command.Payload);
				msg.Payload.resize((int32_t)payloadSize);
				inputCommand.read_pod(msg.Payload.begin(), msg.Payload.size());
				client.QueueReceive.push_back(msg);
				}
				break;
			}
		}
		::recvfrom(sdRead, (char*)&command, (int)sizeof(::gpk::SEndpointCommand), 0, (sockaddr*)&sa_remote, &server_length);
		Sleep(10);
	}
	sdsafeS.Handle							= INVALID_SOCKET;
	sdsafeR.Handle							= INVALID_SOCKET;
	safe_closesocket(client.SocketReceive);
	safe_closesocket(client.SocketSend);
	return 0;
}

static	void									tcpipNodeConnect				(void * client)													{
	error_if(errored(tcpipNodeConnect(*(::dop::STCPIPNode*)client)), "Cannot connect to server.");
	return;
}

		::gpk::error_t							run								(::dop::STCPIPNode& client)										{
	_beginthread(::tcpipNodeConnect, 0, &client);
	return 0;
}

		::gpk::error_t							gme::clientUpdate				(::dop::STCPIPNode & client)			{
	//::gme::mutex_guard									lock							(server.LockClients);
	::dop::tcpipNodeUpdate(client);
	::gpk::array_pod<byte_t>							sendBuffer;
	for(uint32_t iSend = 0; iSend < client.QueueSend.size(); ++iSend) {
		::dop::STCPIPEndpointMessage						& message						= client.QueueSend[iSend];
		sendBuffer.clear();
		sendBuffer.append((byte_t*)&message.Command, sizeof(::gpk::SEndpointCommand));
		sendBuffer.append((byte_t*)&message.Payload.size(), sizeof(uint32_t));
		if(message.Payload.size())
			sendBuffer.append(message.Payload.begin(), message.Payload.size());

		sockaddr_in											sa_client						= {};			// Information about the client */
		::gpk::tcpipAddressToSockaddr(client.AddressRemote, sa_client);
		info_printf("Sending command {%u, %u, %u} to %u.%u.%u.%u:%u."	, (uint32_t)message.Command.Command
																		, (uint32_t)message.Command.Payload
																		, (uint32_t)message.Command.Type
																		, (uint32_t)client.AddressRemote.IP[0]
																		, (uint32_t)client.AddressRemote.IP[1]
																		, (uint32_t)client.AddressRemote.IP[2]
																		, (uint32_t)client.AddressRemote.IP[3]
																		, (uint32_t)client.AddressRemote.Port
																		);
		ree_if(sendto((message.Command.Command == ::gpk::ENDPOINT_COMMAND_CONNECT && message.Command.Payload == 1) ? client.SocketReceive : client.SocketSend, sendBuffer.begin(), sendBuffer.size(), 0, (sockaddr*)&sa_client, (int)sizeof(sockaddr_in)) != (int32_t)sendBuffer.size(), "Error sending datagram.");
		client.QueueSent.push_back(message);
	}
	client.QueueSend.clear();
	return 0;
}
