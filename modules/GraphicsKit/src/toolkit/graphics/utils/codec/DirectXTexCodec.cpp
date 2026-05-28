/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "toolkit/graphics/utils/TextureTool.hpp"

#if WITH_DIRECTXTEX

#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/utils/GfxUtils.hpp"
#include "toolkit/memory/Memory.hpp"
#include <DirectXTex.h>

#include <vector>
#include <string>

namespace toolkit
{
    namespace
    {
        static std::wstring toWidePath(const TkStringView &path)
        {
            std::wstring result;
            result.reserve(path.length());
            for (i32 i = 0; i < path.length(); ++i)
                result.push_back(static_cast<wchar_t>(path[i]));
            return result;
        }

        static PixelFormat toPixelFormat(DXGI_FORMAT format)
        {
            return static_cast<PixelFormat>(format);
        }

        static DXGI_FORMAT toDxgiFormat(PixelFormat format)
        {
            return static_cast<DXGI_FORMAT>(format);
        }

        static bool isBCFormat(PixelFormat format)
        {
            return GfxPixelFormat::isCompressedBC(format);
        }

        static bool buildImages(const TextureData &textureData, std::vector<DirectX::Image> &images)
        {
            const i32 arraySize = textureData.getArraySize();
            const i32 mipLevels = textureData.getMipLevels();
            if (arraySize <= 0 || mipLevels <= 0)
                return false;

            images.resize(static_cast<size_t>(arraySize * mipLevels));
            for (i32 arrayIndex = 0; arrayIndex < arraySize; ++arrayIndex)
            {
                for (i32 mipIndex = 0; mipIndex < mipLevels; ++mipIndex)
                {
                    const TextureMipData &mip = *textureData.getData(arrayIndex, mipIndex);
                    DirectX::Image &image = images[static_cast<size_t>(arrayIndex * mipLevels + mipIndex)];
                    image.width = static_cast<size_t>(TkMath::max(textureData.width >> mipIndex, 1));
                    image.height = static_cast<size_t>(TkMath::max(textureData.height >> mipIndex, 1));
                    image.format = toDxgiFormat(textureData.format);
                    image.rowPitch = mip.rowPitch;
                    image.slicePitch = mip.depthPitch;
                    image.pixels = (u8*)mip.data.get();
                }
            }
            return true;
        }

        static DirectX::TexMetadata makeMetadata(const TextureData &textureData)
        {
            DirectX::TexMetadata metadata{};
            metadata.width = textureData.width;
            metadata.height = textureData.height;
            metadata.depth = 1;
            metadata.arraySize = textureData.getArraySize();
            metadata.mipLevels = textureData.getMipLevels();
            metadata.miscFlags = textureData.getArraySize() == 6 ? DirectX::TEX_MISC_TEXTURECUBE : 0;
            metadata.miscFlags2 = 0;
            metadata.format = toDxgiFormat(textureData.format);
            metadata.dimension = DirectX::TEX_DIMENSION_TEXTURE2D;
            return metadata;
        }

        static bool copyScratchToTexture(TextureData &textureData, const DirectX::ScratchImage &image)
        {
            const DirectX::TexMetadata &metadata = image.GetMetadata();
            textureData.width = static_cast<i32>(metadata.width);
            textureData.height = static_cast<i32>(metadata.height);
            textureData.depth = static_cast<i32>(metadata.depth);
            textureData.format = toPixelFormat(metadata.format);
            textureData.items.resize(static_cast<i32>(metadata.arraySize));

            for (i32 arrayIndex = 0; arrayIndex < static_cast<i32>(metadata.arraySize); ++arrayIndex)
            {
                auto &item = textureData.items[arrayIndex];
                item.mips.resize(static_cast<i32>(metadata.mipLevels));
                for (i32 mipIndex = 0; mipIndex < static_cast<i32>(metadata.mipLevels); ++mipIndex)
                {
                    const DirectX::Image *src = image.GetImage(static_cast<size_t>(mipIndex), static_cast<size_t>(arrayIndex), 0);
                    if (!src)
                        return false;

                    TextureMipData &mip = item.mips[mipIndex];
                    mip.rowPitch = static_cast<u32>(src->rowPitch);
                    mip.depthPitch = static_cast<u32>(src->slicePitch);
                    mip.lines = static_cast<u32>(src->height);
                    mip.data.copy(src->pixels, mip.depthPitch);
                }
            }
            return true;
        }

