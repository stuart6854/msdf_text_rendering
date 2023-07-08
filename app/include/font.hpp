#pragma once

#include <memory>
#include <vector>
#include <filesystem>

struct FontData;

class Font
{
public:
    Font(const std::filesystem::path& fontFilename);
    ~Font();

    auto get_texture_width() const -> std::uint32_t ;
    auto get_texture_height() const -> std::uint32_t ;
    auto get_texture_data() const -> const void*;

    void set_texture_id(void* texture);

private:
    std::unique_ptr<FontData> m_data;
    void* m_textureId{ nullptr };
};