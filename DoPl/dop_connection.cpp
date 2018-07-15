#include "dop_connection.h"
#include "gpk_log.h"

::gpk::error_t								dop::commandByteSend			(SOCKET socket, const ::gpk::SIPv4 & addressRemote, const ::gpk::SEndpointCommand & commandToSend)	{ 
	sockaddr_in										sa_remote						= {AF_INET, htons(addressRemote.Port)}; 
	sa_remote.sin_addr.S_un.S_un_b.s_b1			= addressRemote.IP[0];
	sa_remote.sin_addr.S_un.S_un_b.s_b2			= addressRemote.IP[1];
	sa_remote.sin_addr.S_un.S_un_b.s_b3			= addressRemote.IP[2];
	sa_remote.sin_addr.S_un.S_un_b.s_b4			= addressRemote.IP[3];
	gpk_necall(::sendto(socket, (const char*)&commandToSend, (int)sizeof(::gpk::SEndpointCommand), 0, (sockaddr*)&sa_remote, sizeof(sockaddr_in)), "Error transmitting data.");
	return 0; 
}

::gpk::error_t								dop::commandBytePeek			(SOCKET socket, ::gpk::SIPv4 & addressRemote, ::gpk::SEndpointCommand & commandPeeked)	{ 
	sockaddr_in										sa_remote						= {AF_INET, };				
	int												len								= (int)sizeof(sockaddr_in);
	gpk_necall(::recvfrom(socket, (char*)&commandPeeked, (int)sizeof(::gpk::SEndpointCommand), 0, (sockaddr*)&sa_remote, &(len = (int)sizeof(sockaddr_in))), "Error transmitting data.");
	addressRemote.IP[0]							= sa_remote.sin_addr.S_un.S_un_b.s_b1;
	addressRemote.IP[1]							= sa_remote.sin_addr.S_un.S_un_b.s_b2;
	addressRemote.IP[2]							= sa_remote.sin_addr.S_un.S_un_b.s_b3;
	addressRemote.IP[3]							= sa_remote.sin_addr.S_un.S_un_b.s_b4;
	addressRemote.Port							= ntohs(sa_remote.sin_port);
	return 0; 
}