        static bool initScratchFromTexture(const TextureData &src, DirectX::ScratchImage &scratch)
        {
            std::vector<DirectX::Image> images;
            if (!buildImages(src, images))
                return false;

            HRESULT result = scratch.InitializeArrayFromImages(images.data(), images.size(), src.getArraySize() > 1);
            if (FAILED(result))
            {
                LogWarn("Cannot initialize DirectXTex source image. Error: %x", static_cast<u32>(result));
                return false;
            }
            return true;
        }

        static const GUID &wicCodecForType(i32 type)
        {
            switch (static_cast<TextureTool::ImageType>(type))
            {
            case TextureTool::ImageType::kPNG:
                return DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG);
            case TextureTool::ImageType::kBMP:
                return DirectX::GetWICCodec(DirectX::WIC_CODEC_BMP);
            case TextureTool::ImageType::kGIF:
                return DirectX::GetWICCodec(DirectX::WIC_CODEC_GIF);
            case TextureTool::ImageType::kTIFF:
                return DirectX::GetWICCodec(DirectX::WIC_CODEC_TIFF);
            case TextureTool::ImageType::kJPEG:
                return DirectX::GetWICCodec(DirectX::WIC_CODEC_JPEG);
            default:
                return DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG);
            }
        }
    }

    bool TextureTool::exportTextureDirectXTex(ImageType type, const TkStringView &path, const TextureData &textureData)
    {
        std::vector<DirectX::Image> images;
        if (!buildImages(textureData, images))
            return false;

        const std::wstring filePath = toWidePath(path);
        HRESULT result = E_FAIL;
        if (type == ImageType::kDDS)
        {
            const DirectX::TexMetadata metadata = makeMetadata(textureData);
            result = DirectX::SaveToDDSFile(images.data(), images.size(), metadata, DirectX::DDS_FLAGS_NONE, filePath.c_str());
        }
        else if (type == ImageType::kTGA)
        {
            result = DirectX::SaveToTGAFile(images[0], filePath.c_str());
        }
        else if (type == ImageType::kHDR)
        {
            result = DirectX::SaveToHDRFile(images[0], filePath.c_str());
        }
        else if (type == ImageType::kPNG || type == ImageType::kBMP || type == ImageType::kGIF || type == ImageType::kTIFF || type == ImageType::kJPEG)
        {
            result = DirectX::SaveToWICFile(images[0], DirectX::WIC_FLAGS_NONE, wicCodecForType(static_cast<i32>(type)), filePath.c_str());
        }
        else
        {
            LogWarn("%s", "This image format is not supported by DirectXTex export.");
            return false;
        }

        if (FAILED(result))
        {
            LogWarn("Exporting texture to '%s' failed. Error: %x", path.getText(), static_cast<u32>(result));
            return false;
        }
        return true;
    }

    bool TextureTool::importTextureDirectXTex(ImageType type, const TkStringView &path, TextureData &textureData, bool &hasAlpha)
    {
        const std::wstring filePath = toWidePath(path);
        DirectX::TexMetadata metadata{};
        DirectX::ScratchImage scratch;
        HRESULT result = E_FAIL;

        switch (type)
        {
        case ImageType::kDDS:
            result = DirectX::LoadFromDDSFile(filePath.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, scratch);
            break;
        case ImageType::kTGA:
            result = DirectX::LoadFromTGAFile(filePath.c_str(), &metadata, scratch);
            break;
        case ImageType::kHDR:
            result = DirectX::LoadFromHDRFile(filePath.c_str(), &metadata, scratch);
            break;
        case ImageType::kPNG:
        case ImageType::kBMP:
        case ImageType::kGIF:
        case ImageType::kTIFF:
        case ImageType::kJPEG:
            result = DirectX::LoadFromWICFile(filePath.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, scratch);
            break;
        default:
            LogWarn("%s", "This image format is not supported by DirectXTex import.");
            return false;
        }

        if (FAILED(result))
        {
            LogWarn("Failed to import texture from '%s'. Error: %x", path.getText(), static_cast<u32>(result));
            return false;
        }

        hasAlpha = DirectX::HasAlpha(metadata.format);
        return copyScratchToTexture(textureData, scratch);
    }

    bool TextureTool::importTextureDirectXTex(ImageType type, const TkStringView &path, TextureData &textureData, const Options &options, TkString &errorMsg, bool &hasAlpha)
    {
        bool imported = false;
        if (type == ImageType::kInternal)
        {
            imported = options.internalLoad.isBinded() && options.internalLoad(textureData);
            if (!imported)
                errorMsg = "Internal texture loader failed.";
        }
        else
        {
            imported = importTextureDirectXTex(type, path, textureData, hasAlpha);
            if (!imported)
                errorMsg = "Cannot import texture with DirectXTex.";
        }

        if (!imported)
            return false;

        i32 width = textureData.width;
        i32 height = textureData.height;
        if (options.resize)
        {
            width = options.sizeX;
            height = options.keepAspectRatio ? TkMath::max(1, TkMath::roundToInt(static_cast<f32>(textureData.height) * width / textureData.width)) : options.sizeY;
        }
        else if (options.scale != 1.0f)
        {
            width = TkMath::max(1, TkMath::roundToInt(textureData.width * options.scale));
            height = TkMath::max(1, TkMath::roundToInt(textureData.height * options.scale));
        }

        width = TkMath::min(width, options.maxSize);
        height = TkMath::min(height, options.maxSize);

        if (width != textureData.width || height != textureData.height)
        {
            TextureData resized;
            if (!resizeDirectXTex(resized, textureData, width, height))
            {
                errorMsg = "Cannot resize texture.";
                return false;
            }
            textureData = moveTemp(resized);
        }

        const PixelFormat targetFormat = toPixelFormat(options.type, textureData.width, textureData.height, options.compress);
        if (targetFormat != PixelFormat::kUnknown && targetFormat != textureData.format)
        {
            TextureData converted;
            if (!convertDirectXTex(converted, textureData, targetFormat))
            {
                errorMsg = "Cannot convert texture.";
                return false;
            }
            textureData = moveTemp(converted);
        }

        return true;
    }

    bool TextureTool::convertDirectXTex(TextureData &dst, const TextureData &src, const PixelFormat dstFormat)
    {
#if WITH_ASTC
        if (GfxPixelFormat::isCompressedASTC(dstFormat))
            return convertAstc(dst, src, dstFormat);
#endif

        DirectX::ScratchImage source;
        if (!initScratchFromTexture(src, source))
            return false;

        const DirectX::TexMetadata metadata = source.GetMetadata();
        DirectX::ScratchImage resultImage;
        HRESULT result = S_OK;
        const DXGI_FORMAT targetFormat = toDxgiFormat(dstFormat);

        if (isBCFormat(src.format) && !isBCFormat(dstFormat))
        {
            result = DirectX::Decompress(source.GetImages(), source.GetImageCount(), metadata, targetFormat, resultImage);
        }
        else if (!isBCFormat(src.format) && isBCFormat(dstFormat))
        {
            result = DirectX::Compress(source.GetImages(), source.GetImageCount(), metadata, targetFormat, DirectX::TEX_COMPRESS_DEFAULT, 0.5f, resultImage);
        }
        else
        {
            result = DirectX::Convert(source.GetImages(), source.GetImageCount(), metadata, targetFormat, DirectX::TEX_FILTER_DEFAULT, 0.0f, resultImage);
        }

        if (FAILED(result))
        {
            LogWarn("Cannot convert image. Error: %x", static_cast<u32>(result));
            return false;
        }
        return copyScratchToTexture(dst, resultImage);
    }

    bool TextureTool::resizeDirectXTex(TextureData &dst, const TextureData &src, i32 dstWidth, i32 dstHeight)
    {
        DirectX::ScratchImage source;
        if (!initScratchFromTexture(src, source))
            return false;

        DirectX::ScratchImage resized;
        HRESULT result = DirectX::Resize(
            source.GetImages(),
            source.GetImageCount(),
            source.GetMetadata(),
            dstWidth,
            dstHeight,
            DirectX::TEX_FILTER_DEFAULT,
            resized);
        if (FAILED(result))
        {
            LogWarn("Cannot resize image. Error: %x", static_cast<u32>(result));
            return false;
        }

        return copyScratchToTexture(dst, resized);
    }
}

#endif
