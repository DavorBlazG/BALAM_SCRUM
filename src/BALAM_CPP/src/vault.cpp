#include "vault.hpp"
#include "console.hpp"
#include <algorithm>
#include <cctype>

namespace vault {
SessionVault::~SessionVault()
{
    for (auto& entry : entries_) console::wipe(entry.password);
}
int SessionVault::add(std::string service, std::string username, std::string password)
{
    const int id = nextId_++;
    entries_.push_back({id, std::move(service), std::move(username), std::move(password)});
    return id;
}
const std::vector<Entry>& SessionVault::all() const noexcept { return entries_; }
const Entry* SessionVault::find(int id) const noexcept
{
    const auto it = std::find_if(entries_.begin(), entries_.end(), [id](const Entry& e) { return e.id == id; });
    return it == entries_.end() ? nullptr : &*it;
}
bool SessionVault::update(int id, std::string service, std::string username, std::string password)
{
    for (auto& entry : entries_) {
        if (entry.id != id) continue;
        console::wipe(entry.password);
        entry.service = std::move(service);
        entry.username = std::move(username);
        entry.password = std::move(password);
        return true;
    }
    return false;
}
bool SessionVault::erase(int id)
{
    const auto it = std::find_if(entries_.begin(), entries_.end(), [id](const Entry& e) { return e.id == id; });
    if (it == entries_.end()) return false;
    console::wipe(it->password);
    entries_.erase(it);
    return true;
}
std::vector<Entry> SessionVault::searchByService(std::string_view service) const
{
    const auto lower = [](std::string_view text) {
        std::string out(text);
        for (char& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return out;
    };
    const std::string needle = lower(service);
    std::vector<Entry> matches;
    for (const auto& entry : entries_) {
        if (lower(entry.service).find(needle) != std::string::npos)
            matches.push_back({entry.id, entry.service, entry.username, ""});
    }
    return matches;
}
}
