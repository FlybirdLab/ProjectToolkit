/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>
#include <toolkit/shader/format/ShaderAsset.hpp>

namespace toolkit
{
    // Serializes a ShaderAsset into the on-disk .tksd binary container.
    //
    // The writer is intentionally simple in this phase:
    //   - No compression (compression field is always None).
    //   - Hash slots are written as zero placeholders; the structure is fixed
    //     so a future phase can fill them in without bumping the format.
    //   - Layout is deterministic: chunk order is fixed and integers are
    //     little-endian, so a byte-equal output guarantees a byte-equal input.
    class TksdWriter
    {
    public:
        bool writeToFile(const ShaderAsset &asset, TkStringView path);
        bool writeToMemory(const ShaderAsset &asset, TkArray<byte> &outData);
    };
}
