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

static ::gpk::error_t												handleSentRequest						(::dop::STCPIPNode & node, ::dop::STCPIPEndpointMessage & message)											{ 
	return 0; 
}

static ::gpk::error_t												handleSentResponse						(::dop::STCPIPNode & node, ::dop::STCPIPEndpointMessage & message)											{ 
	return 0; 
}
static ::gpk::error_t												handleRequest							(::dop::STCPIPNode & node, ::dop::STCPIPEndpointMessage & message)											{ 
	return 0; 
}

static ::gpk::error_t												handleResponse							(::dop::STCPIPNode & node, ::dop::STCPIPEndpointMessage & message)											{ 
	return 0; 
}
static ::gpk::error_t												sendRequest								(::dop::STCPIPNode & node, ::dop::STCPIPEndpointMessage & message)											{ 
	return 0; 
}

static ::gpk::error_t												sendResponse							(::dop::STCPIPNode & node, ::dop::STCPIPEndpointMessage & message)											{ return 0; }

::gpk::error_t														dop::tcpipNodeUpdate					(::dop::STCPIPNode & node)																					{
	for(uint32_t iReceived = 0; iReceived < node.QueueReceive.size(); ++iReceived) {
		::dop::STCPIPEndpointMessage											& message								= node.QueueReceive[iReceived];
		switch(message.Command.Type) {
		case ::gpk::ENDPOINT_MESSAGE_TYPE_REQUEST	: handleRequest	(node, message); break;
		case ::gpk::ENDPOINT_MESSAGE_TYPE_RESPONSE	: handleResponse(node, message); break;
		}
	}
	for(uint32_t iSend = 0; iSend < node.QueueSend.size(); ++iSend) {
		::dop::STCPIPEndpointMessage											& message								= node.QueueSend[iSend];
		switch(message.Command.Type) {
		case ::gpk::ENDPOINT_MESSAGE_TYPE_REQUEST	: sendRequest	(node, message); break;
		case ::gpk::ENDPOINT_MESSAGE_TYPE_RESPONSE	: sendResponse	(node, message); break;
		}
	}
	for(uint32_t iSent = 0; iSent < node.QueueSent.size(); ++iSent) {
		::dop::STCPIPEndpointMessage											& message								= node.QueueSent[iSent];
		switch(message.Command.Type) {
		case ::gpk::ENDPOINT_MESSAGE_TYPE_REQUEST	: handleSentRequest	(node, message); break;
		case ::gpk::ENDPOINT_MESSAGE_TYPE_RESPONSE	: handleSentResponse(node, message); break;
		}
	}
	return 0;
}
