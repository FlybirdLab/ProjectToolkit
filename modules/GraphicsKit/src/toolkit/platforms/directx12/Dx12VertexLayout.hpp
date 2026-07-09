/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX12

#include "toolkit/graphics/GfxVertexLayout.hpp"
#include "Dx12Device.hpp"

namespace toolkit
{
	// Vertex layout object for DirectX 12 backend.
	class GPUVertexLayoutDX12 : public DX12Resource<GfxVertexLayout>
	{
	public:
		GPUVertexLayoutDX12(DX12Device *device, const Elements &elements, bool explicitOffsets);

	public:
		u32 inputElementsCount;
		D3D12_INPUT_ELEMENT_DESC inputElements[GFX_MAX_VS_ELEMENTS];
	};

}

#endif
