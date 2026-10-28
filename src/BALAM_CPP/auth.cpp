#include "auth.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace auth {
namespace {
std::filesystem::path dataDirectory()
{
    if (const char* overridePath = std::getenv("BALAM_DATA_DIR"))
        if (*overridePath) return std::filesystem::path(overridePath);
#ifdef _WIN32
    DWORD size = GetEnvironmentVariableW(L"LOCALAPPDATA", nullptr, 0);
    if (size == 0) throw std::runtime_error("LOCALAPPDATA is unavailable");
    std::wstring buffer(size, L'\0');
    DWORD written = GetEnvironmentVariableW(L"LOCALAPPDATA", buffer.data(), size);
    if (written == 0 || written >= size) throw std::runtime_error("LOCALAPPDATA is unavailable");
    buffer.resize(written);
    return std::filesystem::path(buffer) / "BALAM";
#else
    if (const char* xdg = std::getenv("XDG_DATA_HOME"))
        if (*xdg) return std::filesystem::path(xdg) / "BALAM";
    if (const char* home = std::getenv("HOME"))
        return std::filesystem::path(home) / ".local" / "share" / "BALAM";
    throw std::runtime_error("HOME is unavailable");
#endif
}
std::string lowercaseAscii(std::string_view text)
{
    std::string result(text);
    for (char& c : result) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return result;
}
template<std::size_t N>
std::array<std::uint8_t, N> readFixedHex(const std::string& encoded)
{
    auto bytes = crypto::unhex(encoded);
    if (bytes.size() != N) throw std::runtime_error("Corrupted authentication database");
    std::array<std::uint8_t, N> out{};
    std::copy(bytes.begin(), bytes.end(), out.begin());
    return out;
}
std::string nextLine(std::ifstream& stream)
{
    std::string line;
    if (!std::getline(stream, line)) throw std::runtime_error("Incomplete authentication database");
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return line;
}
}
Repository::Repository() : path_(dataDirectory() / "auth.dat") { load(); }
bool Repository::hasAccount() const noexcept { return accountSet_; }
bool Repository::hasMaster() const noexcept { return accountSet_ && record_.masterSet; }
const std::filesystem::path& Repository::storagePath() const noexcept { return path_; }

bool Repository::validEmail(std::string_view value)
{
    if (value.empty() || value.size() > 254) return false;
    const auto at = value.find('@');
    if (at == 0 || at == std::string_view::npos || at != value.rfind('@')) return false;
    const auto dot = value.find('.', at + 2);
    if (dot == std::string_view::npos || dot + 1 >= value.size()) return false;
    for (char c : value) {
        if (static_cast<unsigned char>(c) <= 32 || static_cast<unsigned char>(c) >= 127)
            return false;
    }
    return true;
}

void Repository::load()
{
    if (!std::filesystem::exists(path_)) return;
    std::ifstream file(path_, std::ios::binary);
    if (!file) throw std::runtime_error("Authentication storage cannot be opened");
    if (nextLine(file) != "BALAM_AUTH_V1")
        throw std::runtime_error("Unsupported or damaged authentication file");
    Record loaded;
    const std::string iter = nextLine(file);
    const auto parsed = std::stoul(iter);
    if (parsed < 100000 || parsed > 2000000)
        throw std::runtime_error("Invalid KDF iterations");
    loaded.iterations = static_cast<std::uint32_t>(parsed);
    const auto emailBytes = crypto::unhex(nextLine(file));
    loaded.email.assign(emailBytes.begin(), emailBytes.end());
    if (!validEmail(loaded.email)) throw std::runtime_error("Invalid stored email");
    loaded.accountSalt = readFixedHex<crypto::SALT_SIZE>(nextLine(file));
    loaded.accountVerifier = readFixedHex<crypto::HASH_SIZE>(nextLine(file));
    const std::string hasMaster = nextLine(file);
    if (hasMaster != "0" && hasMaster != "1")
        throw std::runtime_error("Invalid master status");
    loaded.masterSet = hasMaster == "1";
    const std::string ms = nextLine(file);
    const std::string mv = nextLine(file);
    if (loaded.masterSet) {
        loaded.masterSalt = readFixedHex<crypto::SALT_SIZE>(ms);
        loaded.masterVerifier = readFixedHex<crypto::HASH_SIZE>(mv);
    } else if (ms != "-" || mv != "-") {
        throw std::runtime_error("Invalid master data");
    }
    record_ = loaded;
    accountSet_ = true;
}

void Repository::save(const Record& record) const
{
    std::filesystem::create_directories(path_.parent_path());
    auto temporary = path_;
    temporary += ".tmp";
    {
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        if (!file) throw std::runtime_error("Cannot write authentication data");
        file << "BALAM_AUTH_V1\n" << record.iterations << '\n';
        file << crypto::hex(reinterpret_cast<const std::uint8_t*>(record.email.data()), record.email.size()) << '\n';
        file << crypto::hex(record.accountSalt.data(), record.accountSalt.size()) << '\n';
        file << crypto::hex(record.accountVerifier.data(), record.accountVerifier.size()) << '\n';
        file << (record.masterSet ? "1" : "0") << '\n';
        if (record.masterSet) {
            file << crypto::hex(record.masterSalt.data(), record.masterSalt.size()) << '\n';
            file << crypto::hex(record.masterVerifier.data(), record.masterVerifier.size()) << '\n';
        } else file << "-\n-\n";
        file.flush();
        if (!file) throw std::runtime_error("Failed to flush authentication data");
    }
#ifdef _WIN32
    if (!MoveFileExW(temporary.c_str(), path_.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        throw std::runtime_error("Failed to replace authentication file");
#else
    ::chmod(temporary.c_str(), S_IRUSR | S_IWUSR);
    std::filesystem::rename(temporary, path_);
#endif
}

bool Repository::createAccount(std::string email, std::string_view password)
{
    if (accountSet_ || !validEmail(email) || password.size() < 8) return false;
    Record next;
    next.email = lowercaseAscii(email);
    next.accountSalt = crypto::makeSalt();
    next.accountVerifier = crypto::deriveVerifier(password, next.accountSalt, next.iterations);
    save(next);
    record_ = std::move(next);
    accountSet_ = true;
    return true;
}
bool Repository::verifyAccount(std::string_view email, std::string_view password) const
{
    if (!accountSet_ || lowercaseAscii(email) != record_.email) return false;
    auto supplied = crypto::deriveVerifier(password, record_.accountSalt, record_.iterations);
    const bool ok = crypto::equalConstantTime(supplied, record_.accountVerifier);
    volatile std::uint8_t* wipe = supplied.data();
    for (std::size_t i = 0; i < supplied.size(); ++i) wipe[i] = 0;
    return ok;
}
bool Repository::createMaster(std::string_view password)
{
    if (!accountSet_ || record_.masterSet || password.size() < 8) return false;
    Record next = record_;
    next.masterSalt = crypto::makeSalt();
    next.masterVerifier = crypto::deriveVerifier(password, next.masterSalt, next.iterations);
    next.masterSet = true;
    save(next);
    record_ = std::move(next);
    return true;
}
bool Repository::verifyMaster(std::string_view password) const
{
    if (!hasMaster()) return false;
    auto supplied = crypto::deriveVerifier(password, record_.masterSalt, record_.iterations);
    const bool ok = crypto::equalConstantTime(supplied, record_.masterVerifier);
    volatile std::uint8_t* wipe = supplied.data();
    for (std::size_t i = 0; i < supplied.size(); ++i) wipe[i] = 0;
    return ok;
}
}
