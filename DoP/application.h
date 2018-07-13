#include "gpk_framework.h"
#include "gpk_stdsocket.h"
#include "gpk_tcpip.h"

#include "dop_connection.h"

#include <mutex>

#ifndef APPLICATION_H_2078934982734
#define APPLICATION_H_2078934982734

namespace gme // I'm gonna use a different namespace in order to test a few things about the macros.
{
	struct SClient {
		::gpk::SIPv4														AddressLocal						= {};
		::gpk::SIPv4														AddressRemote						= {};
		uint16_t															PortReceive							= 0;
		SOCKET																Socket								= INVALID_SOCKET;
		int64_t																Time								= 0;
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
