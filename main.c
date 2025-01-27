#include "crc.h"
#include "bmp.h"
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define CFG_DEF_WIDTH 4
#define CFG_DEF_SCALE_TARGET 256

enum Clr_Mode {
    Clr_Mode_Random,
    Clr_Mode_Grey,
    Clr_Mode_Redshift,
    Clr_Mode_Greenshift,
    Clr_Mode_Blueshift,
    Clr_Mode_Earth,
    Clr_Mode_Binary,
    Clr_Mode_Primary,
    Clr_Mode_Pastel,
    Clr_Mode_Greyish,
    Clr_Mode_Garden,
};

struct Clr_Mode_Dict {
    const char *name;
    enum Clr_Mode mode;
    void (*color_px_func)(struct BMP *image, int pos);
};

// NOTE: BMP is BGR not RGB
void color_px_random(struct BMP *image, int pos) {
    image->data[pos] = rand() % 256;
    image->data[pos + 1] = rand() % 256;
    image->data[pos + 2] = rand() % 256;
}
void color_px_gray(struct BMP *image, int pos) {
    int rng = rand() % 256;
    image->data[pos] = rng;
    image->data[pos + 1] = rng;
    image->data[pos + 2] = rng;
}
void color_px_redshift(struct BMP *image, int pos) {
    image->data[pos] = rand() % 128;
    image->data[pos + 1] = rand() % 64;
    image->data[pos + 2] = rand() % 200;
}
void color_px_greenshift(struct BMP *image, int pos) {
    image->data[pos] = rand() % 128;
    image->data[pos + 1] = rand() % 200;
    image->data[pos + 2] = rand() % 64;
}
void color_px_blueshift(struct BMP *image, int pos) {
    image->data[pos] = rand() % 200;
    image->data[pos + 1] = rand() % 64;
    image->data[pos + 2] = rand() % 128;
}
void color_px_earth(struct BMP *image, int pos) {
    image->data[pos] = rand() % 128;
    image->data[pos + 1] = rand() % 128;
    image->data[pos + 2] = rand() % 255;
}
void color_px_binary(struct BMP *image, int pos) {
    int rng = (rand() % 256) > 128 ? 255 : 0;
    image->data[pos] = rng;
    image->data[pos + 1] = rng;
    image->data[pos + 2] = rng;
}
void color_px_primary(struct BMP *image, int pos) {
    image->data[pos] = (rand() % 256) > 128 ? 255 : 0;
    image->data[pos + 1] = (rand() % 256) > 128 ? 255 : 0;
    image->data[pos + 2] = (rand() % 256) > 128 ? 255 : 0;
}
void color_px_pastel(struct BMP *image, int pos) {
    image->data[pos] = (rand() % 128) + 128;
    image->data[pos + 1] = (rand() % 128) + 128;
    image->data[pos + 2] = (rand() % 128) + 128;
}
void color_px_greyish(struct BMP *image, int pos) {
    image->data[pos] = 32 + (rand() % 96);
    image->data[pos + 1] = 32 + (rand() % 96);
    image->data[pos + 2] = 32 + (rand() % 96);
}
void color_px_garden(struct BMP *image, int pos) {
    int rng = rand() % 42;
    image->data[pos] = 32 + rng;
    image->data[pos + 1] = 34 + rng;
    image->data[pos + 2] =  32 + rng;
}
const struct Clr_Mode_Dict color_mode_dict[] = {
    {"random", Clr_Mode_Random, color_px_random},
    {"grey", Clr_Mode_Grey, color_px_gray},
    {"redshift", Clr_Mode_Redshift, color_px_redshift},
    {"greenshift", Clr_Mode_Greenshift, color_px_greenshift},
    {"blueshift", Clr_Mode_Blueshift, color_px_blueshift},
    {"earth", Clr_Mode_Earth, color_px_earth},
    {"binary", Clr_Mode_Binary, color_px_binary},
    {"primary", Clr_Mode_Primary, color_px_primary},
    {"pastel", Clr_Mode_Pastel, color_px_pastel},
    {"greyish", Clr_Mode_Greyish, color_px_greyish},
    {"garden", Clr_Mode_Garden, color_px_garden},
    {NULL, 0, color_px_random}
};
void color_px(struct BMP *image, int pos, enum Clr_Mode mode) {
    for (int i = 0; color_mode_dict[i].name != NULL; i++) {
        if (color_mode_dict[i].mode == mode) {
            color_mode_dict[i].color_px_func(image, pos);
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    enum Clr_Mode cfg_mode = Clr_Mode_Random;
    const char *cfg_filename = argv[argc -1]; // last argument
    const char *cfg_filename_out = argv[argc -1]; // last argument
    int cfg_width = -1;
    int cfg_height = -1;
    int cfg_scale = -1;
    int cfg_rng_off = 0;
    int cfg_bilinear = 0;
    int opt;
    while ((opt = getopt(argc, argv, "m:x:y:s:o:r:bh")) != -1) {
        switch (opt) {
            case 'm':
                for (int i = 0; color_mode_dict[i].name != NULL; i++) {
                    if (strcmp(optarg, color_mode_dict[i].name) == 0) {
                        cfg_mode = color_mode_dict[i].mode;
                        break;
                    }
                }
                break;
            case 'x':
                cfg_width = atoi(optarg);
                break;
            case 'y':
                cfg_height = atoi(optarg);
                break;
            case 's':
                cfg_scale = atoi(optarg);
                break;
            case 'o':
                cfg_filename_out = optarg;
                break;
            case 'r':
                cfg_rng_off = atoi(optarg);
                break;
            case 'b':
                cfg_bilinear = 1;
                break;
            case 'h':
            default:
                fprintf(stderr, "Usage: %s [flags] <filename>\n", argv[0]);
                fprintf(stderr, "Flags:\n");
                fprintf(stderr, "  -m <mode>    Set the color mode (default: random)\n");
                fprintf(stderr, "  -x <width>   Set the width of the image (default: 4)\n");
                fprintf(stderr, "  -y <height>  Set the height of the image (default: same as width)\n");
                fprintf(stderr, "  -s <scale>   Set the scale factor (default: 256/width)\n");
                fprintf(stderr, "  -o <output>  Set the output filename (default: crc.filename.bmp)\n");
                fprintf(stderr, "  -r <offset>  Set the RNG offset (default: 0)\n");
                fprintf(stderr, "  -b           Use bilinear scaling (default: nearest neighbor)\n");
                fprintf(stderr, "  -h           Show this help message\n");
                fprintf(stderr, "Available modes:\n");
                for (int i = 0; color_mode_dict[i].name != NULL; i++) {
                    fprintf(stderr, "  %s\n", color_mode_dict[i].name);
                }
                return 1;
        }
    }
    // Defaults
    if (strcmp(cfg_filename, cfg_filename_out) == 0) {
        char new_filename[256];
        snprintf(new_filename, sizeof(new_filename), "%s.crc.bmp", cfg_filename);
        cfg_filename_out = new_filename;
    }
    if (cfg_width == -1) {
        cfg_width = CFG_DEF_WIDTH;
    }
    if (cfg_height == -1) {
        cfg_height = cfg_width;
    }
    if (cfg_scale == -1) {
        cfg_scale = CFG_DEF_SCALE_TARGET/cfg_width;
    }
    uint32_t crc = crc32_file(cfg_filename);
    printf("%s\n", crc32_to_hex(crc));
    srand(crc + cfg_rng_off);
    struct BMP image;
    image.width = cfg_width;
    image.height = cfg_height;
    image.data = malloc(image.width * image.height * 3);
    for (int i = 0; i < image.width * image.height * 3; i+=3) {
        color_px(&image, i, cfg_mode);
    }
    if (cfg_bilinear) {
        bmp_resize_bilinear(&image, image.width * cfg_scale, image.height * cfg_scale);
    } else {
        bmp_resize(&image, image.width * cfg_scale, image.height * cfg_scale);
    }
    char new_filename[256];
    snprintf(new_filename, sizeof(new_filename), "%s", cfg_filename_out);
    printf("Writing to %s\n", new_filename);
    bmp_write(new_filename, &image);
    free(image.data);
    return 0;
}