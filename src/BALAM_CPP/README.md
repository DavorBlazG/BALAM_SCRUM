# BALAM — Gestor de contraseñas en C++ (terminal)

**Grupo 11 — Proyecto académico Scrum, 3 sprints**  
Davor Blaz (Scrum Master / Developer), Camilly Aoki y Gerard Conesa (Developers).

## Estado y alcance de la base

Prototipo modular para **Visual Studio 2022 / Windows**. Incluye:

- Paleta de colores ANSI inspirada en BALAM (verde selva, oliva y dorado).
- Cuatro idiomas: español, catalán, inglés y noruego bokmål.
- **Una única cuenta local**: registrar correo y contraseña, iniciar sesión.
- Contraseña maestra distinta de la cuenta: creación inicial y verificación posterior.
- Menú de bóveda con añadir, listar, buscar, editar, eliminar y mostrar entradas **solo en memoria**.
- Generador de contraseñas con RNG criptográfico del sistema y selección no sesgada.
- Autenticación persistente entre ejecuciones mediante archivo local de verificadores.

**NO es aún un gestor seguro para uso real.** Las entradas NO están cifradas ni guardadas permanentemente; al bloquear o salir se borran. Usar únicamente cuentas y contraseñas **ficticias**. No hay recuperación de credenciales, multiusuario, 2FA ni sincronización.

## Compilar en Visual Studio 2022

1. Instala la carga de trabajo **Desarrollo para el escritorio con C++** (MSVC y CMake).
2. Abre Visual Studio 2022 > **Archivo > Abrir > Carpeta** y elige la carpeta `BALAM_CPP`.
3. Visual Studio detectará `CMakeLists.txt`. Selecciona `BALAM.exe` como elemento de inicio si hace falta.
4. Compila y ejecuta, preferiblemente con **Windows Terminal** para visualizar los colores.

Desde **Developer PowerShell for VS 2022** también puedes usar:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
.\build\Debug\BALAM.exe
```

Si el binario aparece en otra carpeta, localiza `BALAM.exe` dentro de `build`.

**No requiere bibliotecas de terceros en Windows**: la autenticación utiliza la API CNG/BCrypt incluida en Windows. En Linux (opcional para pruebas) requiere OpenSSL y CMake.

## Primer uso

1. Selecciona el idioma.
2. Selecciona `Crear cuenta`, introduce un correo de prueba y una contraseña de cuenta (mínimo 8 caracteres).
3. Crea y confirma una contraseña maestra diferente (mínimo 8 caracteres).
4. Se desbloquea la bóveda. Prueba las operaciones con datos ficticios.
5. Al bloquear se pierde el contenido **temporal**. Al volver a abrir el programa puedes iniciar sesión y verificar la contraseña maestra anterior.

**Nota de seguridad:** el correo y la contraseña de cuenta son credenciales locales; el programa no envía ni verifica correos por Internet. La contraseña de cuenta y la maestra se guardan exclusivamente como **verificadores PBKDF2-HMAC-SHA256 con salt aleatoria e iteraciones**, NO como texto plano. PBKDF2 es una solución sin dependencias para esta primera versión, pero Argon2id sería preferible para un producto real. Un verificador de contraseña no sustituye al futuro cifrado de las entradas.

## Organización

```text
BALAM_CPP/
├── CMakeLists.txt
├── .gitignore
├── README.md
└── src/
    ├── main.cpp         Entrada del programa
    ├── app.hpp/.cpp     Menús y flujo de la aplicación
    ├── console.hpp/.cpp  Entrada oculta y colores ANSI
    ├── i18n.hpp/.cpp    Traducciones ES / CA / EN / NO
    ├── auth.hpp/.cpp    Cuenta local y contraseña maestra
    ├── crypto.hpp/.cpp  RNG seguro y verificadores PBKDF2
    └── vault.hpp/.cpp   Entradas temporales de la bóveda
```

## ¿Dónde están los datos?

Por defecto, Windows guarda los verificadores en `%LOCALAPPDATA%\BALAM\auth.dat`. El archivo contiene correo y verificadores; **no incluye las contraseñas originales**. No subirlo a GitHub ni borrarlo por accidente, ya que eso impediría acceder con las credenciales anteriores. Para pruebas automatizadas se puede definir `BALAM_DATA_DIR` a un directorio temporal.

## Próximos sprints

- **Sprint 2:** sustituir la bóveda en memoria por almacenamiento local **cifrado y autenticado** (p. ej. AES-256-GCM o XChaCha20-Poly1305 mediante biblioteca reconocida). Diseñar derivación de clave de bóveda y recuperación/gestión de errores. Idealmente añadir pruebas automatizadas.
- **Sprint 3:** mejorar las operaciones, validaciones, traducciones, documentación, integración y pruebas finales.

El alcance se replanifica en el Excel de Scrum de acuerdo con el trabajo realmente completado. La estructura modular permite reemplazar `vault.cpp` e integrar `storage.cpp` sin rehacer los menús.
