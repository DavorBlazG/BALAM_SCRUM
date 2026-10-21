// BALAM - Contrasena maestra
//
// Formato del archivo master.dat:
//   una sola linea:  <iteraciones> <salt_hex> <hash_hex>
//   ejemplo:         600000 9f2c...(32 car.) 4b1a...(64 car.)
// Hash = PBKDF2-HMAC-SHA256. Windows: API del sistema (bcrypt.h), sin instalar nada.
// Linux/Mac: OpenSSL. Nunca se guarda la contrasena en texto plano.
#pragma once
#include <functional>
#include <string>

namespace auth {

const std::string ARCHIVO_MAESTRA = "master.dat";
const int ITERACIONES = 600000;
const int MAX_INTENTOS = 3;
const std::size_t LONGITUD_MINIMA = 8;

bool existeContrasenaMaestra(const std::string& archivo = ARCHIVO_MAESTRA);

// ---- CREACION ----
// Lanza std::invalid_argument si es demasiado corta, std::runtime_error si no puede escribir.
void guardarContrasenaMaestra(const std::string& contrasena,
                              const std::string& archivo = ARCHIVO_MAESTRA,
                              int iteraciones = ITERACIONES);

// ---- VERIFICACION ----
// true si coincide; false si es incorrecta o el archivo no existe / esta corrupto.
bool verificarContrasenaMaestra(const std::string& contrasena,
                                const std::string& archivo = ARCHIVO_MAESTRA);

// Lee una contrasena sin mostrarla por pantalla.
std::string leerOculto(const std::string& mensaje);

// Pide la contrasena maestra hasta MAX_INTENTOS veces.
// Devuelve true y deja la contrasena en 'salida' si es correcta (la necesitara el cifrado).
// Devuelve false tras 3 fallos: quien la llama (el menu) decide cerrar el programa.
bool pedirContrasenaMaestra(std::string& salida,
                            const std::string& archivo = ARCHIVO_MAESTRA,
                            std::function<std::string(const std::string&)> entrada = leerOculto);

}  // namespace auth
