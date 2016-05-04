
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
    AttributeId(U32 id, U8 itemBits, AttributeType type);

    const U32 id;
    const U32 mask;
	const U8 itemBits;
	const U8 type;
    const U8 itemsPerWord;
    const U8 itemShift;
};

struct AttributeMap {
    AttributeMap() {}
    AttributeMap(AttributeId* attributes, Size attributeCount, Size cellCount, Size edgeCount, Size vertexCount);
    ~AttributeMap();

    void create(AttributeId* attributes, Size attributeCount, Size cellCount, Size edgeCount, Size vertexCount);

    U32 getRaw(AttributeId id, U32 index);
    void setRaw(AttributeId id, U32 index, U32 value);

    template<class T> void set(AttributeId id, U32 index, T value) {
        union { U32 i; T v; } convert;
        convert.v = value;
        setRaw(id, index, convert.i);
    }

    template<class T> T get(AttributeId id, U32 index) {
        union { U32 i; T v; } convert;
        convert.i = getRaw(id, index);
        return convert.v;
    }

    U32* offsets = nullptr;
    U32* data = nullptr;
};

}}