//
// Created by roggired on 07.05.23.
//

#ifndef JACKALOPE_STORAGE_PID_HPP
#define JACKALOPE_STORAGE_PID_HPP

#include <cstdint>

namespace models {
    class PID {
    private:
        int32_t fileNumber;
        int32_t pageNumber;
        int16_t rowPointerNumber;

    public:
        [[maybe_unused]]
        PID(
                int32_t fileNumber,
                int32_t pageNumber,
                int16_t rowPointerNumber
        ) : fileNumber(fileNumber),
            pageNumber(pageNumber),
            rowPointerNumber(rowPointerNumber) {}

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
        int16_t getRowPointerNumber() const {
            return rowPointerNumber;
        }
    };

    class [[maybe_unused]] PIDModel {
    private:
        PID pid;

    public:
        [[maybe_unused]]
        explicit PIDModel(PID pid) : pid(pid) {}

        [[maybe_unused]]
        [[nodiscard]]
        PID getPid() {
            return pid;
        }

        [[maybe_unused]]
        [[nodiscard]]
        int32_t getFileNumber() const {
            return pid.getFileNumber();
        }

        [[maybe_unused]]
        [[nodiscard]]
        int32_t getPageNumber() const {
            return pid.getPageNumber();
        }

        [[maybe_unused]]
        [[nodiscard]]
        int16_t getRowPointerNumber() const {
            return pid.getRowPointerNumber();
        }
    };
}

#endif //JACKALOPE_STORAGE_PID_HPP
