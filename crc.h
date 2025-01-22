#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

uint32_t crc32(uint32_t crc, const void *buf, size_t size) {
    static uint32_t table[256];
    static int have_table = 0;
    uint32_t rem;
    uint8_t octet;
    const uint8_t *p, *q;

    if (have_table == 0) {
        for (int i = 0; i < 256; i++) {
            rem = i;
            for (int j = 0; j < 8; j++) {
                if (rem & 1) {
                    rem >>= 1;
                    rem ^= 0xedb88320;
                } else {
                    rem >>= 1;
                }
            }
            table[i] = rem;
        }
        have_table = 1;
    }

    crc = ~crc;
    q = buf + size;
    for (p = buf; p < q; p++) {
        octet = *p;
        crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
    }
    return ~crc;
}

uint32_t crc32_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return 0;
    }

    uint8_t buffer[1024];
    size_t bytesRead;
    uint32_t crc = 0;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        crc = crc32(crc, buffer, bytesRead);
    }

    if (ferror(file)) {
        perror("Error reading file");
        fclose(file);
        return 0;
    }

    fclose(file);
    return crc;
}

char *crc32_to_hex(uint32_t crc) {
    static char hex[9];
    snprintf(hex, sizeof(hex), "%08x", crc);
    return hex;
}