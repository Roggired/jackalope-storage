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