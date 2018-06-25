#include <exa/symmetric_algorithm.hpp>
#include <exa/crypto_block_cipher.hpp>

#include <cryptopp/aes.h>
#include <cryptopp/des.h>
#include <cryptopp/serpent.h>
#include <cryptopp/rc5.h>
#include <cryptopp/rc6.h>
#include <cryptopp/twofish.h>
#include <cryptopp/blowfish.h>
#include <cryptopp/idea.h>
#include <cryptopp/camellia.h>
#include <cryptopp/seed.h>
#include <cryptopp/panama.h>
#include <cryptopp/sosemanuk.h>
#include <cryptopp/salsa.h>
#include <cryptopp/modes.h>

namespace exa
{
    std::unique_ptr<symmetric_algorithm> make_symmetric_algorithm(symmetric_algorithm_type type)
    {
        switch (type)
        {
            case symmetric_algorithm_type::aes:
                return std::make_unique<crypto_block_cypher<CryptoPP::AES>>();
            case symmetric_algorithm_type::des_ede3:
                return std::make_unique<crypto_block_cypher<CryptoPP::DES_EDE3>>();
            case symmetric_algorithm_type::serpent:
                return std::make_unique<crypto_block_cypher<CryptoPP::Serpent>>();
            case symmetric_algorithm_type::rc5:
                return std::make_unique<crypto_block_cypher<CryptoPP::RC5>>();
            case symmetric_algorithm_type::rc6:
                return std::make_unique<crypto_block_cypher<CryptoPP::RC6>>();
            case symmetric_algorithm_type::twofish:
                return std::make_unique<crypto_block_cypher<CryptoPP::Twofish>>();
            case symmetric_algorithm_type::blowfish:
                return std::make_unique<crypto_block_cypher<CryptoPP::Blowfish>>();
            case symmetric_algorithm_type::idea:
                return std::make_unique<crypto_block_cypher<CryptoPP::IDEA>>();
            case symmetric_algorithm_type::camellia:
                return std::make_unique<crypto_block_cypher<CryptoPP::Camellia>>();
            case symmetric_algorithm_type::seed:
                return std::make_unique<crypto_block_cypher<CryptoPP::SEED>>();
            default:
                throw std::invalid_argument("Received invalid symmetric algorithm type.");
        }
    }
}
