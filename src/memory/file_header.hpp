//
// Created by roggired on 09.05.23.
//

#ifndef JACKALOPE_STORAGE_FILE_DESCRIPTOR_HPP
#define JACKALOPE_STORAGE_FILE_DESCRIPTOR_HPP

#include <cstdint>

namespace memory {
    #pragma pack(push, 1)

    #define FILE_HEADER_PAGE_8_TYPE 1
    #define FILE_HEADER_RESERVED    7
    struct FileHeader {
        int32_t fileNumber;
        int32_t pagesNumber;
        int8_t type;
        int8_t reserved[FILE_HEADER_RESERVED];
    };
    typedef FileHeader FileHeader;

    #pragma pack(pop)
}
#endif //JACKALOPE_STORAGE_FILE_DESCRIPTOR_HPP
