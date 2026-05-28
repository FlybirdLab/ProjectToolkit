/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "TextureTool.hpp"

#include "PixelFormatSampler.hpp"
#include <toolkit/debug/Logger.hpp>
#include <toolkit/serialization/JsonTools.hpp>
#include <toolkit/serialization/JsonWriter.hpp>
#include <toolkit/types/DateTime.hpp>
#include <toolkit/types/Path.hpp>
#include <toolkit/utils/Color.hpp>
#include <toolkit/utils/StringUtils.hpp>

namespace toolkit
{
    static TkHashMap<TkString, bool> s_texturesHasAlphaCache;

    static void writeVec2(JsonWriter &stream, const FVec2 &value)
    {
        stream.startObject();
        JSON_KEY(stream, "X");
        stream.floatValue(value.x);
        JSON_KEY(stream, "Y");
        stream.floatValue(value.y);
        stream.endObject();
    }

    static FVec2 readVec2(JsonValue value, const FVec2 &defaultValue)
    {
        if (value.isArray() && value.getArrayCount() >= 2)
            return FVec2(value.getArrayItem(0).getFloat(defaultValue.x), value.getArrayItem(1).getFloat(defaultValue.y));
        if (value.isObject())
            return FVec2(JsonTools::getFloat(value, "X", defaultValue.x), JsonTools::getFloat(value, "Y", defaultValue.y));
        return defaultValue;
    }

    static TextureFormatType readTextureFormatType(JsonValue value, TextureFormatType defaultValue)
    {
        if (value.isNumber())
            return static_cast<TextureFormatType>(value.getUint(static_cast<u32>(defaultValue)));

        if (value.isString())
        {
            const TkString text = value.getString().toString();
            for (u32 i = static_cast<u32>(TextureFormatType::kUnknown); i <= static_cast<u32>(TextureFormatType::kHdrRGB); ++i)
            {
                const auto type = static_cast<TextureFormatType>(i);
                if (text.equalsIgnoreCase(toString(type)))
                    return type;
            }
        }

        return defaultValue;
    }

    TkString TextureTool::Options::toString() const
    {
        return TkString::format(
            "Type: %s, IsAtlas: %d, NeverStream: %d, Compress: %d, IndependentChannels: %d, sRGB: %d, GenerateMipMaps: %d, FlipY: %d, FlipX: %d, InvertRed: %d, InvertGreen: %d, InvertBlue: %d, InvertAlpha: %d, ReconstructZ: %d, Scale: %g, MaxSize: %d, Resize: %d, KeepAspectRatio: %d, PreserveAlphaCoverage: %d, PreserveAlphaCoverageReference: %g, SizeX: %d, SizeY: %d",
            toolkit::toString(type),
            isAtlas,
            neverStream,
            compress,
            independentChannels,
            srgb,
            genMipMaps,
            flipY,
            flipX,
            invertRed,
            invertGreen,
            invertBlue,
            invertAlpha,
            reconstructZ,
            scale,
            maxSize,
            resize,
            keepAspectRatio,
            preserveAlphaCoverage,
            preserveAlphaCoverageReference,
            sizeX,
            sizeY);
    }

    void TextureTool::Options::serialize(SerializeStream &stream, const void *otherObj)
    {
        (void)otherObj;

        JSON_KEY(stream, "Type");
        stream.uintValue(static_cast<u32>(type));

        JSON_KEY(stream, "IsAtlas");
        stream.boolValue(isAtlas);

        JSON_KEY(stream, "NeverStream");
        stream.boolValue(neverStream);

        JSON_KEY(stream, "Compress");
        stream.boolValue(compress);

        JSON_KEY(stream, "IndependentChannels");
        stream.boolValue(independentChannels);

        JSON_KEY(stream, "sRGB");
        stream.boolValue(srgb);

        JSON_KEY(stream, "GenerateMipMaps");
        stream.boolValue(genMipMaps);

        JSON_KEY(stream, "FlipY");
        stream.boolValue(flipY);

        JSON_KEY(stream, "FlipX");
        stream.boolValue(flipX);

        JSON_KEY(stream, "InvertRedChannel");
        stream.boolValue(invertRed);

        JSON_KEY(stream, "InvertGreenChannel");
        stream.boolValue(invertGreen);

        JSON_KEY(stream, "InvertBlueChannel");
        stream.boolValue(invertBlue);

        JSON_KEY(stream, "InvertAlphaChannel");
        stream.boolValue(invertAlpha);

        JSON_KEY(stream, "ReconstructZChannel");
        stream.boolValue(reconstructZ);

        JSON_KEY(stream, "Resize");
        stream.boolValue(resize);

        JSON_KEY(stream, "KeepAspectRatio");
        stream.boolValue(keepAspectRatio);

        JSON_KEY(stream, "PreserveAlphaCoverage");
        stream.boolValue(preserveAlphaCoverage);

        JSON_KEY(stream, "PreserveAlphaCoverageReference");
        stream.floatValue(preserveAlphaCoverageReference);

        JSON_KEY(stream, "TextureGroup");
        stream.intValue(textureGroup);

        JSON_KEY(stream, "Scale");
        stream.floatValue(scale);

        JSON_KEY(stream, "MaxSize");
        stream.intValue(maxSize);

        JSON_KEY(stream, "SizeX");
        stream.intValue(sizeX);

        JSON_KEY(stream, "SizeY");
        stream.intValue(sizeY);

        JSON_KEY(stream, "Sprites");
        stream.startArray();
        for (i32 i = 0; i < sprites.count(); ++i)
        {
            const Sprite &s = sprites[i];

            stream.startObject();

            JSON_KEY(stream, "Position");
            writeVec2(stream, s.area.location);

            JSON_KEY(stream, "Size");
            writeVec2(stream, s.area.size);

            JSON_KEY(stream, "Name");
            stream.string(s.name);

            stream.endObject();
        }
        stream.endArray(sprites.count());
    }

