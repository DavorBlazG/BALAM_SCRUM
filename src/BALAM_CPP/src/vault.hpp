#pragma once
#include "crypto.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace vault {
struct Entry {
    int id = 0;
    std::string service;
    std::string username;
    std::string password;
};
class SessionVault {
public:
    SessionVault() = default;
    ~SessionVault();
    SessionVault(const SessionVault&) = delete;
    SessionVault& operator=(const SessionVault&) = delete;
    int add(std::string service, std::string username, std::string password);
    const std::vector<Entry>& all() const noexcept;
    const Entry* find(int id) const noexcept;
    bool update(int id, std::string service, std::string username, std::string password);
    bool erase(int id);
    std::vector<Entry> searchByService(std::string_view service) const;
private:
    int nextId_ = 1;
    std::vector<Entry> entries_;
};
}
