// http://www.gomorgan89.com 
#include "gpk_windows.h"
#include "gpk_stdsocket.h"
#include "gpk_endpoint_command.h"
#include "gpk_view_stream.h"
#include "application.h"

#include <time.h>

#if defined(GPK_WINDOWS)
#	include <process.h>
#endif

::gpk::error_t						run							(::gme::SServer& server);
void								run_thread					(void * server)									{ error_if(errored(run(*(::gme::SServer*)server)), "Listening thread exited with error."); }
int									serverListen				(::gme::SServer& server)						{
	server.Running						= true;
	_beginthread(::run_thread, 0, &server);
	return 0;
}

::gpk::error_t						handleRequest				(::gme::SServer& server, ::gpk::ENDPOINT_COMMAND in_command, sockaddr_in sa_server, sockaddr_in sa_client)		{
	SOCKET									sd							= server.Socket;
	int										client_length				= (int)sizeof(struct sockaddr_in);	// Length of client struct */
	char									send_buffer	[256]			= {};				/* Name of the server */
	::gpk::SEndpointCommand					command						= {};
	switch(in_command) {
	default	: break;
	case ::gpk::ENDPOINT_COMMAND_TIME:
		{	// Check for time request */
		info_printf("Processing TIME request.");
		const int64_t									current_time				= time(NULL);				// Get current time */
		::gpk::view_stream<char>						commandToSend				= {send_buffer};
		command										= {::gpk::ENDPOINT_COMMAND_TIME, ::gpk::ENDPOINT_MESSAGE_TYPE_RESPONSE};
		commandToSend.write_pod(command);
		commandToSend.write_pod(current_time);
		ree_if(sendto(sd, commandToSend.begin(), commandToSend.CursorPosition, 0, (struct sockaddr *)&sa_client, client_length) != (int32_t)commandToSend.CursorPosition, "Error sending datagram.\n");
		}
		break;
	case ::gpk::ENDPOINT_COMMAND_PING:
		{	// Check for ping request */
		info_printf("Processing PING request.");
		::gpk::view_stream<char>						commandToSend				= {send_buffer};
		command										= {::gpk::ENDPOINT_COMMAND_PING, ::gpk::ENDPOINT_MESSAGE_TYPE_RESPONSE};
		commandToSend.write_pod(command);
		ree_if(sendto(sd, commandToSend.begin(), commandToSend.CursorPosition, 0, (struct sockaddr *)&sa_client, client_length) != (int32_t)commandToSend.CursorPosition, "Error sending datagram.\n");
		}
		break;
	case ::gpk::ENDPOINT_COMMAND_DISCONNECT:
		{	// Check for ping request */
		bool isSame 
			=  sa_server.sin_addr.S_un.S_un_b.s_b1 == sa_client.sin_addr.S_un.S_un_b.s_b1
			&& sa_server.sin_addr.S_un.S_un_b.s_b2 == sa_client.sin_addr.S_un.S_un_b.s_b2
			&& sa_server.sin_addr.S_un.S_un_b.s_b3 == sa_client.sin_addr.S_un.S_un_b.s_b3
			&& sa_server.sin_addr.S_un.S_un_b.s_b4 == sa_client.sin_addr.S_un.S_un_b.s_b4
			;
		if(isSame && false == server.Listening) {
			info_printf("Processing DISCONNECT request.");
			return 1;
		}
		}
		break;
	}
	return 0;
}

::gpk::error_t						run							(::gme::SServer& server)		{
	::gpk::SIPv4							& local						= server.Address;

	int										bytes_received				= 0;					/* Bytes received from client */
	SOCKET									& sd						= server.Socket = socket(AF_INET, SOCK_DGRAM, 0);		/* Socket descriptor of server */
	ree_if(sd == INVALID_SOCKET, "Could not create socket.\n");
	::gpk::auto_socket_close				sdsafe						= {};
	sdsafe.Handle						= sd;

	/* Set family and port */
	struct sockaddr_in						sa_server					= {};			/* Information about the server */
	sa_server.sin_family				= AF_INET;
	sa_server.sin_port					= htons(local.Port);

	bool usehostaddress = true;		/* Set address automatically if desired */
	if (usehostaddress) {
		::gpk::tcpipAddress(0, 0, ::gpk::TRANSPORT_PROTOCOL_UDP
			, &sa_server.sin_addr.S_un.S_un_b.s_b1
			, &sa_server.sin_addr.S_un.S_un_b.s_b2
			, &sa_server.sin_addr.S_un.S_un_b.s_b3
			, &sa_server.sin_addr.S_un.S_un_b.s_b4
			);
	}
	else {	/* Otherwise assign it manually */
		sa_server.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)local.IP[0];
		sa_server.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)local.IP[1];
		sa_server.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)local.IP[2];
		sa_server.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)local.IP[3];
	}

	//int32_t value = 1000;
	//setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&value, 4);
	// Set the socket I/O mode: In this case FIONBIO
	// enables or disables the blocking mode for the
	// socket based on the numerical value of iMode.
	// If iMode = 0, blocking is enabled;
	// If iMode != 0, non-blocking mode is enabled.
	//u_long	iMode = 1; // 1 == nonbolcking
	//int32_t iResult = ioctlsocket(sd, FIONBIO, &iMode);
	//if (iResult != NO_ERROR)
	//	printf("ioctlsocket failed with error: %ld\n", iResult);
	gpk_necall(bind(sd, (struct sockaddr *)&sa_server, sizeof(struct sockaddr_in)), "Could not bind name to socket.\n");	// Bind address to socket */
	// Print out server information */
	info_printf("Server running on %u.%u.%u.%u.", (unsigned char)sa_server.sin_addr.S_un.S_un_b.s_b1
												, (unsigned char)sa_server.sin_addr.S_un.S_un_b.s_b2
												, (unsigned char)sa_server.sin_addr.S_un.S_un_b.s_b3
												, (unsigned char)sa_server.sin_addr.S_un.S_un_b.s_b4);
	server.Listening = true;
	while (server.Listening) {		// Loop and get data from clients */
		int												client_length				= (int)sizeof(struct sockaddr_in);	// Length of client struct */
		::gpk::SEndpointCommand							command						= {};
		struct sockaddr_in								sa_client					= {};			// Information about the client */
		bytes_received								= recvfrom(sd, (char*)&command, sizeof(::gpk::SEndpointCommand), MSG_PEEK, (struct sockaddr *)&sa_client, &client_length);		// Receive bytes from client */
		if(SOCKET_ERROR == bytes_received) {
#if defined(GPK_WINDOWS)
			const uint32_t									lastError					= WSAGetLastError();
			bi_if(lastError != WSAEINTR, "Listening socket was interrupted.");
			be_if(lastError != WSAEMSGSIZE && lastError != WSAEINTR, "Could not receive datagram: 0x%X '%s'.", lastError, ::gpk::getWindowsErrorAsString(lastError).begin());		
			WSASetLastError(0);
#endif
		}
		bytes_received						= recvfrom(sd, (char*)&command, sizeof(::gpk::SEndpointCommand), 0, (struct sockaddr *)&sa_client, &client_length);		// Receive bytes from client */
		bi_if(1 == ::handleRequest(server, command.Command, sa_server, sa_client), "Server received a close message.");
	}
	server.Listening							= false;
	sdsafe.Handle								= INVALID_SOCKET;
	closesocket(sd);
	server.Running								= false;
	return 0;
}
