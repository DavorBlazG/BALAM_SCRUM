
#include "console.hpp"
#include <algorithm>
#include <iostream>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace console {

    namespace {

        bool ansi = false;

        // ======================================
        // PALETA DE COLORES BALAM
        // ======================================

        // FONDO PRINCIPAL - VERDE SELVA
        // RGB: 16, 31, 20
        // HEX: #101F14
        constexpr const char* FONDO_SELVA =
            "\x1b[48;2;16;31;20m";

        // COLORES DE TEXTO
        const char* ansiCode(Color color)
        {
            switch (color)
            {
            case Color::Gold:
                return "\x1b[38;2;230;194;128m";

            case Color::Green:
                return "\x1b[38;2;135;175;95m";

            case Color::Olive:
                return "\x1b[38;2;74;93;35m";

            case Color::Red:
                return "\x1b[38;2;235;100;100m";

            case Color::Muted:
                return "\x1b[38;2;169;169;169m";

            default:
                return "\x1b[38;2;229;231;235m";
            }
        }

#ifdef _WIN32
        std::string toUtf8(const std::wstring& w)
        {
            if (w.empty()) return {};

            const int len = WideCharToMultiByte(
                CP_UTF8,
                WC_ERR_INVALID_CHARS,
                w.data(),
                static_cast<int>(w.size()),
                nullptr,
                0,
                nullptr,
                nullptr
            );

            if (len == 0) return {};

            std::string out(
                static_cast<std::size_t>(len), '\0'
            );

            WideCharToMultiByte(
                CP_UTF8,
                WC_ERR_INVALID_CHARS,
                w.data(),
                static_cast<int>(w.size()),
                out.data(),
                len,
                nullptr,
                nullptr
            );

            return out;
        }
#endif

    }

    // ======================================
    // CONFIGURAR TERMINAL
    // ======================================

    void init()
    {
#ifdef _WIN32

        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        const HANDLE handle =
            GetStdHandle(STD_OUTPUT_HANDLE);

        DWORD mode = 0;

        if (handle != INVALID_HANDLE_VALUE &&
            GetConsoleMode(handle, &mode))
        {
            ansi = SetConsoleMode(
                handle,
                mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING
            ) != 0;
        }

#else
        ansi = isatty(STDOUT_FILENO) != 0;
#endif

        if (ansi)
        {
            // Activar fondo verde selva
            std::cout << FONDO_SELVA;

            // Limpiar la pantalla
            std::cout << "\x1b[2J";

            // Mover cursor al inicio
            std::cout << "\x1b[H";

            std::cout.flush();
        }
    }

    // ======================================
    // IMPRIMIR TEXTO CON COLOR
    // ======================================

    void print(std::string_view text, Color color)
    {
        if (ansi)
            std::cout << ansiCode(color);

        std::cout << text;

        // Restaurar solo el color del texto,
        // manteniendo el fondo verde.
        if (ansi)
            std::cout << "\x1b[39m";
    }

    void line(std::string_view text, Color color)
    {
        print(text, color);
        std::cout << '\n';
    }

    // ======================================
    // CABECERA BALAM
    // ======================================


    void banner()
    {
        // Línea superior
        line("====================================================", Color::Olive);

        // Logotipo ASCII de BALAM en dorado
        line("                  ┌──┐   ┌┐       ", Color::Gold);
        line("                  │┌┐│   ││       ", Color::Gold);
        line("                  │└┘└┬──┤│┌──┬┐┌┐", Color::Gold);
        line("                  │┌─┐│┌┐│││┌┐│└┘│", Color::Gold);
        line("                  │└─┘│┌┐│└┤┌┐││││", Color::Gold);
        line("                  └───┴┘└┴─┴┘└┴┴┴┘", Color::Gold);

        // Subtítulo
        line("", Color::Gold);
        line("                  PASSWORD MANAGER", Color::Gold);

        // Línea inferior
        line("====================================================", Color::Olive);
    }


    // ======================================
    // TITULOS DE SECCIONES
    // ======================================

    void section(std::string_view title)
    {
        std::cout << '\n';

        line(
            "----------------------------------------------------",
            Color::Olive
        );

        line(title, Color::Gold);

        line(
            "----------------------------------------------------",
            Color::Olive
        );
    }

    // ======================================
    // OPCIONES DEL MENU
    // ======================================

    void option(int number, std::string_view label)
    {
        print(
            "  [" + std::to_string(number) + "] ",
            Color::Gold
        );

        line(label);
    }

    // ======================================
    // MENSAJES
    // ======================================

    void error(std::string_view msg)
    {
        line(msg, Color::Red);
    }

    void success(std::string_view msg)
    {
        line(msg, Color::Green);
    }

    // ======================================
    // LEER DATOS
    // ======================================

    std::optional<std::string> ask(
        std::string_view prompt)
    {
        print(prompt, Color::Gold);

        std::string answer;

        if (!std::getline(std::cin, answer))
            return std::nullopt;

        return answer;
    }

    // ======================================
    // LEER CONTRASENA OCULTA
    // ======================================

    std::optional<std::string> askHidden(
        std::string_view prompt)
    {
        print(prompt, Color::Gold);
        std::cout.flush();

#ifdef _WIN32

        std::wstring entered;

        while (true)
        {
            const wint_t key = _getwch();

            if (key == 13 || key == 10)
                break;

            if (key == 3 || key == 26)
            {
                if (!entered.empty())
                {
                    SecureZeroMemory(
                        entered.data(),
                        entered.size() * sizeof(wchar_t)
                    );
                }

                std::cout << '\n';
                return std::nullopt;
            }

            if (key == 0 || key == 224)
            {
                (void)_getwch();
                continue;
            }

            if (key == 8)
            {
                if (!entered.empty())
                    entered.pop_back();

                continue;
            }

            if (key >= 32 && entered.size() < 256)
            {
                entered.push_back(
                    static_cast<wchar_t>(key)
                );
            }
        }

        std::string result = toUtf8(entered);

        if (!entered.empty())
        {
            SecureZeroMemory(
                entered.data(),
                entered.size() * sizeof(wchar_t)
            );
        }

        std::cout << '\n';

        return result;

#else

        termios oldSettings{};

        const bool hide =
            isatty(STDIN_FILENO) &&
            tcgetattr(STDIN_FILENO, &oldSettings) == 0;

        if (hide)
        {
            termios noEcho = oldSettings;

            noEcho.c_lflag &=
                static_cast<tcflag_t>(~ECHO);

            tcsetattr(
                STDIN_FILENO,
                TCSANOW,
                &noEcho
            );
        }

        std::string result;

        const bool ok =
            static_cast<bool>(
                std::getline(std::cin, result)
                );

        if (hide)
        {
            tcsetattr(
                STDIN_FILENO,
                TCSANOW,
                &oldSettings
            );

            std::cout << '\n';
        }

        if (!ok)
            return std::nullopt;

        return result;

#endif
    }

    // ======================================
    // LIMPIAR CONTRASENA DE MEMORIA
    // ======================================




    void wipe(std::string& secret)
    {
        if (!secret.empty())
        {
#ifdef _WIN32

            // Windows: limpiar la contraseña de memoria
            SecureZeroMemory(
                secret.data(),
                secret.size()
            );

#else

            // Linux y otros sistemas
            volatile char* p = secret.data();

            for (std::size_t i = 0;
                i < secret.size();
                ++i)
            {
                p[i] = 0;
            }

#endif

            secret.clear();
        }
    }


}
