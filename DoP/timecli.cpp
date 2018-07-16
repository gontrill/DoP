// http://www.gomorgan89.com 
#include "application.h"
#include "gpk_windows.h"
#include "gpk_safe.h"
#include "gpk_view_stream.h"
#include "gpk_endpoint_command.h"

#include <time.h>
#include <chrono>
#include <socketapi.h>


		::gpk::error_t					tcpipNodeConnect			(::dop::STCPIPNode& client)										{
	::gpk::SEndpointCommand						command						= {::gpk::ENDPOINT_COMMAND_CONNECT, 0, ::gpk::ENDPOINT_MESSAGE_TYPE_REQUEST};

	char										recv_buffer	[256]			= {};					// Host name of this computer */
	SOCKET										& sd						= client.SocketSend = ::socket(AF_INET, SOCK_DGRAM, 0);		// Open a datagram socket */
	ree_if(sd == INVALID_SOCKET, "Could not create socket.");
	::gpk::auto_socket_close					sdsafe						= {};
	sdsafe.Handle							= sd;

	::gpk::SIPv4								& addrLocal					= client.AddressLocal;
	sockaddr_in									sa_local					; /* Information about the client */
	::gpk::tcpipAddressToSockaddr(addrLocal, sa_local);
	gpk_necall(::bind(sd, (sockaddr*)&sa_local, sizeof(sockaddr_in)), "Cannot bind address to socket.");

	::gpk::SIPv4								& addrConn					= client.AddressConnection;// = {{192, 168, 1, 79}, 6667, };
	sockaddr_in									sa_remote					= {AF_INET, htons(addrConn.Port)};					/* Information about the server */
	::gpk::tcpipAddressToSockaddr(addrConn, sa_remote);
	gpk_necall(::sendto(sd, (const char*)&command, (int)sizeof(::gpk::SEndpointCommand), 0, (sockaddr*)&sa_remote, sizeof(sockaddr_in)), "Error transmitting data.");

	int											server_length				= sizeof(sockaddr_in);	/* Length of server struct */
	if(SOCKET_ERROR == ::recvfrom(sd, (char *)&command, (int)sizeof(::gpk::SEndpointCommand), MSG_PEEK, (sockaddr*)&sa_remote, &server_length)) {
#if defined(GPK_WINDOWS)
		ree_if(::WSAGetLastError() != WSAEMSGSIZE, "recvfrom failed with code 0x%X: '%s'.", ::WSAGetLastError(), ::gpk::getWindowsErrorAsString(::WSAGetLastError()).begin());
#endif
	}
	if(command.Type == ::gpk::ENDPOINT_MESSAGE_TYPE_RESPONSE) {
		switch(command.Command) {
		default:
			{
			info_printf("Received generic response.");
			::gpk::view_stream<char>					inputCommand				= {recv_buffer};
			const uint32_t								sizeToRead					= sizeof(::gpk::SEndpointCommand) + command.PayloadBytes;
			if(SOCKET_ERROR == ::recvfrom(sd, inputCommand.begin(), (int)sizeToRead, MSG_PEEK, (sockaddr*)&sa_remote, &server_length)) {
#if defined(GPK_WINDOWS)
				warning_printf("recvfrom failed with code 0x%X: '%s'.", ::WSAGetLastError(), ::gpk::getWindowsErrorAsString(::WSAGetLastError()).begin());
				::WSASetLastError(0);
#endif
			}
			::dop::STCPIPEndpointMessage				msg							= {};
			msg.Payload.resize(command.PayloadBytes);
			inputCommand.read_pod(msg.Command);
			inputCommand.read_pod(msg.Payload.begin(), msg.Payload.size());
			client.QueueReceive.push_back(msg);
			}
			break;
		case ::gpk::ENDPOINT_COMMAND_CONNECT:
			{
			::gpk::view_stream<char>					inputCommand				= {recv_buffer};
			info_printf("Received CONNECT response.");
			if(SOCKET_ERROR == ::recvfrom(sd, inputCommand.begin(), (int)(sizeof(::gpk::SEndpointCommand)), MSG_PEEK, (sockaddr*)&sa_remote, &server_length)) {
#if defined(GPK_WINDOWS)
				warning_printf("recvfrom failed with code 0x%X: '%s'.", ::WSAGetLastError(), ::gpk::getWindowsErrorAsString(::WSAGetLastError()).begin());
				::WSASetLastError(0);
#endif
			}
			inputCommand.read_pod(command);
			::gpk::tcpipAddressFromSockaddr(sa_remote, client.AddressRemote);
			}
			break;
		case ::gpk::ENDPOINT_COMMAND_PAYLOAD:
			{
			info_printf("Received generic response.");
			::gpk::view_stream<char>					inputCommand				= {recv_buffer};
			const uint32_t								sizeToRead					= sizeof(::gpk::SEndpointCommand) + command.PayloadBytes;
			if(SOCKET_ERROR == ::recvfrom(sd, inputCommand.begin(), (int)sizeToRead, MSG_PEEK, (sockaddr*)&sa_remote, &server_length)) {
#if defined(GPK_WINDOWS)
				warning_printf("recvfrom failed with code 0x%X: '%s'.", ::WSAGetLastError(), ::gpk::getWindowsErrorAsString(::WSAGetLastError()).begin());
				::WSASetLastError(0);
#endif
			}
			::dop::STCPIPEndpointMessage				msg							= {};
			int32_t										payloadSize					= 0;
			inputCommand.read_pod(msg.Command);
			inputCommand.read_pod((ubyte_t*)&payloadSize, command.PayloadBytes);
			msg.Payload.resize((int32_t)payloadSize);
			inputCommand.read_pod(msg.Payload.begin(), msg.Payload.size());
			client.QueueReceive.push_back(msg);
			}
			break;
		}
	}
	::recvfrom(sd, (char*)&command, (int)sizeof(::gpk::SEndpointCommand), 0, (sockaddr*)&sa_remote, &server_length);
	sdsafe.Handle							= INVALID_SOCKET;
	return 0;
}

		::gpk::error_t					run							(::dop::STCPIPNode& client)										{

	closesocket(sd);
	return 0;
}

