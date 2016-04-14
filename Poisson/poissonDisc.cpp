

/**
 * \file Poisson.cpp
 * \brief
 *
 * Poisson Disk Points Generator
 *
 * \version 1.1.3
 * \date 10/03/2016
 * \author Sergey Kosarevsky, 2014-2016
 * \author support@linderdaum.com   http://www.linderdaum.com   http://blog.linderdaum.com
 */

/*
	Usage example:

		PoissonGenerator::DefaultPRNG PRNG;
		const auto Points = PoissonGenerator::GeneratePoissonPoints( pointCount, PRNG );
*/

// Fast Poisson Disk Sampling in Arbitrary Dimensions
// http://people.cs.ubc.ca/~rbridson/docs/bridson-siggraph07-poissondisk.pdf

// Implementation based on http://devmag.org.za/2009/05/03/poisson-disk-sampling/

/* Versions history:
 *		1.1.3		Mar 10, 2016		Header-only library, no global mutable state
 *		1.1.2		Apr  9, 2015		Output a text file with XY coordinates
 *		1.1.1		May 23, 2014		Initialize PRNG seed, fixed uninitialized fields
 *    1.1		May  7, 2014		Support of density maps
 *		1.0		May  6, 2014
*/

#include "poissonDisc.h"

namespace Poisson
{
    float GetDistance( const sPoint& P1, const sPoint& P2 )
    {
        return sqrtf( ( P1.x - P2.x ) * ( P1.x - P2.x ) + ( P1.y - P2.y ) * ( P1.y - P2.y ) );
    }

    sGridPoint ImageToGrid( const sPoint& P, float CellSize )
    {
        return sGridPoint( ( int )( P.x / CellSize ), ( int )( P.y / CellSize ) );
    }

    template <typename PRNG>
    sPoint PopRandom( std::vector<sPoint>& Points, PRNG& Generator )
    {
        const int Idx = Generator.RandomInt( Points.size()-1 );
        const sPoint P = Points[ Idx ];
        Points.erase( Points.begin() + Idx );
        return P;
    }

    template <typename PRNG>
    sPoint GeneratePointAround( const sPoint& P, float pointRadius, PRNG& Generator )
    {
        // start with non-uniform distribution
        float R1 = Generator.RandomFloat();
        float R2 = Generator.RandomFloat();

        // radius should be between MinDist and 2 * MinDist
        float Radius = (P.radius + pointRadius) * ( R1 + 1.0f );

        // random angle
        float Angle = 2 * 3.141592653589f * R2;

        // the new point is generated around the point (x, y)
        float X = P.x + Radius * cosf( Angle );
        float Y = P.y + Radius * sinf( Angle );

        return sPoint( X, Y, pointRadius);
    }

