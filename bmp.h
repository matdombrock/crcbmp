#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

struct BMP {
    int width;
    int height;
    uint8_t *data;
};

void bmp_resize(struct BMP *image, int new_width, int new_height){
    // Allocate memory for the new image
    uint8_t *new_data = (uint8_t *)malloc(new_width * new_height * 3);
    if (!new_data) {
        fprintf(stderr, "Failed to allocate memory for scaled image\n");
        return;
    }

    // Calculate scaling factors
    float x_ratio = (float)image->width / new_width;
    float y_ratio = (float)image->height / new_height;

    // Scale the image
    for (int i = 0; i < new_height; i++) {
        for (int j = 0; j < new_width; j++) {
            int x = (int)(j * x_ratio);
            int y = (int)(i * y_ratio);
            new_data[(i * new_width + j) * 3] = image->data[(y * image->width + x) * 3];
            new_data[(i * new_width + j) * 3 + 1] = image->data[(y * image->width + x) * 3 + 1];
            new_data[(i * new_width + j) * 3 + 2] = image->data[(y * image->width + x) * 3 + 2];
        }
    }

    // Free the old data and update the image structure
    free(image->data);
    image->data = new_data;
    image->width = new_width;
    image->height = new_height;
}

void bmp_resize_bilinear(struct BMP *image, int new_width, int new_height) {
    // Allocate memory for the new image
    uint8_t *new_data = (uint8_t *)malloc(new_width * new_height * 3);
    if (!new_data) {
        fprintf(stderr, "Failed to allocate memory for scaled image\n");
        return;
    }

    // Calculate scaling factors
    float x_ratio = (float)image->width / new_width;
    float y_ratio = (float)image->height / new_height;

    // Scale the image using bilinear interpolation
    for (int i = 0; i < new_height; i++) {
        for (int j = 0; j < new_width; j++) {
            int x = (int)(j * x_ratio);
            int y = (int)(i * y_ratio);
            float x_diff = (j * x_ratio) - x;
            float y_diff = (i * y_ratio) - y;

            // Get the pixel values
            uint8_t a_r = image->data[(y * image->width + x) * 3];
            uint8_t a_g = image->data[(y * image->width + x) * 3 + 1];
            uint8_t a_b = image->data[(y * image->width + x) * 3 + 2];
            uint8_t b_r = image->data[(y * image->width + (x + 1)) * 3];
            uint8_t b_g = image->data[(y * image->width + (x + 1)) * 3 + 1];
            uint8_t b_b = image->data[(y * image->width + (x + 1)) * 3 + 2];
            uint8_t c_r = image->data[((y + 1) * image->width + x) * 3];
            uint8_t c_g = image->data[((y + 1) * image->width + x) * 3 + 1];
            uint8_t c_b = image->data[((y + 1) * image->width + x) * 3 + 2];
            uint8_t d_r = image->data[((y + 1) * image->width + (x + 1)) * 3];
            uint8_t d_g = image->data[((y + 1) * image->width + (x + 1)) * 3 + 1];
            uint8_t d_b = image->data[((y + 1) * image->width + (x + 1)) * 3 + 2];

            // Apply bilinear interpolation
            new_data[(i * new_width + j) * 3]     = a_r * (1 - x_diff) * (1 - y_diff) + b_r * x_diff * (1 - y_diff) + c_r * y_diff * (1 - x_diff) + d_r * x_diff * y_diff;
            new_data[(i * new_width + j) * 3 + 1] = a_g * (1 - x_diff) * (1 - y_diff) + b_g * x_diff * (1 - y_diff) + c_g * y_diff * (1 - x_diff) + d_g * x_diff * y_diff;
            new_data[(i * new_width + j) * 3 + 2] = a_b * (1 - x_diff) * (1 - y_diff) + b_b * x_diff * (1 - y_diff) + c_b * y_diff * (1 - x_diff) + d_b * x_diff * y_diff;
        }
    }
    // Free the old data and update the image structure
    free(image->data);
    image->data = new_data;
    image->width = new_width;
    image->height = new_height;
}

void bmp_write(const char *filename, struct BMP *image) {
    // Define the BMP file header
    BITMAPFILEHEADER fileHeader;
    fileHeader.bfType = 0x4D42; // 'BM'
    fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + image->width * image->height * 3;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Define the BMP info header
    BITMAPINFOHEADER infoHeader;
    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = image->width;
    infoHeader.biHeight = image->height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24; // 24-bit bitmap
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = image->width * image->height * 3;
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;

    // Write the BMP file
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Unable to open file");
        return;
    }

    fwrite(&fileHeader, sizeof(fileHeader), 1, file);
    fwrite(&infoHeader, sizeof(infoHeader), 1, file);
    fwrite(image->data, image->width * image->height * 3, 1, file);

    fclose(file);
}