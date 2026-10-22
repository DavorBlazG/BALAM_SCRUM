cmake_minimum_required(VERSION 3.18)
project(BALAM VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(BALAM
    src/main.cpp
    src/console.cpp
    src/i18n.cpp
    src/crypto.cpp
    src/auth.cpp
    src/vault.cpp
    src/app.cpp
)
target_include_directories(BALAM PRIVATE src)

if(WIN32)
    target_compile_definitions(BALAM PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(BALAM PRIVATE bcrypt)
    if(MSVC)
        target_compile_options(BALAM PRIVATE /W4 /utf-8)
    endif()
else()
    find_package(OpenSSL REQUIRED COMPONENTS Crypto)
    target_link_libraries(BALAM PRIVATE OpenSSL::Crypto)
    target_compile_options(BALAM PRIVATE -Wall -Wextra -Wpedantic)
endif()
