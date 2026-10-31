#pragma once

#include <charconv>
#include <concepts>
#include <system_error>

#include "format_string.hpp"
#include "types.hpp"

namespace stdx::details {

// Шаблонная функция, возвращающая пару позиций в строке с исходными данными, соотвествующих I-ому плейсхолдеру
template <int I, format_string fmt, fixed_string source>
consteval auto get_current_source_for_parsing() {
    static_assert(I >= 0 &&                                   //
                      fmt.number_placeholders.has_value() &&  //
                      I < fmt.number_placeholders.value(),
                  "Invalid placeholder index");

    static_assert(fmt.placeholder_positions.has_value(), "Invalid placeholder positions");

    constexpr auto to_sv = [](const auto &fs) { return std::string_view(fs.data.data(), fs.size() - 1); };

    constexpr auto fmt_sv = to_sv(fmt.fmt);
    constexpr auto src_sv = to_sv(source);
    constexpr auto &positions = fmt.placeholder_positions.value();

    // Получаем границы текущего плейсхолдера в формате
    constexpr auto pos_i = positions[I];
    constexpr size_t fmt_start = pos_i.first, fmt_end = pos_i.second;

    // Находим начало в исходной строке
    constexpr auto src_start = [&] {
        if constexpr (I == 0) {
            return fmt_start;
        } else {
            // Находим конец предыдущего плейсхолдера в исходной строке
            constexpr auto prev_bounds = get_current_source_for_parsing<I - 1, fmt, source>();
            const auto prev_end = prev_bounds.second;

            // Получаем разделитель между текущим и предыдущим плейсхолдерами
            constexpr auto prev_fmt_end = positions[I - 1].second;
            constexpr auto sep = fmt_sv.substr(prev_fmt_end + 1, fmt_start - (prev_fmt_end + 1));

            // Ищем разделитель после предыдущего значения
            auto pos = src_sv.find(sep, prev_end);
            return pos != std::string_view::npos ? pos + sep.size() : src_sv.size();
        }
    }();

    // Находим конец в исходной строке
    constexpr auto src_end = [&] {
        // Получаем разделитель после текущего плейсхолдера
        if constexpr (fmt_end == (fmt_sv.size() - 1)) {
            return src_sv.size();
        }
        constexpr auto sep = fmt_sv.substr(fmt_end + 1, (I < fmt.number_placeholders.value() - 1)
                                                            ? positions[I + 1].first - (fmt_end + 1)
                                                            : fmt_sv.size() - (fmt_end + 1));
        // Ищем разделитель после текущего значения
        constexpr auto pos = src_sv.find(sep, src_start);
        return pos != std::string_view::npos ? pos : src_sv.size();
    }();
    return std::pair{src_start, src_end};
}

// Реализуйте семейство функция parse_value

// int
template <std::integral T, fixed_string value, fixed_string specifier = "">
    requires(!std::same_as<T, bool>)
consteval T parse_value() {
    constexpr std::string_view spec{specifier.data.data(), specifier.size() - 1};
    constexpr char expected = std::signed_integral<T> ? 'd' : 'u';

    static_assert(spec.empty() || (spec.size() == 2 && spec[0] == '%' && spec[1] == expected), "Error int format type");

    const char *first = value.data.data();
    const char *last = first + value.size() - 1;

    // @note: cos 'std::from_chars' can't work with '+'
    if (first != last && *first == '+') {
        ++first;
        if (first == last || *first == '+' || *first == '-') {
            throw "Invalid int";
        }
    }
    T result{};
    const auto parsed = std::from_chars(first, last, result, 10);

    if (parsed.ec == std::errc::result_out_of_range) {
        throw "Int is out of range";
    }
    if (parsed.ec != std::errc{} || parsed.ptr != last) {
        throw "Invalid int";
    }
    return result;
}

// string
template <typename T, fixed_string value, fixed_string specifier = "">
    requires std::same_as<T, std::string_view>
consteval T parse_value() {
    constexpr std::string_view spec{specifier.data.data(), specifier.size() - 1};
    static_assert(spec.empty() || spec == "%s", "Error string format type");

    return {value.data.data(), value.size() - 1};
}

// Шаблонная функция, выполняющая преобразования исходных данных в конкретный тип на основе I-го плейсхолдера
template <int I, format_string fmt, fixed_string source, typename T>
consteval auto parse_input() {
    static_assert(I >= 0 &&                                   //
                      fmt.number_placeholders.has_value() &&  //
                      I < fmt.number_placeholders.value(),
                  "Invalid placeholder index");

    static_assert(fmt.placeholder_positions.has_value(), "Invalid placeholder positions");

    // @note: value from source
    constexpr auto value = [] {
        constexpr auto bounds = get_current_source_for_parsing<I, fmt, source>();

        static_assert(bounds.first <= bounds.second && bounds.second < source.size(), "Invalid source bounds");

        constexpr auto length = bounds.second - bounds.first;
        using Char = typename decltype(source)::value_type;

        fixed_string<Char, length> result{};

        for (std::size_t i = 0; i < length; ++i) {
            result.data[i] = source.data[bounds.first + i];
        }
        result.data[length] = Char{};
        return result;
    }();

    // @note: get data from placeholders (exp. "" or "%d")
    constexpr auto specifier = [] {
        constexpr auto bounds = fmt.placeholder_positions.value()[I];

        // @note: remove '{' '}'
        constexpr auto length = bounds.second - bounds.first - 1;
        using Char = typename decltype(fmt.fmt)::value_type;

        fixed_string<Char, length> result{};

        for (std::size_t i = 0; i < length; ++i) {
            result.data[i] = fmt.fmt.data[bounds.first + 1 + i];
        }
        result.data[length] = Char{};
        return result;
    }();
    return parse_value<T, value, specifier>();
}

}  // namespace stdx::details
