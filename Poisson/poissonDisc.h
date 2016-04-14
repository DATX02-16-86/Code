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

#include <vector>
#include <random>
#include <stdint.h>
#include <time.h>
#include <algorithm>

namespace Poisson
{

    class DefaultPRNG
    {
    public:
        DefaultPRNG(int seed)
                : m_RD()
                , m_Gen( m_RD() )
                , m_Dis( 0.0f, 1.0f )
        {
            // prepare PRNG
            m_Gen.seed( seed );
        }

        float RandomFloat()
        {
            return static_cast<float>( m_Dis( m_Gen ) );
        }

        int RandomInt( int Max )
        {
            std::uniform_int_distribution<> DisInt( 0, Max );
            return DisInt( m_Gen );
        }

    private:
        std::random_device m_RD;
        std::mt19937 m_Gen;
        std::uniform_real_distribution<float> m_Dis;
    };

    struct sPoint
    {
        sPoint()
                : x( 0 )
                , y( 0 )
                , radius(0)
                , m_Valid( false )
        {}
        sPoint( float X, float Y, float r)
                : x( X )
                , y( Y )
                , radius(r)
                , m_Valid( true )
        {}
        float x;
        float y;
        float radius;
        bool m_Valid;
        //
        bool IsInRectangle() const
        {
            return x >= 0 && y >= 0 && x <= 1 && y <= 1;
        }
    };

    struct sGridPoint
    {
        sGridPoint( int X, int Y )
                : x( X )
                , y( Y )
        {}
        int x;
        int y;
    };

    struct Rules {

    };

    float GetDistance( const sPoint& P1, const sPoint& P2 );

    sGridPoint ImageToGrid( const sPoint& P, float CellSize );

    struct sGrid
    {
        sGrid( int W, int H, float CellSize )
                : m_W( W )
                , m_H( H )
                , m_CellSize( CellSize )
        {
            m_Grid.resize( m_H );

            for ( auto i = m_Grid.begin(); i != m_Grid.end(); i++ ) { i->resize( m_W ); }
        }
        void Insert( const sPoint& P )
        {
            sGridPoint G = ImageToGrid( P, m_CellSize );
            m_Grid[ G.x ][ G.y ].push_back(P);
        }
        void Remove( const sPoint& P)
        {
            sGridPoint G = ImageToGrid( P, m_CellSize );

            for (int i = 0; i < m_Grid[ G.x ][ G.y ].size(); ++i) {

                sPoint p = m_Grid[ G.x ][ G.y ][i];

                if (p.x == P.x && p.y == P.y ){
                    m_Grid[ G.x ][ G.y ][i] = m_Grid[ G.x ][ G.y ].back();
                    m_Grid[ G.x ][ G.y ].pop_back();
                    break;
                }
            }

        }
        bool IsInNeighbourhood( sPoint Point)
        {
            sGridPoint G = ImageToGrid( Point, m_CellSize);

            // number of adjacent cells to look for neighbour points
            const int D = 5;

            // scan the neighbourhood of the point in the grid
            for ( int i = G.x - D; i < G.x + D; i++ )
            {
                for ( int j = G.y - D; j < G.y + D; j++ )
                {
                    if ( i >= 0 && i < m_W && j >= 0 && j < m_H )
                    {
                        for (int k = 0; k < m_Grid[i][j].size(); ++k) {
                            sPoint P = m_Grid[i][j][k];

                            if ( P.m_Valid &&  GetDistance( P, Point ) < (Point.radius + P.radius) ) {
                                return true;
                            }
                        }

                    }
                }
            }

            return false;
        }

    private:
        int m_W;
        int m_H;
        float m_CellSize;

        std::vector< std::vector<std::vector<sPoint>> > m_Grid;
    };

    template <typename PRNG>
    sPoint PopRandom( std::vector<sPoint>& Points, PRNG& Generator );

    template <typename PRNG>
    sPoint GeneratePointAround( const sPoint& P, float pointRadius, PRNG& Generator );

    std::vector<sPoint> join(std::vector<std::vector<sPoint>> const& outer);

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
    );

    template <typename PRNG = DefaultPRNG>
    std::vector<sPoint> GenerateMultiPoissonPoints(
            int* radii,
            int radiiCount,
            int pointCount,
            PRNG& Generator,
            std::vector<float> mask,
            int maskSize,
            int attempts = 70
    );

    template <typename PRNG = DefaultPRNG>
    std::vector<sPoint> GenerateMultiPoissonPoints2(
            int* radii,
            int radiiCount,
            int pointCount,
            PRNG& Generator,
            std::vector<float> mask,
            int maskSize,
            int attempts = 70
    );

}