    std::vector<sPoint> join(std::vector<std::vector<sPoint>> const& outer)
    {
        std::vector<sPoint> joined;
        size_t total_size{ 0 };
        for (auto const& items: outer){
            total_size += items.size();
        }

        joined.reserve(total_size);
        for (auto& items: outer){
            std::move(items.begin(), items.end(), std::back_inserter(joined));
        }

        return joined;
    }

/**
	Return a vector of generated points

	NewPointsCount - refer to bridson-siggraph07-poissondisk.pdf for details (the value 'k')
	Circle  - 'true' to fill a circle, 'false' to fill a rectangle
	MinDist - minimal distance estimator, use negative value for default
**/
    template <typename PRNG = DefaultPRNG>
    std::vector<sPoint> GeneratePoissonPoints(
            int radius,
            int pointCount,
            PRNG& Generator,
            std::vector<float> mask,
            int maskSize,
            int attempts = 70
    )
    {
        std::vector<sPoint> ProcessList;
        std::vector<sPoint> SamplePoints;
        // create the grid
        float minDistance = (float)(radius) / maskSize;

        float CellSize = minDistance / sqrtf( 2.0f );

        int GridW = ( int )ceil( 1.0f / CellSize );
        int GridH = ( int )ceil( 1.0f / CellSize );

        sGrid Grid(GridW, GridH, CellSize);

        sPoint FirstPoint;

        do {
            FirstPoint = sPoint( Generator.RandomFloat(), Generator.RandomFloat(), minDistance);
        } while (!(FirstPoint.IsInRectangle() && !Grid.IsInNeighbourhood(FirstPoint)));

        // update containers
        ProcessList.push_back( FirstPoint );
        SamplePoints.push_back( FirstPoint );
        Grid.Insert( FirstPoint );

        // generate new points for each point in the queue
        while ( !ProcessList.empty() && SamplePoints.size() < pointCount )
        {
            sPoint Point = PopRandom<PRNG>( ProcessList, Generator );

            for ( int i = 0; i < attempts; i++ )
            {
                sPoint NewPoint = GeneratePointAround( Point, minDistance, Generator );

                if (NewPoint.IsInRectangle() && !Grid.IsInNeighbourhood( NewPoint ) )
                {
                    ProcessList.push_back( NewPoint );
                    SamplePoints.push_back( NewPoint );
                    Grid.Insert( NewPoint );
                    continue;
                }
            }
        }

        float bias = 0.0f;

        for ( auto i = SamplePoints.begin(); i != SamplePoints.end(); i++ )
        {
            int x = int( i->x * maskSize );
            int y = int( i->y * maskSize );

            // dice
            float R = Generator.RandomFloat() + bias;
            float P = mask[ x + y * maskSize ];

            if ( R > P ) {

                *i = SamplePoints.back();
                SamplePoints.pop_back();
                i--;
            }
        }

        return SamplePoints;
    }

    template <typename PRNG = DefaultPRNG>
    std::vector<sPoint> GenerateMultiPoissonPoints(
            int* radii,
            int radiiCount,
            int pointCount,
            PRNG& Generator,
            std::vector<float> mask,
            int maskSize,
            int attempts = 70
    )
    {
        std::vector<sPoint> ProcessList;
        std::vector<sPoint> SamplePoints;

        int radius = 0;

        for (int i = 0; i < radiiCount; ++i) {
            if (radius < radii[i])
                radius = radii[i];
        }

        // create the grid
        float minDistance = (float)(radius) / maskSize;

        float CellSize = minDistance / sqrtf( 2.0f );
        int GridW = ( int )ceil( 1.0f / CellSize );
        int GridH = ( int )ceil( 1.0f / CellSize );

        sGrid Grid(GridW, GridH,CellSize);


        sPoint FirstPoint;

        do {
            minDistance = (float)(radii[Generator.RandomInt(radiiCount - 1)]) / maskSize;
            FirstPoint = sPoint( Generator.RandomFloat(), Generator.RandomFloat(),minDistance);
        } while (!(FirstPoint.IsInRectangle()));

        // update containers
        ProcessList.push_back( FirstPoint );
        SamplePoints.push_back( FirstPoint );
        Grid.Insert( FirstPoint );

        // generate new points for each point in the queue
        while ( !ProcessList.empty() && SamplePoints.size() < pointCount )
        {
            sPoint Point = PopRandom<PRNG>( ProcessList, Generator );

            for ( int i = 0; i < attempts; i++ )
            {
                minDistance = (float)(radii[Generator.RandomInt(radiiCount - 1)]) / maskSize;

                sPoint NewPoint = GeneratePointAround( Point, minDistance, Generator);

                if ( NewPoint.IsInRectangle() && !Grid.IsInNeighbourhood( NewPoint ) )
                {
                    ProcessList.push_back( NewPoint );
                    SamplePoints.push_back( NewPoint );
                    Grid.Insert( NewPoint );
                    continue;
                }

            }
        }

        float bias = 0.0f;

        for ( auto i = SamplePoints.begin(); i != SamplePoints.end(); i++ )
        {
            int x = int( i->x * maskSize );
            int y = int( i->y * maskSize );

            // dice
            float R = Generator.RandomFloat() + bias;
            float P = mask[ x + y * maskSize ];

            if ( R > P ) {
                SamplePoints.erase(i);
                i--;
            }
        }

        return SamplePoints;
    }

