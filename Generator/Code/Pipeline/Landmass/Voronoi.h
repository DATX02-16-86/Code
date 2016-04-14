
#ifndef GENERATOR_VORONOI_H
#define GENERATOR_VORONOI_H

#define BOOST_POLYGON_NO_DEPS
#define GRID_DIVISIONS 10

#include <boost/polygon/voronoi.hpp>

#include <Base.h>
#include <Types/Array.h>
#include <Types/Maybe.h>
#include <Math/Half.h>
#include <Math/Vector.h>

namespace generator {
namespace landmass {

using Tritium::Math::F16;
using Tritium::Float2;
using Tritium::Int2;
typedef Float2 Vertex;

static const U32 kMaxCellEdges = 9;
static const U32 kMaxVertexEdges = 4;

typedef F32 CoordinateType;
typedef boost::polygon::point_data<CoordinateType> Point;
typedef boost::polygon::voronoi_diagram<CoordinateType> Diagram;
typedef boost::polygon::voronoi_edge<CoordinateType> DiagramEdge;
typedef boost::polygon::voronoi_cell<CoordinateType> DiagramCell;

struct VertexIndex { U32 chunkIndex : 4; U32 index : 28; };
struct EdgeIndex { U32 chunkIndex : 4; U32 index : 28; };

struct Edge {
    VertexIndex a;
    VertexIndex b;
};

struct UnconnectedEdge {
    Vertex a;
    Vertex b;
    U32 position: 28;
    U8 connectToChunk : 4;
};

template<class T>
inline bool almostEqual(T answer, T value, T epsilon) {
    return value >= answer - epsilon && value <= answer + epsilon;
}

inline bool almostEqual(const Vertex &lhs, const Vertex &rhs) {
    return almostEqual(lhs.x, rhs.x, 0.00001f) && almostEqual(lhs.y, rhs.y, 0.00001f);
}

inline bool operator == (const VertexIndex &lhs, const VertexIndex &rhs) {
    return lhs.chunkIndex == rhs.chunkIndex && lhs.index == rhs.index;
}

inline bool operator == (const Edge &lhs, const Edge &rhs) {
    return (lhs.a == rhs.a && lhs.b == rhs.b) || (lhs.a == rhs.b && lhs.b == rhs.a);
}

inline bool operator < (Vertex lhs, Vertex rhs) {
    if(lhs.x < rhs.x) return true;
    else if(lhs.x > rhs.x) return false;
    else return lhs.y < rhs.y;
}

}} // namespace generator::landmass

namespace boost { namespace polygon { namespace detail {

template <>
struct ulp_comparison<F32> {
    enum Result {
        LESS = -1,
        EQUAL = 0,
        MORE = 1
    };

    Result operator() (F32 a, F32 b, U32 maxUlps) const {
        Tritium::Math::float_to_int a_int; a_int.f = a;
        Tritium::Math::float_to_int b_int; b_int.f = b;

        // Map negative zero to positive zero.
        if(a_int.i < 0x80000000U) a_int.i = 0x80000000U - a_int.i;
        if(b_int.i < 0x80000000U) b_int.i = 0x80000000U - b_int.i;

        if(a_int.i > b_int.i) {
            return (a_int.i - b_int.i <= maxUlps) ? EQUAL : LESS;
        } else {
            return (b_int.i - a_int.i <= maxUlps) ? EQUAL : MORE;
        }
    }
};

}}}

#endif // GENERATOR_VORONOI_H
