// Pruebas de la contrasena maestra.
// Visual Studio: anadir auth.cpp y este archivo a un proyecto de consola y ejecutar.
// Linux: g++ -std=c++17 auth.cpp test_auth.cpp -o test_auth -lcrypto
#include "auth.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

static int fallos = 0;
static void comprobar(bool ok, const std::string& nombre) {
    std::cout << (ok ? "[OK]    " : "[FALLO] ") << nombre << '\n';
    if (!ok) ++fallos;
}

static std::string leerTodo(const std::string& archivo) {
    std::ifstream f(archivo);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

int main() {
    const std::string A = "test_master.dat", B = "test_master2.dat";
    const std::string BUENA = "Correcta123!";
    auth::guardarContrasenaMaestra(BUENA, A, 1000);  // pocas iteraciones: solo para que los tests vayan rapido

    comprobar(auth::verificarContrasenaMaestra(BUENA, A), "Contrasena correcta -> entra");
    comprobar(!auth::verificarContrasenaMaestra("correcta123!", A), "Mayusculas/minusculas importan");
    comprobar(!auth::verificarContrasenaMaestra("", A), "Contrasena vacia -> no entra");
    comprobar(leerTodo(A).find(BUENA) == std::string::npos, "No se guarda en texto plano");

    auth::guardarContrasenaMaestra(BUENA, B, 1000);
    comprobar(leerTodo(A) != leerTodo(B), "Misma contrasena, salt distinto -> hash distinto");

    comprobar(!auth::verificarContrasenaMaestra(BUENA, "no_existe.dat"), "Archivo inexistente -> no entra");

    { std::ofstream f(B); f << "basura"; }
    comprobar(!auth::verificarContrasenaMaestra(BUENA, B), "Archivo corrupto -> no entra");

    bool lanzada = false;
    try { auth::guardarContrasenaMaestra("corta", B); } catch (const std::invalid_argument&) { lanzada = true; }
    comprobar(lanzada, "Contrasena demasiado corta -> rechazada");

    std::vector<std::string> respuestas = {"mala", BUENA};
    std::size_t i = 0;
    std::string salida;
    bool ok = auth::pedirContrasenaMaestra(salida, A, [&](const std::string&) { return respuestas[i++]; });
    comprobar(ok && salida == BUENA, "Entra en el segundo intento");

    ok = auth::pedirContrasenaMaestra(salida, A, [](const std::string&) { return std::string("mala"); });
    comprobar(!ok, "3 intentos fallidos -> devuelve false");

    std::remove(A.c_str());
    std::remove(B.c_str());
    std::cout << '\n' << (fallos == 0 ? "TODAS LAS PRUEBAS OK" : "HAY PRUEBAS FALLIDAS") << '\n';
    return fallos == 0 ? 0 : 1;
}
