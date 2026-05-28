/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/common.hpp>
#include <toolkit/graphics/GfxTexture.hpp>
#include <toolkit/serialization/ISerializable.hpp>
#include <toolkit/graphics/utils/TextureData.hpp>

namespace toolkit
{
    class JsonWriter;

    // Textures importing, processing and exporting utilities.
    class TK_API TextureTool
    {
    public:
        // Texture import options.
        struct TK_API Options : public ISerializable
        {
            // Texture format type.
            TextureFormatType type = TextureFormatType::kColorRGB;

            // True if texture should be imported as a texture atlas (with sprites).
            bool isAtlas : 1 = false;
            // True if disable dynamic texture streaming.
            bool neverStream : 1 = false;
            // True if disable dynamic texture streaming.
            bool compress : 1 = true;
            // True if texture channels have independent data (for compression methods).
            bool independentChannels : 1 = false;
            // True if use sRGB format for texture data. Recommended for color maps and diffuse color textures.
            bool srgb : 1 = false;
            // True if generate mip maps chain for the texture.
            bool genMipMaps : 1 = true;

            // True if flip Y coordinate of the texture (Flips over X axis).
            bool flipY : 1 = false;
            // True if flip X coordinate of the texture (Flips over Y axis).
            bool flipX : 1 = false;

            // Invert the red channel.
            bool invertRed : 1 = false;
            // Invert the green channel. Good for OpenGL to DirectX conversion.
            bool invertGreen : 1 = false;
            // Invert the blue channel.
            bool invertBlue : 1 = false;
            // Invert the alpha channel.
            bool invertAlpha : 1 = false;

            // Rebuild Z (blue) channel assuming X/Y are normals.
            bool reconstructZ : 1 = false;
            // Texture size scale. Allows increasing or decreasing the imported texture resolution. Default is 1.
            f32 scale = 1.0f;

            // Maximum size of the texture (for both width and height). Higher resolution textures will be resized during importing process. Used to clip textures that are too big.
            i32 maxSize = 8192;

            // True if resize texture on import. Use SizeX/SizeY properties to define texture width and height. Texture scale property will be ignored.
            bool resize : 1 = false;
            // Keeps the aspect ratio when resizing.
            bool keepAspectRatio : 1 = false;

            // The width of the imported texture. If Resize property is set to true then texture will be resized during the import to this value during the import, otherwise it will be ignored.
            i32 sizeX = 1024;
            // The height of the imported texture. If Resize property is set to true then texture will be resized during the import to this value during the import, otherwise it will be ignored.
            i32 sizeY = 1024;

            // Check to preserve alpha coverage in generated mips for alpha test reference. Scales mipmap alpha values to preserve alpha coverage based on an alpha test reference value.
            bool preserveAlphaCoverage : 1 = false;

            // The reference value for the alpha coverage preserving.
            f32 preserveAlphaCoverageReference = 0.5f;

            // The texture group for streaming (negative if unused). See Streaming Settings.
            i32 textureGroup = -1;

            // The sprites for the sprite sheet import mode.
            TkArray<Sprite> sprites;
            // Function used for fast importing textures used by internal parts of the engine
            TkFunc<bool(TextureData &)> internalLoad;

        public:
            TkString toString() const;

            void serialize(SerializeStream &stream, const void *otherObj) override;
            void deserialize(DeserializeStream &stream, ISerializeModifier *modifier) override;
        };

    public:
        // Checks whenever the given texture file contains alpha channel data with values different than solid fill of 1 (non fully opaque).
        static bool hasAlpha(const TkStringView &path);

        // Imports the texture.
        static bool importTexture(const TkStringView &path, TextureData &textureData);
        // Imports the texture.
        static bool importTexture(const TkStringView &path, TextureData &textureData, Options options, TkString &errorMsg);

        // Exports the texture.
        static bool exportTexture(const TkStringView &path, const TextureData &textureData);

        // Converts the specified source texture data into an another format.
        static bool convert(TextureData &dst, const TextureData &src, const PixelFormat dstFormat);
        // Resizes the specified source texture data into an another dimensions.
        static bool resize(TextureData &dst, const TextureData &src, i32 dstWidth, i32 dstHeight);

    public:
        static PixelFormat toPixelFormat(TextureFormatType format, i32 width, i32 height, bool canCompress);

        enum class ImageType
        {
            kDDS,
            kTGA,
            kPNG,
            kBMP,
            kGIF,
            kTIFF,
            kJPEG,
            kHDR,
            kRAW,
            kEXR,
            kInternal,
        };

    private:
        static bool getImageType(const TkStringView &path, ImageType &type);
        static bool transform(TextureData &texture, const TkFunc<void(Color &)> &transformation);

#if WITH_DIRECTXTEX
        static bool exportTextureDirectXTex(ImageType type, const TkStringView &path, const TextureData &textureData);
        static bool importTextureDirectXTex(ImageType type, const TkStringView &path, TextureData &textureData, bool &hasAlpha);
        static bool importTextureDirectXTex(ImageType type, const TkStringView &path, TextureData &textureData, const Options &options, TkString &errorMsg, bool &hasAlpha);
        static bool convertDirectXTex(TextureData &dst, const TextureData &src, const PixelFormat dstFormat);
        static bool resizeDirectXTex(TextureData &dst, const TextureData &src, i32 dstWidth, i32 dstHeight);
#endif
#if WITH_STB
        static bool exportTextureStb(ImageType type, const TkStringView &path, const TextureData &textureData);
        static bool importTextureStb(ImageType type, const TkStringView &path, TextureData &textureData, bool &hasAlpha);
        static bool importTextureStb(ImageType type, const TkStringView &path, TextureData &textureData, const Options &options, TkString &errorMsg, bool &hasAlpha);
        static bool convertStb(TextureData &dst, const TextureData &src, const PixelFormat dstFormat);
        static bool resizeStb(PixelFormat format, TextureMipData &dstMip, const TextureMipData &srcMip, i32 dstMipWidth, i32 dstMipHeight);
        static bool resizeStb(TextureData &dst, const TextureData &src, i32 dstWidth, i32 dstHeight);
#endif
#if WITH_ASTC
        static bool convertAstc(TextureData &dst, const TextureData &src, const PixelFormat dstFormat);
#endif
    };

}