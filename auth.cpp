#include "auth.h"

#ifdef _WIN32
// Windows: API criptografica del sistema (CNG). No hace falta instalar nada.
#include <windows.h>
#include <bcrypt.h>
#include <conio.h>
#pragma comment(lib, "bcrypt.lib")  // Visual Studio enlaza la biblioteca solo
#else
// Linux/Mac: OpenSSL (-lcrypto)
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <termios.h>
#include <unistd.h>
#endif

#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace auth {
namespace {

using Bytes = std::vector<unsigned char>;

std::string aHex(const Bytes& b) {
    static const char* d = "0123456789abcdef";
    std::string s;
    for (unsigned char c : b) { s += d[c >> 4]; s += d[c & 0xF]; }
    return s;
}

bool deHex(const std::string& s, Bytes& salida) {
    if (s.size() % 2 != 0) return false;
    salida.clear();
    for (std::size_t i = 0; i < s.size(); i += 2) {
        unsigned int v;
        if (std::sscanf(s.substr(i, 2).c_str(), "%2x", &v) != 1) return false;
        salida.push_back(static_cast<unsigned char>(v));
    }
    return true;
}

Bytes calcularHash(const std::string& contrasena, const Bytes& salt, int iteraciones) {
    Bytes hash(32);
#ifdef _WIN32
    BCRYPT_ALG_HANDLE alg = nullptr;
    if (BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr,
                                    BCRYPT_ALG_HANDLE_HMAC_FLAG) != 0)
        throw std::runtime_error("Error abriendo SHA-256");
    NTSTATUS st = BCryptDeriveKeyPBKDF2(
        alg, (PUCHAR)contrasena.data(), (ULONG)contrasena.size(),
        (PUCHAR)salt.data(), (ULONG)salt.size(), (ULONGLONG)iteraciones,
        hash.data(), (ULONG)hash.size(), 0);
    BCryptCloseAlgorithmProvider(alg, 0);
    if (st != 0) throw std::runtime_error("Error calculando el hash");
#else
    if (PKCS5_PBKDF2_HMAC(contrasena.c_str(), static_cast<int>(contrasena.size()),
                          salt.data(), static_cast<int>(salt.size()), iteraciones,
                          EVP_sha256(), static_cast<int>(hash.size()), hash.data()) != 1)
        throw std::runtime_error("Error calculando el hash");
#endif
    return hash;
}

void bytesAleatorios(Bytes& b) {
#ifdef _WIN32
    if (BCryptGenRandom(nullptr, b.data(), (ULONG)b.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0)
#else
    if (RAND_bytes(b.data(), static_cast<int>(b.size())) != 1)
#endif
        throw std::runtime_error("No se ha podido generar el salt");
}

// Comparacion en tiempo constante: no se detiene en el primer byte distinto
// (evita ataques de temporizacion)
bool igualesTiempoConstante(const Bytes& a, const Bytes& b) {
    if (a.size() != b.size()) return false;
    unsigned char dif = 0;
    for (std::size_t i = 0; i < a.size(); ++i) dif |= a[i] ^ b[i];
    return dif == 0;
}

}  // namespace

bool existeContrasenaMaestra(const std::string& archivo) {
    std::ifstream f(archivo);
    return f.good();
}

void guardarContrasenaMaestra(const std::string& contrasena, const std::string& archivo, int iteraciones) {
    if (contrasena.size() < LONGITUD_MINIMA)
        throw std::invalid_argument("La contrasena maestra debe tener minimo 8 caracteres.");
    Bytes salt(16);
    bytesAleatorios(salt);
    std::ofstream f(archivo, std::ios::trunc);
    if (!f) throw std::runtime_error("No se ha podido escribir " + archivo);
    f << iteraciones << ' ' << aHex(salt) << ' ' << aHex(calcularHash(contrasena, salt, iteraciones)) << '\n';
}

bool verificarContrasenaMaestra(const std::string& contrasena, const std::string& archivo) {
    std::ifstream f(archivo);
    if (!f) return false;  // no existe: no dejamos entrar

    int iteraciones = 0;
    std::string saltHex, hashHex;
    if (!(f >> iteraciones >> saltHex >> hashHex) || iteraciones <= 0) return false;  // corrupto

    Bytes salt, esperado;
    if (!deHex(saltHex, salt) || !deHex(hashHex, esperado) || esperado.size() != 32) return false;

    return igualesTiempoConstante(calcularHash(contrasena, salt, iteraciones), esperado);
}

std::string leerOculto(const std::string& mensaje) {
    std::cout << mensaje << std::flush;
    std::string s;
#ifdef _WIN32
    for (int c = _getch(); c != '\r' && c != '\n'; c = _getch()) {
        if (c == '\b') { if (!s.empty()) s.pop_back(); }
        else s += static_cast<char>(c);
    }
#else
    termios anterior{};
    tcgetattr(STDIN_FILENO, &anterior);
    termios nuevo = anterior;
    nuevo.c_lflag &= ~ECHO;  // desactiva el eco
    tcsetattr(STDIN_FILENO, TCSANOW, &nuevo);
    std::getline(std::cin, s);
    tcsetattr(STDIN_FILENO, TCSANOW, &anterior);
#endif
    std::cout << '\n';
    return s;
}

bool pedirContrasenaMaestra(std::string& salida, const std::string& archivo,
                            std::function<std::string(const std::string&)> entrada) {
    if (!existeContrasenaMaestra(archivo)) {
        std::cout << "No hay ninguna contrasena maestra creada.\n";
        return false;
    }
    for (int intento = 1; intento <= MAX_INTENTOS; ++intento) {
        std::string c = entrada("Contrasena maestra: ");
        if (verificarContrasenaMaestra(c, archivo)) {
            std::cout << "Acceso concedido.\n";
            salida = c;
            return true;
        }
        int restantes = MAX_INTENTOS - intento;
        if (restantes > 0) std::cout << "Contrasena incorrecta. Te quedan " << restantes << " intentos.\n";
    }
    std::cout << "Demasiados intentos fallidos.\n";
    return false;
}

}  // namespace auth
