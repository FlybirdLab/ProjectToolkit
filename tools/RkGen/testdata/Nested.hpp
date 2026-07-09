#pragma rkgen
#pragma once

namespace rkgen_test::nested {

cstruct InventoryItem
{
    int itemId;
    cfield(GetCount, SetCount) int count;
    const char* debugName;

    int ignoredFunction(int value);
};

cclass Inventory
{
public:
    cfield() InventoryItem* selected;
    cfunc void Select(const InventoryItem& item);
    cfunc const InventoryItem& GetSelected() const;
};

}
