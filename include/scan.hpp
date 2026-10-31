#pragma once

#include "format_string.hpp"
#include "parse.hpp"
#include "types.hpp"

namespace stdx {

// Главная функция
template <details::format_string fmt, details::fixed_string source, typename... Ts>
consteval details::scan_result<Ts...> scan() {
    static_assert(sizeof...(Ts) == fmt.number_placeholders, "Number of types != number of placeholders");

    return []<std::size_t... Is>(std::index_sequence<Is...>) consteval {
        return details::scan_result<Ts...>{details::parse_input<static_cast<int>(Is), fmt, source, Ts>()...};
    }(std::index_sequence_for<Ts...>{});
}

}  // namespace stdx