//		case ::gpk::ENDPOINT_COMMAND_TIME:
//			{
//			::gpk::view_stream<char>					inputCommand				= {recv_buffer};
//			info_printf("Received TIME response.");
//			if(SOCKET_ERROR == ::recvfrom(sd, inputCommand.begin(), (int)(sizeof(::gpk::SEndpointCommand) + sizeof(uint64_t)), MSG_PEEK, (sockaddr*)&sa_remote, &server_length)) {
//#if defined(GPK_WINDOWS)
//				warning_printf("recvfrom failed with code 0x%X: '%s'.", ::WSAGetLastError(), ::gpk::getWindowsErrorAsString(::WSAGetLastError()).begin());
//				::WSASetLastError(0);
//#endif
//			}
//			::std::chrono::system_clock::time_point		now							= ::std::chrono::system_clock::now();
//			int64_t										current_time				= ::std::chrono::system_clock::to_time_t(now);					/* Time received */
//			char										curtime [16]				= {};	/* Display time */
//			inputCommand.read_pod(command);
//			inputCommand.read_pod(current_time);
//			client.RemoteTime						= current_time;
//			ctime_s(curtime, 256, &current_time);
//			info_printf("Current time: %s.", curtime);
//			}
//			break;
//		case ::gpk::ENDPOINT_COMMAND_PING:
//			{
//			::gpk::view_stream<char>					inputCommand				= {recv_buffer};
//			info_printf("Received PING response.");
//			if(SOCKET_ERROR == ::recvfrom(sd, inputCommand.begin(), (int)(sizeof(::gpk::SEndpointCommand)), MSG_PEEK, (sockaddr*)&sa_remote, &server_length)) {
//#if defined(GPK_WINDOWS)
//				warning_printf("recvfrom failed with code 0x%X: '%s'.", ::WSAGetLastError(), ::gpk::getWindowsErrorAsString(::WSAGetLastError()).begin());
//				::WSASetLastError(0);
//#endif
//			}
//			inputCommand.read_pod(command);
//			}
//			break;
