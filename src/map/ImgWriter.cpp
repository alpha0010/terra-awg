#include "ImgWriter.h"

#include "World.h"
#include "map/TileColor.h"
#include "vendor/fpng.h"

void savePreviewImage(std::string basename, World &world)
{
    std::vector<uint8_t> img;
    img.reserve(3 * world.getWidth() * world.getHeight());
    for (int y = 0; y < world.getHeight(); ++y) {
        for (int x = 0; x < world.getWidth(); ++x) {
            const uint8_t *tileColor = getTileColor(x, y, world);
            img.insert(img.end(), tileColor, tileColor + 3);
        }
    }
    fpng::fpng_init();
    basename += "-map.png";
    fpng::fpng_encode_image_to_file(
        basename.c_str(),
        img.data(),
        world.getWidth(),
        world.getHeight(),
        3);
}
