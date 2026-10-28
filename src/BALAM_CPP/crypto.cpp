#include "crypto.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
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
Key deriveKey(std::string_view secret, const Salt& salt, std::uint32_t iterations)
{
    return deriveVerifier(secret, salt, iterations);
}

#ifdef _WIN32
namespace {
// Abre AES en modo GCM con la clave indicada. Quien llama libera clave y proveedor.
void openAesGcm(const Key& key, BCRYPT_ALG_HANDLE& provider, BCRYPT_KEY_HANDLE& handle)
{
    if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&provider, BCRYPT_AES_ALGORITHM, nullptr, 0)))
        throw std::runtime_error("AES provider failed");
    if (!BCRYPT_SUCCESS(BCryptSetProperty(provider, BCRYPT_CHAINING_MODE,
            reinterpret_cast<PUCHAR>(const_cast<wchar_t*>(BCRYPT_CHAIN_MODE_GCM)),
            sizeof(BCRYPT_CHAIN_MODE_GCM), 0)) ||
        !BCRYPT_SUCCESS(BCryptGenerateSymmetricKey(provider, &handle, nullptr, 0,
            const_cast<PUCHAR>(key.data()), static_cast<ULONG>(key.size()), 0))) {
        BCryptCloseAlgorithmProvider(provider, 0);
        throw std::runtime_error("AES key setup failed");
    }
}
}
#endif

std::vector<std::uint8_t> encrypt(const Key& key, const Nonce& nonce, std::string_view plain, Tag& tag)
{
    std::vector<std::uint8_t> cipher(plain.size());
#ifdef _WIN32
    BCRYPT_ALG_HANDLE provider = nullptr;
    BCRYPT_KEY_HANDLE handle = nullptr;
    openAesGcm(key, provider, handle);
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO info;
    BCRYPT_INIT_AUTH_MODE_INFO(info);
    info.pbNonce = const_cast<PUCHAR>(nonce.data());
    info.cbNonce = static_cast<ULONG>(nonce.size());
    info.pbTag = tag.data();
    info.cbTag = static_cast<ULONG>(tag.size());
    ULONG written = 0;
    const NTSTATUS status = BCryptEncrypt(handle,
        reinterpret_cast<PUCHAR>(const_cast<char*>(plain.data())), static_cast<ULONG>(plain.size()),
        &info, nullptr, 0, cipher.data(), static_cast<ULONG>(cipher.size()), &written, 0);
    BCryptDestroyKey(handle);
    BCryptCloseAlgorithmProvider(provider, 0);
    if (!BCRYPT_SUCCESS(status)) throw std::runtime_error("AES-GCM encryption failed");
#else
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    int length = 0;
    const bool ok = ctx &&
        EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1 &&
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(nonce.size()), nullptr) == 1 &&
        EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), nonce.data()) == 1 &&
        EVP_EncryptUpdate(ctx, cipher.data(), &length,
                          reinterpret_cast<const unsigned char*>(plain.data()), static_cast<int>(plain.size())) == 1 &&
        EVP_EncryptFinal_ex(ctx, cipher.data() + length, &length) == 1 &&
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, static_cast<int>(tag.size()), tag.data()) == 1;
    EVP_CIPHER_CTX_free(ctx);
    if (!ok) throw std::runtime_error("AES-GCM encryption failed");
#endif
    return cipher;
}

bool decrypt(const Key& key, const Nonce& nonce, const std::vector<std::uint8_t>& cipher,
             const Tag& tag, std::string& plain)
{
    plain.assign(cipher.size(), '\0');
#ifdef _WIN32
    BCRYPT_ALG_HANDLE provider = nullptr;
    BCRYPT_KEY_HANDLE handle = nullptr;
    openAesGcm(key, provider, handle);
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO info;
    BCRYPT_INIT_AUTH_MODE_INFO(info);
    info.pbNonce = const_cast<PUCHAR>(nonce.data());
    info.cbNonce = static_cast<ULONG>(nonce.size());
    info.pbTag = const_cast<PUCHAR>(tag.data());
    info.cbTag = static_cast<ULONG>(tag.size());
    ULONG written = 0;
    const NTSTATUS status = BCryptDecrypt(handle,
        const_cast<PUCHAR>(cipher.data()), static_cast<ULONG>(cipher.size()), &info, nullptr, 0,
        reinterpret_cast<PUCHAR>(plain.data()), static_cast<ULONG>(plain.size()), &written, 0);
    BCryptDestroyKey(handle);
    BCryptCloseAlgorithmProvider(provider, 0);
    if (!BCRYPT_SUCCESS(status)) { plain.clear(); return false; }  // etiqueta incorrecta
    return true;
#else
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    int length = 0;
    Tag expected = tag;
    const bool ok = ctx &&
        EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1 &&
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(nonce.size()), nullptr) == 1 &&
        EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.data(), nonce.data()) == 1 &&
        EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(plain.data()), &length,
                          cipher.data(), static_cast<int>(cipher.size())) == 1 &&
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, static_cast<int>(expected.size()), expected.data()) == 1 &&
        EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(plain.data()) + length, &length) == 1;
    EVP_CIPHER_CTX_free(ctx);
    if (!ok) plain.clear();
    return ok;
#endif
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
