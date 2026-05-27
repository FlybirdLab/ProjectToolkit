/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/format/TksdWriter.hpp>
#include <toolkit/shader/format/TksdFormat.hpp>

#include <toolkit/io/MemoryStream.hpp>

#include <cstring>

namespace toolkit
{
    namespace
    {
        // Length-prefixed string serialization: [u32 size][raw bytes, no null].
        void writeString(MemoryOStream &out, const TkString &s)
        {
            const u32 length = static_cast<u32>(s.length());
            out.writeU32(length);
            if (length > 0)
                out.writeBytes(s.get(), length);
        }

        // Skip ahead to the next 8-byte boundary by emitting zero padding.
        void padToAlignment(MemoryOStream &out, u64 alignment)
        {
            const u64 pos = static_cast<u64>(out.getPosition());
            const u64 target = tksdAlignUp(pos, alignment);
            const u64 delta = target - pos;
            for (u64 i = 0; i < delta; i++)
                out.writeU8(0);
        }

        void overwriteAt(MemoryOStream &out, u32 offset, const void *data, u32 size)
        {
            std::memcpy(out.getHandle() + offset, data, size);
        }

        // ---- Per-chunk payload writers. Each one starts at the current
        // stream position and returns nothing -- the caller records the
        // before/after offsets and builds the chunk header from them. ----

        void writeMetadataChunk(MemoryOStream &out, const ShaderAsset &asset)
        {
            writeString(out, asset.name);
            out.writeU32(static_cast<u32>(asset.passes.count()));
            out.writeU32(static_cast<u32>(asset.stages.count()));
        }

        void writePropertiesChunk(MemoryOStream &out, const ShaderAsset &asset)
        {
            out.writeU32(static_cast<u32>(asset.properties.count()));
            for (i32 i = 0; i < asset.properties.count(); i++)
            {
                const ShaderProperty &p = asset.properties[i];
                writeString(out, p.type);
                writeString(out, p.name);
                writeString(out, p.defaultValue);
            }
        }

        void writeTagsChunk(MemoryOStream &out, const ShaderAsset &asset)
        {
            out.writeU32(static_cast<u32>(asset.tags.count()));
            for (i32 i = 0; i < asset.tags.count(); i++)
            {
                writeString(out, asset.tags[i].key);
                writeString(out, asset.tags[i].value);
            }
        }

        void writePassesChunk(MemoryOStream &out, const ShaderAsset &asset)
        {
            out.writeU32(static_cast<u32>(asset.passes.count()));
            for (i32 pi = 0; pi < asset.passes.count(); pi++)
            {
                const ShaderPass &p = asset.passes[pi];
                writeString(out, p.name);

                out.writeU32(static_cast<u32>(p.tags.count()));
                for (i32 i = 0; i < p.tags.count(); i++)
                {
                    writeString(out, p.tags[i].key);
                    writeString(out, p.tags[i].value);
                }

                out.writeU32(static_cast<u32>(p.renderStates.count()));
                for (i32 i = 0; i < p.renderStates.count(); i++)
                {
                    writeString(out, p.renderStates[i].name);
                    writeString(out, p.renderStates[i].value);
                }

                out.writeU32(static_cast<u32>(p.stageIndices.count()));
                for (i32 i = 0; i < p.stageIndices.count(); i++)
                    out.writeU32(p.stageIndices[i]);
            }
        }

        void writeStageBytecodeChunk(MemoryOStream &out, const ShaderAsset &asset)
        {
            out.writeU32(static_cast<u32>(asset.stages.count()));
            for (i32 i = 0; i < asset.stages.count(); i++)
            {
                const ShaderStageData &s = asset.stages[i];
                out.writeU32(s.kind);
                out.writeU32(static_cast<u32>(s.stage));
                out.writeU32(static_cast<u32>(s.profile));
                writeString(out, s.entry);
                const u32 size = static_cast<u32>(s.bytecode.count());
                out.writeU32(size);
                if (size > 0)
                    out.writeBytes(s.bytecode.get(), size);
            }
        }

