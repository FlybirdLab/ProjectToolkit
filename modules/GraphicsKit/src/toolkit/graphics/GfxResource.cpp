/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "GfxResource.hpp"

namespace toolkit {

    static const char* s_GfxResType[] = {
        "GfxRenderTarget",
        "GfxTexture",
        "GfxCubeTexture",
        "GfxVolumeTexture",
        "GfxBuffer",
        "GfxShader",
        "GfxPipelineState",
        "GfxDescriptor",
        "GfxQuery",
        "GfxSampler"
    };

    TkStringView GfxResource::toString() const
    {
        return s_GfxResType[(i32)getType()];
    }
}