//
// Created by roggired on 07.05.23.
//

#ifndef JACKALOPE_STORAGE_SIN_HPP
#define JACKALOPE_STORAGE_SIN_HPP

#include <cstdint>


namespace models {
    class SIN {
    private:
        int32_t senseNumber;
        int32_t instanceNumber;

    public:
        [[maybe_unused]]
        SIN(
                int32_t senseNumber,
                int32_t instanceNumber
        ) : senseNumber(senseNumber),
            instanceNumber(instanceNumber) {}

        [[maybe_unused]]
        [[nodiscard]]
        int32_t getSenseNumber() const {
            return senseNumber;
        }

        [[maybe_unused]]
        [[nodiscard]]
        int32_t getInstanceNumber() const {
            return instanceNumber;
        }
    };

    class SINModel {
    private:
        SIN sin;

    public:
        [[maybe_unused]]
        explicit SINModel(SIN sin) : sin(sin) {}

        [[maybe_unused]]
        [[nodiscard]]
        SIN getSin() const {
            return sin;
        }

        [[maybe_unused]]
        [[nodiscard]]
        int32_t getSenseNumber() const {
            return sin.getSenseNumber();
        }

        [[maybe_unused]]
        [[nodiscard]]
        int32_t getInstanceNumber() const {
            return sin.getInstanceNumber();
        }
    };
}

#endif //JACKALOPE_STORAGE_SIN_HPP
