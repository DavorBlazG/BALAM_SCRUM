#pragma once
#include "crypto.hpp"
#include <cstdint>
#include <filesystem>
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

enum class OpenResult { Opened, Created, Damaged };

// Sprint 2: las entradas se guardan cifradas en disco (vault.dat).
// Formato del archivo:
//   "BLMV1" | iteraciones (4 bytes) | salt (16) | nonce (12) | etiqueta (16) | datos AES-256-GCM
// La clave se deriva de la contrasena maestra con PBKDF2 y un salt propio de la boveda.
class SessionVault {
public:
    SessionVault() = default;
    ~SessionVault();
    SessionVault(const SessionVault&) = delete;
    SessionVault& operator=(const SessionVault&) = delete;

    // Abre la boveda: si el archivo existe lo descifra; si no, prepara una nueva.
    // Damaged = contrasena incorrecta o archivo modificado/corrupto (no se sobrescribe).
    OpenResult open(const std::filesystem::path& file, std::string_view master,
                    std::uint32_t iterations = crypto::PBKDF2_ITERATIONS);
    // Cifra y guarda todas las entradas. Devuelve false si no se puede escribir.
    bool save() const;

    int add(std::string service, std::string username, std::string password);
    const std::vector<Entry>& all() const noexcept;
    const Entry* find(int id) const noexcept;
    bool update(int id, std::string service, std::string username, std::string password);
    bool erase(int id);
    std::vector<Entry> searchByService(std::string_view service) const;

    // Texto plano antes de cifrar / despues de descifrar (publicos para las pruebas).
    static std::string serialize(const std::vector<Entry>& entries);
    static bool deserialize(std::string_view text, std::vector<Entry>& entries);

private:
    int nextId_ = 1;
    std::vector<Entry> entries_;
    std::filesystem::path file_;
    crypto::Key key_{};
    crypto::Salt salt_{};
    std::uint32_t iterations_ = crypto::PBKDF2_ITERATIONS;
    bool opened_ = false;
};
}
