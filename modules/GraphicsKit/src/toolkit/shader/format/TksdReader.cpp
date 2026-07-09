/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/format/TksdReader.hpp>
#include <toolkit/shader/format/TksdFormat.hpp>

#include <toolkit/platforms/FileSystem.hpp>

#include <cstring>

namespace toolkit
{
    const char *toString(TksdReadResult result)
    {
        switch (result)
        {
        case TksdReadResult::Ok:                     return "ok";
        case TksdReadResult::ReadFailed:             return "read failed";
        case TksdReadResult::InvalidHeader:          return "invalid tksd header";
        case TksdReadResult::UnsupportedVersion:     return "unsupported tksd version";
        case TksdReadResult::InvalidChunkTable:      return "invalid chunk table";
        case TksdReadResult::ChunkOutOfRange:        return "chunk out of range";
        case TksdReadResult::UnsupportedCompression: return "unsupported compression";
        case TksdReadResult::TruncatedChunk:         return "truncated chunk";
        }
        return "unknown";
    }

    namespace
    {
        struct Cursor
        {
            const byte *data;
            u32 size;
            u32 pos;

            bool readU32(u32 &out)
            {
                if (pos + 4 > size) return false;
                std::memcpy(&out, data + pos, 4);
                pos += 4;
                return true;
            }

            bool readU8(u8 &out)
            {
                if (pos + 1 > size) return false;
                out = data[pos];
                pos += 1;
                return true;
            }

            bool readString(TkString &out)
            {
                u32 length;
                if (!readU32(length)) return false;
                if (pos + length > size) return false;
                if (length == 0)
                {
                    out = TkString();
                    return true;
                }
                out = TkString(reinterpret_cast<const char *>(data + pos), static_cast<i32>(length));
                pos += length;
                return true;
            }

            bool readBytes(TkArray<byte> &out, u32 length)
            {
                if (pos + length > size) return false;
                out.ensureCapacity(static_cast<i32>(length), false);
                for (u32 i = 0; i < length; i++)
                    out.add(data[pos + i]);
                pos += length;
                return true;
            }
        };

        bool parseMetadata(const byte *data, u32 size, ShaderAsset &asset)
        {
            Cursor c{data, size, 0};
            if (!c.readString(asset.name)) return false;
            u32 passCount, stageCount;
            if (!c.readU32(passCount)) return false;
            if (!c.readU32(stageCount)) return false;
            // pass/stage counts are advisory -- the actual data is in
            // Passes / StageBytecode chunks. We don't pre-allocate here so
            // that out-of-order chunks still parse correctly.
            (void)passCount; (void)stageCount;
            return true;
        }

        bool parseProperties(const byte *data, u32 size, ShaderAsset &asset)
        {
            Cursor c{data, size, 0};
            u32 count;
            if (!c.readU32(count)) return false;
            for (u32 i = 0; i < count; i++)
            {
                ShaderProperty p;
                if (!c.readString(p.type)) return false;
                if (!c.readString(p.name)) return false;
                if (!c.readString(p.defaultValue)) return false;
                asset.properties.add(p);
            }
            return true;
        }

        bool parseTags(const byte *data, u32 size, ShaderAsset &asset)
        {
            Cursor c{data, size, 0};
            u32 count;
            if (!c.readU32(count)) return false;
            for (u32 i = 0; i < count; i++)
            {
                ShaderTag t;
                if (!c.readString(t.key)) return false;
                if (!c.readString(t.value)) return false;
                asset.tags.add(t);
            }
            return true;
        }

