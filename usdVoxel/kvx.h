// 2024 - Danny Spencer

#ifndef __KVX_H__
#define __KVX_H__

#include <stdint.h>
#include <stdlib.h>

template <class T> 
static bool KvxRead(const unsigned char *contents, size_t contents_size, T &cubePlacer) {
#define ERROR(message) return false

    if (contents_size < 768) {
        // Needs enough space for the color palette
        ERROR("File too small");
    }

    size_t read_offset = 0;

    // It's expected to run out of room in the file - it means there are no levels left. So we just exit successfully.
#define READ_BUF(var, size, type) \
    if (read_offset + (size) * sizeof(type) > contents_size) { return true; } \
    const type *var = (type*)(contents + read_offset); \
    read_offset += (size) * sizeof(type)

#define READ_U32(var) uint32_t var; { READ_BUF(buf, 4, uint8_t); var = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3]<<24); }

    // it's easier to read the palette first
    // this is always at the end of the file
    const uint8_t *palette = contents + contents_size - 768;
    // shrink the perceived size of the contents
    contents_size -= 768;

    // Up to 5 levels
    for (int level = 0; level < 5; level++) {
        READ_U32(numbytes);
        READ_U32(xsiz);
        READ_U32(ysiz);
        READ_U32(zsiz);
        READ_U32(xpivot);
        READ_U32(ypivot);
        READ_U32(zpivot);

        READ_BUF(xoffset, xsiz+1, uint32_t);
        READ_BUF(xyoffset, xsiz * (ysiz+1), uint16_t);

        // header size, excluding numbytes (so: xsiz, ysiz, zsiz, xpivot, ypivot, zpivot)
        uint32_t header_size = 24 + (xsiz+1)*4 + xsiz*(ysiz+1)*2;
        if (numbytes < header_size) {
            ERROR("numbytes is smaller than header");
        }
        uint32_t voxdata_size = numbytes - header_size;

        READ_BUF(voxdata, voxdata_size, uint8_t);

        cubePlacer.setLevel(level);
        // KVX is opinionated with X=right, Y=front, and Z=down.
        // Reorient to: X=right, Y=up, Z=front
        // (x,y,z) = (x,-z,y)
        cubePlacer.setCentroid((float)xpivot / 256.0f, -(float)zpivot / 256.0f, (float)ypivot / 256.0f);

        size_t off = 0;

        for (int32_t x = 0; x < xsiz; x++) {
            for (int32_t y = 0; y < ysiz; y++) {
                size_t start = xoffset[x] + xyoffset[x*(ysiz+1) + y];
                size_t end   = xoffset[x] + xyoffset[x*(ysiz+1) + y + 1];

                int32_t n = end - start;

                while (n > 0) {
                    const uint8_t *startptr = voxdata + off;

                    uint8_t slabztop = startptr[0];
                    uint8_t slabzleng = startptr[1];

                    // unused for now
                    uint8_t slabbackfacecullinfo = startptr[2];

                    for (int32_t i = 0; i < slabzleng; i++) {
                        int32_t z = slabztop + i;
                        uint8_t val = startptr[3 + i];

                        uint8_t r = palette[val*3 + 0];
                        uint8_t g = palette[val*3 + 1];
                        uint8_t b = palette[val*3 + 2];

                        float fr = (float)r / 63.0f;
                        float fg = (float)g / 63.0f;
                        float fb = (float)b / 63.0f;

                        // KVX is opinionated with X=right, Y=front, and Z=down.
                        // Reorient to: X=right, Y=up, Z=front
                        // (x,y,z) = (x,-z,y)
                        cubePlacer.place(x, -z, y, fr, fg, fb, slabbackfacecullinfo);
                    }

                    n -= slabzleng + 3;
                    off += slabzleng + 3;
                }

            }
        }
    }

#undef READ_U32
#undef READ_BUF
#undef ERROR

    return true;
}

#endif
