// http://www.gomorgan89.com 
#include "gpk_tcpip.h"
#include "gpk_log.h"
#include "gpk_windows.h"
#include "gpk_safe.h"
#include "gpk_stdsocket.h"
#include "gpk_view_stream.h"
#include "gpk_endpoint_command.h"

#include <time.h>
#include <socketapi.h>

		int							run							()										{
	::gpk::SEndpointCommand					command						= {::gpk::ENDPOINT_COMMAND_PING, ::gpk::ENDPOINT_MESSAGE_TYPE_REQUEST};

	::gpk::SIPv4							addrRemote					= {{192, 168, 1, 79}, 6667, };
	char									host_name	[256]			= {};					/* Host name of this computer */
	SOCKET									sd							= socket(AF_INET, SOCK_DGRAM, 0);		/* Open a datagram socket */
	ree_if(sd == INVALID_SOCKET, "Could not create socket.\n");
	::gpk::auto_socket_close				sdsafe						= {};
	sdsafe.Handle						= sd;

	/* Set family and port */
	struct sockaddr_in						server						= {};					/* Information about the server */
	server.sin_family					= AF_INET;
	server.sin_port						= htons(addrRemote.Port);

	/* Set server address */
	server.sin_addr.S_un.S_un_b.s_b1	= addrRemote.IP[0];
	server.sin_addr.S_un.S_un_b.s_b2	= addrRemote.IP[1];
	server.sin_addr.S_un.S_un_b.s_b3	= addrRemote.IP[2];
	server.sin_addr.S_un.S_un_b.s_b4	= addrRemote.IP[3];

	/* Set family and port */
	struct sockaddr_in						local						= {};					/* Information about the client */
	local.sin_family					= AF_INET;
	local.sin_port						= htons(0);

	bool									resolveLocalHost			= true;
	if (resolveLocalHost) { 
		gethostname(host_name, sizeof(host_name));	// Get host name of this computer 
		::gpk::tcpipAddress(host_name, 0, 0, ::gpk::TRANSPORT_PROTOCOL_UDP
			, &local.sin_addr.S_un.S_un_b.s_b1
			, &local.sin_addr.S_un.S_un_b.s_b2
			, &local.sin_addr.S_un.S_un_b.s_b3
			, &local.sin_addr.S_un.S_un_b.s_b4
			);
	}
	else {
		::gpk::SIPv4							addrLocal					= {};
		local.sin_addr.S_un.S_un_b.s_b1		= addrLocal.IP[0];
		local.sin_addr.S_un.S_un_b.s_b2		= addrLocal.IP[1];
		local.sin_addr.S_un.S_un_b.s_b3		= addrLocal.IP[2];
		local.sin_addr.S_un.S_un_b.s_b4		= addrLocal.IP[3];
	}

	gpk_necall(bind		(sd, (struct sockaddr *)&local, sizeof(struct sockaddr_in)), "Cannot bind address to socket.");

	int										server_length				= sizeof(struct sockaddr_in);	/* Length of server struct */
	gpk_necall(sendto	(sd, (const char*)&command, (int)sizeof(::gpk::SEndpointCommand), 0, (struct sockaddr *)&server, server_length), "Error transmitting data.");

	
	if(SOCKET_ERROR == recvfrom(sd, (char *)&command, (int)sizeof(::gpk::SEndpointCommand), MSG_PEEK, (struct sockaddr *)&server, &server_length)) {
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
			if(SOCKET_ERROR == recvfrom(sd, inputCommand.begin(), (int)(sizeof(::gpk::SEndpointCommand) + sizeof(uint64_t)), 0, (struct sockaddr *)&server, &server_length)) {
#if defined(GPK_WINDOWS)
				warning_printf("recvfrom failed with code 0x%X: '%s'.", ::WSAGetLastError(), ::gpk::getWindowsErrorAsString(::WSAGetLastError()).begin());
				WSASetLastError(0);
#endif
			}
			time_t									current_time				= 0;					/* Time received */
			char									curtime [256]				= {};	/* Display time */
			inputCommand.read_pod(command);
			inputCommand.read_pod(current_time);
			ctime_s(curtime, 256, &current_time);
			info_printf("Current time: %s.", curtime);
			}
			break;
		case ::gpk::ENDPOINT_COMMAND_PING:
			{
			::gpk::view_stream<char>				inputCommand				= {host_name};
			info_printf("Received PING response.");
			if(SOCKET_ERROR == recvfrom(sd, inputCommand.begin(), (int)(sizeof(::gpk::SEndpointCommand)), 0, (struct sockaddr *)&server, &server_length)) {
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
		recvfrom(sd, (char*)&command, (int)sizeof(::gpk::SEndpointCommand), 0, (struct sockaddr *)&server, &server_length);

	sdsafe.Handle						= INVALID_SOCKET;
	closesocket(sd);
	return 0;
}
