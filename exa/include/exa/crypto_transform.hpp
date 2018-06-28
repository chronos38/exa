#pragma once

#include <gsl/gsl>

#include <vector>
#include <cstdint>
#include <cstddef>

namespace CryptoPP
{
    class StreamTransformation;
}

namespace exa
{
    class crypto_transform
    {
    public:
        virtual ~crypto_transform() = default;

        virtual size_t block_size() const = 0;
        virtual size_t optimal_input_size() const = 0;

        virtual std::vector<uint8_t> transform_final_block(gsl::span<const uint8_t> input) = 0;
        virtual void transform_block(gsl::span<const uint8_t> input, gsl::span<uint8_t> output) = 0;

    private:
        friend class crypto_stream;

        virtual CryptoPP::StreamTransformation& stream_transformation() = 0;
        virtual const CryptoPP::StreamTransformation& stream_transformation() const = 0;
    };
}
