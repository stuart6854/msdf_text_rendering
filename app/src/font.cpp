#include "font.hpp"

#include <cassert>

#define DEFAULT_ANGLE_THRESHOLD 3
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ul
#define THREAD_COUNT 8

template <typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
static void CreateAndCacheAtlas(float fontSize,
                                const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
                                const msdf_atlas::FontGeometry& fontGeometry,
                                std::uint32_t width,
                                std::uint32_t height,
                                std::vector<T>& outData)
{
    msdf_atlas::GeneratorAttributes attributes{};
    attributes.config.overlapSupport = true;
    attributes.scanlinePass = true;

    msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
    generator.setAttributes(attributes);
    generator.setThreadCount(8);
    generator.generate(glyphs.data(), std::uint32_t(glyphs.size()));

    auto bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();
    msdfgen::savePng(bitmap, "cached_atlas.png");

    outData.resize(width * height * N);
    std::memcpy(outData.data(), bitmap.pixels, outData.size());
}

Font::Font(const std::filesystem::path& fontFilename) : m_data(new FontData)
{
    msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
    assert(ft);

    const auto fontFilenameStr = fontFilename.string();
    // #TODO: msdfgen::loadFontData - loads font from memory buffer
    msdfgen::FontHandle* font = msdfgen::loadFont(ft, fontFilenameStr.c_str());
    if (!font)
    {
        throw std::runtime_error("Failed to load font: " + fontFilenameStr);
    }

    struct CharsetRange
    {
        std::uint32_t begin{};
        std::uint32_t end{};
    };

    // Taken from ImGui - imgui_draw.cpp (GetGlyphRanges)
    static const std::vector<CharsetRange> charsetRanges{
        { 0x0020, 0x00FF },  // Basic Latin + Latin Supplement
    };

    msdf_atlas::Charset charset{};
    for (const auto& range : charsetRanges)
    {
        for (auto character = range.begin; character <= range.end; ++character)
        {
            charset.add(character);
        }
    }

    double fontScale = 1.0;
    m_data->geometry = msdf_atlas::FontGeometry(&m_data->glyphs);
    auto glyphsLoaded = m_data->geometry.loadCharset(font, fontScale, charset);
    (void)(glyphsLoaded);  // `charset.size() - glyphsLoaded` glyphs were loaded

    double emSize = 32.0;
    double pixelRange = 2;  // 2.5 + 1.0 / 0.1;//8.0; // Shader should use this value

    msdf_atlas::TightAtlasPacker atlasPacker{};
    // atlasPacker.setDimensionsConstraint();
    atlasPacker.setPixelRange(pixelRange);
    atlasPacker.setMiterLimit(1.0);
    atlasPacker.setPadding(1);
    atlasPacker.setScale(emSize);
    int remaining = atlasPacker.pack(m_data->glyphs.data(), std::int32_t(m_data->glyphs.size()));
    assert(remaining == 0);

    std::int32_t width{};
    std::int32_t height{};
    atlasPacker.getDimensions(width, height);
    emSize = atlasPacker.getScale();

    // if MSDF || MTSDF
    std::uint64_t coloringSeed = 0;
    bool expensiveColoring = true;
    if (expensiveColoring)
    {
        msdf_atlas::Workload(
            [&glyphs = m_data->glyphs, &coloringSeed](int i, int threadNo) -> bool
            {
                unsigned long long glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
                glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
                return true;
            },
            m_data->glyphs.size())
            .finish(THREAD_COUNT);
    }
    else
    {
        unsigned long long glyphSeed = coloringSeed;
        for (msdf_atlas::GlyphGeometry& glyph : m_data->glyphs)
        {
            glyphSeed *= LCG_MULTIPLIER;
            glyph.edgeColoring(msdfgen::edgeColoringByDistance, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
        }
    }

    m_data->textureWidth = width;
    m_data->textureHeight = height;
    CreateAndCacheAtlas<std::uint8_t, float, 3, msdf_atlas::msdfGenerator>(
        float(emSize), m_data->glyphs, m_data->geometry, width, height, m_data->textureData);

#if 0
    msdfgen::Shape shape;
    if (msdfgen::loadGlyph(shape, font, 'A'))
    {
        shape.normalize();
        //                      max. angle
        edgeColoringSimple(shape, 3.0);
        //           image width, height
        msdfgen::Bitmap<float, 3> msdf(32, 32);
        //                     range, scale, translation
        msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
        msdfgen::savePng(msdf, "output.png");
    }
#endif

    msdfgen::destroyFont(font);
    msdfgen::deinitializeFreetype(ft);
}

Font::~Font() = default;

auto Font::get_texture_width() const -> std::uint32_t
{
    return m_data->textureWidth;
}

auto Font::get_texture_height() const -> std::uint32_t
{
    return m_data->textureHeight;
}

auto Font::get_texture_data() const -> const void*
{
    return m_data->textureData.data();
}

void Font::set_texture_id(void* texture)
{
    m_textureId = texture;
}

auto Font::get_geometry() const -> const msdf_atlas::FontGeometry&
{
    return m_data->geometry;
}
