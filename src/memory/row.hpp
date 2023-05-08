//
// Created by ego on 07.05.23.
//

#ifndef JACKALOPE_STORAGE_ROW_HPP
#define JACKALOPE_STORAGE_ROW_HPP

#include <cstdint>

namespace memory {
    #pragma pack(push, 1)

    struct RowHeader {
        int16_t payloadSize;
        int32_t extIdFile;
        int16_t extIdPointer;
    };

    #pragma pack(pop)

    struct Row {
        RowHeader* rowHeader;
        void* payload;

        [[maybe_unused]]
        [[nodiscard]]
        int16_t sizeWithHeader() const {
            return (int16_t) (sizeof(RowHeader) + rowHeader->payloadSize);
        }
    };
    typedef Row Row;
}

#endif //JACKALOPE_STORAGE_ROW_HPP
