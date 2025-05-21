#include "Asset/Serializer/FontSerializer.h"

#include "Asset/AssetManager.h"
#include "ScopedBuffer.h"

// Windows defines this and msdf-atlas-gen wont compile
#ifdef INFINITE
#undef INFINITE
#endif

#include <msdf-atlas-gen/msdf-atlas-gen.h>

namespace Mule
{
    FontSerializer::FontSerializer(Ref<ServiceManager> serviceManager)
        :
        IAssetSerializer(serviceManager)
    {
    }

    FontSerializer::~FontSerializer()
    {
    }

    Ref<UIFont> FontSerializer::Load(const fs::path& filepath)
    {
        Ref<UIFont> uiFont = nullptr;

        if (msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype()) {
            // Load font file
            if (msdfgen::FontHandle* font = msdfgen::loadFont(ft, filepath.string().c_str())) {

                std::vector<msdf_atlas::GlyphGeometry> glyphs;
                msdf_atlas::FontGeometry fontGeometry(&glyphs);

                fontGeometry.loadCharset(font, 1.0, msdf_atlas::Charset::ASCII);

                const double maxCornerAngle = 1.0;
                for (msdf_atlas::GlyphGeometry& glyph : glyphs)
                {
                    glyph.edgeColoring(&msdfgen::edgeColoringByDistance, maxCornerAngle, 0);
                }

                msdf_atlas::TightAtlasPacker packer;

                packer.setDimensionsConstraint(msdf_atlas::TightAtlasPacker::DimensionsConstraint::SQUARE);

                packer.setMinimumScale(128.0);

                packer.setPixelRange(12.0);
                packer.setMiterLimit(40.0);

                packer.pack(glyphs.data(), glyphs.size());

                int width = 0, height = 0;
                packer.getDimensions(width, height);

                msdf_atlas::ImmediateAtlasGenerator<
                    float,
                    3,
                    msdf_atlas::msdfGenerator,
                    msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3>> generator(width, height);

                msdf_atlas::GeneratorAttributes attributes;
                generator.setAttributes(attributes);
                generator.setThreadCount(4);

                generator.generate(glyphs.data(), glyphs.size());

                msdfgen::BitmapConstRef<msdf_atlas::byte, 3> bitmap = generator.atlasStorage();

                uint64_t bitMapSize = bitmap.width * bitmap.height * 4;
                ScopedBuffer textureBuffer(bitMapSize);
                uint8_t* dst = static_cast<uint8_t*>(textureBuffer.GetData());
                const uint8_t* src = bitmap.pixels;

                for (int y = 0; y < bitmap.height; ++y)
                {
                    for (int x = 0; x < bitmap.width; ++x)
                    {
                        int srcIndex = (y * bitmap.width + x) * 3;
                        int dstIndex = (y * bitmap.width + x) * 4;

                        dst[dstIndex + 0] = src[srcIndex + 0]; // R
                        dst[dstIndex + 1] = src[srcIndex + 1]; // G
                        dst[dstIndex + 2] = src[srcIndex + 2]; // B
                        dst[dstIndex + 3] = 255;               // A (opaque)
                    }
                }

                std::string textureName = filepath.filename().replace_extension().string() + "-FontAtlas";
                Ref<Texture2D> atlasTexture = Texture2D::Create(textureName, textureBuffer, width, height, TextureFormat::RGBA_8U, TextureFlags::TransferDst);
                std::unordered_map<uint32_t, UIFontGlyph> uiGlyphs;

                for (auto glyph : glyphs)
                {
                    UIFontGlyph fontGlyph;
                    fontGlyph.Codepoint = glyph.getCodepoint();

                    msdfgen::Shape::Bounds uvBounds;
                    glyph.getQuadAtlasBounds(uvBounds.l, uvBounds.b, uvBounds.r, uvBounds.t);

                    auto& metrics = fontGeometry.getMetrics();

                    fontGlyph.MinUV = glm::vec2(uvBounds.l / width, uvBounds.t / height);
                    fontGlyph.MaxUV = glm::vec2(uvBounds.r / width, uvBounds.b / height);

                    msdfgen::Shape::Bounds planeBounds;
                    glyph.getQuadPlaneBounds(planeBounds.l, planeBounds.b, planeBounds.r, planeBounds.t);

                    fontGlyph.PlaneMin = glm::vec2(planeBounds.l, -planeBounds.t);
                    fontGlyph.PlaneMax = glm::vec2(planeBounds.r, -planeBounds.b);

                    fontGlyph.Advance = glyph.getAdvance();

                    uiGlyphs[glyph.getCodepoint()] = fontGlyph;
                }

                auto assetManager = mServiceManager->Get<AssetManager>();
                assetManager->Insert(atlasTexture);

                uiFont = MakeRef<UIFont>(atlasTexture, uiGlyphs, atlasTexture->Handle());
                uiFont->SetFilePath(filepath);

                msdfgen::destroyFont(font);
            }
            msdfgen::deinitializeFreetype(ft);
        }
        return uiFont;
    }

    void FontSerializer::Save(Ref<UIFont> asset)
    {
    }
}
