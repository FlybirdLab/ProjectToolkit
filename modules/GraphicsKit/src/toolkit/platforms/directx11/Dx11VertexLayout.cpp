/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx11VertexLayout.hpp"

#if GFX_API_DIRECTX11

#include "toolkit/platforms/directx/DxUtils.hpp"

namespace toolkit
{

    DX11VertexLayout::DX11VertexLayout(DX11Device *device, const Elements &elements, bool explicitOffsets)
        : GfxResourceBase<DX11Device, GfxVertexLayout>(device, TkStringView::Empty), inputElementsCount(elements.count())
    {
        setElements(elements, explicitOffsets);
        for (i32 i = 0; i < elements.count(); i++)
        {
            const VertexElement &src = getElements().get()[i];
            D3D11_INPUT_ELEMENT_DESC &dst = inputElements[i];
            UINT semanticIndex = 0;
            dst.SemanticName = DxUtils::getVertexInputSemantic(src.type, semanticIndex);
            dst.SemanticIndex = semanticIndex;
            dst.Format = DxUtils::toDxgiFormat(src.format);
            dst.InputSlot = src.slot;
            dst.AlignedByteOffset = src.offset;
            dst.InputSlotClass = src.perInstance ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
            dst.InstanceDataStepRate = src.perInstance ? 1 : 0;
        }
    }

}

#endif
