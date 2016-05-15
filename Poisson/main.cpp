/**
 * \file Poisson.cpp
 * \brief
 *
 * Poisson Disk Points Generator example
 *
 * \version 1.1.3
 * \date 10/03/2016
 * \author Sergey Kosarevsky, 2014-2016
 * \author support@linderdaum.com   http://www.linderdaum.com   http://blog.linderdaum.com
 */

/*
	To compile:
		gcc Poisson.cpp -std=c++11 -lstdc++
*/

#include <vector>
#include <iostream>
#include <fstream>
#include <memory.h>
#include <time.h>
#include <stdio.h>

#include "poisson.h"

const int   ImageSize   = 1024;	// generate RGB image [ImageSize x ImageSize]

////////////////////////////////////////////////////////////////////////////

std::vector<float> g_DensityMap(ImageSize * ImageSize, 1.0f);

#if defined( __GNUC__ )
#	define GCC_PACK(n) __attribute__((packed,aligned(n)))
#else
#	define GCC_PACK(n) __declspec(align(n))
#endif // __GNUC__

#pragma pack(push, 1)
struct GCC_PACK( 1 ) sBMPHeader
{
    // BITMAPFILEHEADER
    unsigned short    bfType;
    uint32_t          bfSize;
    unsigned short    bfReserved1;
    unsigned short    bfReserved2;
    uint32_t          bfOffBits;
    // BITMAPINFOHEADER
    uint32_t          biSize;
    uint32_t          biWidth;
    uint32_t          biHeight;
    unsigned short    biPlanes;
    unsigned short    biBitCount;
    uint32_t          biCompression;
    uint32_t          biSizeImage;
    uint32_t          biXPelsPerMeter;
    uint32_t          biYPelsPerMeter;
    uint32_t          biClrUsed;
    uint32_t          biClrImportant;
};
#pragma pack(pop)

void SaveBMP( const char* FileName, const void* RawBGRImage, int Width, int Height )
{
    sBMPHeader Header;

    int ImageSize = Width * Height * 3;

    Header.bfType          = 0x4D * 256 + 0x42;
    Header.bfSize          = ImageSize + sizeof( sBMPHeader );
    Header.bfReserved1     = 0;
    Header.bfReserved2     = 0;
    Header.bfOffBits       = 0x36;
    Header.biSize          = 40;
    Header.biWidth         = Width;
    Header.biHeight        = Height;
    Header.biPlanes        = 1;
    Header.biBitCount      = 24;
    Header.biCompression   = 0;
    Header.biSizeImage     = ImageSize;
    Header.biXPelsPerMeter = 6000;
    Header.biYPelsPerMeter = 6000;
    Header.biClrUsed       = 0;
    Header.biClrImportant  = 0;

    std::ofstream File( FileName, std::ios::out | std::ios::binary );

    File.write( (const char*)&Header, sizeof( Header ) );
    File.write( (const char*)RawBGRImage, ImageSize );

    std::cout << "Saved " << FileName << std::endl;
}

unsigned char* LoadBMP( const char* FileName, int* OutWidth, int* OutHeight )
{
    sBMPHeader Header;

    std::ifstream File( FileName, std::ifstream::binary );

    File.read( (char*)&Header, sizeof( Header ) );

    *OutWidth  = Header.biWidth;
    *OutHeight = Header.biHeight;

    size_t DataSize = 3 * Header.biWidth * Header.biHeight;

    unsigned char* Img = new unsigned char[ DataSize ];

    File.read( (char*)Img, DataSize );

    return Img;
}

void LoadDensityMap( const char* FileName )
{
    std::cout << "Loading density map " << FileName << std::endl;

    int W, H;
    unsigned char* Data = LoadBMP( FileName, &W, &H );

    std::cout << "Loaded ( " << W << " x " << H << " ) " << std::endl;

    if ( W != ImageSize || H != ImageSize )
    {
        std::cout << "ERROR: density map should be " << ImageSize << " x " << ImageSize << std::endl;

        exit( 255 );
    }

    for ( int y = 0; y != H; y++ )
    {
        for ( int x = 0; x != W; x++ )
        {
            g_DensityMap[ x + y * W ] = float( Data[ 3 * (x + y * W) ] ) / 255.0f;
        }
    }
}

void PrintBanner()
{
    std::cout << "Poisson disk points generator" << std::endl;
    std::cout << "Sergey Kosarevsky, 2014-2016" << std::endl;
    std::cout << "support@linderdaum.com http://www.linderdaum.com http://blog.linderdaum.com" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: Poisson [density-map-rgb24.bmp]" << std::endl;
    std::cout << std::endl;
}

void addEntity(
        std::vector<std::vector<Poisson::EntityTemplate>>& groups,
        int radius,
        float selfSpawn,
        int group,
        int id,
        int count){

    groups[group].push_back(Poisson::EntityTemplate((float) radius / ImageSize, selfSpawn, group, id, count));

}

