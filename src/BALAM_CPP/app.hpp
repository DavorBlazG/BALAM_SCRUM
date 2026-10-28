#pragma once
#include "auth.hpp"
#include "i18n.hpp"
#include "vault.hpp"
#include <optional>
#include <string>
#include <string_view>

namespace app {
class Application {
public:
    int run();
private:
    i18n::Language language_ = i18n::Language::Spanish;
    auth::Repository auth_;
    std::string tr(i18n::Message key) const;
    bool selectLanguage();
    void homeMenu();
    void registerAccount();
    void login();
    void enterWithMaster();
    void vaultMenu(std::string_view master);
    void vaultAdd(vault::SessionVault& session);
    void saveOrWarn(const vault::SessionVault& session);
    void vaultList(const vault::SessionVault& session) const;
    void vaultSearch(const vault::SessionVault& session) const;
    void vaultEdit(vault::SessionVault& session);
    void vaultDelete(vault::SessionVault& session);
    void vaultReveal(const vault::SessionVault& session) const;
    void vaultGenerate() const;
    std::optional<int> askEntryId() const;
};
}
