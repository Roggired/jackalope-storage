//
// Created by ego on 07.05.23.
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
        PGS_POINTER_IS_UNUSED = 2,
        PGS_POINTER_IS_NOT_VISIBLE = 3
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

    enum PageRemoveStatus {
        PRS_OK = 0,
        PRS_INVALID_PID = 1,
        PRS_POINTER_IS_UNUSED = 2,
        PRS_POINTER_IS_NOT_VISIBLE = 3,
    };

    struct PageRemove {
        PageRemoveStatus status;
    };
    typedef PageRemove PageRemove;

    class PidInvalidException : public std::exception {};

    #define PAGE_TYPE_SENSE        1
    #define PAGE_TYPE_INSTANCE     2
    #define PAGE_TYPE_REFERENCE    3
    #define PAGE_TYPE_WORDFORM     4
    #define PAGE_8_HEADER_SIZE     16
    #define PAGE_8_CONTENT_SIZE    8176
    #define PAGE_8_SIZE            8192
    /**
     * Page (currently only 8KB size is supported) is a low-level abstraction which provides interface for
     * put, get, remove, vacuum, commit, abort. It stores rows in itself, so we need to only allocate memory for
     * a page and we will be able to store rows.
     *
     * @author ego
     */
    class [[maybe_unused]] Page {
    private:
        // ---- 0 ----
        uint32_t fileNumber;
        // ---- 4 ----
        uint16_t pageNumber;
        // ---- 6 ----
        uint8_t pageType;
        // ---- 7 ----
        uint8_t typeVersion; // not used yet
        // ---- 8 ----
        uint16_t pointerOffset; // first free byte in content
        // ---- 10 ----
        uint16_t rowsOffset; // last not-free byte in content or undefined
        // ---- 12 ----
        uint32_t reserved;
        // ---- 16 ----
        int8_t content[PAGE_8_CONTENT_SIZE]{0};

        void iterateOverPointers(const std::function<void(PagePointer*)>& consumer);

        [[nodiscard]]
        int16_t findFirstUnusedPointerOffset() const;

        [[nodiscard]]
        uint16_t calcPointersNumber() const;

        [[nodiscard]]
        bool isPidValid(models::PID requestedPid) const;

        [[nodiscard]]
        PagePointer* createPagePointerOnOffset(uint32_t xid, uint16_t targetPointerOffset, Row row);

        void placeRowOnCurrentOffset(Row row);

        [[nodiscard]]
        Row getRowOnPointerOffset(PagePointer* pointer) const;

        [[nodiscard]]
        PagePointer* getPointerByOffset(uint16_t offset) const;

        [[nodiscard]]
        models::PID createPidByPointer(PagePointer* pagePointer) const;

        [[nodiscard]]
        static uint16_t calcPointerOffsetByPid(models::PID requestedPid);
    public:
        Page(
                uint32_t fileNumber,
                uint16_t pageNumber,
                uint8_t pageType
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
        Page(
                uint32_t fileNumber,
                uint16_t pageNumber,
                uint8_t pageType,
                int8_t* content
        ) : Page(fileNumber, pageNumber, pageType) {
            std::copy(content, content + PAGE_8_CONTENT_SIZE, this->content);
        }

        [[maybe_unused]]
        [[nodiscard]]
        uint32_t getFileNumber() const {
            return fileNumber;
        }

        [[maybe_unused]]
        [[nodiscard]]
        uint16_t getPageNumber() const {
            return pageNumber;
        }

        [[maybe_unused]]
        [[nodiscard]]
        uint8_t getType() const {
            return pageType;
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
        uint16_t getFreeSpace() const {
            return (int16_t) (rowsOffset - pointerOffset);
        }

        /**
         * Returns the Row (header and payload) found by provided models::PID.
         * <br></br>
         * <br></br>
         * <b>NOT THREAD SAFE</b>
         * @param xid - active transaction ID
         * @param pid
         * @return the PageGet struct with the operation status and a Row struct.
         */
        [[maybe_unused]]
        [[nodiscard]]
        PageGet getByPid(uint32_t xid, models::PID pid) const;

        /**
         * Place row in the page.
         * <br></br>
         * <br></br>
         * <b>NOT THREAD SAFE</b>
         * @param xid - active transaction ID
         * @param row - the struct of a row header and a row content (payload). Bytes from the header and the payload
         * will be copied into the page content.
         * @return the PagePut struct with the operation status and a PID which has been associated with created pointer.
         */
        [[maybe_unused]]
        [[nodiscard]]
        PagePut put(uint32_t xid, Row row);

        /**
         * 'Removes' (mark xmax for pointer) the pointer associated with given models::PID.
         * <br></br>
         * <br></br>
         * <b>NOT THREAD SAFE</b>
         * @param xid - active transaction ID
         * @param pid
         * @return the PageRemove struct with the operation status.
         */
        [[maybe_unused]]
        [[nodiscard]]
        PageRemove remove(uint32_t xid, models::PID pid);

        /**
         * Packs rows in the page content.
         * <br></br>
         * <br></br>
         * <b>NOT THREAD SAFE</b>
         * @param eventHorizon - the minimum ID of all active transactions.
         * @return numbers of vacuumed (deleted) rows.
         */
        [[maybe_unused]]
        [[nodiscard]]
        uint16_t vacuum(uint32_t eventHorizon);

        /**
         * Sets xmin and/or xmax as committed for given pid. This function will set committed flag for
         * xmin if xid == xmin, will set committed flag for xmax if xid == xmax.
         * @param xid - active transaction ID
         * @param pid
         */
        [[maybe_unused]]
        void setCommitted(uint32_t xid, models::PID pid);

        /**
         * Sets xmin and/or xmax as aborted for given pid. This function will set aborted flag for
         * xmin if xid == xmin, will set aborted flag for xmax if xid == xmax.
         * @param xid - active transaction ID
         * @param pid
         */
        [[maybe_unused]]
        void setAborted(uint32_t xid, models::PID pid);
    };

    #pragma pack(pop)
}
#endif //JACKALOPE_STORAGE_PAGE_HPP
