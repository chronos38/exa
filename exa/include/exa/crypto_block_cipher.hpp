#pragma once

#include <exa/crypto_transform.hpp>
#include <exa/symmetric_algorithm.hpp>

#include <cryptopp/modes.h>
#include <cryptopp/filters.h>

#include <memory>
#include <type_traits>

namespace exa
{
    template <typename T>
    class crypto_transform_algorithm : public crypto_transform
    {
    public:
        virtual ~crypto_transform_algorithm() = default;

        crypto_transform_algorithm(gsl::span<const uint8_t> key, gsl::span<const uint8_t> iv, size_t feedback)
        {
            algorithm_ = feedback == 0 ? T(key.data(), static_cast<size_t>(key.size()), iv.data())
                                       : T(key.data(), static_cast<size_t>(key.size()), iv.data(), feedback);
        }

        virtual size_t block_size() const override
        {
            return algorithm_.OptimalBlockSize();
        }

        virtual size_t optimal_input_size() const override
        {
            return algorithm_.OptimalBlockSize() - algorithm_.GetOptimalBlockSizeUsed();
        }

        virtual std::vector<uint8_t> transform_final_block(gsl::span<const uint8_t> input) override
        {
            std::vector<uint8_t> v(algorithm_.OptimalBlockSize());
            algorithm_.ProcessData(v.data(), input.data(), static_cast<size_t>(input.size()));
            return v;
        }

        virtual size_t transform_block(gsl::span<const uint8_t> i, gsl::span<uint8_t> o) override
        {
            return algorithm_.ProcessLastBlock(o.data(), static_cast<size_t>(o.size()), i.data(),
                                               static_cast<size_t>(i.size()));
        }

    private:
        T algorithm_;
    };

    template <typename T>
    class crypto_block_cipher : public symmetric_algorithm
    {
    public:
        virtual ~crypto_block_cipher() = default;

        virtual size_t block_size() const noexcept override
        {
            return T::BLOCKSIZE;
        }

        virtual size_t default_block_size() const noexcept override
        {
            return T::DEFAULT_BLOCKSIZE;
        }

        virtual size_t min_block_size() const noexcept override
        {
            return T::MIN_BLOCKSIZE;
        }

        virtual size_t max_block_size() const noexcept override
        {
            return T::MAX_BLOCKSIZE;
        }

        virtual size_t min_key_size() const noexcept override
        {
            return T::MIN_KEYLENGTH;
        }

        virtual size_t max_key_size() const noexcept override
        {
            return T::MAX_KEYLENGTH;
        }

        virtual size_t default_key_size() const noexcept override
        {
            return T::DEFAULT_KEYLENGTH;
        }

        virtual size_t iv_requirement() const noexcept override
        {
            return T::IV_REQUIREMENT;
        }

        virtual size_t iv_size() const noexcept override
        {
            return T::IV_LENGTH;
        }

        virtual std::string name() const noexcept override
        {
            return T::StaticAlgorithmName();
        }

        virtual cipher_mode mode() const noexcept override
        {
            return mode_;
        }

        virtual void mode(cipher_mode mode) noexcept override
        {
            mode_ = mode;
        }

        virtual const std::vector<uint8_t>& key() const noexcept override
        {
            return key_;
        }

        virtual void key(gsl::span<const uint8_t> key) noexcept override
        {
            key_.assign(std::begin(key), std::end(key));
        }

        virtual const std::vector<uint8_t>& iv() const noexcept override
        {
            return iv_;
        }

        virtual void iv(gsl::span<const uint8_t> iv) noexcept override
        {
            iv_.assign(std::begin(iv), std::end(iv));
        }

        virtual size_t feedback_size() const noexcept override
        {
            return feedback_;
        }

        virtual void feedback_size(size_t v) noexcept override
        {
            feedback_ = v;
        }

        virtual size_t valid_key_size(size_t size) const noexcept override
        {
            return T::StaticGetValidKeyLength(size);
        }

        virtual size_t valid_block_size(size_t block_size) const noexcept override
        {
            return T::StaticGetValidBlockSize(block_size);
        }

        virtual size_t valid_block_size(size_t key_size, size_t block_size) const noexcept override
        {
            return T::StaticGetValidBlockSize(key_size, block_size);
        }

        virtual std::shared_ptr<crypto_transform> make_shared_decryptor() const override
        {
            switch (mode_)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Decryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Decryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Decryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Decryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Decryption>>(key_, iv_,
                                                                                                           feedback_);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

        virtual std::shared_ptr<crypto_transform> make_shared_decryptor(cipher_mode mode, gsl::span<const uint8_t> key,
                                                                        gsl::span<const uint8_t> iv) const override
        {
            switch (mode)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Decryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Decryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Decryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Decryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Decryption>>(key, iv,
                                                                                                           feedback_);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

        virtual std::shared_ptr<crypto_transform> make_shared_encryptor() const override
        {
            switch (mode_)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Encryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Encryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Encryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Encryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Encryption>>(key_, iv_,
                                                                                                           feedback_);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

        virtual std::shared_ptr<crypto_transform> make_shared_encryptor(cipher_mode mode, gsl::span<const uint8_t> key,
                                                                        gsl::span<const uint8_t> iv) const override
        {
            switch (mode)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Encryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Encryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Encryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Encryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Encryption>>(key, iv,
                                                                                                           feedback_);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

        virtual std::unique_ptr<crypto_transform> make_unique_decryptor() const override
        {
            switch (mode_)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Decryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Decryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Decryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Decryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Decryption>>(key_, iv_,
                                                                                                           feedback_);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

        virtual std::unique_ptr<crypto_transform> make_unique_decryptor(cipher_mode mode, gsl::span<const uint8_t> key,
                                                                        gsl::span<const uint8_t> iv) const override
        {
            switch (mode)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Decryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Decryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Decryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Decryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Decryption>>(key, iv,
                                                                                                           feedback_);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

        virtual std::unique_ptr<crypto_transform> make_unique_encryptor() const override
        {
            switch (mode_)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Encryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Encryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Encryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Encryption>>(key_, iv_,
                                                                                                           feedback_);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Encryption>>(key_, iv_,
                                                                                                           feedback_);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

        virtual std::unique_ptr<crypto_transform> make_unique_encryptor(cipher_mode mode, gsl::span<const uint8_t> key,
                                                                        gsl::span<const uint8_t> iv) const override
        {
            switch (mode)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Encryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Encryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Encryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Encryption>>(key, iv,
                                                                                                           feedback_);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Encryption>>(key, iv,
                                                                                                           feedback_);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

    private:
        cipher_mode mode_ = cipher_mode::ctr;
        std::vector<uint8_t> key_;
        std::vector<uint8_t> iv_;
        size_t feedback_ = 0;
    };
}
