#include <Math/Math.h>
#include <Mem/Memory.h>
#include "Attribute.h"

namespace generator {
namespace landmass {

AttributeId::AttributeId(U16 id, U8 itemBits, AttributeType type):
    id(id), itemBits(itemBits), type((U8)type),
    itemShift((U8)Tritium::Math::findLastBit(sizeof(U32) * 8 / itemBits)),
    itemsPerWord((U8)1 << Tritium::Math::findLastBit(sizeof(U32) * 8 / itemBits)) {}

AttributeMap::AttributeMap(AttributeId* attributes, Size attributeCount, Size cellCount, Size edgeCount, Size vertexCount) {
    create(attributes, attributeCount, cellCount, edgeCount, vertexCount);
}

AttributeMap::~AttributeMap() {
    Tritium::hFree(offsets);
}

void AttributeMap::create(AttributeId* attributes, Size attributeCount, Size cellCount, Size edgeCount, Size vertexCount) {
    U32 totalSize = sizeof(void*) * attributeCount;

    // First we calculate the total allocation size needed.
    for(Size i = 0; i < attributeCount; i++) {
        Size count;
        if(attributes[i].type == (U8)AttributeType::Cell) count = cellCount;
        else if(attributes[i].type == (U8)AttributeType::Edge) count = edgeCount;
        else count = vertexCount;

        auto words = (U32)(count >> attributes[i].itemShift);
        totalSize += (words * sizeof(U32));
    }

    Tritium::hFree(offsets);
    auto p = Tritium::hAlloc(totalSize);
    Tritium::setMem(p, totalSize, 0);

    this->offsets = (U32*)p;
    this->data = (U32*)p + attributeCount;

    // Then, we calculate the offset for each attribute.
    U32 offset = 0;
    for(Size i = 0; i < attributeCount; i++) {
        Size count;
        if(attributes[i].type == (U8)AttributeType::Cell) count = cellCount;
        else if(attributes[i].type == (U8)AttributeType::Edge) count = edgeCount;
        else count = vertexCount;

        auto words = (U32)(count >> attributes[i].itemShift);
        this->offsets[i] = offset;
        offset += words;
    }
}

U32 AttributeMap::getRaw(AttributeId id, U32 index) {
    auto mask = (Size(1) << id.itemBits) - 1;
    auto offset = index >> id.itemShift;
    auto maskOffset = index & (id.itemsPerWord - 1);

    return (data + offsets[id.id])[offset] >> (maskOffset * id.itemBits) & mask;
}

void AttributeMap::setRaw(AttributeId id, U32 index, U32 value) {
    auto offset = index >> id.itemShift;
    auto maskOffset = offset & (id.itemsPerWord - 1);
    auto totalShift = maskOffset * id.itemBits;
    auto mask = ((Size(1) << id.itemBits) - 1) << totalShift;

    auto base = offsets[id.id];
    auto c = (data + base)[offset];
    c &= ~mask;
    c |= (value << totalShift);
    (data + base)[offset] = c;
}

}}