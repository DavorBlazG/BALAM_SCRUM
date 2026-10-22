#include <iostream>

int main() {
    int opcion;

    std::cout << "=======================================\n";
    std::cout << " Selecciona un idioma / Select language \n";
    std::cout << "=======================================\n";
    std::cout << "1. Catala\n";
    std::cout << "2. Espanol\n";
    std::cout << "3. English\n";
    std::cout << "4. Norsk (Noruego)\n";
    std::cout << "Introduce el numero de tu eleccion: ";

    std::cin >> opcion;

    std::cout << "\n";

    switch (opcion) {
    case 1:
        std::cout << "Has seleccionat Catala. Benvingut!\n";
        break;
    case 2:
        std::cout << "Has seleccionado Espanol. ¡Bienvenido!\n";
        break;
    case 3:
        std::cout << "You have selected English. Welcome!\n";
        break;
    case 4:
        std::cout << "Du har valgt Norsk. Velkommen!\n";
        break;
    default:
        std::cout << "Opcion no válida. Por favor, selecciona un numero del 1 al 4.\n";
        break;
    }

    return 0;
}
