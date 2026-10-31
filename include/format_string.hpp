#pragma once

#include "types.hpp"
#include <array>
#include <expected>
#include <utility>

namespace stdx::details {

// Шаблонный класс для хранения форматирующей строчки и ее особенностей
template <fixed_string Text>
class format_string {
    // Функция для получения количества плейсхолдеров и проверки корректности формирующей строки
    template <fixed_string str>
    static consteval std::expected<std::size_t, parse_error<char>> get_number_placeholders() {
        constexpr std::size_t N = str.size();
        if (!N) {
            return 0;
        }
        std::size_t placeholder_count = 0;
        std::size_t pos = 0;
        const std::size_t size = N - 1;  // -1 для игнорирования нуль-терминатора

        while (pos < size) {
            // Пропускаем все символы до '{'
            if (str.data[pos] != '{') {
                ++pos;
                continue;
            }
            // Проверяем незакрытый плейсхолдер
            if (pos + 1 >= size) {
                return std::unexpected(parse_error<char>{"Unclosed last placeholder"});
            }
            // Начало плейсхолдера
            ++placeholder_count;
            ++pos;

            // Проверка спецификатора формата
            if (str.data[pos] == '%') {
                ++pos;
                if (pos >= size) {
                    return std::unexpected(parse_error<char>{"Unclosed last placeholder"});
                }
                // Проверяем допустимые спецификаторы
                // @note: delete 'f', cos "task said it to me" ("Ключевые функциональные требования")
                constexpr char valid_specs[] = {'d', 'u', 's'};
                const char spec = str.data[pos];
                bool valid = false;

                for (const char s : valid_specs) {
                    if (spec == s) {
                        valid = true;
                        break;
                    }
                }
                if (!valid) {
                    return std::unexpected(parse_error<char>{"Invalid specifier."});
                }
                ++pos;
            }
            // Проверяем закрывающую скобку
            if (pos >= size || str.data[pos] != '}') {
                return std::unexpected(parse_error<char>{"\'}\' hasn't been found in appropriate place"});
            }
            ++pos;
        }
        return placeholder_count;
    }

    template <std::size_t Count>
    static consteval std::expected<std::array<std::pair<std::size_t, std::size_t>, Count>, parse_error<char>>
    get_placeholder_positions() {
        constexpr std::size_t N = fmt.size();
        if (!N) {
            return {};
        }
        if (!Count) {
            return {};
        }
        std::array<std::pair<std::size_t, std::size_t>, Count> res;
        std::size_t placeholder_start = 0;
        std::size_t placeholder_count = 0;
        std::size_t pos = 0;
        const std::size_t size = N - 1;  // -1 для игнорирования нуль-терминатора

        while (pos < size) {
            // Пропускаем все символы до '{'
            if (fmt.data[pos] != '{') {
                ++pos;
                continue;
            }
            // Начало плейсхолдера
            placeholder_start = pos++;

            while (pos < size && fmt.data[pos] != '}') {
                ++pos;
            }
            if (pos == size) {
                return std::unexpected(parse_error<char>{"Unclosed last placeholder"});
            }

            if (placeholder_count >= Count) {
                return std::unexpected(parse_error<char>{"Too many placeholders"});
            }

            res[placeholder_count++] = std::make_pair(placeholder_start, pos);
            ++pos;
        }
        if (placeholder_count != Count) {
            return std::unexpected(parse_error<char>{"Placeholder count mismatch"});
        }
        return res;
    }

public:
    static constexpr auto fmt{Text};
    static constexpr auto number_placeholders{get_number_placeholders<fmt>()};
    static_assert(number_placeholders.has_value(), "Failed get_number_placeholders()");

    static constexpr auto placeholder_positions{get_placeholder_positions<number_placeholders.value()>()};
    static_assert(placeholder_positions.has_value(), "Failed get_placeholder_positions()");
};

// Пользовательский литерал
template <fixed_string str>
static consteval auto operator""_fs() {
    return format_string<str>{};
}

}  // namespace stdx::details
