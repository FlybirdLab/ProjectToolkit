/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/directx11/common.hpp>

#if GFX_API_DIRECTX11

#include "toolkit/graphics/GfxVertexLayout.hpp"
#include "Dx11Device.hpp"

namespace toolkit
{
	// Vertex layout object for DirectX 11 backend.
	class DX11VertexLayout : public GfxResourceBase<DX11Device, GfxVertexLayout>
	{
	public:
		DX11VertexLayout(DX11Device *device, const Elements &elements, bool explicitOffsets);

	public:
		u32 inputElementsCount;
		D3D11_INPUT_ELEMENT_DESC inputElements[GFX_MAX_VS_ELEMENTS];
	};
}

#endif
