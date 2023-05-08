//
// Created by ego on 07.05.23.
//

#ifndef JACKALOPE_STORAGE_INSTANCE_HPP
#define JACKALOPE_STORAGE_INSTANCE_HPP

#include <algorithm>
#include <cstdint>
#include <cstddef>

#include "sin.hpp"
#include "pid.hpp"
#include "wordform.hpp"

namespace models {
    class [[maybe_unused]] Instance: SINModel, PIDModel {
    private:
        WordformId canonicalWordformId;
        WordformId* wordformIds;
        size_t wordformIdsSize;

    public:
        [[maybe_unused]]
        Instance(
                SIN sin,
                PID pid,
                WordformId canonicalWordformId,
                WordformId* wordformIds,
                size_t wordformIdsSize
        ): SINModel(sin), PIDModel(pid) {
            this->canonicalWordformId = canonicalWordformId;
            this->wordformIdsSize = wordformIdsSize;
            this->wordformIds = new WordformId[wordformIdsSize];
            std::copy(wordformIds, wordformIds + wordformIdsSize, this->wordformIds);
        }

        ~Instance() {
            delete[] wordformIds;
        }
    };
}
#endif //JACKALOPE_STORAGE_INSTANCE_HPP
