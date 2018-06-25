#pragma once

#include <exa/crypto_transform.hpp>

#include <gsl/span>

#include <vector>
#include <cstdint>
#include <cstddef>
#include <memory>

namespace exa
{
    enum class cipher_mode
    {
        ecb,
        cbc,
        cfb,
        ofb,
        ctr
    };

    class symmetric_algorithm
    {
    public:
        virtual ~symmetric_algorithm() = default;

        virtual size_t block_size() const noexcept = 0;
        virtual size_t min_key_length() const noexcept = 0;
        virtual size_t max_key_length() const noexcept = 0;
        virtual size_t default_key_length() const noexcept = 0;
        virtual size_t iv_requirement() const noexcept = 0;
        virtual size_t iv_length() const noexcept = 0;
        virtual cipher_mode mode() const noexcept = 0;
        virtual void mode(cipher_mode v) noexcept = 0;
        virtual const std::vector<uint8_t>& key() const noexcept = 0;
        virtual void key(gsl::span<const uint8_t> v) noexcept = 0;
        virtual const std::vector<uint8_t>& iv() const noexcept = 0;
        virtual void iv(gsl::span<const uint8_t> v) noexcept = 0;

        virtual std::shared_ptr<crypto_transform> make_shared_decryptor() const = 0;
        virtual std::shared_ptr<crypto_transform> make_shared_decryptor(cipher_mode mode, gsl::span<const uint8_t> key,
                                                                        gsl::span<const uint8_t> iv) const = 0;
        virtual std::shared_ptr<crypto_transform> make_shared_encryptor() const = 0;
        virtual std::shared_ptr<crypto_transform> make_shared_encryptor(cipher_mode mode, gsl::span<const uint8_t> key,
                                                                        gsl::span<const uint8_t> iv) const = 0;

        virtual std::unique_ptr<crypto_transform> make_unique_decryptor() const = 0;
        virtual std::unique_ptr<crypto_transform> make_unique_decryptor(cipher_mode mode, gsl::span<const uint8_t> key,
                                                                        gsl::span<const uint8_t> iv) const = 0;
        virtual std::unique_ptr<crypto_transform> make_unique_encryptor() const = 0;
        virtual std::unique_ptr<crypto_transform> make_unique_encryptor(cipher_mode mode, gsl::span<const uint8_t> key,
                                                                        gsl::span<const uint8_t> iv) const = 0;
    };

    enum class symmetric_algorithm_type
    {
        aes,
        des_ede3,
        serpent,
        rc5,
        rc6,
        twofish,
        blowfish,
        idea,
        camellia,
        seed
    };

    std::unique_ptr<symmetric_algorithm> make_unique_symmetric_algorithm(symmetric_algorithm_type type);
    std::shared_ptr<symmetric_algorithm> make_shared_symmetric_algorithm(symmetric_algorithm_type type);
}
