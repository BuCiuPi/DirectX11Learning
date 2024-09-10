#pragma once
#include <cstdint>
#include <fstream>
#include <vector>


class BitmapBuilder
{
public:
    struct BmpHeader {
        char bitmapSignatureBytes[2] = { 'B', 'M' };
        uint32_t sizeOfBitmapFile = 54 + 12582912;
        uint32_t reservedBytes = 0;
        uint32_t pixelDataOffset = 54;
    } bmpHeader;

    struct BmpInfoHeader {
        uint32_t sizeOfThisHeader = 40;
        int32_t width = 2048; // in pixels
        int32_t height = 2048; // in pixels
        uint16_t numberOfColorPlanes = 1; // must be 1
        uint16_t colorDepth = 24;
        uint32_t compressionMethod = 0;
        uint32_t rawBitmapDataSize = 0; // generally ignored
        int32_t horizontalResolution = 37800; // in pixel per meter
        int32_t verticalResolution = 37800; // in pixel per meter
        uint32_t colorTableEntries = 0;
        uint32_t importantColors = 0;
    } bmpInfoHeader;

    struct Pixel {
        uint8_t blue = 255;
        uint8_t green = 255;
        uint8_t red = 0;
    } pixel;

	void Save(std::vector<float> in)
	{
        std::ofstream fout("output.bmp", std::ios::binary);
        fout.write((char*)&bmpHeader, 14);
        fout.write((char*)&bmpInfoHeader, 40);

        size_t numberOfPixel = in.size();
        for (int i = 0; i < numberOfPixel; ++i)
        {
            Pixel currentPixel;
            currentPixel.blue = in[i] * 255;
            currentPixel.green = in[i] * 255;
            currentPixel.red = in[i]* 255;

            fout.write((char *)& currentPixel, 3);
        }

        fout.close();
	}
};

