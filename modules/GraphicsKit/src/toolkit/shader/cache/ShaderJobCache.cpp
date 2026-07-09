/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/cache/ShaderJobCache.hpp>
#include <toolkit/shader/compilegraph/ShaderCompileJob.hpp>

#include <toolkit/debug/Logger.hpp>
#include <toolkit/io/MemoryStream.hpp>
#include <toolkit/platforms/FileSystem.hpp>
#include <toolkit/types/Path.hpp>

#include <cstring>

namespace toolkit
{
    namespace
    {
        constexpr u32 kSjcbMagic   = 0x42434A53; // 'SJCB' little endian

        // v1 = blob-only cache entries. v2 adds reflection (bindings +
        // vertexInputs) and the dependency-path list, so a cache hit
        // produces the same .tksd metadata as a fresh compile. v1 files are
        // treated as miss -- the user just gets one extra recompile to
        // upgrade the directory.
        constexpr u32 kSjcbVersion = 2;

        // Cursor helper -- bounds-checked read out of a flat byte buffer.
        // Returns false on any read past the end so the load path can fall
        // back to "MISS (truncated)" without UB.
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

            bool readU64(u64 &out)
            {
                if (pos + 8 > size) return false;
                std::memcpy(&out, data + pos, 8);
                pos += 8;
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

        // Mirror writers -- compact length-prefixed strings, little-endian
        // primitives (matches MemoryOStream's writeU32 / writeU64).
        void writeString(MemoryOStream &out, const TkString &s)
        {
            const u32 length = static_cast<u32>(s.length());
            out.writeU32(length);
            if (length > 0)
                out.writeBytes(s.get(), length);
        }
    }

    void ShaderJobCache::setCacheDirectory(TkStringView dir)
    {
        m_dir = TkString(dir);
        TkPath::normalize(m_dir);
    }

    void ShaderJobCache::ensureHash(ShaderCompileJob &job)
    {
        if (job.jobHashLow != 0 || job.jobHashHigh != 0)
            return;
        const ShaderHash128 h = ShaderHash::hashJob(job);
        job.jobHashLow = h.low;
        job.jobHashHigh = h.high;
    }

    TkString ShaderJobCache::makeCachePath(const ShaderHash128 &hash) const
    {
        const TkString hex = ShaderHash::toHex(hash);
        return TkString(m_dir) / TkString::format("%s.jobcache", hex.get());
    }

