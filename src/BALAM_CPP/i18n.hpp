#pragma once
#include <string_view>

namespace i18n {

enum class Language { Spanish = 0, Catalan = 1, English = 2, Norwegian = 3 };

enum class Message {
    SelectLanguage, ChooseLanguage, InvalidOption, Welcome, Register, Login,
    ChangeLanguage, Exit, ChooseOption, AccountExists, CreateAccount,
    Email, AccountPassword, ConfirmPassword, InvalidEmail, MinPassword,
    PasswordMismatch, AccountCreated, StorageError, LoginTitle, LoginFailed,
    LoginSuccess, AttemptsExhausted, MasterCreate, MasterUnlock, MasterPassword,
    ConfirmMaster, MasterCreated, MasterWrong, VaultTitle, VaultWarning,
    AddEntry, ShowEntries, SearchEntries, EditEntry, DeleteEntry, GeneratePassword,
    RevealEntry, LockVault, Service, Username, EntryPassword, EntryAdded,
    EntriesEmpty, EntryNumber, EntryMissing, EntryUpdated, EntryDeleted,
    ConfirmDelete, Cancelled, SearchQuery, NoResults, PasswordLength,
    GeneratedPassword, InvalidLength, NoRealSecrets, ExitThanks, LanguageChanged,
    GeneralError, AskAgain, ShowPassword, MinPasswordInfo,
    VaultDamaged, VaultSaveError
};

std::string_view text(Language language, Message key);
const char* languageName(Language language);
}
