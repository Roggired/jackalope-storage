//
// Created by ego on 07.05.23.
//

#ifndef JACKALOPE_STORAGE_PAGE_POINTER_HPP
#define JACKALOPE_STORAGE_PAGE_POINTER_HPP

#include <cstdint>

namespace memory {
    #pragma pack(push, 1)

    #define POINTER_STATUS_UNUSED 0
    #define POINTER_STATUS_USED   1
    #define POINTER_XMIN_COMMITTED_MASK 0x01 // 0000 0001
    #define POINTER_XMIN_ABORTED_MASK   0x02 // 0000 0010
    #define POINTER_XMAX_COMMITTED_MASK 0x04 // 0000 0100
    #define POINTER_XMAX_ABORTED_MASK   0x08 // 0000 1000
    struct PagePointer {
        int8_t status;
        int8_t flags;
        int16_t rowPositionOffset;
        uint32_t xmin;
        uint32_t xmax;
        int16_t size;
        int16_t number;

        [[maybe_unused]]
        void setXminCommited() {
            flags |= POINTER_XMIN_COMMITTED_MASK;
        }

        [[maybe_unused]]
        void setXminAborted() {
            flags |= POINTER_XMIN_ABORTED_MASK;
        }

        [[maybe_unused]]
        void setXmaxCommited() {
            flags |= POINTER_XMAX_COMMITTED_MASK;
        }

        [[maybe_unused]]
        void setXmaxAborted() {
            flags |= POINTER_XMAX_ABORTED_MASK;
        }

        [[maybe_unused]]
        [[nodiscard]]
        bool isXminCommitted() const {
            return (flags & POINTER_XMIN_COMMITTED_MASK) == POINTER_XMIN_COMMITTED_MASK;
        }

        [[maybe_unused]]
        [[nodiscard]]
        bool isXminAborted() const {
            return (flags & POINTER_XMIN_ABORTED_MASK) == POINTER_XMIN_ABORTED_MASK;
        }

        [[maybe_unused]]
        [[nodiscard]]
        bool isXmaxCommitted() const {
            return (flags & POINTER_XMAX_COMMITTED_MASK) == POINTER_XMAX_COMMITTED_MASK;
        }

        [[maybe_unused]]
        [[nodiscard]]
        bool isXmaxAborted() const {
            return (flags & POINTER_XMAX_ABORTED_MASK) == POINTER_XMAX_ABORTED_MASK;
        }

        [[maybe_unused]]
        [[nodiscard]]
        bool isStatusUsed() const {
            return status == POINTER_STATUS_USED;
        }

        [[maybe_unused]]
        [[nodiscard]]
        bool isStatusUnused() const {
            return status == POINTER_STATUS_UNUSED;
        }
    };
    typedef struct PagePointer PagePointer;

    [[maybe_unused]]
    [[nodiscard]]
    PagePointer unusedPagePointer();

    #pragma pack(pop)
}
#endif //JACKALOPE_STORAGE_PAGE_POINTER_HPP