        bool parsePasses(const byte *data, u32 size, ShaderAsset &asset)
        {
            Cursor c{data, size, 0};
            u32 count;
            if (!c.readU32(count)) return false;
            for (u32 i = 0; i < count; i++)
            {
                ShaderPass pass;
                if (!c.readString(pass.name)) return false;

                u32 tagCount;
                if (!c.readU32(tagCount)) return false;
                for (u32 t = 0; t < tagCount; t++)
                {
                    ShaderTag tag;
                    if (!c.readString(tag.key)) return false;
                    if (!c.readString(tag.value)) return false;
                    pass.tags.add(tag);
                }

                u32 rsCount;
                if (!c.readU32(rsCount)) return false;
                for (u32 r = 0; r < rsCount; r++)
                {
                    ShaderRenderState rs;
                    if (!c.readString(rs.name)) return false;
                    if (!c.readString(rs.value)) return false;
                    pass.renderStates.add(rs);
                }

                u32 idxCount;
                if (!c.readU32(idxCount)) return false;
                for (u32 s = 0; s < idxCount; s++)
                {
                    u32 idx;
                    if (!c.readU32(idx)) return false;
                    pass.stageIndices.add(idx);
                }
                asset.passes.add(pass);
            }
            return true;
        }

        bool parseStages(const byte *data, u32 size, ShaderAsset &asset)
        {
            Cursor c{data, size, 0};
            u32 count;
            if (!c.readU32(count)) return false;
            for (u32 i = 0; i < count; i++)
            {
                ShaderStageData s;
                u32 kind;
                if (!c.readU32(kind)) return false;
                s.kind = kind;

                u32 stageValue;
                if (!c.readU32(stageValue)) return false;
                s.stage = static_cast<ShaderStage>(stageValue);

                u32 profileValue;
                if (!c.readU32(profileValue)) return false;
                s.profile = static_cast<GfxShaderProfile>(profileValue);

                if (!c.readString(s.entry)) return false;

                u32 bytecodeSize;
                if (!c.readU32(bytecodeSize)) return false;
                if (bytecodeSize > 0)
                {
                    if (!c.readBytes(s.bytecode, bytecodeSize)) return false;
                }
                asset.stages.add(s);
            }
            return true;
        }

        bool parseBindings(const byte *data, u32 size, ShaderAsset &asset)
        {
            Cursor c{data, size, 0};
            u32 count;
            if (!c.readU32(count)) return false;
            for (u32 i = 0; i < count; i++)
            {
                ShaderBindingInfo b;
                if (!c.readString(b.name)) return false;
                if (!c.readU32(b.slot)) return false;
                u32 stage;
                if (!c.readU32(stage)) return false;
                b.stage = static_cast<ShaderStage>(stage);
                if (!c.readString(b.type)) return false;
                asset.bindings.add(b);
            }
            return true;
        }

        bool parseVertexInput(const byte *data, u32 size, ShaderAsset &asset)
        {
            Cursor c{data, size, 0};
            u32 count;
            if (!c.readU32(count)) return false;
            for (u32 i = 0; i < count; i++)
            {
                ShaderVertexInputInfo v;
                if (!c.readString(v.semantic)) return false;
                if (!c.readU32(v.location)) return false;
                if (!c.readU32(v.format)) return false;
                if (!c.readU32(v.slot)) return false;
                if (!c.readU32(v.offset)) return false;
                asset.vertexInputs.add(v);
            }
            return true;
        }

        bool parseDependencies(const byte *data, u32 size, ShaderAsset &asset)
        {
            Cursor c{data, size, 0};
            u32 count;
            if (!c.readU32(count)) return false;
            for (u32 i = 0; i < count; i++)
            {
                TkString path;
                if (!c.readString(path)) return false;
                asset.dependencies.add(path);
            }
            return true;
        }
    }

    TksdReadResult TksdReader::parseChunk(u32 type, const byte *data, u32 size, ShaderAsset &outAsset)
    {
        bool ok = true;
        switch (static_cast<TksdChunkType>(type))
        {
        case TksdChunkType::Metadata:      ok = parseMetadata(data, size, outAsset); break;
        case TksdChunkType::Properties:    ok = parseProperties(data, size, outAsset); break;
        case TksdChunkType::Tags:          ok = parseTags(data, size, outAsset); break;
        case TksdChunkType::Passes:        ok = parsePasses(data, size, outAsset); break;
        case TksdChunkType::StageBytecode: ok = parseStages(data, size, outAsset); break;
        case TksdChunkType::Bindings:      ok = parseBindings(data, size, outAsset); break;
        case TksdChunkType::VertexInput:   ok = parseVertexInput(data, size, outAsset); break;
        case TksdChunkType::Dependencies:  ok = parseDependencies(data, size, outAsset); break;
        default:
            // Unknown chunk: by spec, silently skip so a v1 reader can ingest
            // future minor-version files without breaking.
            return TksdReadResult::Ok;
        }
        return ok ? TksdReadResult::Ok : TksdReadResult::TruncatedChunk;
    }

