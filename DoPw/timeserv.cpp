// http://www.gomorgan89.com 
#include "application.h"

#include <ctime>

#if defined(GPK_WINDOWS)
#	include <process.h>
#endif

static constexpr	const uint32_t		BUFFER_SIZE											= 4096;

void									runClientThread										(void* httpServer)							{
	httpServer;
}

int										run													(::gme::SHTTPServer& httpServer)							{
	::gpk::SIPv4								& addrServer										= httpServer.AddressServer;
	::gpk::SIPv4								addrClient											= {};
	::gpk::auto_socket_close					sdSafeSrv											= {};
	httpServer.Listener						 = sdSafeSrv.Handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// Open a stream socket 
	ree_if(httpServer.Listener == INVALID_SOCKET, "Could not create socket.");
	
	::gpk::tcpipAddress(80, 0, ::gpk::TRANSPORT_PROTOCOL_TCP, addrServer);

#if defined(GPK_WINDOWS)
	_beginthread(runClientThread, 0, &httpServer);
#else
#	error "Not imlpemented."
#endif

	sockaddr_in									sa_server											= {AF_INET, htons(addrServer.Port)};		// Information about the server 
	::gpk::tcpipAddressToSockaddr(addrServer, sa_server);
	ree_if(bind(httpServer.Listener, (struct sockaddr *)&sa_server, sizeof(struct sockaddr_in)) == -1, "Could not bind name to socket.");
	info_printf("Server running on %u.%u.%u.%u."	/* Print out server information */
		, (uint32_t)addrServer.IP[0]
		, (uint32_t)addrServer.IP[1]
		, (uint32_t)addrServer.IP[2]
		, (uint32_t)addrServer.IP[3]
		);
	info_printf("Press CTRL + C to quit");
	while (1) {	// Loop and get data from clients
		int											sa_client_length										= (int)sizeof(struct sockaddr_in);							// Length of client struct 
		sockaddr_in									sa_client												= {};		// Information about the client 
		if(0 == listen(httpServer.Listener, 1)) {
			::gpk::auto_socket_close					sdSafeCli												= {};
			sdSafeCli.Handle						= accept(httpServer.Listener, (struct sockaddr *)&sa_client, &sa_client_length);	/* Receive bytes from client */
			ce_if(sdSafeCli == INVALID_SOCKET, "Could not receive datagram.");

			::gpk::ptr_pod<::gme::SHTTPServerClient>	newClient;
			::gpk::tcpipAddressFromSockaddr(sa_client, newClient->Address);
			newClient->Socket						= sdSafeCli;
			ce_if(errored(httpServer.Clients.push_back(newClient)), "Out of memory?");
			sdSafeCli.Handle						= 0;
		}
	}
	return 0;
}
