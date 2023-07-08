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

private:
    std::unique_ptr<FontData> m_data;
    void* m_atlasTexture{ nullptr };
};