#include "gpk_error.h"

#ifndef ENDPOINT_H_98273498237
#define ENDPOINT_H_98273498237

namespace dop
{
	struct SEndpointIPv4 {
		char			IP		[4]	;
		int16_t			Port		;
		int16_t			Adapter		;
	};

	::gpk::error_t									communicationsInitialize					();
	::gpk::error_t									communicationsShutdown						();
}

#endif // ENDPOINT_H_98273498237
