#include "gpk_framework.h"

#include "gpk_tcpip.h"
#include "gpk_stdsocket.h"

#include "dop_connection.h"
#include "dop_tcpip_node_udp.h"

#include <mutex>

#ifndef APPLICATION_H_2078934982734
#define APPLICATION_H_2078934982734

namespace gme // I'm gonna use a different namespace in order to test a few things about the macros.
{
	struct SServer {
					::gpk::SIPv4														Address						= {{192, 168, 1, 79}, 6667, };
					bool																Listening					= false;
					bool																Running						= false;
					::gpk::array_obj<::gpk::ptr_obj<::dop::STCPIPNode>>					Clients						= {};		
					SOCKET																Socket						= INVALID_SOCKET;
					::std::mutex														LockClients					;
	};

	struct SApplication {
					::gpk::SFramework													Framework;
					::gpk::SImage<::gpk::SColorBGRA>									TextureFont					= {};
					::gpk::ptr_obj<::gpk::SRenderTarget<::gpk::SColorBGRA, uint32_t>>	Offscreen					= {};
					int32_t																IdExit						= -1;
					::std::mutex														LockGUI						;
					::std::mutex														LockRender					;
					::gme::SServer														Server						;

																						SApplication				(::gpk::SRuntimeValues& runtimeValues)	: Framework(runtimeValues)		{}
	};

				::gpk::error_t														serverUpdate				(SServer& server);

	typedef		::std::lock_guard<::std::mutex>										mutex_guard;

} // namespace

#endif // APPLICATION_H_2078934982734
