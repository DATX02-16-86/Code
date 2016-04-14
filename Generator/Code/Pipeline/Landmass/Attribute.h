
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
    AttributeMap(AttributeId* attributes, Size attributeCount, Size cellCount, Size edgeCount, Size vertexCount);
    ~AttributeMap();

    void create(AttributeId* attributes, Size attributeCount, Size cellCount, Size edgeCount, Size vertexCount);

    U32 get(AttributeId id, U32 index);
    void setRaw(AttributeId id, U32 index, U32 value);

    template<class T> void set(AttributeId id, U32 index, T value) {
        union { U32 i; T v; } convert;
        convert.v = value;
        setRaw(id, index, convert.i);
    }

    U32* offsets = nullptr;
    U32* data = nullptr;
};

}}