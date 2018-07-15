#include "dop_tcpip_node_udp.h"
#include "dop_connection.h"
#include "gpk_log.h"

#if defined(GPK_WINDOWS)
#	include <process.h>
#endif
// Step	- Message			- Payload	- Description:
// 0	- CONNECT_REQUEST	(Payload:0)	- El cliente  crea el socket send y envía el pedido de conexión. 
// 1	- CONNECT_RESPONSE	(Payload:0)	- El servidor crea un socket send y envía el pedido de conexión junto con el puerto del socket send. 
// 2	- CONNECT_REQUEST	(Payload:1)	- El cliente  crea el socket receive y envía el pedido de conexión junto con el puerto receive al socket send en el servidor.
// 3	- CONNECT_RESPONSE	(Payload:1)	- El servidor crea el socket receive y envía la confirmación al socket receive del cliente junto con el puerto del socket receive del servidor.
// 4	- CONNECT_REQUEST	(Payload:2)	- El cliente  envía el pedido de confirmación de la conexión a través del socket send al socket receive del servidor.
// 5	- CONNECT_RESPONSE	(Payload:2)	- El servidor envía la confirmación de la conexión a través del socket send al socket receive del cliente.
::gpk::error_t														tcpipNodeHandshake						(::dop::STCPIPNode & node);
::gpk::error_t														dop::tcpipNodeHandshake					(::dop::STCPIPNode & node, const ::gpk::SIPv4 & addressRemote)	{
	ree_if(addressRemote.Port == 0, "Invalid remote port.");
	node.AddressRemote													= addressRemote;
	return ::tcpipNodeHandshake(node);
}

::gpk::error_t														attemptHandshake						(::dop::STCPIPNode & node)										{
	::gpk::SIPv4															& addrLocal								= node.AddressLocal;
	::gpk::SIPv4															& addrRemote							= node.AddressRemote;
	SOCKET																	& socketSend							= node.SocketSend;
	SOCKET																	& socketReceive							= node.SocketReceive;

	::gpk::SEndpointCommand													command									= {::gpk::ENDPOINT_COMMAND_CONNECT, 0, ::gpk::ENDPOINT_MESSAGE_TYPE_REQUEST};

	char																	recv_buffer	[256]						= {};					// Host name of this computer */
	SOCKET																	sd										= ::socket(AF_INET, SOCK_DGRAM, 0);		// Open a datagram socket */
	ree_if(sd == INVALID_SOCKET, "Could not create socket.");
	::gpk::auto_socket_close													sdsafe								= {};
	sdsafe.Handle															= sd;

	sockaddr_in																	sa_local							; /* Information about the client */
	::gpk::tcpipAddressToSockaddr(addrLocal, sa_local);
	gpk_necall(::bind(sd, (sockaddr*)&sa_local, sizeof(sockaddr_in)), "Cannot bind address to socket.");

	sockaddr_in																	sa_remote							= {AF_INET, htons(addrRemote.Port)};					/* Information about the server */
	::gpk::tcpipAddressToSockaddr(addrRemote, sa_remote);
	gpk_necall(::sendto(sd, (const char*)&command, (int)sizeof(::gpk::SEndpointCommand), 0, (sockaddr*)&sa_remote, sizeof(sockaddr_in)), "Error transmitting data.");
	return 0;
}

::gpk::error_t														tcpipNodeHandshake						(::dop::STCPIPNode & node)										{
	ree_if(node.AddressRemote.Port == 0, "Invalid remote port.");

	return 0;	
}

::gpk::error_t														dop::tcpipNodeAccept					(::dop::STCPIPNode & node, const ::gpk::SIPv4 & addressRemote)	{

	return 0;
}