    void TextureTool::Options::deserialize(DeserializeStream &stream, ISerializeModifier *modifier)
    {
        (void)modifier;

        type = readTextureFormatType(stream.findMember("Type"), type);
        isAtlas = JsonTools::getBool(stream, "IsAtlas", isAtlas);
        neverStream = JsonTools::getBool(stream, "NeverStream", neverStream);
        compress = JsonTools::getBool(stream, "Compress", compress);
        independentChannels = JsonTools::getBool(stream, "IndependentChannels", independentChannels);
        srgb = JsonTools::getBool(stream, "sRGB", srgb);
        genMipMaps = JsonTools::getBool(stream, "GenerateMipMaps", genMipMaps);
        flipY = JsonTools::getBool(stream, "FlipY", flipY);
        flipX = JsonTools::getBool(stream, "FlipX", flipX);
        invertRed = JsonTools::getBool(stream, "InvertRedChannel", invertRed);
        invertGreen = JsonTools::getBool(stream, "InvertGreenChannel", invertGreen);
        invertBlue = JsonTools::getBool(stream, "InvertBlueChannel", invertBlue);
        invertAlpha = JsonTools::getBool(stream, "InvertAlphaChannel", invertAlpha);
        reconstructZ = JsonTools::getBool(stream, "ReconstructZChannel", reconstructZ);
        resize = JsonTools::getBool(stream, "Resize", resize);
        keepAspectRatio = JsonTools::getBool(stream, "KeepAspectRatio", keepAspectRatio);
        preserveAlphaCoverage = JsonTools::getBool(stream, "PreserveAlphaCoverage", preserveAlphaCoverage);
        preserveAlphaCoverageReference = JsonTools::getFloat(stream, "PreserveAlphaCoverageReference", preserveAlphaCoverageReference);
        textureGroup = JsonTools::getInt(stream, "TextureGroup", textureGroup);
        scale = JsonTools::getFloat(stream, "Scale", scale);
        maxSize = JsonTools::getInt(stream, "MaxSize", maxSize);
        sizeX = JsonTools::getInt(stream, "SizeX", sizeX);
        sizeY = JsonTools::getInt(stream, "SizeY", sizeY);

        JsonValue spritesMember = stream.findMember("Sprites");
        if (spritesMember.isArray() && sprites.isEmpty())
        {
            const i32 spriteCount = spritesMember.getArrayCount();
            sprites.resize(spriteCount, false);

            for (i32 i = 0; i < spriteCount; ++i)
            {
                JsonValue spriteData = spritesMember.getArrayItem(i);
                Sprite &s = sprites[i];
                s.area.location = readVec2(spriteData.findMember("Position"), FVec2::kZero);
                s.area.size = readVec2(spriteData.findMember("Size"), FVec2::kOne);
                s.name = JsonTools::getString(spriteData, "Name");
            }
        }
    }

    bool TextureTool::hasAlpha(const TkStringView &path)
    {
        const TkString key(path);
        if (!s_texturesHasAlphaCache.containsKey(key))
        {
            TextureData textureData;
            if (!importTexture(path, textureData))
                return false;
        }

        auto cached = s_texturesHasAlphaCache.find(key);
        return cached != s_texturesHasAlphaCache.End() ? cached->value : false;
    }

