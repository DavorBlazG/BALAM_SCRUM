#include "app.hpp"
#include "console.hpp"
#include "crypto.hpp"
#include <charconv>
#include <iostream>
#include <string_view>

namespace app {
using M = i18n::Message;
namespace {
int readInteger(std::string_view text)
{
    int number = -1;
    const auto* begin = text.data();
    const auto* end = begin + text.size();
    const auto [ptr, error] = std::from_chars(begin, end, number);
    if (error != std::errc{} || ptr != end) return -1;
    return number;
}
}
std::string Application::tr(M key) const
{
    return std::string(i18n::text(language_, key));
}
int Application::run()
{
    console::init();
    console::banner();
    if (!selectLanguage()) return 0;
    homeMenu();
    console::success(tr(M::ExitThanks));
    return 0;
}
bool Application::selectLanguage()
{
    while (true) {
        console::section(tr(M::SelectLanguage));
        console::option(1, "Español");
        console::option(2, "Català");
        console::option(3, "English");
        console::option(4, "Norsk bokmål");
        const auto selection = console::ask(tr(M::ChooseLanguage));
        if (!selection) return false;
        const int choice = readInteger(*selection);
        if (choice >= 1 && choice <= 4) {
            language_ = static_cast<i18n::Language>(choice - 1);
            console::success(tr(M::LanguageChanged));
            return true;
        }
        console::error(tr(M::InvalidOption));
    }
}
void Application::homeMenu()
{
    while (true) {
        console::section(tr(M::Welcome));
        console::option(1, tr(M::Register));
        console::option(2, tr(M::Login));
        console::option(3, tr(M::ChangeLanguage));
        console::option(0, tr(M::Exit));
        const auto choice = console::ask(tr(M::ChooseOption));
        if (!choice || *choice == "0") break;
        if (*choice == "1") registerAccount();
        else if (*choice == "2") login();
        else if (*choice == "3") { if (!selectLanguage()) break; }
        else console::error(tr(M::InvalidOption));
    }
}
void Application::registerAccount()
{
    if (auth_.hasAccount()) {
        console::error(tr(M::AccountExists));
        return;
    }
    console::section(tr(M::CreateAccount));
    const auto email = console::ask(tr(M::Email));
    if (!email) return;
    if (!auth::Repository::validEmail(*email)) {
        console::error(tr(M::InvalidEmail));
        return;
    }
    console::line(tr(M::MinPasswordInfo), console::Color::Muted);
    auto password = console::askHidden(tr(M::AccountPassword));
    if (!password) return;
    if (password->size() < 8) {
        console::wipe(*password);
        console::error(tr(M::MinPassword));
        return;
    }
    auto confirmation = console::askHidden(tr(M::ConfirmPassword));
    if (!confirmation) { console::wipe(*password); return; }
    const bool same = *confirmation == *password;
    console::wipe(*confirmation);
    if (!same) {
        console::wipe(*password);
        console::error(tr(M::PasswordMismatch));
        return;
    }
    try {
        const bool created = auth_.createAccount(*email, *password);
        console::wipe(*password);
        if (!created) { console::error(tr(M::GeneralError)); return; }
    } catch (...) {
        console::wipe(*password);
        console::error(tr(M::StorageError));
        return;
    }
    console::success(tr(M::AccountCreated));
    enterWithMaster();
}
void Application::login()
{
    console::section(tr(M::LoginTitle));
    if (!auth_.hasAccount()) {
        console::error(tr(M::CreateAccount));
        return;
    }
    for (int attempt = 0; attempt < 3; ++attempt) {
        const auto email = console::ask(tr(M::Email));
        if (!email) return;
        auto password = console::askHidden(tr(M::AccountPassword));
        if (!password) return;
        bool ok = false;
        try { ok = auth_.verifyAccount(*email, *password); }
        catch (...) { console::wipe(*password); console::error(tr(M::StorageError)); return; }
        console::wipe(*password);
        if (ok) {
            console::success(tr(M::LoginSuccess));
            enterWithMaster();
            return;
        }
        console::error(tr(M::LoginFailed));
    }
    console::error(tr(M::AttemptsExhausted));
}
void Application::enterWithMaster()
{
    if (!auth_.hasMaster()) {
        console::section(tr(M::MasterCreate));
        console::line(tr(M::MinPasswordInfo), console::Color::Muted);
        auto password = console::askHidden(tr(M::MasterPassword));
        if (!password) return;
        if (password->size() < 8) {
            console::wipe(*password);
            console::error(tr(M::MinPassword));
            return;
        }
        auto confirmation = console::askHidden(tr(M::ConfirmMaster));
        if (!confirmation) { console::wipe(*password); return; }
        const bool same = *password == *confirmation;
        console::wipe(*confirmation);
        if (!same) {
            console::wipe(*password);
            console::error(tr(M::PasswordMismatch));
            return;
        }
        try {
            const bool saved = auth_.createMaster(*password);
            console::wipe(*password);
            if (!saved) { console::error(tr(M::StorageError)); return; }
        } catch (...) {
            console::wipe(*password);
            console::error(tr(M::StorageError));
            return;
        }
        console::success(tr(M::MasterCreated));
        vaultMenu();
        return;
    }
    console::section(tr(M::MasterUnlock));
    for (int attempt = 0; attempt < 3; ++attempt) {
        auto password = console::askHidden(tr(M::MasterPassword));
        if (!password) return;
        bool matched = false;
        try { matched = auth_.verifyMaster(*password); }
        catch (...) { console::wipe(*password); console::error(tr(M::StorageError)); return; }
        console::wipe(*password);
        if (matched) { vaultMenu(); return; }
        console::error(tr(M::MasterWrong));
    }
    console::error(tr(M::AttemptsExhausted));
}
void Application::vaultMenu()
{
    // Por ahora las entradas solo viven en esta sesion: bloqueo => destruccion de SessionVault.
    vault::SessionVault session;
    console::section(tr(M::VaultTitle));
    console::line(tr(M::VaultWarning), console::Color::Red);
    console::line(tr(M::NoRealSecrets), console::Color::Muted);
    while (true) {
        console::section(tr(M::VaultTitle));
        console::option(1, tr(M::AddEntry));
        console::option(2, tr(M::ShowEntries));
        console::option(3, tr(M::SearchEntries));
        console::option(4, tr(M::EditEntry));
        console::option(5, tr(M::DeleteEntry));
        console::option(6, tr(M::GeneratePassword));
        console::option(7, tr(M::RevealEntry));
        console::option(8, tr(M::ChangeLanguage));
        console::option(0, tr(M::LockVault));
        const auto choice = console::ask(tr(M::ChooseOption));
        if (!choice || *choice == "0") return;
        if (*choice == "1") vaultAdd(session);
        else if (*choice == "2") vaultList(session);
        else if (*choice == "3") vaultSearch(session);
        else if (*choice == "4") vaultEdit(session);
        else if (*choice == "5") vaultDelete(session);
        else if (*choice == "6") vaultGenerate();
        else if (*choice == "7") vaultReveal(session);
        else if (*choice == "8") { if (!selectLanguage()) return; }
        else console::error(tr(M::InvalidOption));
    }
}
void Application::vaultAdd(vault::SessionVault& session)
{
    console::section(tr(M::AddEntry));
    const auto service = console::ask(tr(M::Service));
    if (!service || service->empty()) { console::error(tr(M::InvalidOption)); return; }
    const auto username = console::ask(tr(M::Username));
    if (!username || username->empty()) { console::error(tr(M::InvalidOption)); return; }
    auto password = console::askHidden(tr(M::EntryPassword));
    if (!password || password->empty()) { console::error(tr(M::InvalidOption)); return; }
    session.add(*service, *username, std::move(*password));
    console::success(tr(M::EntryAdded));
}
void Application::vaultList(const vault::SessionVault& session) const
{
    console::section(tr(M::ShowEntries));
    if (session.all().empty()) { console::line(tr(M::EntriesEmpty), console::Color::Muted); return; }
    for (const auto& entry : session.all()) {
        console::print("  [" + std::to_string(entry.id) + "] ", console::Color::Gold);
        console::line(entry.service + " | " + entry.username + " | ********");
    }
}
void Application::vaultSearch(const vault::SessionVault& session) const
{
    const auto query = console::ask(tr(M::SearchQuery));
    if (!query) return;
    const auto results = session.searchByService(*query);
    if (results.empty()) { console::line(tr(M::NoResults), console::Color::Muted); return; }
    for (const auto& item : results) {
        console::print("  [" + std::to_string(item.id) + "] ", console::Color::Gold);
        console::line(item.service + " | " + item.username);
    }
}
std::optional<int> Application::askEntryId() const
{
    const auto source = console::ask(tr(M::EntryNumber));
    if (!source) return std::nullopt;
    const int id = readInteger(*source);
    if (id <= 0) { console::error(tr(M::InvalidOption)); return std::nullopt; }
    return id;
}
void Application::vaultEdit(vault::SessionVault& session)
{
    vaultList(session);
    if (session.all().empty()) return;
    const auto id = askEntryId();
    if (!id) return;
    if (!session.find(*id)) { console::error(tr(M::EntryMissing)); return; }
    const auto service = console::ask(tr(M::Service));
    if (!service || service->empty()) return;
    const auto username = console::ask(tr(M::Username));
    if (!username || username->empty()) return;
    auto password = console::askHidden(tr(M::EntryPassword));
    if (!password || password->empty()) return;
    if (session.update(*id, *service, *username, std::move(*password)))
        console::success(tr(M::EntryUpdated));
}
void Application::vaultDelete(vault::SessionVault& session)
{
    vaultList(session);
    if (session.all().empty()) return;
    const auto id = askEntryId();
    if (!id) return;
    if (!session.find(*id)) { console::error(tr(M::EntryMissing)); return; }
    const auto confirm = console::ask(tr(M::ConfirmDelete));
    if (!confirm) return;
    const bool accepted = *confirm == "s" || *confirm == "S" ||
                          *confirm == "y" || *confirm == "Y" ||
                          *confirm == "j" || *confirm == "J";
    if (!accepted) { console::line(tr(M::Cancelled), console::Color::Muted); return; }
    if (session.erase(*id)) console::success(tr(M::EntryDeleted));
}
void Application::vaultReveal(const vault::SessionVault& session) const
{
    vaultList(session);
    if (session.all().empty()) return;
    const auto id = askEntryId();
    if (!id) return;
    const auto entry = session.find(*id);
    if (!entry) { console::error(tr(M::EntryMissing)); return; }
    console::print(tr(M::ShowPassword), console::Color::Gold);
    console::line(entry->password);
}
void Application::vaultGenerate() const
{
    const auto value = console::ask(tr(M::PasswordLength));
    if (!value) return;
    const int length = readInteger(*value);
    if (length < 8 || length > 64) { console::error(tr(M::InvalidLength)); return; }
    std::string generated = crypto::generatePassword(static_cast<std::size_t>(length));
    console::print(tr(M::GeneratedPassword), console::Color::Gold);
    console::line(generated, console::Color::Green);
    console::wipe(generated);
}
}
