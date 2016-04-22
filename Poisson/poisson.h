
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
#include <map>
#include <mshtml.h>

namespace Poisson
{

    // RNG
    class PRNG
    {
    public:
        PRNG(unsigned int seed)
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

    // Possion entity
    struct Entity
    {

        Entity()
            : x(0)
            , y(0)
            , radius(0)
            , group(0)
            , id(0)
            , selfSpawn(0)
            , valid(false)
        {}

        Entity(float x, float y, float radius, float selfSpawnBias, int group, int id)
                : x(x)
                , y(y)
                , radius(radius)
                , selfSpawn(selfSpawnBias)
                , group(group)
                , id(id)
                , valid(true)
        {
        }

        int group;
        int id;

        float radius;
        float x;
        float y;

        float selfSpawn;

        bool valid;

        Entity copy(){
            return Entity(x, y, radius, selfSpawn, group, id);
        }

        bool IsInRectangle() const
        {
            return x >= 0 && y >= 0 && x <= 1 && y <= 1;
        }

    };

    struct EntityTemplate
    {
        EntityTemplate()
                : radius(0)
                , selfSpawn(0)
                , group(0)
                , id(0)
                , valid(false)
        {}

        EntityTemplate(float radius, float selfSpawnBias, int group, int id, int count = 0)
                : radius(radius)
                , selfSpawn(selfSpawnBias)
                , group(group)
                , id(id)
                , count(count)
                , valid(true)
        {
        }

        int id;
        int group;
        int count;

        float spawnSpanMin;
        float spawnSpanMax;

        float selfSpawn;
        float radius;

        bool valid;

        Entity toEntityAt(float x, float y)
        {
            return Entity(x, y, radius, selfSpawn, group, id);
        }
    };

    struct EntityCategorizer
    {
        EntityCategorizer(const unsigned int numberOfGroups)
            : count(numberOfGroups)
        {
            groups.resize(numberOfGroups);
            templatesPerGroup[numberOfGroups];
        }

        void AddEntityTemplate(int group, float radius, float selfSpawn, int id, int count)
        {
            EntityTemplate t(radius, selfSpawn, group, id, count);
            templatesPerGroup[group] += 1;
            groups[group].push_back(t);
        }

        EntityTemplate GetEntityTemplate(int group, int index){
            return groups[group][index];
        }

        EntityTemplate LookupEntityTemplate(int group, float f){

            // Quicksearch for new blueprint
            int tries = 0;
            int L = 0;
            int R = templatesPerGroup[group] - 1;

            EntityTemplate t;

            // TODO: ADD ERROR IF NOTHING IS FOUND
            while (tries < templatesPerGroup[group]) {
                int k = (L + R) / 2;
                t = groups[group][k];

                if( f < groups[group][k].spawnSpanMin){
                    R = k - 1;
                }
                else if( f > groups[group][k].spawnSpanMax) {
                    L = k + 1;
                }
                else {
                    break;
                }

                tries++;
            }

            return t;
        }

        int TemplatesInGroup(int group)
        {
            return templatesPerGroup[group];
        }

        void AddGroupDistance(const int groupA, const int groupB, float distance)
        {
            distanceMap[std::pair<int, int>(groupA, groupB)] = distance;
        }
        float GetGroupDistance(const int groupA, const int groupB)
        {
            return distanceMap[std::pair<int, int>(groupA, groupB)];
        }


        const unsigned int count;

    private:
        std::map<std::pair<int, int>, float> distanceMap;
        std::vector<std::vector<EntityTemplate>> groups;
        int templatesPerGroup[];

    };

    // Distance between entities
    float GetDistance( const Entity& P1, const Entity& P2 )
    {
        return sqrtf( ( P1.x - P2.x ) * ( P1.x - P2.x ) + ( P1.y - P2.y ) * ( P1.y - P2.y ) );
    }

    // Integer coordinate gridpoint
    struct GridPoint
    {
        GridPoint( int X, int Y )
                : x( X )
                , y( Y )
        {}
        int x;
        int y;
    };

    // Projection help function
    GridPoint ImageToGrid( const Entity& P, float CellSize )
    {
        return GridPoint( (int)( P.x / CellSize ), ( int )( P.y / CellSize ) );
    }

    // Collision grid
    struct Grid
    {
        Grid(int layers  ,int* W, int* H, float* CellSizes)
                : layers(layers)
                , m_W(W)
                , m_H(H)
                , m_CellSizes( CellSizes )
        {
            m_Grid.resize(layers);

            for (int i = 0 ; i < layers; i++) {
                m_Grid[i].resize(H[i]);

                for (int j = 0; j < H[i]; j++ ) {
                    m_Grid[i][j].resize(W[i]);
                }
            }
        }
        void Insert( const Entity& P )
        {
            GridPoint G = ImageToGrid(P, m_CellSizes[P.group]);
            m_Grid[P.group][ G.x ][ G.y ].push_back(P);
        }

