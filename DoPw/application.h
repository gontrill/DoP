#include "gpk_framework.h"
#include "gpk_gui.h"

#include "gpk_tcpip.h"
#include "gpk_stdsocket.h"

#include <mutex>

#ifndef APPLICATION_H_2078934982734
#define APPLICATION_H_2078934982734

namespace gme // I'm gonna use a different namespace in order to test a few things about the macros.
{
	struct SHTTPServerClient {
					::gpk::SIPv4															Address								= {{0, 0, 0, 0}, 80};
					SOCKET																	Socket								= INVALID_SOCKET;
	};

	struct SHTTPServer {
					::gpk::SIPv4															AddressServer						= {{0, 0, 0, 0}, 80};
					SOCKET																	Listener							= INVALID_SOCKET;
					::gpk::array_obj<::gpk::ptr_pod<SHTTPServerClient>>						Clients;
	};

	struct SApplication {
					::gpk::SFramework														Framework							;
					::gpk::SImage<::gpk::SColorBGRA>										TextureFont							= {};
					::gpk::SImage<::gpk::SColorBGRA>										VerticalAtlas						;
					::gpk::ptr_obj<::gpk::SRenderTarget<::gpk::SColorBGRA, uint32_t>>		Offscreen							= {};

					int32_t																	IdExit								= -1;
					int32_t																	IdMode								= -1;
					int32_t																	IdTheme								= -1;

					::std::mutex															LockGUI;
					::std::mutex															LockRender;

					SHTTPServer																Server;

																							SApplication		(::gpk::SRuntimeValues& runtimeValues)	: Framework(runtimeValues)		{}
	};

	typedef		::std::lock_guard<::std::mutex>											mutex_guard;

} // namespace

#endif // APPLICATION_H_2078934982734
