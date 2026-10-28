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
constexpr std::size_t NONCE_SIZE = 12;   // AES-GCM
constexpr std::size_t TAG_SIZE = 16;     // AES-GCM
using Salt = std::array<std::uint8_t, SALT_SIZE>;
using Digest = std::array<std::uint8_t, HASH_SIZE>;
using Key = std::array<std::uint8_t, HASH_SIZE>;     // clave AES-256
using Nonce = std::array<std::uint8_t, NONCE_SIZE>;
using Tag = std::array<std::uint8_t, TAG_SIZE>;

void randomBytes(std::uint8_t* output, std::size_t size);
Salt makeSalt();
Digest deriveVerifier(std::string_view secret, const Salt& salt, std::uint32_t iterations);
bool equalConstantTime(const Digest& a, const Digest& b);

// Sprint 2 - cifrado de la boveda
// Clave AES-256 derivada de la contrasena maestra (PBKDF2, con un salt distinto del verificador).
Key deriveKey(std::string_view secret, const Salt& salt, std::uint32_t iterations);
// AES-256-GCM: cifra y genera una etiqueta que detecta cualquier modificacion del archivo.
std::vector<std::uint8_t> encrypt(const Key& key, const Nonce& nonce, std::string_view plain, Tag& tag);
// Devuelve false si la clave es incorrecta o los datos han sido modificados.
bool decrypt(const Key& key, const Nonce& nonce, const std::vector<std::uint8_t>& cipher,
             const Tag& tag, std::string& plain);
std::string hex(const std::uint8_t* bytes, std::size_t size);
std::vector<std::uint8_t> unhex(std::string_view source);
std::size_t uniformIndex(std::size_t maximumExclusive);
std::string generatePassword(std::size_t length);
}
