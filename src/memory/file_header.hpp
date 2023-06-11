//
// Created by ego on 09.05.23.
//

#ifndef JACKALOPE_STORAGE_FILE_DESCRIPTOR_HPP
#define JACKALOPE_STORAGE_FILE_DESCRIPTOR_HPP

#include <cstdint>

namespace memory {
    #pragma pack(push, 1)

    #define FILE_HEADER_PAGE_8_TYPE 1
    #define FILE_HEADER_RESERVED    1
    struct FileHeader {
        uint32_t fileNumber;
        uint8_t type;
        uint8_t reserved[FILE_HEADER_RESERVED];
        uint16_t pagesNumber;
    };
    typedef FileHeader FileHeader;

    #pragma pack(pop)
}
#endif //JACKALOPE_STORAGE_FILE_DESCRIPTOR_HPP
