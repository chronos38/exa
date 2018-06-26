#pragma once

#include <gsl/span>

#include <vector>
#include <cstdint>
#include <cstddef>

namespace exa
{
    class crypto_transform
    {
    public:
        virtual ~crypto_transform() = default;

        virtual size_t block_size() const = 0;
        virtual size_t optimal_input_size() const = 0;

        virtual std::vector<uint8_t> transform_final_block(gsl::span<const uint8_t> input) = 0;
        virtual size_t transform_block(gsl::span<const uint8_t> input, gsl::span<uint8_t> output) = 0;
    };
}
