//
// Created by ego on 07.05.23.
//

#include "page_pointer.hpp"

using namespace memory;


PagePointer memory::unusedPagePointer() {
    return {
            POINTER_STATUS_UNUSED,
            0,
            0,
            0,
            0,
            0,
            0
    };
}

[[maybe_unused]]
[[nodiscard]]
bool PagePointer::isVisibleToTransaction(uint32_t xid) const {
    if (xmin > xid || (xmin < xid && !isXminCommitted())) {
        return false;
    }

    if (xmax == xid || (xmax > 0 && xmax < xid && isXmaxCommitted())) {
        return false;
    }

    return true;
}