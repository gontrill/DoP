#include "gpk_framework.h"
#include "gpk_gui.h"
#include "gpk_tcpip.h"
#include "gpk_stdsocket.h"

#include <mutex>

#ifndef APPLICATION_H_2078934982734
#define APPLICATION_H_2078934982734

namespace gme // I'm gonna use a different namespace in order to test a few things about the macros.
{
	struct SServerClient {
		::gpk::SIPv4							Address						= {{192, 168, 1, 79}, 6667, };
		SOCKET									Socket						= INVALID_SOCKET;
	};

	struct SServer {
		::gpk::SIPv4							Address						= {{192, 168, 1, 79}, 6667, };
		bool									Listening					= false;
		bool									Running						= false;
		::gpk::array_pod<SServerClient>			Clients						= {};		
		SOCKET									Socket						= INVALID_SOCKET;
	};

	struct SApplication {
		::gpk::SFramework													Framework;
		::gpk::SImage<::gpk::SColorBGRA>									TextureFont							= {};
		::gpk::ptr_obj<::gpk::SRenderTarget<::gpk::SColorBGRA, uint32_t>>	Offscreen							= {};

		int32_t																IdExit								= -1;

		::std::mutex														LockGUI;
		::std::mutex														LockRender;
		::gme::SServer														Server;

																			SApplication		(::gpk::SRuntimeValues& runtimeValues)	: Framework(runtimeValues)		{}
	};

	typedef ::std::lock_guard<::std::mutex>								mutex_guard;

} // namespace

#endif // APPLICATION_H_2078934982734
