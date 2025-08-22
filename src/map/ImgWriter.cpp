#include "ImgWriter.h"

#include "Util.h"
#include "World.h"
#include "map/TileColor.h"
#include "vendor/fpng.h"

void savePreviewImage(std::string basename, World &world)
{
    std::vector<uint8_t> img(3 * world.getWidth() * world.getHeight());
    parallelFor(std::views::iota(0, world.getWidth()), [&img, &world](int x) {
        for (int y = 0; y < world.getHeight(); ++y) {
            Color color = getTileColor(x, y, world);
            std::copy(
                color.rgb,
                color.rgb + 3,
                img.begin() + 3 * (x + y * world.getWidth()));
        }
    });
    fpng::fpng_init();
    basename += "-map.png";
    fpng::fpng_encode_image_to_file(
        basename.c_str(),
        img.data(),
        world.getWidth(),
        world.getHeight(),
        3);
}