    bool TextureTool::importTexture(const TkStringView &path, TextureData &textureData)
    {
        LogInfo("Importing texture from '%s'", path.getText());
        const TkDateTime startTime = TkDateTime::nowUTC();

        ImageType type;
        if (!getImageType(path, type))
            return false;

        bool hasAlphaValue = false;
#if WITH_DIRECTXTEX
        const bool success = importTextureDirectXTex(type, path, textureData, hasAlphaValue);
#elif WITH_STB
        const bool success = importTextureStb(type, path, textureData, hasAlphaValue);
#else
        const bool success = false;
        LogWarn("%s", "Importing textures is not supported on this platform.");
#endif

        if (!success)
        {
            LogWarn("%s", "Importing texture failed.");
        }
        else
        {
            s_texturesHasAlphaCache[TkString(path)] = hasAlphaValue;
            const TkDateTime endTime = TkDateTime::nowUTC();
            LogInfo("Texture imported in %d ms", static_cast<i32>((endTime - startTime).getTotalMilliseconds()));
        }

        return success;
    }

    bool TextureTool::importTexture(const TkStringView &path, TextureData &textureData, Options options, TkString &errorMsg)
    {
        LogInfo("Importing texture from '%s'. Options: %s", path.getText(), options.toString().getText());
        const TkDateTime startTime = TkDateTime::nowUTC();

        ImageType type;
        if (options.internalLoad.isBinded())
        {
            type = ImageType::kInternal;
        }
        else if (!getImageType(path, type))
        {
            return false;
        }

        options.maxSize = TkMath::clamp(options.maxSize, 1, GFX_MAX_TEXTURE_SIZE);
        options.sizeX = TkMath::clamp(options.sizeX, 1, GFX_MAX_TEXTURE_SIZE);
        options.sizeY = TkMath::clamp(options.sizeY, 1, GFX_MAX_TEXTURE_SIZE);

        bool hasAlphaValue = false;
#if WITH_DIRECTXTEX
        const bool success = importTextureDirectXTex(type, path, textureData, options, errorMsg, hasAlphaValue);
#elif WITH_STB
        const bool success = importTextureStb(type, path, textureData, options, errorMsg, hasAlphaValue);
#else
        const bool success = false;
        LogWarn("%s", "Importing textures is not supported on this platform.");
#endif

        if (!success)
        {
            LogWarn("Importing texture failed. %s", errorMsg.getText());
        }
        else
        {
            s_texturesHasAlphaCache[TkString(path)] = hasAlphaValue;
            const TkDateTime endTime = TkDateTime::nowUTC();
            LogInfo("Texture imported in %d ms", static_cast<i32>((endTime - startTime).getTotalMilliseconds()));
        }

        return success;
    }

    bool TextureTool::exportTexture(const TkStringView &path, const TextureData &textureData)
    {
        LogInfo("Exporting texture to '%s'.", path.getText());
        const TkDateTime startTime = TkDateTime::nowUTC();

        ImageType type;
        if (!getImageType(path, type))
            return false;
        if (textureData.items.isEmpty())
        {
            LogWarn("%s", "Missing texture data.");
            return false;
        }

#if WITH_DIRECTXTEX
        const bool success = exportTextureDirectXTex(type, path, textureData);
#elif WITH_STB
        const bool success = exportTextureStb(type, path, textureData);
#else
        const bool success = false;
        LogWarn("%s", "Exporting textures is not supported on this platform.");
#endif

        if (!success)
        {
            LogWarn("%s", "Exporting failed.");
        }
        else
        {
            const TkDateTime endTime = TkDateTime::nowUTC();
            LogInfo("Texture exported in %d ms", static_cast<i32>((endTime - startTime).getTotalMilliseconds()));
        }

        return success;
    }

    bool TextureTool::convert(TextureData &dst, const TextureData &src, const PixelFormat dstFormat)
    {
        if (src.getMipLevels() == 0)
        {
            LogWarn("%s", "Missing source data.");
            return false;
        }
        if (src.format == dstFormat)
        {
            LogWarn("%s", "Source data and destination format are the same. Cannot perform conversion.");
            return false;
        }
        if (src.depth != 1)
        {
            LogWarn("%s", "Converting volume texture data is not supported.");
            return false;
        }

#if WITH_ASTC
        if (GfxPixelFormat::isCompressedASTC(dstFormat))
            return convertAstc(dst, src, dstFormat);
#endif
#if WITH_DIRECTXTEX
        return convertDirectXTex(dst, src, dstFormat);
#elif WITH_STB
        return convertStb(dst, src, dstFormat);
#else
        LogWarn("%s", "Converting textures is not supported on this platform.");
        return false;
#endif
    }