    bool ShaderJobCache::tryLoad(ShaderCompileJob &job, CachedShaderJobResult &outResult)
    {
        outResult = CachedShaderJobResult();

        if (!m_enabled)
        {
            if (m_verbose)
                LogInfo("[ShaderJobCache] tryLoad: disabled (--no-cache)");
            return false;
        }
        if (m_dir.isEmpty())
        {
            if (m_verbose)
                LogInfo("[ShaderJobCache] tryLoad: cache directory is empty");
            return false;
        }

        ensureHash(job);
        const ShaderHash128 hash{job.jobHashLow, job.jobHashHigh};
        const TkString path = makeCachePath(hash);
        const TkString hex = ShaderHash::toHex(hash);

        if (m_verbose)
        {
            LogInfo("[ShaderJobCache] tryLoad job='%s/%s' profile=%i stage=%i hash=%s",
                    job.shaderName.get(), job.passName.get(),
                    static_cast<i32>(job.profile), static_cast<i32>(job.stage),
                    hex.get());
            LogInfo("[ShaderJobCache] tryLoad path='%s'", path.get());
        }

        if (!TkFileSystem::isFileExist(path))
        {
            if (m_verbose)
                LogInfo("[ShaderJobCache] tryLoad: MISS (file not found)");
            return false;
        }

        OpenFileParams params{
            .path = path,
            .mode = FileMode::kOpenExisting,
            .access = FileAccess::kRead,
            .share = FileShare::kRead,
        };
        FileHandle handle = TkFileSystem::openFile(params);
        if (!TkFileSystem::isFileOpened(handle))
        {
            if (m_verbose)
                LogInfo("[ShaderJobCache] tryLoad: MISS (openFile failed)");
            return false;
        }

        TkArray<byte> raw;
        const bool ok = TkFileSystem::readAsBytes(handle, raw);
        TkFileSystem::closeFile(handle);
        if (!ok || static_cast<u32>(raw.count()) < 28) // 4+4+8+8+4 = 28 bytes header
        {
            if (m_verbose)
                LogInfo("[ShaderJobCache] tryLoad: MISS (read failed or short, got %i bytes)",
                        static_cast<i32>(raw.count()));
            return false;
        }

        Cursor c{raw.get(), static_cast<u32>(raw.count()), 0};

        u32 magic = 0, version = 0;
        u64 fileHashLow = 0, fileHashHigh = 0;
        u32 blobSize = 0;
        if (!c.readU32(magic) || !c.readU32(version)
            || !c.readU64(fileHashLow) || !c.readU64(fileHashHigh)
            || !c.readU32(blobSize))
        {
            if (m_verbose)
                LogInfo("[ShaderJobCache] tryLoad: MISS (header truncated)");
            return false;
        }

        if (magic != kSjcbMagic)
        {
            if (m_verbose)
                LogInfo("[ShaderJobCache] tryLoad: MISS (bad magic 0x%08x, expected 0x%08x)",
                        magic, kSjcbMagic);
            return false;
        }
        if (version != kSjcbVersion)
        {
            // v1 files are valid for an older binary but we can no longer
            // reproduce metadata from them -- treat as miss so the next run
            // upgrades the entry. The caller's `cacheMiss++` accounting
            // correctly tracks this.
            if (m_verbose)
                LogInfo("[ShaderJobCache] tryLoad: MISS (file version %u, expected %u -- "
                        "older entries get rewritten by the next miss)",
                        version, kSjcbVersion);
            return false;
        }
        if (fileHashLow != hash.low || fileHashHigh != hash.high)
        {
            if (m_verbose)
                LogInfo("[ShaderJobCache] tryLoad: MISS (hash mismatch in file -- name collision, "
                        "file says %016llx%016llx but we asked for %s)",
                        static_cast<unsigned long long>(fileHashHigh),
                        static_cast<unsigned long long>(fileHashLow),
                        hex.get());
            return false;
        }

        // Bytecode payload.
        if (!c.readBytes(outResult.bytecode, blobSize))
        {
            if (m_verbose)
                LogInfo("[ShaderJobCache] tryLoad: MISS (bytecode truncated -- declared %u bytes)",
                        blobSize);
            return false;
        }

        // Bindings.
        u32 bindingCount = 0;
        if (!c.readU32(bindingCount))
        {
            if (m_verbose)
                LogInfo("[ShaderJobCache] tryLoad: MISS (binding count truncated)");
            return false;
        }
        for (u32 i = 0; i < bindingCount; i++)
        {
            ShaderBindingInfo b;
            u32 stageValue = 0;
            if (!c.readString(b.name) || !c.readU32(b.slot)
                || !c.readU32(stageValue) || !c.readString(b.type))
            {
                if (m_verbose)
                    LogInfo("[ShaderJobCache] tryLoad: MISS (binding %u truncated)", i);
                return false;
            }
            b.stage = static_cast<ShaderStage>(stageValue);
            outResult.reflectionBindings.add(b);
        }

        // Vertex inputs.
        u32 vertexInputCount = 0;
        if (!c.readU32(vertexInputCount))
        {
            if (m_verbose)
                LogInfo("[ShaderJobCache] tryLoad: MISS (vertex input count truncated)");
            return false;
        }
        for (u32 i = 0; i < vertexInputCount; i++)
        {
            ShaderVertexInputInfo v;
            if (!c.readString(v.semantic) || !c.readU32(v.location)
                || !c.readU32(v.format) || !c.readU32(v.slot) || !c.readU32(v.offset))
            {
                if (m_verbose)
                    LogInfo("[ShaderJobCache] tryLoad: MISS (vertex input %u truncated)", i);
                return false;
            }
            outResult.reflectionVertexInputs.add(v);
        }

        // Dependencies.
        u32 dependencyCount = 0;
        if (!c.readU32(dependencyCount))
        {
            if (m_verbose)
                LogInfo("[ShaderJobCache] tryLoad: MISS (dependency count truncated)");
            return false;
        }
        for (u32 i = 0; i < dependencyCount; i++)
        {
            TkString path;
            if (!c.readString(path))
            {
                if (m_verbose)
                    LogInfo("[ShaderJobCache] tryLoad: MISS (dependency %u truncated)", i);
                return false;
            }
            outResult.dependencies.add(path);
        }

        if (m_verbose)
            LogInfo("[ShaderJobCache] tryLoad: HIT (%u bytes blob + %u bindings + %u vertex inputs + %u deps)",
                    blobSize, bindingCount, vertexInputCount, dependencyCount);
        return true;
    }

