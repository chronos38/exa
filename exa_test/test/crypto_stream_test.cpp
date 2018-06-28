#include <pch.h>
#include <exa/crypto_stream.hpp>
#include <exa/symmetric_algorithm.hpp>
#include <exa/memory_stream.hpp>

#include <string>

using namespace exa;
using namespace testing;
using namespace std::string_literals;

class crypto_stream_test : public TestWithParam<symmetric_algorithm_type>
{
public:
    std::vector<cipher_mode> ciphers = {cipher_mode::cbc, cipher_mode::cfb, cipher_mode::ctr, cipher_mode::ecb,
                                        cipher_mode::ofb};

    static std::vector<uint8_t> encrypt_string_to_bytes(cipher_mode cipher, const std::shared_ptr<symmetric_algorithm>& algo,
                                                        const std::string& text)
    {
        auto encryptor = algo->make_shared_encryptor(cipher);
        auto buffer = std::make_shared<memory_stream>();
        auto stream = std::make_unique<crypto_stream>(buffer, encryptor, crypto_stream_mode::write);
        stream->write(gsl::make_span(reinterpret_cast<const uint8_t*>(text.data()), text.size()));
        return buffer->to_array();
    }

    static std::vector<uint8_t> encrypt_string_to_bytes(cipher_mode cipher, const std::shared_ptr<symmetric_algorithm>& algo,
                                                        const std::string& text, const std::vector<uint8_t>& key,
                                                        const std::vector<uint8_t>& iv)
    {
        auto encryptor = algo->make_shared_encryptor(cipher, key, iv);
        auto buffer = std::make_shared<memory_stream>();
        auto stream = std::make_unique<crypto_stream>(buffer, encryptor, crypto_stream_mode::write);
        stream->write(gsl::make_span(reinterpret_cast<const uint8_t*>(text.data()), text.size()));
        return buffer->to_array();
    }

    static std::string decrypt_string_from_bytes(cipher_mode cipher, const std::shared_ptr<symmetric_algorithm>& algo,
                                                 const std::vector<uint8_t>& text)
    {
        auto encryptor = algo->make_shared_encryptor(cipher);
        auto buffer = std::make_shared<memory_stream>(text);
        auto stream = std::make_unique<crypto_stream>(buffer, encryptor, crypto_stream_mode::write);

        std::ostringstream result;
        std::vector<uint8_t> v(1024);
        auto n = stream->read(v);

        while (n > 0)
        {
            auto p = reinterpret_cast<char*>(v.data());
            result << std::string(p, p + v.size());
            n = stream->read(v);
        }

        return result.str();
    }

    static std::string decrypt_string_from_bytes(cipher_mode cipher, const std::shared_ptr<symmetric_algorithm>& algo,
                                                 const std::vector<uint8_t>& text, const std::vector<uint8_t>& key,
                                                 const std::vector<uint8_t>& iv)
    {
        auto encryptor = algo->make_shared_encryptor(cipher, key, iv);
        auto buffer = std::make_shared<memory_stream>(text);
        auto stream = std::make_unique<crypto_stream>(buffer, encryptor, crypto_stream_mode::write);

        std::ostringstream result;
        std::vector<uint8_t> v(1024);
        auto n = stream->read(v);

        while (n > 0)
        {
            auto p = reinterpret_cast<char*>(v.data());
            result << std::string(p, p + v.size());
            n = stream->read(v);
        }

        return result.str();
    }
};

INSTANTIATE_TEST_CASE_P(symmetric_algorithms, crypto_stream_test,
                        Values(symmetric_algorithm_type::aes, symmetric_algorithm_type::blowfish,
                               symmetric_algorithm_type::camellia, symmetric_algorithm_type::des_ede3,
                               symmetric_algorithm_type::idea, symmetric_algorithm_type::rc5, symmetric_algorithm_type::rc6,
                               symmetric_algorithm_type::seed, symmetric_algorithm_type::serpent,
                               symmetric_algorithm_type::twofish));

TEST_P(crypto_stream_test, encrypt_and_decrypt_string)
{
    for (auto&& cipher : ciphers)
    {
        auto algorithm = make_shared_symmetric_algorithm(GetParam());
        // auto key_size = algorithm->default_key_size();
        // auto key = std::vector<uint8_t>(key_size);
        // auto iv_size = algorithm->block_size();
        // auto iv = std::vector<uint8_t>(iv_size, 0);

        // for (size_t i = 0; i < key_size; ++i)
        //{
        //    key[i] = static_cast<uint8_t>(i);
        //}

        auto text = "This is a string for encryption testing. Let's get real here!"s;
        auto encrypted = encrypt_string_to_bytes(cipher, algorithm, text);
        auto decrypted = decrypt_string_from_bytes(cipher, algorithm, encrypted);

        ASSERT_THAT(decrypted, StrEq(text));
    }
}
