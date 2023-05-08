//
// Created by ego on 07.05.23.
//

#ifndef JACKALOPE_STORAGE_WORDFORM_HPP
#define JACKALOPE_STORAGE_WORDFORM_HPP

#include <cstdint>
#include "pid.hpp"

namespace models {
    typedef int64_t WordformId;

    class [[maybe_unused]] Wordform : PIDModel {
    private:
        WordformId id;

    public:
        [[maybe_unused]]
        Wordform(
                PID pid,
                WordformId id
        ) : PIDModel(pid),
            id(id) {}

        [[maybe_unused]]
        [[nodiscard]]
        WordformId getId() const {
            return id;
        }
    };
}

#endif //JACKALOPE_STORAGE_WORDFORM_HPP