        void writeBindingsChunk(MemoryOStream &out, const ShaderAsset &asset)
        {
            out.writeU32(static_cast<u32>(asset.bindings.count()));
            for (i32 i = 0; i < asset.bindings.count(); i++)
            {
                const ShaderBindingInfo &b = asset.bindings[i];
                writeString(out, b.name);
                out.writeU32(b.slot);
                out.writeU32(static_cast<u32>(b.stage));
                writeString(out, b.type);
            }
        }

        void writeVertexInputChunk(MemoryOStream &out, const ShaderAsset &asset)
        {
            out.writeU32(static_cast<u32>(asset.vertexInputs.count()));
            for (i32 i = 0; i < asset.vertexInputs.count(); i++)
            {
                const ShaderVertexInputInfo &v = asset.vertexInputs[i];
                writeString(out, v.semantic);
                out.writeU32(v.location);
                out.writeU32(v.format);
                out.writeU32(v.slot);
                out.writeU32(v.offset);
            }
        }

        void writeDependenciesChunk(MemoryOStream &out, const ShaderAsset &asset)
        {
            out.writeU32(static_cast<u32>(asset.dependencies.count()));
            for (i32 i = 0; i < asset.dependencies.count(); i++)
                writeString(out, asset.dependencies[i]);
        }

        struct PendingChunk
        {
            TksdChunkType type;
            u32 chunkVersion;
            u64 startOffset;
            u64 size;
        };
    }

    bool TksdWriter::writeToMemory(const ShaderAsset &asset, TkArray<byte> &outData)
    {
        MemoryOStream body(1024 * 1024); // grows on demand

        // ----- 1. Reserve space for the header. We backfill it once we know
        //         total fileSize / chunkTableOffset / chunkCount. -----
        TksdHeader headerStub;
        std::memset(&headerStub, 0, sizeof(headerStub));
        headerStub.magic = TKSD_MAGIC;
        headerStub.versionMajor = TKSD_VERSION_MAJOR;
        headerStub.versionMinor = TKSD_VERSION_MINOR;
        headerStub.headerSize = static_cast<u32>(sizeof(TksdHeader));
        body.writeBytes(&headerStub, sizeof(headerStub));

        // ----- 2. Reserve space for the chunk header table immediately after
        //         the header. We fix the chunk order here so the table size
        //         is known before any payload is written. -----
        struct ChunkSpec
        {
            TksdChunkType type;
            u32 version;
            void (*writer)(MemoryOStream &, const ShaderAsset &);
        };
        const ChunkSpec specs[] = {
            {TksdChunkType::Metadata,      1, &writeMetadataChunk},
            {TksdChunkType::Properties,    1, &writePropertiesChunk},
            {TksdChunkType::Tags,          1, &writeTagsChunk},
            {TksdChunkType::Passes,        1, &writePassesChunk},
            {TksdChunkType::StageBytecode, 1, &writeStageBytecodeChunk},
            {TksdChunkType::Bindings,      1, &writeBindingsChunk},
            {TksdChunkType::VertexInput,   1, &writeVertexInputChunk},
            {TksdChunkType::Dependencies,  1, &writeDependenciesChunk},
        };
        constexpr u32 chunkCount = static_cast<u32>(sizeof(specs) / sizeof(specs[0]));

        const u32 chunkTableOffset = body.getPosition();
        const u32 chunkTableSize = chunkCount * static_cast<u32>(sizeof(TksdChunkHeader));
        for (u32 i = 0; i < chunkTableSize; i++)
            body.writeU8(0);

        // ----- 3. Write chunk payloads in order, recording (offset, size). -----
        PendingChunk pending[chunkCount];
        for (u32 i = 0; i < chunkCount; i++)
        {
            padToAlignment(body, 8);
            const u64 startOffset = body.getPosition();
            specs[i].writer(body, asset);
            const u64 endOffset = body.getPosition();
            pending[i] = PendingChunk{specs[i].type, specs[i].version, startOffset, endOffset - startOffset};
        }

        // ----- 4. Back-fill the chunk header table. -----
        for (u32 i = 0; i < chunkCount; i++)
        {
            TksdChunkHeader ch;
            std::memset(&ch, 0, sizeof(ch));
            ch.type = static_cast<u32>(pending[i].type);
            ch.version = pending[i].chunkVersion;
            ch.offset = pending[i].startOffset;
            ch.size = pending[i].size;
            ch.uncompressedSize = pending[i].size;
            ch.compression = static_cast<u32>(TksdCompression::None);
            ch.checksum = 0;
            const u32 headerSlot = chunkTableOffset + i * static_cast<u32>(sizeof(TksdChunkHeader));
            overwriteAt(body, headerSlot, &ch, sizeof(ch));
        }

        // ----- 5. Back-fill the file header. -----
        const u64 fileSize = body.getPosition();
        TksdHeader finalHeader = headerStub;
        finalHeader.chunkTableOffset = chunkTableOffset;
        finalHeader.chunkCount = chunkCount;
        finalHeader.fileSize = fileSize;
        // Hash slots stay zero in Phase 5 -- the on-disk slots are reserved so
        // a future deterministic-hash pass can drop hashes in without a
        // breaking format change.
        overwriteAt(body, 0, &finalHeader, sizeof(finalHeader));

        outData.clear();
        outData.ensureCapacity(static_cast<i32>(fileSize), false);
        for (u64 i = 0; i < fileSize; i++)
            outData.add(body.getHandle()[i]);
        return true;
    }

