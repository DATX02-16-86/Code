
#ifndef GENERATOR_VOXEL_H
#define GENERATOR_VOXEL_H

#include <Base.h>

namespace generator {

/// Defines the location of a subset of the world.
struct Area {
    I32 x; /// The x-position of the chunk left-upper corner.
    I32 y; /// The y-position of the chunk left-upper corner.
    U16 z; /// The z-position of the chunk left-lower corner.
    U16 width; /// The size over the x-axis in local units.
    U16 height; /// The size over the y-axis in local units.
    U16 depth: 13; /// The size over the z-axis in local units, starting from 0.
    U8 lod: 3; /// The lod index of this chunk, as a power of two.

    /// Returns the chunk width in world units.
    Size worldWidth() const {return width << lod;}

    /// Returns the chunk height in world units.
    Size worldHeight() const {return height << lod;}

    /// Returns the chunk depth in world units.
    Size worldDepth() const {return depth << lod;}
};

/// Represents the data for a single voxel.
struct Voxel {
    Voxel(Size blockType, Size metadata = 0, Size baseLight = 0, Size skyLight = 15):
        blockType((U16)blockType), metadata((U8)metadata), baseLight((U8)baseLight), skyLight((U8)skyLight) {}

    U16 blockType; /// Material type.
    U8 metadata; /// Material metadata.
    U8 baseLight : 4; /// The base light level that reaches this voxel.
    U8 skyLight : 4; /// The light level originating from the sky that reaches this voxel.
};

/// Represents a chunk of generated voxel data.
struct Chunk {
    /// A chunk ID value that can be used by clients to identify this chunk.
    Size id = 0;

    /// Creates a chunk of the provided size and initializes the voxels to air.
    Chunk(Area area);
    ~Chunk();

    /// Returns the voxel at the provided local position.
    Voxel at(Size x, Size y, Size z) const;

    /// Returns a modifiable voxel at the provided local position.
    Voxel& at(Size x, Size y, Size z);

    /// Sets voxel data at the provided local position.
    void set(Size x, Size y, Size z, Voxel voxel);

    /// The area this chunk consists of.
    const Area area;

    /// Updates the heightmap of terrain height in the chunk.
    void updateHeightMap();

    /// Calls the provided builder for each voxel in this area. The mapper should return a voxel for that location.
    template<class F> void build(F&& f) {
        auto step = Size(1) << area.lod;
        auto x = area.x * area.width;
        auto y = area.y * area.height;
        auto z = area.z * area.depth;
        Size height = area.height;
        Size width = area.width;
        Size depth = area.depth;

        for(Size row = 0; row < height; row++) {
            for(Size column = 0; column < width; column++) {
                for(Size zi = 0; zi < depth; zi++) {
                    Voxel& voxel = at(column, row, zi);
                    voxel = f(voxel, x + column * step, y + row * step, z + zi * step);
                }
            }
        }
    }

private:

    /**
     * The voxel data for this chunk, laid out as a 3D texture.
     * Rows follow the x-axis,
     * Slices follow the y-axis.
     */
    Voxel* voxels;

    /**
     * The highest z-value that is filled for each terrain pillar.
     * This contains fractional values to allow using it for heightmap generation (when not using voxels).
     */
    U16* heightMap;
};

} // namespace generator

#endif //GENERATOR_VOXEL_H
