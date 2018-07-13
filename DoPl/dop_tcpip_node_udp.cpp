#include "dop_tcpip_node_udp.h"
#include "dop_connection.h"
#include "gpk_log.h"


::gpk::error_t														dop::tcpipNodeHandshake					(::dop::STCPIPNode & node, const ::gpk::SIPv4 & addressRemote)	{
	ree_if(addressRemote.Port == 0, "Invalid remote port.");

	return 0;	
}