    TksdReadResult TksdReader::parseAll(const byte *data, u32 size, ShaderAsset &outAsset)
    {
        if (data == nullptr || size < sizeof(TksdHeader))
        {
            m_lastError = TkString("file too small for TksdHeader");
            return TksdReadResult::InvalidHeader;
        }

        TksdHeader header;
        std::memcpy(&header, data, sizeof(header));
        if (header.magic != TKSD_MAGIC)
        {
            m_lastError = TkString("magic mismatch");
            return TksdReadResult::InvalidHeader;
        }
        if (header.versionMajor != TKSD_VERSION_MAJOR)
        {
            m_lastError = TkString::format("unsupported major version %u", static_cast<u32>(header.versionMajor));
            return TksdReadResult::UnsupportedVersion;
        }
        if (header.headerSize < sizeof(TksdHeader))
        {
            m_lastError = TkString("header size too small");
            return TksdReadResult::InvalidHeader;
        }
        if (header.fileSize > size)
        {
            m_lastError = TkString("declared file size exceeds buffer");
            return TksdReadResult::InvalidHeader;
        }

        const u64 tableEnd = static_cast<u64>(header.chunkTableOffset)
            + static_cast<u64>(header.chunkCount) * sizeof(TksdChunkHeader);
        if (tableEnd > header.fileSize)
        {
            m_lastError = TkString("chunk table extends past file end");
            return TksdReadResult::InvalidChunkTable;
        }

        for (u32 i = 0; i < header.chunkCount; i++)
        {
            TksdChunkHeader ch;
            const u64 slot = static_cast<u64>(header.chunkTableOffset)
                + static_cast<u64>(i) * sizeof(TksdChunkHeader);
            std::memcpy(&ch, data + slot, sizeof(ch));

            if (ch.compression != static_cast<u32>(TksdCompression::None))
            {
                m_lastError = TkString::format("chunk %u uses unsupported compression %u",
                                               i, ch.compression);
                return TksdReadResult::UnsupportedCompression;
            }
            if (ch.offset + ch.size > header.fileSize)
            {
                m_lastError = TkString::format("chunk %u payload out of range", i);
                return TksdReadResult::ChunkOutOfRange;
            }
            if (ch.size > 0xffffffffull)
            {
                m_lastError = TkString::format("chunk %u larger than 4GiB", i);
                return TksdReadResult::ChunkOutOfRange;
            }

            const TksdReadResult sub = parseChunk(ch.type, data + ch.offset, static_cast<u32>(ch.size), outAsset);
            if (sub != TksdReadResult::Ok)
            {
                m_lastError = TkString::format("chunk %u parse failed: %s",
                                               i, toString(sub));
                return sub;
            }
        }
        return TksdReadResult::Ok;
    }

    TksdReadResult TksdReader::readFromMemory(const byte *data, u32 size, ShaderAsset &outAsset)
    {
        return parseAll(data, size, outAsset);
    }

    TksdReadResult TksdReader::readFromFile(TkStringView path, ShaderAsset &outAsset)
    {
        OpenFileParams params{
            .path = path,
            .mode = FileMode::kOpenExisting,
            .access = FileAccess::kRead,
            .share = FileShare::kRead,
        };
        FileHandle file = TkFileSystem::openFile(params);
        if (!TkFileSystem::isFileOpened(file))
        {
            m_lastError = TkString("failed to open tksd file");
            return TksdReadResult::ReadFailed;
        }

        TkArray<byte> contents;
        const bool ok = TkFileSystem::readAsBytes(file, contents);
        TkFileSystem::closeFile(file);
        if (!ok || contents.isEmpty())
        {
            m_lastError = TkString("failed to read tksd file");
            return TksdReadResult::ReadFailed;
        }

        return parseAll(contents.get(), static_cast<u32>(contents.count()), outAsset);
    }
}