    bool TksdWriter::writeToFile(const ShaderAsset &asset, TkStringView path)
    {
        MemoryOStream body(1024 * 1024);

        TksdHeader headerStub;
        std::memset(&headerStub, 0, sizeof(headerStub));
        headerStub.magic = TKSD_MAGIC;
        headerStub.versionMajor = TKSD_VERSION_MAJOR;
        headerStub.versionMinor = TKSD_VERSION_MINOR;
        headerStub.headerSize = static_cast<u32>(sizeof(TksdHeader));
        body.writeBytes(&headerStub, sizeof(headerStub));

        struct ChunkSpec
        {
            TksdChunkType type;
            u32 version;
            void (*writer)(MemoryOStream &, const ShaderAsset &);
        };
        const ChunkSpec specs[] = {
            {TksdChunkType::Metadata,      1, &writeMetadataChunk},
            {TksdChunkType::Properties,    1, &writePropertiesChunk},
            {TksdChunkType::Tags,          1, &writeTagsChunk},
            {TksdChunkType::Passes,        1, &writePassesChunk},
            {TksdChunkType::StageBytecode, 1, &writeStageBytecodeChunk},
            {TksdChunkType::Bindings,      1, &writeBindingsChunk},
            {TksdChunkType::VertexInput,   1, &writeVertexInputChunk},
            {TksdChunkType::Dependencies,  1, &writeDependenciesChunk},
        };
        constexpr u32 chunkCount = static_cast<u32>(sizeof(specs) / sizeof(specs[0]));

        const u32 chunkTableOffset = body.getPosition();
        const u32 chunkTableSize = chunkCount * static_cast<u32>(sizeof(TksdChunkHeader));
        for (u32 i = 0; i < chunkTableSize; i++)
            body.writeU8(0);

        PendingChunk pending[chunkCount];
        for (u32 i = 0; i < chunkCount; i++)
        {
            padToAlignment(body, 8);
            const u64 startOffset = body.getPosition();
            specs[i].writer(body, asset);
            const u64 endOffset = body.getPosition();
            pending[i] = PendingChunk{specs[i].type, specs[i].version, startOffset, endOffset - startOffset};
        }

        for (u32 i = 0; i < chunkCount; i++)
        {
            TksdChunkHeader ch;
            std::memset(&ch, 0, sizeof(ch));
            ch.type = static_cast<u32>(pending[i].type);
            ch.version = pending[i].chunkVersion;
            ch.offset = pending[i].startOffset;
            ch.size = pending[i].size;
            ch.uncompressedSize = pending[i].size;
            ch.compression = static_cast<u32>(TksdCompression::None);
            ch.checksum = 0;
            const u32 headerSlot = chunkTableOffset + i * static_cast<u32>(sizeof(TksdChunkHeader));
            overwriteAt(body, headerSlot, &ch, sizeof(ch));
        }

        const u64 fileSize = body.getPosition();
        TksdHeader finalHeader = headerStub;
        finalHeader.chunkTableOffset = chunkTableOffset;
        finalHeader.chunkCount = chunkCount;
        finalHeader.fileSize = fileSize;
        overwriteAt(body, 0, &finalHeader, sizeof(finalHeader));

        return body.saveToFile(path);
    }
}