    bool TextureTool::resize(TextureData &dst, const TextureData &src, i32 dstWidth, i32 dstHeight)
    {
        if (src.getMipLevels() == 0)
        {
            LogWarn("%s", "Missing source data.");
            return false;
        }
        if (src.width == dstWidth && src.height == dstHeight)
        {
            LogWarn("%s", "Source data and destination dimensions are the same. Cannot perform resizing.");
            return false;
        }
        if (src.depth != 1)
        {
            LogWarn("%s", "Resizing volume texture data is not supported.");
            return false;
        }

#if WITH_DIRECTXTEX
        return resizeDirectXTex(dst, src, dstWidth, dstHeight);
#elif WITH_STB
        return resizeStb(dst, src, dstWidth, dstHeight);
#else
        LogWarn("%s", "Resizing textures is not supported on this platform.");
        return false;
#endif
    }

    PixelFormat TextureTool::toPixelFormat(TextureFormatType format, i32 width, i32 height, bool canCompress)
    {
        const bool canUseBlockCompression = width % 4 == 0 && height % 4 == 0;
        if (canCompress && canUseBlockCompression)
        {
            switch (format)
            {
            case TextureFormatType::kColorRGB:
                return PixelFormat::kBC1_UNorm;
            case TextureFormatType::kColorRGBA:
                return PixelFormat::kBC3_UNorm;
            case TextureFormatType::kNormalMap:
                return PixelFormat::kBC5_UNorm;
            case TextureFormatType::kGrayScale:
                return PixelFormat::kBC4_UNorm;
            case TextureFormatType::kHdrRGBA:
                return PixelFormat::kBC7_UNorm;
            case TextureFormatType::kHdrRGB:
                return PixelFormat::kBC6H_Uf16;
            default:
                return PixelFormat::kUnknown;
            }
        }

        switch (format)
        {
        case TextureFormatType::kColorRGB:
        case TextureFormatType::kColorRGBA:
            return PixelFormat::kR8G8B8A8_UNorm;
        case TextureFormatType::kNormalMap:
            return PixelFormat::kR16G16_UNorm;
        case TextureFormatType::kGrayScale:
            return PixelFormat::kR8_UNorm;
        case TextureFormatType::kHdrRGBA:
            return PixelFormat::kR16G16B16A16_Float;
        case TextureFormatType::kHdrRGB:
            return PixelFormat::kR11G11B10_Float;
        default:
            return PixelFormat::kUnknown;
        }
    }

    bool TextureTool::getImageType(const TkStringView &path, ImageType &type)
    {
        const TkString extension = TkPath::getExtension(path).toString().toLowerAscii();
        if (extension == "tga")
        {
            type = ImageType::kTGA;
        }
        else if (extension == "dds")
        {
            type = ImageType::kDDS;
        }
        else if (extension == "png")
        {
            type = ImageType::kPNG;
        }
        else if (extension == "bmp")
        {
            type = ImageType::kBMP;
        }
        else if (extension == "gif")
        {
            type = ImageType::kGIF;
        }
        else if (extension == "tiff" || extension == "tif")
        {
            type = ImageType::kTIFF;
        }
        else if (extension == "hdr")
        {
            type = ImageType::kHDR;
        }
        else if (extension == "jpeg" || extension == "jpg")
        {
            type = ImageType::kJPEG;
        }
        else if (extension == "raw")
        {
            type = ImageType::kRAW;
        }
        else if (extension == "exr")
        {
            type = ImageType::kEXR;
        }
        else
        {
            LogWarn("Unknown texture file type for '%s'.", path.getText());
            return false;
        }

        return true;
    }

    bool TextureTool::transform(TextureData &texture, const TkFunc<void(Color &)> &transformation)
    {
        const PixelFormatSampler *sampler = PixelFormatSampler::get(texture.format);
        if (!sampler)
            return false;

        for (auto &slice : texture.items)
        {
            for (i32 mipIndex = 0; mipIndex < slice.mips.count(); ++mipIndex)
            {
                TextureMipData &mip = slice.mips[mipIndex];
                const i32 mipWidth = TkMath::max(texture.width >> mipIndex, 1);
                const i32 mipHeight = TkMath::max(texture.height >> mipIndex, 1);
                for (i32 y = 0; y < mipHeight; ++y)
                {
                    for (i32 x = 0; x < mipWidth; ++x)
                    {
                        Color color = sampler->samplePoint(mip.data.get(), x, y, mip.rowPitch);
                        transformation(color);
                        sampler->store(mip.data.get(), x, y, mip.rowPitch, color);
                    }
                }
            }
        }
        return true;
    }
}
