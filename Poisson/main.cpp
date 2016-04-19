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

const int   ImageSize   = 2048;	// generate RGB image [ImageSize x ImageSize]

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
        std::string value,
        int count){
    Poisson::EntityTemplate t((float)radius / ImageSize, selfSpawn, group, value, count);
    groups[group].push_back(t);
}

void addRule(int group1, int group2, int distance){
    Poisson::AddDistanceRule(group1, group2, (float) distance  / ImageSize);
}

int main( int argc, char** argv )
{
    PrintBanner();


    std::string s = "noise" + std::to_string(ImageSize) + ".bmp";
    LoadDensityMap(s.c_str());

    int seed = 1284;
    Poisson::PRNG generator(seed);

    const unsigned int groupCount = 3;

    std::vector<std::vector<Poisson::EntityTemplate>> groups(groupCount);


    int giantCrownSize = 100;
    //addRule(0, 0, giantCrownSize * 3);

    int treeCrownSize = 15;
    //addRule(1, 0, giantCrownSize / 2);
    addRule(1, 1, treeCrownSize);

    //addRule(2, 0, 0);
    addRule(2, 1, 0);
    addRule(2, 2, 0);


    int group0 = 0;
    //addEntity(groups, 80, 0.0f, 0, "GiantMaple", 1);

    int group1 = 3;
    addEntity(groups, 10, 0.95f, 1, "Maple", 100);
    addEntity(groups, 8 , 0.95f, 1, "Birch", 100);
    addEntity(groups, 6,  0.01f, 1, "Bush" , 40);

    int group2 = 5;
    addEntity(groups, 2, 0.0f, 2, "RareFlower", 1);
    addEntity(groups, 2, 0.3f, 2, "YellowFlower", 550);
    addEntity(groups, 2, 0.3f, 2, "OrangeFlower", 550);
    addEntity(groups, 2, 0.3f, 2, "WhiteFlower", 550);
    addEntity(groups, 2, 0.8f, 2, "Grass", 9500);

    int entitiesPerGroup[groupCount] = {group0, group1, group2};

    const clock_t begin_time = clock();

    const auto Entities = Poisson::GeneratePoisson(
            groups,
            3,
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

        if (i->val == "GiantMaple") {
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

        if (i->val == "Maple") {
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

        if (i->val == "Birch") {
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

        if (i->val == "Bush") {
            R = 120;
            G = 160;
            B = 90;
        }

        if (i->val == "OrangeFlower") {
            R = 255;
            G = 128;
            B = 45;
        }

        if (i->val == "YellowFlower") {
            R = 255;
            G = 255;
            B = 0;
        }

        if (i->val == "WhiteFlower") {
            R = 255;
            G = 255;
            B = 255;
        }

        if (i->val == "RareFlower") {
            R = 255;
            G = 0;
            B = 255;
        }

        if (i->val == "Grass") {
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
