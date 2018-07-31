// http://www.gomorgan89.com 
#include "dop_tcpip_node_udp.h"

//#include "gpk_stdsocket.h"
#include "gpk_endpoint_command.h"
#include "gpk_view_stream.h"

#include "gpk_chrono.h"

#if defined(GPK_WINDOWS)
#	include "gpk_windows.h"
#	include <process.h>
#endif

typedef ::std::lock_guard<::std::mutex>								mutex_guard;

::gpk::error_t									run										(::dop::SServer& server);
static void										run_thread								(void * server)									{ error_if(errored(run(*(::dop::SServer*)server)), "Listening thread exited with error."); }
int												dop::serverListen						(::dop::SServer& server)						{
	server.Running									= true;
	_beginthread(::run_thread, 0, &server);
	return 0;
}

static ::gpk::error_t							handleConnect							(::dop::SServer& server, ::gpk::SEndpointCommand in_command, sockaddr_in sa_client)		{
	::gpk::SIPv4										remoteJustReceived						;;
	::gpk::tcpipAddressFromSockaddr(sa_client, remoteJustReceived);
	info_printf("Processing CONNECT request. Stage: %u. From address: %u.%u.%u.%u:%u.", (uint32_t)in_command.Payload
		, (uint32_t)remoteJustReceived.IP[0]
		, (uint32_t)remoteJustReceived.IP[1]
		, (uint32_t)remoteJustReceived.IP[2]
		, (uint32_t)remoteJustReceived.IP[3]
		, (uint32_t)remoteJustReceived.Port
		);
	switch(in_command.Payload) {
	case 0: 
		{
		::gpk::ptr_obj<::dop::STCPIPNode>					client;
		client->AddressLocal							= server.Address;
		::gpk::tcpipAddressFromSockaddr(sa_client, client->AddressRemote);
		client->Mode									= ::dop::TCPIP_NODE_MODE_HOST;
		client->State									= ::dop::TCPIP_NODE_STATE_HANDSHAKE_0;
		client->QueueSend.push_back({{::gpk::ENDPOINT_COMMAND_CONNECT, 0, ::gpk::ENDPOINT_MESSAGE_TYPE_RESPONSE}, });
		client->AddressLocal.Port						= 0;

		client->SocketSend								= socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		ree_if(client->SocketSend == INVALID_SOCKET, "Could not create socket.");
		::gpk::auto_socket_close							sdsafe						= {};
		sdsafe.Handle									= client->SocketSend;
		::gpk::SIPv4										& addrLocal					= client->AddressLocal;
		sockaddr_in											sa_local					; /* Information about the client */
		::gpk::tcpipAddressToSockaddr(addrLocal, sa_local);
		gpk_necall(::bind(client->SocketSend, (sockaddr*)&sa_local, sizeof(sockaddr_in)), "Cannot bind address to socket.");
		sockaddr_in											sin;
		int32_t												len							= (int32_t)sizeof(sin);
		gpk_necall(::getsockname(client->SocketSend, (sockaddr *)&sin, &len), "Failed to get socket information.");
		client->AddressLocal.Port						= ntohs(sin.sin_port);
		info_printf("Socket Send port number: %i.", (int32_t)client->AddressLocal.Port);
		{
			::mutex_guard										lock								(server.LockClients);
			gpk_necall(server.Clients.push_back(client), "Out of memory?");
		}
		sdsafe.Handle									= 0;
		} break;
	case 1: 
		{
		::dop::STCPIPNode									* pClientFound								= 0;
		for(uint32_t iClient = 0; iClient < server.Clients.size(); ++iClient) {
			::dop::STCPIPNode									* pClientTest								= 0;
			{
				::mutex_guard										lock				(server.LockClients);
				pClientTest										= server.Clients[iClient];
				if(0 == pClientTest)
					continue;
			}
			if( pClientTest->AddressRemote.IP[0] == remoteJustReceived.IP[0]
			 && pClientTest->AddressRemote.IP[1] == remoteJustReceived.IP[1]
			 && pClientTest->AddressRemote.IP[2] == remoteJustReceived.IP[2]
			 && pClientTest->AddressRemote.IP[3] == remoteJustReceived.IP[3]
			) {
				pClientFound									= pClientTest;
				break;
			}
		}
		ree_if(0 == pClientFound, "Client not found for address: %u.%u.%u.%u:%u."
			, (uint32_t)remoteJustReceived.IP[0]
			, (uint32_t)remoteJustReceived.IP[1]
			, (uint32_t)remoteJustReceived.IP[2]
			, (uint32_t)remoteJustReceived.IP[3]
			, (uint32_t)remoteJustReceived.Port
			);
		pClientFound->AddressLocal						= server.Address;
		pClientFound->PortReceive						= pClientFound->AddressRemote.Port;
		::gpk::tcpipAddressFromSockaddr(sa_client, pClientFound->AddressRemote);
		pClientFound->Mode								= ::dop::TCPIP_NODE_MODE_HOST;
		pClientFound->State								= ::dop::TCPIP_NODE_STATE_HANDSHAKE_1;
		pClientFound->AddressLocal.Port					= 0;
		pClientFound->SocketReceive						= socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		ree_if(pClientFound->SocketReceive == INVALID_SOCKET, "Could not create socket.");
		::gpk::auto_socket_close							sdsafe						= {};
		sdsafe.Handle									= pClientFound->SocketReceive;
		::gpk::SIPv4										& addrLocal					= pClientFound->AddressLocal;
		sockaddr_in											sa_local					; /* Information about the client */
		::gpk::tcpipAddressToSockaddr(addrLocal, sa_local);
		gpk_necall(::bind(pClientFound->SocketReceive, (sockaddr*)&sa_local, sizeof(sockaddr_in)), "Cannot bind address to socket.");
		sockaddr_in											sin;
		int32_t												len							= (int32_t)sizeof(sin);
		gpk_necall(::getsockname(pClientFound->SocketReceive, (sockaddr *)&sin, &len), "Failed to get socket information.");
		pClientFound->AddressLocal.Port					= ntohs(sin.sin_port);
		info_printf("Socket Receive port number: %i.", (int32_t)pClientFound->AddressLocal.Port);
		pClientFound->QueueSend.push_back({{::gpk::ENDPOINT_COMMAND_CONNECT, 1, ::gpk::ENDPOINT_MESSAGE_TYPE_RESPONSE}, });
		sdsafe.Handle									= 0;
		}
		break;
	case 2: 
		{
		::dop::STCPIPNode									* pClientFound								= 0;
		for(uint32_t iClient = 0; iClient < server.Clients.size(); ++iClient) {
			::dop::STCPIPNode									* pClientTest								= 0;
			{
				::mutex_guard										lock				(server.LockClients);
				pClientTest										= server.Clients[iClient];
				if(0 == pClientTest)
					continue;
			}
			if( pClientTest->AddressRemote.IP[0] == remoteJustReceived.IP[0]
			 && pClientTest->AddressRemote.IP[1] == remoteJustReceived.IP[1]
			 && pClientTest->AddressRemote.IP[2] == remoteJustReceived.IP[2]
			 && pClientTest->AddressRemote.IP[3] == remoteJustReceived.IP[3]
			) {
				pClientFound									= pClientTest;
				break;
			}
		}
		ree_if(0 == pClientFound, "Client not found for address: %u.%u.%u.%u:%u."
			, (uint32_t)remoteJustReceived.IP[0]
			, (uint32_t)remoteJustReceived.IP[1]
			, (uint32_t)remoteJustReceived.IP[2]
			, (uint32_t)remoteJustReceived.IP[3]
			, (uint32_t)remoteJustReceived.Port
			);
		pClientFound->State								= ::dop::TCPIP_NODE_STATE_HANDSHAKE_2;
		uint16_t											actualPortReceive						= pClientFound->AddressRemote.Port;
		pClientFound->AddressRemote.Port				= pClientFound->PortReceive;
		pClientFound->PortReceive						= actualPortReceive;
		pClientFound->QueueSend.push_back({{::gpk::ENDPOINT_COMMAND_CONNECT, 2, ::gpk::ENDPOINT_MESSAGE_TYPE_RESPONSE}, });
		}
		break;
	}
	return 0;
}

