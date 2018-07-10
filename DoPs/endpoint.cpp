#include "endpoint.h"
#include "gpk_log.h"
#include "gpk_windows.h"

#include <WinSock2.h>

::gpk::error_t								dop::communicationsInitialize					()																															{
#if defined(GPK_WINDOWS)
	::WSADATA										w												= {};	
	ree_if(::WSAStartup(0x0202, &w) != 0, "Could not open Windows sockets: 0x%X '%s'", WSAGetLastError(), ::gpk::getWindowsErrorAsString(WSAGetLastError()).begin());		
#endif
	return 0;
}

::gpk::error_t								dop::communicationsShutdown						()																															{
#if defined(GPK_WINDOWS)
	ree_if(::WSACleanup() != 0, "Could not shut down Windows sockets: 0x%X '%s'", WSAGetLastError(), ::gpk::getWindowsErrorAsString(WSAGetLastError()).begin());		// Open windows connection
#endif
	return 0;
}
