#include "font.hpp"

#include <msdf-atlas-gen.h>
#include <msdfgen.h>

Font::Font(const std::filesystem::path& fontFilename)
{
    msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
    if (ft)
    {
        msdfgen::FontHandle* font = msdfgen::loadFont(ft, fontFilename.string().c_str());
        if (font)
        {
            msdfgen::Shape shape;
            if (loadGlyph(shape, font, 'A'))
            {
                shape.normalize();
                //                      max. angle
                edgeColoringSimple(shape, 3.0);
                //           image width, height
                msdfgen::Bitmap<float, 3> msdf(32, 32);
                //                     range, scale, translation
                generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
                savePng(msdf, "output.png");
            }
            destroyFont(font);
        }
        deinitializeFreetype(ft);
    }
}