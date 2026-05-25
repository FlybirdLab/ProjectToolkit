/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/directx/common.hpp>

#if GFX_API_DIRECTX12

namespace toolkit
{
	struct DxShaderHeader
	{
		// The SRV dimensions per-slot.
		byte srDimensions[32];
		// The UAV dimensions per-slot.
		byte uaDimensions[4];
	};
}

#endif
