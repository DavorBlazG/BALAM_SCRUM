#include "app.hpp"
#include <exception>
#include <iostream>

int main()
{
    try {
        app::Application program;
        return program.run();
    } catch (const std::exception& error) {
        std::cerr << "BALAM: error al iniciar / startup error: " << error.what() << '\n';
        std::cerr << "Comprueba el archivo local de autenticacion (no lo borres si necesitas acceder).\n";
        return 1;
    }
}
