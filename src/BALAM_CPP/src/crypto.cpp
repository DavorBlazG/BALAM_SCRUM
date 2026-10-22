#include "crypto.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#else
#include <openssl/evp.h>
#include <openssl/rand.h>
#endif

namespace crypto {
void randomBytes(std::uint8_t* output, std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<unsigned long>::max()))
        throw std::runtime_error("Random request too large");
#ifdef _WIN32
    const NTSTATUS status = BCryptGenRandom(nullptr, output,
        static_cast<ULONG>(size), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (!BCRYPT_SUCCESS(status)) throw std::runtime_error("Windows RNG failed");
#else
    if (RAND_bytes(output, static_cast<int>(size)) != 1)
        throw std::runtime_error("OpenSSL RNG failed");
#endif
}
Salt makeSalt()
{
    Salt salt{};
    randomBytes(salt.data(), salt.size());
    return salt;
}
Digest deriveVerifier(std::string_view secret, const Salt& salt, std::uint32_t iterations)
{
    Digest out{};
#ifdef _WIN32
    BCRYPT_ALG_HANDLE provider = nullptr;
    const NTSTATUS opened = BCryptOpenAlgorithmProvider(&provider, BCRYPT_SHA256_ALGORITHM,
                                                        nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG);
    if (!BCRYPT_SUCCESS(opened)) throw std::runtime_error("BCrypt provider failed");
    const NTSTATUS status = BCryptDeriveKeyPBKDF2(provider,
        reinterpret_cast<PUCHAR>(const_cast<char*>(secret.data())),
        static_cast<ULONG>(secret.size()), const_cast<PUCHAR>(salt.data()),
        static_cast<ULONG>(salt.size()), iterations,
        out.data(), static_cast<ULONG>(out.size()), 0);
    BCryptCloseAlgorithmProvider(provider, 0);
    if (!BCRYPT_SUCCESS(status)) throw std::runtime_error("PBKDF2 failed");
#else
    if (PKCS5_PBKDF2_HMAC(secret.data(), static_cast<int>(secret.size()),
                          salt.data(), static_cast<int>(salt.size()),
                          static_cast<int>(iterations), EVP_sha256(),
                          static_cast<int>(out.size()), out.data()) != 1)
        throw std::runtime_error("PBKDF2 failed");
#endif
    return out;
}
bool equalConstantTime(const Digest& a, const Digest& b)
{
    unsigned int difference = 0;
    for (std::size_t i = 0; i < a.size(); ++i)
        difference |= static_cast<unsigned int>(a[i] ^ b[i]);
    return difference == 0;
}
std::string hex(const std::uint8_t* bytes, std::size_t size)
{
    static constexpr char digits[] = "0123456789abcdef";
    std::string result;
    result.reserve(size * 2);
    for (std::size_t i = 0; i < size; ++i) {
        result.push_back(digits[bytes[i] >> 4]);
        result.push_back(digits[bytes[i] & 15]);
    }
    return result;
}
std::vector<std::uint8_t> unhex(std::string_view source)
{
    if (source.size() % 2 != 0) throw std::runtime_error("Invalid hex size");
    auto value = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + c - 'a';
        if (c >= 'A' && c <= 'F') return 10 + c - 'A';
        return -1;
    };
    std::vector<std::uint8_t> result;
    result.reserve(source.size() / 2);
    for (std::size_t i = 0; i < source.size(); i += 2) {
        const int high = value(source[i]);
        const int low = value(source[i+1]);
        if (high < 0 || low < 0) throw std::runtime_error("Invalid hex character");
        result.push_back(static_cast<std::uint8_t>((high << 4) | low));
    }
    return result;
}
std::size_t uniformIndex(std::size_t maximumExclusive)
{
    // Rejection sampling: evitamos el sesgo de aplicar modulo sobre un byte.
    if (maximumExclusive == 0 || maximumExclusive > 256)
        throw std::invalid_argument("Range out of bounds");
    const std::size_t bound = 256 - (256 % maximumExclusive);
    std::uint8_t byte = 0;
    do { randomBytes(&byte, 1); } while (byte >= bound);
    return byte % maximumExclusive;
}
std::string generatePassword(std::size_t length)
{
    if (length < 8 || length > 64) throw std::invalid_argument("Invalid password length");
    constexpr std::string_view uppercase = "ABCDEFGHJKLMNPQRSTUVWXYZ";
    constexpr std::string_view lowercase = "abcdefghijkmnopqrstuvwxyz";
    constexpr std::string_view digits = "23456789";
    constexpr std::string_view symbols = "!@#$%&*+-_?";
    const std::string alphabet = std::string(uppercase) + std::string(lowercase)
                                + std::string(digits) + std::string(symbols);
    std::string output;
    output.reserve(length);
    const std::string_view groups[] = {uppercase, lowercase, digits, symbols};
    for (auto group : groups) output.push_back(group[uniformIndex(group.size())]);
    while (output.size() < length)
        output.push_back(alphabet[uniformIndex(alphabet.size())]);
    for (std::size_t i = output.size(); i > 1; --i)
        std::swap(output[i-1], output[uniformIndex(i)]);
    return output;
}
}