    bool ShaderJobCache::store(ShaderCompileJob &job, const CachedShaderJobResult &result)
    {
        if (!m_enabled)
        {
            if (m_verbose)
                LogInfo("[ShaderJobCache] store: skipped (cache disabled)");
            return false;
        }
        if (m_dir.isEmpty())
        {
            if (m_verbose)
                LogInfo("[ShaderJobCache] store: skipped (cache directory is empty)");
            return false;
        }

        if (!TkFileSystem::isDirExist(m_dir))
        {
            if (!TkFileSystem::makeDir(m_dir))
            {
                if (m_verbose)
                    LogInfo("[ShaderJobCache] store: FAILED (makeDir '%s' failed)", m_dir.get());
                return false;
            }
            if (m_verbose)
                LogInfo("[ShaderJobCache] store: created cache dir '%s'", m_dir.get());
        }

        ensureHash(job);
        const ShaderHash128 hash{job.jobHashLow, job.jobHashHigh};
        const TkString path = makeCachePath(hash);
        const TkString hex = ShaderHash::toHex(hash);
        const u32 blobSize = static_cast<u32>(result.bytecode.count());

        if (m_verbose)
        {
            LogInfo("[ShaderJobCache] store job='%s/%s' profile=%i stage=%i hash=%s",
                    job.shaderName.get(), job.passName.get(),
                    static_cast<i32>(job.profile), static_cast<i32>(job.stage),
                    hex.get());
            LogInfo("[ShaderJobCache] store path='%s' blob=%u bindings=%i vertexInputs=%i deps=%i",
                    path.get(),
                    blobSize,
                    result.reflectionBindings.count(),
                    result.reflectionVertexInputs.count(),
                    result.dependencies.count());
        }

        MemoryOStream out(64 + blobSize + 256);
        out.writeU32(kSjcbMagic);
        out.writeU32(kSjcbVersion);
        out.writeU64(hash.low);
        out.writeU64(hash.high);
        out.writeU32(blobSize);
        if (blobSize > 0)
            out.writeBytes(result.bytecode.get(), blobSize);

        out.writeU32(static_cast<u32>(result.reflectionBindings.count()));
        for (i32 i = 0; i < result.reflectionBindings.count(); i++)
        {
            const ShaderBindingInfo &b = result.reflectionBindings[i];
            writeString(out, b.name);
            out.writeU32(b.slot);
            out.writeU32(static_cast<u32>(b.stage));
            writeString(out, b.type);
        }

        out.writeU32(static_cast<u32>(result.reflectionVertexInputs.count()));
        for (i32 i = 0; i < result.reflectionVertexInputs.count(); i++)
        {
            const ShaderVertexInputInfo &v = result.reflectionVertexInputs[i];
            writeString(out, v.semantic);
            out.writeU32(v.location);
            out.writeU32(v.format);
            out.writeU32(v.slot);
            out.writeU32(v.offset);
        }

        out.writeU32(static_cast<u32>(result.dependencies.count()));
        for (i32 i = 0; i < result.dependencies.count(); i++)
            writeString(out, result.dependencies[i]);

        const bool saved = out.saveToFile(path);
        if (m_verbose)
        {
            if (saved)
                LogInfo("[ShaderJobCache] store: OK (%u bytes written)", out.getPosition());
            else
                LogInfo("[ShaderJobCache] store: FAILED (MemoryOStream::saveToFile returned false)");
        }
        return saved;
    }
}
