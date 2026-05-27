/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "M4aDecoder.hpp"

#include <toolkit/base/Math.hpp>
#include <toolkit/io/Stream.hpp>
#include <toolkit/memory/Memory.hpp>

#if TK_CODECS_M4A && TK_WINDOWS
#include <toolkit/platforms/win32/ComPointer.hpp>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <objbase.h>
#include <propvarutil.h>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace toolkit
{
    namespace
    {
        constexpr u64 kMediaFoundationTicksPerSecond = 10000000ULL;

        class ScopedComInitialization final
        {
        public:
            ScopedComInitialization()
                : m_result(CoInitializeEx(nullptr, COINIT_MULTITHREADED))
            {
            }

            ~ScopedComInitialization()
            {
                if (SUCCEEDED(m_result))
                    CoUninitialize();
            }

            bool usable() const
            {
                return SUCCEEDED(m_result) || m_result == RPC_E_CHANGED_MODE;
            }

        private:
            HRESULT m_result;
        };

        class TkReadOnlyComStream final : public IStream
        {
        public:
            TkReadOnlyComStream(ITkStream *stream, u32 baseOffset)
                : m_stream(stream),
                  m_baseOffset(baseOffset),
                  m_length(stream->length() - baseOffset)
            {
                m_stream->setPos(baseOffset);
            }

            HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **object) override
            {
                if (object == nullptr)
                    return E_POINTER;

                if (iid == __uuidof(IUnknown) || iid == __uuidof(ISequentialStream) || iid == __uuidof(IStream))
                {
                    *object = static_cast<IStream *>(this);
                    AddRef();
                    return S_OK;
                }

                *object = nullptr;
                return E_NOINTERFACE;
            }

            ULONG STDMETHODCALLTYPE AddRef() override
            {
                return static_cast<ULONG>(InterlockedIncrement(&m_refs));
            }

            ULONG STDMETHODCALLTYPE Release() override
            {
                const ULONG refs = static_cast<ULONG>(InterlockedDecrement(&m_refs));
                if (refs == 0)
                    delete this;
                return refs;
            }

            HRESULT STDMETHODCALLTYPE Read(void *data, ULONG bytes, ULONG *bytesRead) override
            {
                if (data == nullptr)
                    return STG_E_INVALIDPOINTER;

                const u32 absolutePosition = m_stream->pos();
                const u32 position = absolutePosition >= m_baseOffset ? absolutePosition - m_baseOffset : 0;
                const u32 available = position < m_length ? m_length - position : 0;
                const u32 readCount = TkMath::min(static_cast<u32>(bytes), available);
                if (readCount > 0)
                    m_stream->readBytes(data, readCount);
                if (bytesRead != nullptr)
                    *bytesRead = readCount;
                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE Write(const void *, ULONG, ULONG *) override
            {
                return STG_E_ACCESSDENIED;
            }

            HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) override
            {
                LONGLONG base = 0;
                switch (origin)
                {
                case STREAM_SEEK_SET:
                    break;
                case STREAM_SEEK_CUR:
                    base = static_cast<LONGLONG>(m_stream->pos() - m_baseOffset);
                    break;
                case STREAM_SEEK_END:
                    base = static_cast<LONGLONG>(m_length);
                    break;
                default:
                    return STG_E_INVALIDFUNCTION;
                }

                const LONGLONG position = base + move.QuadPart;
                if (position < 0 || static_cast<u64>(position) > m_length)
                    return STG_E_INVALIDFUNCTION;

                m_stream->setPos(m_baseOffset + static_cast<u32>(position));
                if (newPosition != nullptr)
                    newPosition->QuadPart = static_cast<ULONGLONG>(position);
                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER) override
            {
                return STG_E_ACCESSDENIED;
            }

            HRESULT STDMETHODCALLTYPE CopyTo(IStream *, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *) override
            {
                return E_NOTIMPL;
            }

            HRESULT STDMETHODCALLTYPE Commit(DWORD) override
            {
                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE Revert() override
            {
                return E_NOTIMPL;
            }

            HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override
            {
                return STG_E_INVALIDFUNCTION;
            }

            HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override
            {
                return STG_E_INVALIDFUNCTION;
            }

            HRESULT STDMETHODCALLTYPE Stat(STATSTG *stats, DWORD) override
            {
                if (stats == nullptr)
                    return STG_E_INVALIDPOINTER;

                TkMemory::memoryClear(stats, sizeof(*stats));
                stats->type = STGTY_STREAM;
                stats->cbSize.QuadPart = m_length;
                stats->grfMode = STGM_READ;
                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE Clone(IStream **) override
            {
                return E_NOTIMPL;
            }

        private:
            ~TkReadOnlyComStream() = default;

            volatile LONG m_refs = 1;
            ITkStream *m_stream = nullptr;
            u32 m_baseOffset = 0;
            u32 m_length = 0;
        };
    }

    M4aDecoder::~M4aDecoder()
    {
        close();
    }

    bool M4aDecoder::open(ITkStream *stream, AudioDataInfo &info, u32 offset)
    {
        close();
        if (!isValid(stream, offset))
            return false;

        ScopedComInitialization com;
        if (!com.usable() || FAILED(MFStartup(MF_VERSION, MFSTARTUP_LITE)))
            return false;
        m_mediaFoundationStarted = true;

        ComPtr<IStream> inputStream;
        inputStream.attach(new TkReadOnlyComStream(stream, offset));

        ComPtr<IMFByteStream> byteStream;
        HRESULT hr = MFCreateMFByteStreamOnStream(inputStream.get(), byteStream.releaseAndGetAddressOf());
        if (FAILED(hr))
        {
            close();
            return false;
        }

        hr = MFCreateSourceReaderFromByteStream(byteStream.get(), nullptr, &m_reader);
        if (FAILED(hr) || m_reader == nullptr)
        {
            close();
            return false;
        }

        m_reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
        if (FAILED(m_reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE)))
        {
            close();
            return false;
        }

        ComPtr<IMFMediaType> pcmType;
        hr = MFCreateMediaType(pcmType.releaseAndGetAddressOf());
        if (FAILED(hr) ||
            FAILED(pcmType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio)) ||
            FAILED(pcmType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM)) ||
            FAILED(pcmType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16)) ||
            FAILED(m_reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pcmType.get())))
        {
            close();
            return false;
        }

        ComPtr<IMFMediaType> decodedType;
        hr = m_reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, decodedType.releaseAndGetAddressOf());
        if (FAILED(hr))
        {
            close();
            return false;
        }

        AudioDataInfo decodedInfo{};
        decodedInfo.bitDepth = 16;
        if (FAILED(decodedType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &decodedInfo.numChannels)) ||
            FAILED(decodedType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &decodedInfo.sampleRate)) ||
            decodedInfo.numChannels == 0 || decodedInfo.sampleRate == 0)
        {
            close();
            return false;
        }

        PROPVARIANT duration;
        PropVariantInit(&duration);
        hr = m_reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &duration);
        if (FAILED(hr) || duration.vt != VT_UI8)
        {
            PropVariantClear(&duration);
            close();
            return false;
        }

        const u64 durationTicks = duration.uhVal.QuadPart;
        PropVariantClear(&duration);
        const u64 totalFrames = (durationTicks * decodedInfo.sampleRate + kMediaFoundationTicksPerSecond - 1) /
                                kMediaFoundationTicksPerSecond;
        const u64 totalSamples = totalFrames * decodedInfo.numChannels;
        if (totalSamples == 0 || totalSamples > static_cast<u64>(kMaxU32))
        {
            close();
            return false;
        }

        decodedInfo.numSamples = static_cast<u32>(totalSamples);
        m_info = decodedInfo;
        info = m_info;
        return true;
    }

    void M4aDecoder::seek(u32 offset)
    {
        if (m_reader == nullptr || m_info.numChannels == 0 || m_info.sampleRate == 0)
            return;

        const u32 requestedSample = TkMath::clamp(offset, 0U, m_info.numSamples);
        const u32 targetFrame = requestedSample / m_info.numChannels;
        const u32 targetSample = targetFrame * m_info.numChannels;
        if (targetSample == m_readSampleOffset)
            return;

        ScopedComInitialization com;
        if (!com.usable())
            return;

        PROPVARIANT position;
        PropVariantInit(&position);
        position.vt = VT_I8;
        position.hVal.QuadPart = static_cast<LONGLONG>(
            (static_cast<u64>(targetFrame) * kMediaFoundationTicksPerSecond) / m_info.sampleRate);

        if (SUCCEEDED(m_reader->SetCurrentPosition(GUID_NULL, position)))
        {
            m_pendingData.clear();
            m_pendingOffset = 0;
            m_readSampleOffset = targetSample;
        }
        PropVariantClear(&position);
    }

    void M4aDecoder::read(byte *samples, u32 numSamples)
    {
        readSamples(samples, numSamples);
    }

    u32 M4aDecoder::readSamples(byte *samples, u32 numSamples)
    {
        if (m_reader == nullptr || samples == nullptr || numSamples == 0)
            return 0;

        const u64 bytesRequested64 = static_cast<u64>(numSamples) * sizeof(i16);
        if (bytesRequested64 > static_cast<u64>(kMaxU32))
            return 0;

        ScopedComInitialization com;
        if (!com.usable())
            return 0;

        const u32 bytesRequested = static_cast<u32>(bytesRequested64);
        u32 bytesCopied = 0;
        while (bytesCopied < bytesRequested)
        {
            const u32 pendingBytes = static_cast<u32>(m_pendingData.count()) - m_pendingOffset;
            if (pendingBytes > 0)
            {
                const u32 copyBytes = TkMath::min(bytesRequested - bytesCopied, pendingBytes);
                TkMemory::memoryCopy(samples + bytesCopied, m_pendingData.get() + m_pendingOffset, copyBytes);
                bytesCopied += copyBytes;
                m_pendingOffset += copyBytes;
                continue;
            }

            m_pendingData.clear();
            m_pendingOffset = 0;

            DWORD flags = 0;
            LONGLONG timestamp = 0;
            ComPtr<IMFSample> sample;
            const HRESULT hr = m_reader->ReadSample(
                MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                0,
                nullptr,
                &flags,
                &timestamp,
                sample.releaseAndGetAddressOf());
            if (FAILED(hr) || (flags & MF_SOURCE_READERF_ENDOFSTREAM) != 0)
                break;
            if (!sample)
                continue;

            ComPtr<IMFMediaBuffer> buffer;
            if (FAILED(sample->ConvertToContiguousBuffer(buffer.releaseAndGetAddressOf())))
                break;

            BYTE *data = nullptr;
            DWORD currentLength = 0;
            if (FAILED(buffer->Lock(&data, nullptr, &currentLength)))
                break;
            if (currentLength > static_cast<DWORD>(kMaxI32))
            {
                buffer->Unlock();
                break;
            }
            m_pendingData.set(reinterpret_cast<byte *>(data), static_cast<i32>(currentLength));
            buffer->Unlock();
        }

        if (bytesCopied < bytesRequested)
            TkMemory::memoryClear(samples + bytesCopied, bytesRequested - bytesCopied);

        const u32 decodedSamples = bytesCopied / sizeof(i16);
        m_readSampleOffset += decodedSamples;
        return decodedSamples;
    }

    bool M4aDecoder::isValid(ITkStream *stream, u32 offset)
    {
        if (stream == nullptr || offset > stream->length() || stream->length() - offset < 12)
            return false;

        const u32 savedPosition = stream->pos();
        byte header[12]{};
        stream->setPos(offset);
        stream->readBytes(header, sizeof(header));
        stream->setPos(savedPosition);

        return header[4] == 'f' && header[5] == 't' && header[6] == 'y' && header[7] == 'p';
    }

    bool M4aDecoder::isEOF() const
    {
        return m_reader == nullptr || m_readSampleOffset >= m_info.numSamples;
    }

    void M4aDecoder::close()
    {
        if (m_reader != nullptr)
        {
            m_reader->Release();
            m_reader = nullptr;
        }

        m_pendingData.clear();
        m_pendingOffset = 0;
        m_readSampleOffset = 0;
        m_info = {};

        if (m_mediaFoundationStarted)
        {
            MFShutdown();
            m_mediaFoundationStarted = false;
        }
    }
}
#else
namespace toolkit
{
    M4aDecoder::~M4aDecoder() = default;

    bool M4aDecoder::open(ITkStream *, AudioDataInfo &, u32)
    {
        return false;
    }

    void M4aDecoder::seek(u32)
    {
    }

    void M4aDecoder::read(byte *, u32)
    {
    }

    u32 M4aDecoder::readSamples(byte *, u32)
    {
        return 0;
    }

    bool M4aDecoder::isValid(ITkStream *, u32)
    {
        return false;
    }

    bool M4aDecoder::isEOF() const
    {
        return true;
    }

    void M4aDecoder::close()
    {
    }
}
#endif
