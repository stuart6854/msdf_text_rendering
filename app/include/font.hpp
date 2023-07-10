#pragma once

#include <memory>
#include <vector>
#include <filesystem>

#include <msdf-atlas-gen.h>
#include <FontGeometry.h>

/* A structure that describes a glyph. */
struct GlyphInfo
{
    int Width;       // Glyphs width in pixels.
    int Height;      // Glyphs height in pixels.
    int OffsetX;     // The distance from the origin ("pen position") to the left of the glyph.
    int OffsetY;     // The distance from the origin to the top of the glyph. This is usally a value < 0.
    float AdvanceX;  // The distance from the origin to the origin of the next glyph. This is usually a value > 0.
};

/* A structure that describes a fonts parameters & metrics. */
struct FontInfo
{
    std::uint32_t PixelHeight;  // Size this font was generated with.
    float Ascender;             // The pixel extents above the baseline in pixels(usually positive).
    float Descender;            // The extens below the baseline in pixels (usually negative).
    float LineSpacing;      // The baseline-to-baseline distance. Note: This is usually larger than the sum of the ascender and descender.
    float LineGap;          // The spacing in pixels between one rows descent and the next rows ascent.
    float MaxAdvanceWidth;  // This field gives the maximum horizontal cursor advance for all glyphs in the font.
};

struct FontData
{
    std::vector<msdf_atlas::GlyphGeometry> glyphs{};
    msdf_atlas::FontGeometry geometry{};

    std::uint32_t textureWidth{};
    std::uint32_t textureHeight{};
    std::vector<std::uint8_t> textureData{};
};

class Font
{
public:
    Font(const std::filesystem::path& fontFilename);
    ~Font();

    auto get_texture_width() const -> std::uint32_t;
    auto get_texture_height() const -> std::uint32_t;
    auto get_texture_data() const -> const void*;

    void set_texture_id(void* texture);

    auto get_geometry() const -> const msdf_atlas::FontGeometry&;

private:
    std::unique_ptr<FontData> m_data;
    void* m_textureId{ nullptr };
};