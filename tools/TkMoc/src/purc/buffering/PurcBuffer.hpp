#pragma once

#include <toolkit/types/StringView.hpp>
#include <toolkit/collection/HashMap.hpp>

namespace toolkit {

    // Simple K-V map for mapping parsed data to infomations
    class PurcBuffer
    {
    public:
        struct FileInfo {
            u64 fileSize;
            i64 fileLastEdit;

            TK_FORCE_INLINE bool operator==(const FileInfo& o) const {
                return fileSize == o.fileSize && fileLastEdit == o.fileLastEdit;
            }
            TK_FORCE_INLINE bool operator!=(const FileInfo& o) const {
                return !(*this == o);
            }
        };

        static FileInfo Invalid;

    public:
        PurcBuffer(const TkStringView& bufPath, class PurcCtx* ctx);
        virtual ~PurcBuffer() = default;

        virtual FileInfo& getFileInfo(const TkStringView& name) = 0;
        virtual void* getFileData(const TkStringView& name) = 0;
        virtual bool tryAddToBuf(const TkStringView& name, FileInfo& info, void* data) = 0;
        virtual void removeItem(const TkStringView& name) = 0;

        virtual void flush() = 0;
        virtual void save() = 0;
        virtual void close() = 0;
        virtual void clear() = 0;

    protected:
        PurcCtx* m_ctx;
    };


#if TK_DEBUG
    // An implement of PurcBuffer just using CHashMap saving in memory
    class MemoryPurcBuffer final : public PurcBuffer
    {
    public:
        MemoryPurcBuffer(const TkStringView& bufPath, class PurcCtx* ctx);
        virtual ~MemoryPurcBuffer() override;

        virtual FileInfo& getFileInfo(const TkStringView& name) override;
        virtual void* getFileData(const TkStringView& name) override;
        virtual bool tryAddToBuf(const TkStringView& name, FileInfo& info, void* data) override;
        virtual void removeItem(const TkStringView& name) override;

        virtual void flush() override;
        virtual void save() override;
        virtual void close() override;
        virtual void clear() override;

    private:
        TkHashMap<TkString, FileInfo> m_datas;
        TkHashMap<TkString, void*> m_buf;
    };
    typedef MemoryPurcBuffer PCBuffer;
#endif


    // class LDBPurcBuffer final : public PurcBuffer 
    // {

    // };

}