        void Remove( const Entity& P)
        {
            GridPoint G = ImageToGrid( P, m_CellSizes[P.group] );

            for (int i = 0; i < m_Grid[P.group][ G.x ][ G.y ].size(); ++i) {

                Entity p = m_Grid[P.group][ G.x ][ G.y ][i];

                if (p.x == P.x && p.y == P.y ){
                    m_Grid[P.group][ G.x ][ G.y ][i] = m_Grid[P.group][ G.x ][ G.y ].back();
                    m_Grid[P.group][ G.x ][ G.y ].pop_back();
                    break;
                }
            }
        }

        bool IsInNeighbourhood(Entity newEntity, float extra)
        {
            // number of adjacent cells to look for neighbour points
            const int D = 5;

            for (int layer = 0; layer <= newEntity.group; ++layer) {

                // scan the neighbourhood of the point in the grid
                GridPoint G = ImageToGrid(newEntity, m_CellSizes[layer]);

                for (int i = G.x - D; i < G.x + D; i++) {
                    for (int j = G.y - D; j < G.y + D; j++) {
                        if (i >= 0 && i < m_W[layer] && j >= 0 && j < m_H[layer]) {
                            for (int k = 0; k < m_Grid[layer][i][j].size(); ++k) {
                                Entity P = m_Grid[layer][i][j][k];

                                if (P.valid ) {
                                    float minDist = (newEntity.radius + P.radius + extra);

                                    if (GetDistance(P, newEntity) < minDist)
                                        return true;
                                }
                            }
                        }
                    }
                }
            }

            return false;
        }

    private:
        int* m_W;
        int* m_H;
        int layers;
        float* m_CellSizes;

        std::vector<std::vector<std::vector<std::vector<Entity>>>> m_Grid;
    };

    Entity PopRandom( std::vector<Entity>& Entities, PRNG& Generator ) {

        const int Idx = Generator.RandomInt( Entities.size()-1 );
        const Entity P = Entities[ Idx ];
        Entities[Idx] = Entities.back();
        Entities.pop_back();
        return P;
    }

    Entity GetRandom(std::vector<Entity>& Entities, PRNG& Generator){
        const int Idx = Generator.RandomInt( Entities.size()-1 );
        const Entity P = Entities[ Idx ];
        return P;
    }

    Entity GeneratePointAround(const Entity& oldEntity, EntityTemplate& newEntity, PRNG& Generator, float extra)
    {
        // start with non-uniform distribution
        float R1 = Generator.RandomFloat();
        float R2 = Generator.RandomFloat();

        // radius should be between min dist of entities plus their innate distance rule
        float Radius = (oldEntity.radius + newEntity.radius +  extra) * ( R1 + 1.0f );

        // random angle
        float Angle = 2 * 3.141592653589f * R2;

        // the new point is generated around the point (x, y)
        float X = oldEntity.x + Radius * cosf( Angle );
        float Y = oldEntity.y + Radius * sinf( Angle );

        return newEntity.toEntityAt(X, Y);
    }