    template <typename PRNG = DefaultPRNG>
    std::vector<sPoint> GenerateMultiPoissonPoints2(
            int* radii,
            int radiiCount,
            int pointCount,
            PRNG& Generator,
            std::vector<float> mask,
            int maskSize,
            int attempts = 70
    )
    {
        std::vector<sPoint> ProcessList;
        std::vector<std::vector<sPoint>> SamplePoints(radiiCount);

        int radius = 0;

        for (int i = 0; i < radiiCount; ++i) {
            if (radius < radii[i])
                radius = radii[i];
        }

        // create the grid
        float minDistance = (float)(radius) / maskSize;

        sGrid Grid((int)ceil( sqrtf(2.0f) / minDistance ),(int)ceil( sqrtf(2.0f) / minDistance) , minDistance / sqrtf(2.0f));

        sPoint FirstPoint;

        int vIdx = Generator.RandomInt(radiiCount - 1);

        do {
            minDistance = (float)(radii[vIdx]) / maskSize;
            FirstPoint = sPoint( Generator.RandomFloat(), Generator.RandomFloat(),minDistance);
        } while (!(FirstPoint.IsInRectangle()));


        // update containers
        ProcessList.push_back( FirstPoint );
        SamplePoints[0].push_back( FirstPoint );
        Grid.Insert( FirstPoint );

        // generate new points for each point in the queue
        while ( !ProcessList.empty() && SamplePoints[vIdx].size() < pointCount )
        {
            sPoint Point = PopRandom<PRNG>( ProcessList, Generator );

            for ( int i = 0; i < attempts; i++ )
            {
                vIdx = Generator.RandomInt(radiiCount - 1);
                minDistance = (float)(radii[vIdx]) / maskSize;

                sPoint NewPoint = GeneratePointAround( Point, minDistance, Generator);

                if ( NewPoint.IsInRectangle() && !Grid.IsInNeighbourhood( NewPoint ) )
                {
                    ProcessList.push_back( NewPoint );
                    SamplePoints[vIdx].push_back( NewPoint );
                    Grid.Insert( NewPoint );
                    continue;
                }
            }
        }

        float bias = 0.0f;

        ProcessList.clear();

        for (int v = 0; v < radiiCount; ++v) {
            for ( auto i = SamplePoints[v].begin(); i != SamplePoints[v].end(); i++ )
            {
                int x = int( i->x * maskSize );
                int y = int( i->y * maskSize );

                // dice
                float R = Generator.RandomFloat() + bias;
                float P = mask[ x + y * maskSize ];

                if ( R > P ) {
                    sPoint p (i->x, i->y, i->radius);
                    *i = SamplePoints[v].back();
                    SamplePoints[v].pop_back();
                    i--;

                    if (v < radiiCount - 1 ) {
                        Grid.Remove(p);

                        minDistance = (float)(radii[v + 1]) / maskSize * 3;
                        sPoint P(p.x, p.y, minDistance);

                        // update containers
                        ProcessList.push_back(P);

                        int spawned = 0;
                        // generate new points for each point in the queue
                        while (!ProcessList.empty() && spawned < 20) {
                            sPoint Point = PopRandom<PRNG>(ProcessList, Generator);

                            int fill = (int)((float)radii[v] * radii[v] /( radii[v + 1] * radii[v + 1]));

                            for (int j = 0; j < fill; j++) {
                                minDistance = (float) (radii[v + 1]) / maskSize;

                                sPoint NewPoint = GeneratePointAround(Point, minDistance, Generator);

                                if (NewPoint.IsInRectangle() && !Grid.IsInNeighbourhood(NewPoint)) {
                                    ProcessList.push_back(NewPoint);
                                    SamplePoints[v + 1].push_back(NewPoint);
                                    Grid.Insert(NewPoint);
                                    spawned++;
                                    continue;
                                }
                            }
                        }
                    }
                }
            }
        }


        return join(SamplePoints);
    }



} // namespace PoissonGenerator