static ::gpk::error_t							handleRequest							(::dop::SServer& server, ::gpk::SEndpointCommand in_command, const ::gpk::SIPv4& addressLocal, sockaddr_in sa_client)		{
	SOCKET												sd										= server.Socket;
	//int													client_length							= (int)sizeof(struct sockaddr_in);	// Length of client struct */
	char												send_buffer	[16]						= {};				/* Name of the server */
	::gpk::SEndpointCommand								send_command							= {};
	switch(in_command.Command) {
	default	: break;
	case ::gpk::ENDPOINT_COMMAND_CONNECT	: handleConnect(server, in_command, sa_client); break;
	case ::gpk::ENDPOINT_COMMAND_TIME		:
		{	// Check for time request
		info_printf("Processing TIME request.");
		const int64_t										current_time							= ::gpk::timeCurrent();
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

static	::gpk::error_t							handleDisconnect					(::dop::STCPIPNode& client, ::gpk::SEndpointCommand in_command, sockaddr_in sa_client)		{
	::gpk::SIPv4										addrRemote							;
	::gpk::tcpipAddressFromSockaddr(sa_client, addrRemote);
	if(client.AddressRemote == addrRemote && in_command.Payload == 0 && in_command.Type == ::gpk::ENDPOINT_MESSAGE_TYPE_REQUEST) {
		safe_closesocket(client.SocketReceive);
		safe_closesocket(client.SocketSend);
		client.QueueReceive								= 
		client.QueueSend								= 
		client.QueueSent								= {};
		client.State									= ::dop::TCPIP_NODE_STATE_DISCONNECTED;
	}
	return 0;
}

::gpk::error_t									handleReceive						(::dop::SServer& server, ::dop::STCPIPNode& client)		{
	::gpk::SEndpointCommand								command								= {};
	int													client_length						= (int)sizeof(sockaddr_in);	// Length of client struct */
	sockaddr_in											sa_client							= {};			// Information about the client */
	int32_t												bytes_received						= recvfrom(client.SocketReceive, (char*)&command, sizeof(::gpk::SEndpointCommand), MSG_PEEK, (sockaddr*)&sa_client, &client_length);		// Receive bytes from client */
	::gpk::SIPv4										addrLocal							;
	::gpk::tcpipAddressFromSockaddr(sa_client, addrLocal);
	ree_if(errored(bytes_received) && WSAGetLastError() != WSAEMSGSIZE, "Failed to receive message from client.");
	char												rcv_buffer	[32]					;
	switch(command.Command) {
	case ::gpk::ENDPOINT_COMMAND_DISCONNECT	: handleDisconnect	(client, command, sa_client); break;
	case ::gpk::ENDPOINT_COMMAND_CONNECT	: handleConnect		(server, command, sa_client); break;
	case ::gpk::ENDPOINT_COMMAND_PAYLOAD: 
	{
		info_printf("Received PAYLOAD {%u, %u, %u} from %u.%u.%u.%u:%u."
			, (uint32_t)command.Command
			, (uint32_t)command.Payload
			, (uint32_t)command.Type
			, (uint32_t)addrLocal.IP[0]
			, (uint32_t)addrLocal.IP[1]
			, (uint32_t)addrLocal.IP[2]
			, (uint32_t)addrLocal.IP[3]
			, (uint32_t)addrLocal.Port
			);
		::gpk::view_stream<char>							inputCommand						= {rcv_buffer};
		const uint32_t										sizeToRead							= sizeof(::gpk::SEndpointCommand) + command.Payload;
		if(SOCKET_ERROR == ::recvfrom(client.SocketReceive, inputCommand.begin(), (int)sizeToRead, MSG_PEEK, (sockaddr*)&sa_client, &client_length)) {
#if defined(GPK_WINDOWS)
			warning_printf("recvfrom failed with code 0x%X: '%s'.", ::WSAGetLastError(), ::gpk::getWindowsErrorAsString(::WSAGetLastError()).begin());
			::WSASetLastError(0);
#endif
		}
		::dop::STCPIPEndpointMessage						msg							= {};
		uint32_t											payloadSize	[4]				= {};
		inputCommand.read_pod(msg.Command);
		inputCommand.read_pod((ubyte_t*)payloadSize, command.Payload);
		gpk_necall(msg.Payload.resize(payloadSize[0]), "Out of memory?");
		inputCommand.read_pod(msg.Payload.begin(), msg.Payload.size());
		gpk_necall(client.QueueReceive.push_back(msg), "Out of memory?");
	}
		break;
	default:
		info_printf("Received command {%u, %u, %u} from %u.%u.%u.%u:%u."
			, (uint32_t)command.Command
			, (uint32_t)command.Payload
			, (uint32_t)command.Type
			, (uint32_t)addrLocal.IP[0]
			, (uint32_t)addrLocal.IP[1]
			, (uint32_t)addrLocal.IP[2]
			, (uint32_t)addrLocal.IP[3]
			, (uint32_t)addrLocal.Port
			);
		break;
	}
	return 0;
}

::gpk::error_t									runClientUpdate							(::dop::SServer& server)		{
	while(server.Listening)	{
		fd_set												sread;
		::gpk::array_pod<SOCKET>							reads	;
		::gpk::array_pod<int32_t>							clients	;
		{
			::mutex_guard										lock								(server.LockClients);
			for(uint32_t iClient = 0; iClient < server.Clients.size(); ++iClient) {
				if(INVALID_SOCKET != server.Clients[iClient]->SocketReceive) {
					sread.fd_array[iClient]						= server.Clients[iClient]->SocketReceive;
					clients.push_back(iClient);
				}
			}
		}
		sread.fd_count									= clients.size();
		if(sread.fd_count) {
			timeval												timeOut									= {};
			timeOut.tv_sec									= 1;
			gpk_necall(select(0, &sread, 0, 0, &timeOut), "Fuck!");
			{
				for(uint32_t iReady = 0; iReady < sread.fd_count; ++iReady) {
					SOCKET												readys								= sread.fd_array[iReady];
					for(uint32_t indexClient = 0; indexClient < clients.size(); ++indexClient) {
						::dop::STCPIPNode									* pClient							= 0;
						{
							::mutex_guard										lock								(server.LockClients);
							::gpk::ptr_obj<::dop::STCPIPNode>					& rclient							= server.Clients[clients[indexClient]];
							if(0 == rclient)
								continue;
							pClient											= rclient;
						}
						if(pClient->SocketReceive == readys) {
							::gpk::SEndpointCommand								command								= {};
							error_if(errored(handleReceive(server, *pClient)), "Failed to handle request for client %u", indexClient);
							int													client_length						= (int)sizeof(sockaddr_in);	// Length of client struct */
							sockaddr_in											sa_client							= {};						// Information about the client */
							recvfrom(pClient->SocketReceive, (char*)&command, sizeof(::gpk::SEndpointCommand), 0, (sockaddr*)&sa_client, &client_length);
						}
					}
				}
			}
		}
	}
	return 0;
}

static	void									runClientUpdate							(void* server)					{ ::runClientUpdate(*(::dop::SServer*)server); }
static	::gpk::error_t							run										(::dop::SServer& server)		{
	::gpk::SIPv4										& addrLocal								= server.Address;
	int													bytes_received							= 0;					/* Bytes received from client */
	SOCKET												& sd									= server.Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);		/* Socket descriptor of server */
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

::gpk::error_t									dop::serverUpdate				(::dop::SServer& server)			{
	::mutex_guard										lock							(server.LockClients);
	for(uint32_t iClient = 0; iClient < server.Clients.size(); ++iClient) {
		::dop::STCPIPNode									& client						= *server.Clients[iClient];
		::dop::tcpipNodeUpdate(client);
		for(uint32_t iSend = 0; iSend < client.QueueSend.size(); ++iSend) {
			::dop::STCPIPEndpointMessage						& message						= client.QueueSend[iSend];
			::gpk::array_pod<byte_t>							sendBuffer;
			ce_if(errored(sendBuffer.append((byte_t*)&message.Command, sizeof(::gpk::SEndpointCommand))), "Out of memory?");
			ce_if(errored(sendBuffer.append((byte_t*)&message.Payload.size(), sizeof(uint32_t))), "Out of memory?");
			if(message.Payload.size())
				ce_if(errored(sendBuffer.append(message.Payload.begin(), message.Payload.size())), "Out of memory?");

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
			ce_if(sendto((message.Command.Command == ::gpk::ENDPOINT_COMMAND_CONNECT && message.Command.Payload == 1) ? client.SocketReceive : client.SocketSend, sendBuffer.begin(), sendBuffer.size(), 0, (sockaddr*)&sa_client, (int)sizeof(sockaddr_in)) != (int32_t)sendBuffer.size(), "Error sending datagram.");
			ce_if(errored(client.QueueSend.remove(iSend)), "Logic got broken!");
			ce_if(errored(client.QueueSent.push_back(message)), "Out of memory?");
		}
		//client.QueueSend.clear();
	}
	return 0;
}


			int													dop::serverShutdown				(::dop::SServer& server)						{
	server.Listening													= false;
	char																	commandbytes	[256]		= {};
	::gpk::view_stream<char>												commandToSend				= {commandbytes};
	::gpk::SEndpointCommand													command						= {::gpk::ENDPOINT_COMMAND_DISCONNECT, 0, ::gpk::ENDPOINT_MESSAGE_TYPE_REQUEST};
	commandToSend.write_pod(command);

	// Set family and port */
	::gpk::SIPv4															& local						= server.Address;
	sockaddr_in																sa_remote					= {};			/* Information about the server */
	sa_remote.sin_family												= AF_INET;
	sa_remote.sin_port													= htons(local.Port);
	sa_remote.sin_addr.S_un.S_un_b.s_b1									= (unsigned char)local.IP[0];
	sa_remote.sin_addr.S_un.S_un_b.s_b2									= (unsigned char)local.IP[1];
	sa_remote.sin_addr.S_un.S_un_b.s_b3									= (unsigned char)local.IP[2];
	sa_remote.sin_addr.S_un.S_un_b.s_b4									= (unsigned char)local.IP[3];
	while(server.Running) {
		warn_if(sendto(server.Socket, (const char*)&command, sizeof(::gpk::SEndpointCommand), 0, (sockaddr *)&sa_remote, (int)sizeof(sockaddr_in)) != (int32_t)sizeof(::gpk::SEndpointCommand), "Error sending disconnect command.");
		::gpk::sleep(10);
	}
	return 0;
}
