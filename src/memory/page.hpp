//
// Created by roggired on 07.05.23.
//

#ifndef JACKALOPE_STORAGE_PAGE_HPP
#define JACKALOPE_STORAGE_PAGE_HPP

#include <cstdint>
#include "model/semnet_models.hpp"
#include "page_pointer.hpp"
#include "row.hpp"

namespace memory {
    #pragma pack(push, 1)

    enum PageGetStatus {
        PGS_OK = 0,
        PGS_INVALID_PID = 1,
        PGS_POINTER_IS_UNUSED = 2
    };

    struct PageGet {
        PageGetStatus status;
        Row row;
    };
    typedef PageGet PageGet;
    PageGet emptyPageGet(PageGetStatus status);

    enum PagePutStatus {
        PPS_OK = 0,
        PPS_NOT_ENOUGH_SPACE = 1
    };

    struct PagePut {
        PagePutStatus status;
        models::PID pid;
    };
    typedef PagePut PagePut;
    PagePut notEnoughSpacePagePut();

    #define PAGE_TYPE_SENSE        1
    #define PAGE_TYPE_INSTANCE     2
    #define PAGE_TYPE_REFERENCE    3
    #define PAGE_TYPE_WORDFORM     4
    #define PAGE_8_HEADER_SIZE     16
    #define PAGE_8_CONTENT_SIZE    8176
    #define PAGE_8_SIZE            8192
    class [[maybe_unused]] Page8 {
    private:
        // ---- 0 ----
        int32_t fileNumber;
        // ---- 4 ----
        int32_t pageNumber;
        // ---- 8 ----
        int8_t pageType;
        // ---- 9 ----
        int8_t typeVersion; // not used yet
        // ---- 10 ----
        int16_t pointerOffset; // first free byte in content
        // ---- 12 ----
        int16_t rowsOffset; // last not-free byte in content or undefined
        // ---- 14 ----
        int16_t reserved;
        // ---- 16 ----
        int8_t content[PAGE_8_CONTENT_SIZE]{};

        [[nodiscard]]
        bool isRequestedPidValid(models::PID requestedPid) const;
        PagePointer* getPointerByOffset(int16_t offset);
        void* getUndefinedByOffset(int16_t offset);
        models::PID pidForPointer(PagePointer* pagePointer) const;
        static int16_t calcPointerOffsetForPid(models::PID requestedPid);
    public:
        Page8(
                int32_t fileNumber,
                int32_t pageNumber,
                int8_t pageType
        ) {
            this->fileNumber = fileNumber;
            this->pageNumber = pageNumber;
            this->pageType = pageType;
            this->typeVersion = 0;
            this->pointerOffset = 0;
            this->rowsOffset = PAGE_8_CONTENT_SIZE;
            this->reserved = 0;
        }

        [[maybe_unused]]
        Page8(
                int32_t fileNumber,
                int32_t pageNumber,
                int8_t pageType,
                int8_t* content
        ) : Page8(fileNumber, pageNumber, pageType) {
            std::copy(content, content + PAGE_8_CONTENT_SIZE, this->content);
        }

        [[maybe_unused]]
        [[nodiscard]]
        int32_t getFileNumber() const {
            return fileNumber;
        }

        [[maybe_unused]]
        [[nodiscard]]
        int32_t getPageNumber() const {
            return pageNumber;
        }

        [[maybe_unused]]
        [[nodiscard]]
        bool isTypeSense() const {
            return pageType == PAGE_TYPE_SENSE;
        }

        [[maybe_unused]]
        [[nodiscard]]
        bool isTypeInstance() const {
            return pageType == PAGE_TYPE_INSTANCE;
        }

        [[maybe_unused]]
        [[nodiscard]]
        bool isTypeReference() const {
            return pageType == PAGE_TYPE_REFERENCE;
        }

        [[maybe_unused]]
        [[nodiscard]]
        bool isTypeWordform() const {
            return pageType == PAGE_TYPE_WORDFORM;
        }

        [[maybe_unused]]
        [[nodiscard]]
        int16_t getFreeSpace() const {
            return (int16_t) (rowsOffset - pointerOffset);
        }

        [[maybe_unused]]
        PageGet getByPid(models::PID pid);

        [[maybe_unused]]
        PagePut put(Row row);
    };

    #pragma pack(pop)
}
#endif //JACKALOPE_STORAGE_PAGE_HPP