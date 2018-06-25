#pragma once

#include <exa/crypto_transform.hpp>
#include <exa/symmetric_algorithm.hpp>

#include <cryptopp/modes.h>
#include <cryptopp/filters.h>

#include <memory>

namespace exa
{
    template <typename T>
    class crypto_transform_algorithm : public crypto_transform
    {
    public:
        virtual ~crypto_transform_algorithm() = default;

        crypto_transform_algorithm(gsl::span<const uint8_t> key, gsl::span<const uint8_t> iv)
            : algorithm_(key.data(), static_cast<size_t>(key.size()), iv.data())
        {
        }

        virtual std::vector<uint8_t> transform_final_block(gsl::span<const uint8_t> input) override
        {
            std::vector<uint8_t> output(static_cast<size_t>(input.size()));
            auto sink = new CryptoPP::ArraySink(output.data(), output.size());

            try
            {
                CryptoPP::StreamTransformationFilter filter(algorithm_, sink);
                auto n = filter.Put(input.data(), static_cast<size_t>(input.size()));
                filter.MessageEnd();
                output.resize(static_cast<size_t>(input.size() - n));
                return output;
            }
            catch (...)
            {
                delete sink;
                std::rethrow_exception(std::current_exception());
            }
        }

        virtual size_t transform_block(gsl::span<const uint8_t> input, gsl::span<uint8_t> output) override
        {
            auto sink = new CryptoPP::ArraySink(output.data(), static_cast<size_t>(input.size()));

            try
            {
                CryptoPP::StreamTransformationFilter filter(algorithm_, sink);
                auto n = filter.Put(input.data(), static_cast<size_t>(input.size()));
                filter.MessageEnd();
                return static_cast<size_t>(input.size() - n);
            }
            catch (...)
            {
                delete sink;
                std::rethrow_exception(std::current_exception());
            }
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

        virtual size_t min_key_length() const noexcept override
        {
            return T::MIN_KEYLENGTH;
        }

        virtual size_t max_key_length() const noexcept override
        {
            return T::MAX_KEYLENGTH;
        }

        virtual size_t default_key_length() const noexcept override
        {
            return T::DEFAULT_KEYLENGTH;
        }

        virtual size_t iv_requirement() const noexcept override
        {
            return T::IV_REQUIREMENT;
        }

        virtual size_t iv_length() const noexcept override
        {
            return T::IV_LENGTH;
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

        virtual std::shared_ptr<crypto_transform> make_shared_decryptor() const override
        {
            switch (mode_)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Decryption>>(key_, iv_);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Decryption>>(key_, iv_);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Decryption>>(key_, iv_);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Decryption>>(key_, iv_);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Decryption>>(key_, iv_);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

        virtual std::shared_ptr<crypto_transform> make_shared_decryptor(cipher_mode mode, gsl::span<const uint8_t> key,
                                                                        gsl::span<const uint8_t> iv) const override
        {
            switch (mode)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Decryption>>(key, iv);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Decryption>>(key, iv);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Decryption>>(key, iv);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Decryption>>(key, iv);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Decryption>>(key, iv);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

        virtual std::shared_ptr<crypto_transform> make_shared_encryptor() const override
        {
            switch (mode_)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Encryption>>(key_, iv_);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Encryption>>(key_, iv_);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Encryption>>(key_, iv_);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Encryption>>(key_, iv_);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Encryption>>(key_, iv_);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

        virtual std::shared_ptr<crypto_transform> make_shared_encryptor(cipher_mode mode, gsl::span<const uint8_t> key,
                                                                        gsl::span<const uint8_t> iv) const override
        {
            switch (mode)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Encryption>>(key, iv);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Encryption>>(key, iv);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Encryption>>(key, iv);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Encryption>>(key, iv);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Encryption>>(key, iv);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

        virtual std::unique_ptr<crypto_transform> make_unique_decryptor() const override
        {
            switch (mode_)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Decryption>>(key_, iv_);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Decryption>>(key_, iv_);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Decryption>>(key_, iv_);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Decryption>>(key_, iv_);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Decryption>>(key_, iv_);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

        virtual std::unique_ptr<crypto_transform> make_unique_decryptor(cipher_mode mode, gsl::span<const uint8_t> key,
                                                                        gsl::span<const uint8_t> iv) const override
        {
            switch (mode)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Decryption>>(key, iv);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Decryption>>(key, iv);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Decryption>>(key, iv);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Decryption>>(key, iv);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Decryption>>(key, iv);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

        virtual std::unique_ptr<crypto_transform> make_unique_encryptor() const override
        {
            switch (mode_)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Encryption>>(key_, iv_);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Encryption>>(key_, iv_);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Encryption>>(key_, iv_);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Encryption>>(key_, iv_);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Encryption>>(key_, iv_);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

        virtual std::unique_ptr<crypto_transform> make_unique_encryptor(cipher_mode mode, gsl::span<const uint8_t> key,
                                                                        gsl::span<const uint8_t> iv) const override
        {
            switch (mode)
            {
                case cipher_mode::cbc:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CBC_Mode<T>::Encryption>>(key, iv);
                case cipher_mode::cfb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CFB_Mode<T>::Encryption>>(key, iv);
                case cipher_mode::ctr:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::CTR_Mode<T>::Encryption>>(key, iv);
                case cipher_mode::ecb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::ECB_Mode<T>::Encryption>>(key, iv);
                case cipher_mode::ofb:
                    return std::make_unique<crypto_transform_algorithm<CryptoPP::OFB_Mode<T>::Encryption>>(key, iv);
            }

            throw std::invalid_argument("Received invalid cipher mode.");
        }

    private:
        cipher_mode mode_ = cipher_mode::ctr;
        std::vector<uint8_t> key_;
        std::vector<uint8_t> iv_;
    };
}
