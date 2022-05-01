#include <vector>
#include "sample_args.h"
#include "encode_c.h"
#include "BMP.h"

using namespace std;

int main(int argc, char** argv) {
    std::string* image_path = nullptr;
    std::string* output_path = nullptr;

    std::vector<arg_desc_t> args = {
    	{"-i", "Image path",  ARG_TYPE_TEXT, &image_path },
		{"-o", "Output path", ARG_TYPE_TEXT, &output_path} };

    args_parser args_parser(args, argc, argv);

    if (image_path) {
        BMP image(image_path->c_str());

        const int num_bytes = image.bmp_info_header.bit_count / 8;
        const int width = image.bmp_info_header.width;
        const int height = image.bmp_info_header.height;

        vector<uint8_t> planes[CHANNEL_NUM], luma, out_planes[CHANNEL_NUM];
        for (int i = 0; i < CHANNEL_NUM; i++) {
            planes[i].resize(width * height);
            out_planes[i].resize(width * height);
        }
        luma.resize(width * height);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                auto pix_offset = (x + y * width) * num_bytes;
                uint8_t r = image.data[pix_offset];
                uint8_t g = image.data[pix_offset + 1];
                uint8_t b = image.data[pix_offset + 2];

                planes[CHANNEL_RED][x + y * width] = r;
                planes[CHANNEL_GREEN][x + y * width] = g;
                planes[CHANNEL_BLUE][x + y * width] = b;

                luma[x + y * width] = uint8_t((int(r) + int(g) + int(b) + 1) / 3);
            }
        }

        uint8_t* ref[CHANNEL_NUM];
        uint8_t* dst[CHANNEL_NUM];
        for (int i = 0; i < CHANNEL_NUM; i++) {
            ref[i] = planes[i].data();
            dst[i] = out_planes[i].data();
        }

        intra_pred_image(dst, ref, luma.data(), width, width, height);

        if (output_path) {
            BMP out_image(width, height, false);

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    auto pix_offset = (x + y * width) * num_bytes;
                    out_image.data[pix_offset]     = out_planes[CHANNEL_RED][x + y * width];
                    out_image.data[pix_offset + 1] = out_planes[CHANNEL_GREEN][x + y * width];
                    out_image.data[pix_offset + 2] = out_planes[CHANNEL_BLUE][x + y * width];
                }
            }

            out_image.write(output_path->c_str());
        }
    }
    return 0;
}