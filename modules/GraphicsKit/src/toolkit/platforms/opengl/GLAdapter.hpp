/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/types/String.hpp"
#include "toolkit/types/Version.hpp"
#include <toolkit/graphics/GfxAdapter.hpp>
#include <toolkit/platforms/opengl/common.hpp>

#if GFX_API_OPENGL || GFX_API_OPENGLES

namespace toolkit
{

    class GLAdapter : public GfxAdapter
    {
    public:
        bool isValid() const override
        {
            return true;
        }
        void *getNativePtr() const override
        {
            return nullptr;
        }
        u32 getVendorId() const override
        {
            return 0;
        }
        TkString getDescription() const override
        {
            return "OpenGL (skeleton)";
        }
        TkVersion getDriverVersion() const override
        {
            return {0, 0};
        }
    };

}

#endif
