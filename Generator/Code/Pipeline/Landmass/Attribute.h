
#pragma once

#include <Base.h>

namespace generator {
namespace landmass {

struct Chunk;

enum class AttributeType {
    Cell,
    Edge,
    Vertex
};

struct Attribute {
    const U8 itemBits;
    const AttributeType type;
};

struct AttributeId {
    AttributeId(U16 id, U8 itemBits, AttributeType type);

    const U16 id;
    const U8 itemsPerWord: 4;
    const U8 itemShift: 4;
    const U8 itemBits: 6;
    const AttributeType type: 2;
};

struct AttributeMap {
    AttributeMap() {}
    AttributeMap(AttributeId* attributes, U32 attributeCount, U32 cellCount, U32 edgeCount, U32 vertexCount);
    ~AttributeMap();

    void create(AttributeId* attributes, U32 attributeCount, U32 cellCount, U32 edgeCount, U32 vertexCount);

    U32 get(AttributeId id, U32 index);
    void set(AttributeId id, U32 index, U32 value);

    U32* offsets = nullptr;
    U32* data = nullptr;
};

}}