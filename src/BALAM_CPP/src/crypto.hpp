#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace crypto {
constexpr std::size_t SALT_SIZE = 16;
constexpr std::size_t HASH_SIZE = 32;
constexpr std::uint32_t PBKDF2_ITERATIONS = 600000;
using Salt = std::array<std::uint8_t, SALT_SIZE>;
using Digest = std::array<std::uint8_t, HASH_SIZE>;

void randomBytes(std::uint8_t* output, std::size_t size);
Salt makeSalt();
Digest deriveVerifier(std::string_view secret, const Salt& salt, std::uint32_t iterations);
bool equalConstantTime(const Digest& a, const Digest& b);
std::string hex(const std::uint8_t* bytes, std::size_t size);
std::vector<std::uint8_t> unhex(std::string_view source);
std::size_t uniformIndex(std::size_t maximumExclusive);
std::string generatePassword(std::size_t length);
}
