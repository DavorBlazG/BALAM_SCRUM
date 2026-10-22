#pragma once
#include <optional>
#include <string>
#include <string_view>

namespace console {
enum class Color { Gold, Green, Olive, White, Red, Muted };
void init();
void print(std::string_view text, Color color = Color::White);
void line(std::string_view text = {}, Color color = Color::White);
void banner();
void section(std::string_view title);
void option(int number, std::string_view label);
void error(std::string_view msg);
void success(std::string_view msg);
std::optional<std::string> ask(std::string_view prompt);
std::optional<std::string> askHidden(std::string_view prompt);
void wipe(std::string& secret);
}
