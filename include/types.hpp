#pragma once

#include <algorithm>
#include <array>
#include <iterator>
#include <string>
#include <tuple>
#include <utility>

namespace stdx::details {

// @note: from 'get_number_placeholders' func
// @todo: mb hardcode is stupid, think
constexpr std::size_t MAX_PARSE_ERROR_SYMBOLS{50};

// Шаблонный класс, хранящий C-style строку фиксированной длины
template <typename Char, std::size_t N>
struct fixed_string {
    using value_type = Char;
    using storage_type = std::array<value_type, N + 1>;
    storage_type data{};

    constexpr fixed_string() noexcept = default;
    constexpr fixed_string(const value_type (&array)[N + 1]) noexcept {  //
        std::copy(std::begin(array), std::end(array), data.begin());
    }
    template <std::size_t M>
    constexpr fixed_string(const value_type (&array)[M]) noexcept {  //
        static_assert(M - 1 <= N, "Incorrect size of array");
        std::copy_n(std::begin(array), M, data.begin());
    }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return data.size(); }
};

// @note: fixing literal
template <typename Char, std::size_t M>
fixed_string(const Char (&)[M]) -> fixed_string<Char, M - 1>;

// Шаблонный класс, хранящий fixed_string достаточной длины для хранения ошибки парсинга
template <typename Char>
struct parse_error : public fixed_string<Char, MAX_PARSE_ERROR_SYMBOLS> {};

// Шаблонный класс для хранения результатов парсинга
template <typename... Ts>
struct scan_result {
    std::tuple<Ts...> values;

    template <typename... Args>
    constexpr scan_result(Args &&...args) : values(std::forward<Args>(args)...) {}
};

}  // namespace stdx::details