    // help function for adding two vectors
    std::vector<Entity> join(std::vector<std::vector<Entity>> const& outer)
    {
        std::vector<Entity> joined;
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

    std::vector<Entity> GeneratePoisson(
            EntityCategorizer groups,
            PRNG& Generator,
            std::vector<float> mask,
            int maskSize,
            bool prune = true,
            int attempts = 4
    )
    {
        // TODO: ADD CHECKS SO PEOPLE CANT FUCK SHIT UP
        // TODO: Make ints right type, like unsigned n shit

        int numberOfGroups = groups.count;

        // Support empty groups
        float maxRadius = 0;
        int start = 0;
        for (int i = 0; i < numberOfGroups; ++i) {
            if(groups.TemplatesInGroup(i) != 0)
                break;
            start++;
        }

        // Find biggest entity per entity group
        float cellSizes[numberOfGroups];
        int H[numberOfGroups];
        int W[numberOfGroups];

        for (int i = 0; i < numberOfGroups; ++i) {
            maxRadius = 0;

            for (int j = 0; j < groups.TemplatesInGroup(i); ++j) {
                if (maxRadius < groups.GetEntityTemplate(i,j).radius){
                    maxRadius = groups.GetEntityTemplate(i,j).radius;
                }
            }

            if(maxRadius != 0){
                maxRadius += 2 * groups.GetGroupDistance(i, i);

            }
            else{ //  UGLY FIX SHOULD BE ERROR
                maxRadius = 0.5f;
            }

            cellSizes[i] = maxRadius / sqrtf(2.0f);
            H[i] = (int)ceil( sqrtf(2.0f) / maxRadius);
            W[i] = (int)ceil( sqrtf(2.0f) / maxRadius );

        }

        // Setup grid using biggest entity (max 1 per grid of biggest)
        // Scales really bad with large size-differences
        Grid grid(groups.count, W, H, cellSizes);

        // Entity vector per group
        std::vector<std::vector<Entity>> output(numberOfGroups);

        //Entity vector to spawn new entities from
        std::vector<Entity> processList;

        // Loop over and spawn entities from each group
        for (int i = start; i < numberOfGroups; ++i) {

            // Calculate total number of entities in group
            // Adjust for selfspawn chance
            int totalEntities = 0;
            for (int j = 0; j < groups.TemplatesInGroup(i); ++j) {
                groups.GetEntityTemplate(i, j).count = (int)((1 - groups.GetEntityTemplate(i, j).selfSpawn) * groups.GetEntityTemplate(i, j).count);
                totalEntities += groups.GetEntityTemplate(i, j).count;
            }

            // Set up selection span and adjust count for self spawn
            int countSum = 0;
            for (int j = 0; j < groups.TemplatesInGroup(i); ++j) {

                float min = 0;

                if (j > 0){
                    min = groups.GetEntityTemplate(i, j - 1).spawnSpanMax;
                    groups.GetEntityTemplate(i, j).spawnSpanMax = (float)groups.GetEntityTemplate(i, j).count / totalEntities + min;
                }

                groups.GetEntityTemplate(i, j).spawnSpanMin = min;
                groups.GetEntityTemplate(i, j).spawnSpanMax = (float)groups.GetEntityTemplate(i, j).count / totalEntities + min;

                countSum += groups.GetEntityTemplate(i, j).count;
            }

            // Current groups entities
            std::vector<Entity> entities;

            // Get first entity from first group
            EntityTemplate blueprint = groups.GetEntityTemplate(i, 0);
            Entity oldEntity;

            if (i == start){ // If spawning very first entity, choose random point

                do {
                    oldEntity = blueprint.toEntityAt(Generator.RandomFloat(), Generator.RandomFloat());
                } while (!(oldEntity.IsInRectangle())); // Make sure coordinates are in unit square
            }
            else { // Pick random point from last process

                for (int j = 0; j < attempts * 3; ++j) {

                    oldEntity = GeneratePointAround(GetRandom(output[i - 1], Generator), blueprint, Generator, groups.GetGroupDistance(blueprint.group, oldEntity.group));

                    if(oldEntity.IsInRectangle())
                        break;
                }
            }

            // Update containers with first entity
            processList.push_back(oldEntity);
            entities.push_back(oldEntity);
            grid.Insert(oldEntity);

            // Spawn new entities until map is full
            while ( !processList.empty()){

                // Get last placed entity
                oldEntity = processList.back();
                processList.pop_back();

                // Attempt to spawn it somewhere around last
                for (int j = 0; j < attempts; ++j) {

                    // Get next entity type
                    float f = Generator.RandomFloat();

                    if ( f < oldEntity.selfSpawn){ // Spawn new of same type
                        blueprint = EntityTemplate(oldEntity.radius, oldEntity.selfSpawn, oldEntity.group, oldEntity.id);
                    }
                    else {
                        // GET SPAWN-FROM-POS
                        int X = (int)(maskSize * oldEntity.x);
                        int Y = (int)(maskSize * oldEntity.y);

                        // MASK OR RANDOM
                        f = mask[X + Y * maskSize];

                        blueprint = groups.LookupEntityTemplate(i, f);
                    }

                    Entity newEntity = GeneratePointAround(oldEntity, blueprint, Generator, groups.GetGroupDistance(blueprint.group, oldEntity.group));

                    if( newEntity.IsInRectangle() && !grid.IsInNeighbourhood(newEntity, groups.GetGroupDistance(newEntity.group, oldEntity.group))){
                        processList.push_back(newEntity);
                        entities.push_back(newEntity);
                        grid.Insert(newEntity);
                        continue;
                    }
                }
            }

            // Prune based on mask
            if(prune) {
                for (auto s = entities.begin(); s != entities.end(); s++) {

                    int x = int(s->x * maskSize);
                    int y = int(s->y * maskSize);

                    // dice
                    float R = Generator.RandomFloat();
                    float P = mask[x + y * maskSize];

                    if (P > 0.9f || R > P) {
                        grid.Remove(*s);
                        *s = entities.back();
                        entities.pop_back();
                        s--;
                    }
                }
            }

            output[i] = entities;
        }

        return join(output);

    }

}