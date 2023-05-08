//
// Created by ego on 07.05.23.
//

#ifndef JACKALOPE_STORAGE_SENSE_HPP
#define JACKALOPE_STORAGE_SENSE_HPP

#include <cstdint>

#include "sin.hpp"
#include "pid.hpp"
#include "wordform.hpp"

namespace models {
    class [[maybe_unused]] Sense : SINModel, PIDModel {
    private:
        WordformId nameId;
    public:
        [[maybe_unused]]
        Sense(
                PID pid,
                SIN sin,
                WordformId nameId
        ) : SINModel(sin),
            PIDModel(pid),
            nameId(nameId) {}

        [[maybe_unused]]
        [[nodiscard]]
        WordformId getNameId() const {
            return nameId;
        }
    };
}

#endif //JACKALOPE_STORAGE_SENSE_HPP
