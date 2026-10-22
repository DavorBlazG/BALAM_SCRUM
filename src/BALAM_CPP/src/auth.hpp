#pragma once
#include "crypto.hpp"
#include <filesystem>
#include <string>
#include <string_view>

namespace auth {
class Repository {
public:
    Repository();
    bool hasAccount() const noexcept;
    bool hasMaster() const noexcept;
    bool createAccount(std::string email, std::string_view password);
    bool verifyAccount(std::string_view email, std::string_view password) const;
    bool createMaster(std::string_view password);
    bool verifyMaster(std::string_view password) const;
    static bool validEmail(std::string_view value);
    const std::filesystem::path& storagePath() const noexcept;
private:
    struct Record {
        std::string email;
        std::uint32_t iterations = crypto::PBKDF2_ITERATIONS;
        crypto::Salt accountSalt{};
        crypto::Digest accountVerifier{};
        bool masterSet = false;
        crypto::Salt masterSalt{};
        crypto::Digest masterVerifier{};
    };
    std::filesystem::path path_;
    Record record_;
    bool accountSet_ = false;
    void load();
    void save(const Record& record) const;
};
}