int main( int argc, char** argv )
{
    PrintBanner();


    std::string s = "noise" + std::to_string(ImageSize) + ".bmp";
    LoadDensityMap(s.c_str());

    unsigned int seed = 1284;
    Poisson::PRNG generator(seed);

    const unsigned int groupCount = 2;

    std::vector<std::vector<Poisson::EntityTemplate>> groups(groupCount);

    groups.resize(groupCount);

    int id = 1;
    int giantCrownSize = 100;

    int n_group0 = 3;
    int treeCrownSize = 15;
    Poisson::AddDistanceRule(0, 0, (float)treeCrownSize / ImageSize);
    addEntity(groups, 9, 0.95f, 0, id++, 100); // Maple
    addEntity(groups, 8 , 0.95f, 0, id++, 100); // Birch
    addEntity(groups, 6,  0.01f, 0, id++, 5);   // Bush

    int n_group1 = 5;
    Poisson::AddDistanceRule(1, 0, 0);
    Poisson::AddDistanceRule(1, 1, 0);
    addEntity(groups, 2, 0.0f, 1, id++, 1);    // Rare flower
    addEntity(groups, 2, 0.3f, 1, id++, 550);  // Orange flower
    addEntity(groups, 2, 0.3f, 1, id++, 550);  // Yellow flower
    addEntity(groups, 2, 0.3f, 1, id++, 550);  // White flower
    addEntity(groups, 2, 0.8f, 1, id++, 9500); // Grass


    int entitiesPerGroup[groupCount] = {n_group0, n_group1};

    const clock_t begin_time = clock();

    const auto Entities = Poisson::GeneratePoisson(
            groups,
            groupCount,
            entitiesPerGroup,
            generator,
            g_DensityMap,
            ImageSize,
            false
    );

    std::cout << "Elapsed time: " << float( clock() - begin_time) / CLOCKS_PER_SEC << std::endl;

    // prepare BGR image
    size_t DataSize = 3 * ImageSize * ImageSize;

    unsigned char* Img = new unsigned char[ DataSize ];

    memset( Img, 0, DataSize );

    for ( auto i = Entities.begin(); i != Entities.end(); i++ )
    {
        int x = int( i->x * ImageSize );
        int y = int( i->y * ImageSize );

        unsigned char R = 0;
        unsigned char G = 0;
        unsigned char B = 0;

        float r = i->radius * ImageSize;

        if (i->id == 0) {
            // Draw tree crown
            for (float q = 0; q < 2 * 3.141592653589f; q += 0.01f) {

                float rCrown = r + giantCrownSize;
                int u = x + (int) (rCrown * 0.9f * cosf(q));
                int v = y + (int) (rCrown * 0.9f * sinf(q));

                int Z = 3 * (u + v * ImageSize);

                if ( Z > 0 && Z < 3 * ImageSize * ImageSize && Img[Z + 0] == 0) {
                    Img[Z + 2] = 0;
                    Img[Z + 1] = 255;
                    Img[Z + 0] = 0;
                }
            }

            R = 139;
            G =  69;
            B =  19;
        }

        if (i->id == 1) {
            // Draw tree crown
            for (float q = 0; q < 2 * 3.141592653589f; q += 0.02f) {

                float rCrown = r + treeCrownSize;
                int u = x + (int) (rCrown * 0.9f * cosf(q));
                int v = y + (int) (rCrown * 0.9f * sinf(q));

                int Z = 3 * (u + v * ImageSize);

                if ( Z > 0 && Z < 3 * ImageSize * ImageSize && Img[Z + 0] == 0) {
                    Img[Z + 2] = 100;
                    Img[Z + 1] = 255;
                    Img[Z + 0] = 80;
                }
            }
            R = 139;
            G = 69;
            B = 19;
        }

        if (i->id ==  2) {
            // Draw tree crown
            for (float q = 0; q < 2 * 3.141592653589f; q += 0.03f) {

                float rCrown = r + treeCrownSize;
                int u = x + (int) ((rCrown) * 0.9f * cosf(q));
                int v = y + (int) ((rCrown) * 0.9f * sinf(q));

                int Z = 3 * (u + v * ImageSize);

                if ( Z > 0 && Z < 3 * ImageSize * ImageSize && Img[Z + 0] == 0) {
                    Img[Z + 2] = 0;
                    Img[Z + 1] = 200;
                    Img[Z + 0] = 0;
                }
            }
            R = 255;
            G = 255;
            B = 255;
        }

        if (i->id ==  3) {
            R = 120;
            G = 160;
            B = 90;
        }

        if (i->id ==  4) {
            R = 255;
            G = 0;
            B = 255;
        }

        if (i->id ==  5) {
            R = 255;
            G = 128;
            B = 45;
        }

        if (i->id ==  6) {
            R = 255;
            G = 255;
            B = 0;
        }

        if (i->id ==  7) {
            R = 255;
            G = 255;
            B = 255;
        }



        if (i->id ==  8) {
            R =20;
            G =100;
            B =20;
        }

        for (float q = 0; q < 2 * 3.141592653589f; q += 0.1f) {

            int u = x + (int) (r * 0.9f * cosf(q));
            int v = y + (int) (r * 0.9f * sinf(q));

            int Z = 3 * (u + v * ImageSize);

            if ( Z > 0 && Z < 3 * ImageSize * ImageSize
                 && Img[Z + 0] == 0
                 && Img[Z + 1] == 0
                 && Img[Z + 2] == 0) {

                Img[Z + 2] = R;
                Img[Z + 1] = G;
                Img[Z + 0] = B;
            }
        }
    }

    SaveBMP( "Poisson.bmp", Img, ImageSize, ImageSize );

    delete[]( Img );

    // dump points to a text file
    std::ofstream File( "Poisson.txt", std::ios::out );

    File << "NumPoints = " << Entities.size() << std::endl;

    for ( const auto& p : Entities )
    {
        File << "X = " << p.x << "; Y = " << p.y << std::endl;
    }

    return 0;
}
