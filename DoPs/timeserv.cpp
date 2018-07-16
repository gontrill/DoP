// http://www.gomorgan89.com 
#include "application.h"

#include "gpk_stdsocket.h"
#include "gpk_endpoint_command.h"
#include "gpk_view_stream.h"

#include <time.h>

#if defined(GPK_WINDOWS)
#	include "gpk_windows.h"
#	include <process.h>
#endif

::gpk::error_t									run										(::gme::SServer& server);
static void										run_thread								(void * server)									{ error_if(errored(run(*(::gme::SServer*)server)), "Listening thread exited with error."); }
int												serverListen							(::gme::SServer& server)						{
	server.Running									= true;
	_beginthread(::run_thread, 0, &server);
	return 0;
}

static ::gpk::error_t							handleConnect							(::gme::SServer& server, ::gpk::SEndpointCommand in_command, const ::gpk::SIPv4& addressLocal, sockaddr_in sa_client)		{
	info_printf("Processing CONNECT request.");
	switch(in_command.PayloadBytes) {
	case 0: {
		::gpk::ptr_obj<::dop::STCPIPNode>					client;
		client->AddressLocal							= server.Address;
		::gpk::tcpipAddressFromSockaddr(sa_client, client->AddressRemote);
		client->Mode									= ::dop::TCPIP_NODE_MODE_HOST;
		client->State									= ::dop::TCPIP_NODE_STATE_HANDSHAKE_0;
		client->QueueSend.push_back({{::gpk::ENDPOINT_COMMAND_CONNECT, 0, ::gpk::ENDPOINT_MESSAGE_TYPE_RESPONSE}, });
		client->AddressLocal.Port						= 0;

		client->SocketSend								= socket(AF_INET, SOCK_DGRAM, 0);
		ree_if(client->SocketSend == INVALID_SOCKET, "Could not create socket.");
		::gpk::auto_socket_close					sdsafe						= {};
		sdsafe.Handle							= client->SocketSend;
		::gpk::SIPv4								& addrLocal					= client->AddressLocal;
		sockaddr_in									sa_local					; /* Information about the client */
		::gpk::tcpipAddressToSockaddr(addrLocal, sa_local);
		gpk_necall(::bind(client->SocketSend, (sockaddr*)&sa_local, sizeof(sockaddr_in)), "Cannot bind address to socket.");
		sdsafe.Handle							= 0;
		{
			::gme::mutex_guard									lock								(server.LockClients);
			server.Clients.push_back(client);
		}
	} break;
	case 1: break;
	case 2: break;
	case 3: break;
	}
	sa_client;addressLocal;
	return 0;
}

