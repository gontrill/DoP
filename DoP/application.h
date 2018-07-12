#include "gpk_stdsocket.h"
#include "gpk_framework.h"
#include "gpk_gui.h"
#include "gpk_tcpip.h"

#include <mutex>

#ifndef APPLICATION_H_2078934982734
#define APPLICATION_H_2078934982734

namespace gme // I'm gonna use a different namespace in order to test a few things about the macros.
{
	struct SClient {
		::gpk::SIPv4														AddressLocal						= {{192, 168, 1, 79}, 6668, };
		::gpk::SIPv4														AddressRemote						= {{192, 168, 1, 79}, 6667, };
		SOCKET																Socket								= INVALID_SOCKET;
	};

	struct SApplication {
		::gpk::SFramework													Framework;
		::gpk::SImage<::gpk::SColorBGRA>									TextureFont							= {};
		::gpk::ptr_obj<::gpk::SRenderTarget<::gpk::SColorBGRA, uint32_t>>	Offscreen							= {};

		int32_t																IdExit								= -1;

		::std::mutex														LockGUI;
		::std::mutex														LockRender;

		SClient																Client								= {};

																			SApplication						(::gpk::SRuntimeValues& runtimeValues)	noexcept	: Framework(runtimeValues)		{}
	};

	typedef ::std::lock_guard<::std::mutex>								mutex_guard;

} // namespace

#endif // APPLICATION_H_2078934982734
