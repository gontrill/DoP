// http://www.gomorgan89.com 
#include "application.h"
#include "gpk_windows.h"
#include "gpk_safe.h"
#include "gpk_view_stream.h"
#include "gpk_endpoint_command.h"

#include <time.h>
#include <chrono>
#include <socketapi.h>

		int							run							(::gme::SClient& client)										{
	::gpk::SEndpointCommand					command						= {::gpk::ENDPOINT_COMMAND_PING, ::gpk::ENDPOINT_MESSAGE_TYPE_REQUEST};

	char									host_name	[256]			= {};					/* Host name of this computer */
	SOCKET									sd							= socket(AF_INET, SOCK_DGRAM, 0);		/* Open a datagram socket */
	ree_if(sd == INVALID_SOCKET, "Could not create socket.\n");
	::gpk::auto_socket_close				sdsafe						= {};
	sdsafe.Handle						= sd;

	::gpk::SIPv4							& addrLocal					= client.Connection.AddressLocal;
	sockaddr_in								sa_local					= {AF_INET, htons(addrLocal.Port)};					/* Information about the client */
	sa_local.sin_addr.S_un.S_un_b.s_b1	= addrLocal.IP[0];
	sa_local.sin_addr.S_un.S_un_b.s_b2	= addrLocal.IP[1];
	sa_local.sin_addr.S_un.S_un_b.s_b3	= addrLocal.IP[2];
	sa_local.sin_addr.S_un.S_un_b.s_b4	= addrLocal.IP[3];

	::gpk::SIPv4							& addrRemote				= client.Connection.AddressRemote;// = {{192, 168, 1, 79}, 6667, };
	sockaddr_in								server						= {AF_INET, htons(addrRemote.Port)};					/* Information about the server */
	server.sin_addr.S_un.S_un_b.s_b1	= addrRemote.IP[0];
	server.sin_addr.S_un.S_un_b.s_b2	= addrRemote.IP[1];
	server.sin_addr.S_un.S_un_b.s_b3	= addrRemote.IP[2];
	server.sin_addr.S_un.S_un_b.s_b4	= addrRemote.IP[3];
	gpk_necall(bind		(sd, (sockaddr *)&sa_local, sizeof(sockaddr_in)), "Cannot bind address to socket.");

	gpk_necall(sendto	(sd, (const char*)&command, (int)sizeof(::gpk::SEndpointCommand), 0, (sockaddr *)&server, sizeof(sockaddr_in)), "Error transmitting data.");

	int										server_length				= sizeof(sockaddr_in);	/* Length of server struct */
	if(SOCKET_ERROR == recvfrom(sd, (char *)&command, (int)sizeof(::gpk::SEndpointCommand), MSG_PEEK, (sockaddr *)&server, &server_length)) {
#if defined(GPK_WINDOWS)
		ree_if(::WSAGetLastError() != WSAEMSGSIZE, "recvfrom failed with code 0x%X: '%s'.", ::WSAGetLastError(), ::gpk::getWindowsErrorAsString(::WSAGetLastError()).begin());
#endif
	}
	if(command.Type == ::gpk::ENDPOINT_MESSAGE_TYPE_RESPONSE) {
		switch(command.Command) {
		case ::gpk::ENDPOINT_COMMAND_TIME:
			{
			::gpk::view_stream<char>				inputCommand				= {host_name};
			info_printf("Received TIME response.");
			if(SOCKET_ERROR == recvfrom(sd, inputCommand.begin(), (int)(sizeof(::gpk::SEndpointCommand) + sizeof(uint64_t)), 0, (sockaddr *)&server, &server_length)) {
#if defined(GPK_WINDOWS)
				warning_printf("recvfrom failed with code 0x%X: '%s'.", ::WSAGetLastError(), ::gpk::getWindowsErrorAsString(::WSAGetLastError()).begin());
				WSASetLastError(0);
#endif
			}
			::std::chrono::system_clock::time_point	now							= ::std::chrono::system_clock::now();
			int64_t									current_time				= ::std::chrono::system_clock::to_time_t(now);					/* Time received */
			char									curtime [256]				= {};	/* Display time */
			inputCommand.read_pod(command);
			inputCommand.read_pod(current_time);
			client.Time							= current_time;
			ctime_s(curtime, 256, &current_time);
			info_printf("Current time: %s.", curtime);
			}
			break;
		case ::gpk::ENDPOINT_COMMAND_PING:
			{
			::gpk::view_stream<char>				inputCommand				= {host_name};
			info_printf("Received PING response.");
			if(SOCKET_ERROR == recvfrom(sd, inputCommand.begin(), (int)(sizeof(::gpk::SEndpointCommand)), 0, (sockaddr *)&server, &server_length)) {
#if defined(GPK_WINDOWS)
				warning_printf("recvfrom failed with code 0x%X: '%s'.", ::WSAGetLastError(), ::gpk::getWindowsErrorAsString(::WSAGetLastError()).begin());
				WSASetLastError(0);
#endif
			}
			inputCommand.read_pod(command);
			}
			break;
		}
	}
	else
		recvfrom(sd, (char*)&command, (int)sizeof(::gpk::SEndpointCommand), 0, (sockaddr *)&server, &server_length);

	sdsafe.Handle						= INVALID_SOCKET;
	closesocket(sd);
	return 0;
}