static ::gpk::error_t							handleRequest							(::gme::SServer& server, ::gpk::SEndpointCommand in_command, const ::gpk::SIPv4& addressLocal, sockaddr_in sa_client)		{
	SOCKET												sd										= server.Socket;
	//int													client_length							= (int)sizeof(struct sockaddr_in);	// Length of client struct */
	char												send_buffer	[16]						= {};				/* Name of the server */
	::gpk::SEndpointCommand								send_command							= {};
	switch(in_command.Command) {
	default	: break;
	case ::gpk::ENDPOINT_COMMAND_TIME:
		{	// Check for time request
		info_printf("Processing TIME request.");
		::std::chrono::system_clock::time_point				nowclock								= std::chrono::system_clock::now();
		const int64_t										current_time							= std::chrono::system_clock::to_time_t(nowclock);
		::gpk::view_stream<char>							commandToSend							= {send_buffer};
		send_command									= {::gpk::ENDPOINT_COMMAND_TIME, 8, ::gpk::ENDPOINT_MESSAGE_TYPE_RESPONSE};
		commandToSend.write_pod(send_command);
		commandToSend.write_pod(current_time);
		ree_if(sendto(sd, commandToSend.begin(), commandToSend.CursorPosition, 0, (sockaddr*)&sa_client, (int)sizeof(sockaddr_in)) != (int32_t)commandToSend.CursorPosition, "Error sending datagram.");
		}
		break;
	case ::gpk::ENDPOINT_COMMAND_PING:
		{	// Check for ping request
		info_printf("Processing PING request.");
		::gpk::view_stream<char>							commandToSend							= {send_buffer};
		send_command									= {::gpk::ENDPOINT_COMMAND_PING, 0, ::gpk::ENDPOINT_MESSAGE_TYPE_RESPONSE};
		commandToSend.write_pod(send_command);
		ree_if(sendto(sd, commandToSend.begin(), commandToSend.CursorPosition, 0, (sockaddr*)&sa_client, (int)sizeof(sockaddr_in)) != (int32_t)commandToSend.CursorPosition, "Error sending datagram.");
		}
		break;
	case ::gpk::ENDPOINT_COMMAND_CONNECT:
		{	// Check for ping request */
		handleConnect(server, in_command, addressLocal, sa_client);
		}
		break;
	case ::gpk::ENDPOINT_COMMAND_DISCONNECT:
		{	// Check for ping request */
		bool isSame 
			=  addressLocal.IP[0] == sa_client.sin_addr.S_un.S_un_b.s_b1
			&& addressLocal.IP[1] == sa_client.sin_addr.S_un.S_un_b.s_b2
			&& addressLocal.IP[2] == sa_client.sin_addr.S_un.S_un_b.s_b3
			&& addressLocal.IP[3] == sa_client.sin_addr.S_un.S_un_b.s_b4
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

::gpk::error_t									runClientUpdate							(::gme::SServer& server)		{
	while(server.Listening)	{
		fd_set												sread;
		fd_set												sexcp;
		::gpk::array_pod<SOCKET>							reads	;
		::gpk::array_pod<SOCKET>							writs	;
		::gpk::array_pod<SOCKET>							excps	;
		::gpk::array_pod<int32_t>							clients	;
		{
			::gme::mutex_guard									lock								(server.LockClients);
			for(uint32_t iClient = 0; iClient < server.Clients.size(); ++iClient) {
				if(INVALID_SOCKET != server.Clients[iClient]->SocketReceive) {
					sread.fd_array[iClient]						= server.Clients[iClient]->SocketReceive;
					clients.push_back(iClient);
				}
			}
		}
		sread.fd_count									= clients.size();
		if(sread.fd_count) {
			gpk_necall(select(0, &sread, 0, &sexcp, 0), "Fuck!");
			{
				::gme::mutex_guard									lock								(server.LockClients);
				for(uint32_t iReady = 0; iReady < sread.fd_count; ++iReady) {
					SOCKET												readys								= sread.fd_array[iReady];
					uint32_t											indexClient							= 0;
					for(; indexClient < clients.size(); ++indexClient) {
						::gpk::ptr_obj<::dop::STCPIPNode>					& client							= server.Clients[clients[indexClient]];
						if(0 == client)
							continue;
						if(client->SocketReceive == readys) {
							int													client_length						= (int)sizeof(sockaddr_in);	// Length of client struct */
							::gpk::SEndpointCommand								command								= {};
							sockaddr_in											sa_client							= {};			// Information about the client */
							int32_t												bytes_received						= recvfrom(client->SocketReceive, (char*)&command, sizeof(::gpk::SEndpointCommand), MSG_PEEK, (sockaddr*)&sa_client, &client_length);		// Receive bytes from client */
							ree_if(errored(bytes_received), "");
						}
					}
				}
			}
		}
	}
	return 0;
}

static void								runClientUpdate							(void* server)		{
	::runClientUpdate(*(::gme::SServer*)server);
}

::gpk::error_t									run										(::gme::SServer& server)		{
	::gpk::SIPv4										& addrLocal								= server.Address;
	int													bytes_received							= 0;					/* Bytes received from client */
	SOCKET												& sd									= server.Socket = socket(AF_INET, SOCK_DGRAM, 0);		/* Socket descriptor of server */
	ree_if(sd == INVALID_SOCKET, "Could not create socket.");
	::gpk::auto_socket_close							sdsafe									= {};
	sdsafe.Handle									= sd;

	/* Set family and port */
	sockaddr_in											sa_server								= {};			/* Information about the server */
	::gpk::tcpipAddressToSockaddr(addrLocal, sa_server);
	gpk_necall(bind(sd, (sockaddr *)&sa_server, sizeof(sockaddr_in)), "This could be caused by incorrect address/port combination.");	// Bind address to socket */
	info_printf("Server running on %u.%u.%u.%u:%u."	, (uint32_t)addrLocal.IP[0]
													, (uint32_t)addrLocal.IP[1]
													, (uint32_t)addrLocal.IP[2]
													, (uint32_t)addrLocal.IP[3]
													, (uint32_t)addrLocal.Port
													);
	server.Listening								= true;
	_beginthread(::runClientUpdate, 0, &server);
	while (server.Listening) {		// Loop and get data from clients */
		int													client_length							= (int)sizeof(sockaddr_in);	// Length of client struct */
		::gpk::SEndpointCommand								command									= {};
		sockaddr_in											sa_client								= {};			// Information about the client */
		bytes_received									= recvfrom(sd, (char*)&command, sizeof(::gpk::SEndpointCommand), MSG_PEEK, (sockaddr*)&sa_client, &client_length);		// Receive bytes from client */
		if(SOCKET_ERROR == bytes_received) {
#if defined(GPK_WINDOWS)
			const uint32_t										lastError								= WSAGetLastError();
			bi_if(lastError != WSAEINTR, "Listening socket was interrupted.");
			be_if(lastError != WSAEMSGSIZE && lastError != WSAEINTR, "Could not receive datagram: 0x%X '%s'.", lastError, ::gpk::getWindowsErrorAsString(lastError).begin());		
			WSASetLastError(0);
#else
#	error "Not implemented"
#endif
		}
		bytes_received									= recvfrom(sd, (char*)&command, sizeof(::gpk::SEndpointCommand), 0, (sockaddr*)&sa_client, &client_length);		// Receive bytes from client */
		bi_if(1 == ::handleRequest(server, command, addrLocal, sa_client), "Server received a close message.");
	}
	server.Listening								= false;
	sdsafe.Handle									= INVALID_SOCKET;
	closesocket(sd);
	server.Running									= false;
	info_printf("Server on %u.%u.%u.%u:%u terminating."	, (uint32_t)addrLocal.IP[0]
														, (uint32_t)addrLocal.IP[1]
														, (uint32_t)addrLocal.IP[2]
														, (uint32_t)addrLocal.IP[3]
														, (uint32_t)addrLocal.Port
														);
	return 0;
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


::gpk::error_t														gme::serverUpdate				(SServer& server)			{
	::gme::mutex_guard														lock							(server.LockClients);
	for(uint32_t iClient = 0; iClient < server.Clients.size(); ++iClient) {
		::dop::STCPIPNode														& client						= *server.Clients[iClient];
		::dop::tcpipNodeUpdate(client);
		for(uint32_t iSend = 0; iSend < client.QueueSend.size(); ++iSend) {
			::dop::STCPIPEndpointMessage							& message				= client.QueueSend[iSend];
			::gpk::array_pod<byte_t>								sendBuffer;
			sendBuffer.append((byte_t*)&message.Command, sizeof(::gpk::SEndpointCommand));
			if(message.Payload.size())
				sendBuffer.append(message.Payload.begin(), message.Payload.size());

			sockaddr_in											sa_client								= {};			// Information about the client */
			::gpk::tcpipAddressToSockaddr(client.AddressRemote, sa_client);
			ree_if(sendto(client.SocketSend, sendBuffer.begin(), sendBuffer.size(), 0, (sockaddr*)&sa_client, (int)sizeof(sockaddr_in)) != (int32_t)sendBuffer.size(), "Error sending datagram.");
			client.QueueSent.push_back(message);
		}
		client.QueueSend.clear();
	}
	return 0;

}
