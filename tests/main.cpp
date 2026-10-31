#include "scan.hpp"
#include <cstdint>
#include <limits>

using namespace stdx;
using details::operator""_fs;

int main() {
    constexpr auto test_0 = scan<"id={%d}"_fs, "id=-42", int>();
    static_assert((std::get<0>(test_0.values) == -42), "signed_number");

    constexpr auto test_1 = scan<"n={%u}"_fs, "n=17", unsigned>();
    static_assert((std::get<0>(test_1.values) == 17u), "unsigned_number");

    constexpr auto test_2 = scan<"name={%s}"_fs, "name=Viktor", std::string_view>();
    static_assert((std::get<0>(test_2.values) == "Viktor"), "string_value");

    constexpr auto test_3 =
        scan<"id={%d};count={%u};name={%s}"_fs, "id=-7;count=12;name=Ivan", int, unsigned, std::string_view>();
    static_assert((std::get<0>(test_3.values) == -7) && (std::get<1>(test_3.values) == 12u) &&
                      (std::get<2>(test_3.values) == "Ivan"),
                  "mixed_types");

    constexpr auto test_4 = scan<"{}|{}|{}"_fs, "-7|12|Ivan", int, unsigned, std::string_view>();
    static_assert((std::get<0>(test_4.values) == -7) && (std::get<1>(test_4.values) == 12u) &&
                      (std::get<2>(test_4.values) == "Ivan"),
                  "inferred_specifiers");

    constexpr auto test_5 = scan<"{%d},{%d},{%d}"_fs, "1,2,3", int, int, int>();
    static_assert((std::get<0>(test_5.values) == 1) && (std::get<1>(test_5.values) == 2) &&
                      (std::get<2>(test_5.values) == 3),
                  "repeated_types");

    constexpr auto test_6 = scan<"result:({%d})!"_fs, "result:(7)!", int>();
    static_assert((std::get<0>(test_6.values) == 7), "literal_prefix_suffix");

    constexpr auto test_7 = scan<"Hello {%s}!"_fs, "Hello Viktor Alexandrov!", std::string_view>();
    static_assert((std::get<0>(test_7.values) == "Viktor Alexandrov"), "string_with_spaces");

    constexpr auto test_8 = scan<"{%s}"_fs, "", std::string_view>();
    static_assert((std::get<0>(test_8.values).empty()), "empty_string");

    constexpr auto test_9 = scan<""_fs, "">();
    static_assert((std::tuple_size_v<decltype(test_9.values)> == 0), "empty_format_and_source");

    constexpr auto test_10 = scan<"constant"_fs, "constant">();
    static_assert((std::tuple_size_v<decltype(test_10.values)> == 0), "no_placeholders");

    constexpr auto test_11 = scan<"{}"_fs, "+42", int>();
    static_assert((std::get<0>(test_11.values) == 42), "signed_plus");

    constexpr auto test_12 = scan<"{}"_fs, "127", std::int8_t>();
    static_assert((std::get<0>(test_12.values) == std::numeric_limits<std::int8_t>::max()), "int8_max");

    constexpr auto test_13 = scan<"{}"_fs, "255", std::uint8_t>();
    static_assert((std::get<0>(test_13.values) == std::numeric_limits<std::uint8_t>::max()), "uint8_max");

    constexpr auto test_14 = scan<"{}"_fs, "2147483647", std::int32_t>();
    static_assert((std::get<0>(test_14.values) == std::numeric_limits<std::int32_t>::max()), "int32_max");

    constexpr auto test_15 = scan<"{}"_fs, "9223372036854775807", std::int64_t>();
    static_assert((std::get<0>(test_15.values) == std::numeric_limits<std::int64_t>::max()), "int64_max");

    constexpr auto test_16 = scan<"{}"_fs, "18446744073709551615", std::uint64_t>();
    static_assert((std::get<0>(test_16.values) == std::numeric_limits<std::uint64_t>::max()), "uint64_max");

    constexpr auto test_17 = scan<"v={%s}"_fs, "v=Привет", std::string_view>();
    static_assert((std::get<0>(test_17.values) == "Привет"), "utf8_string");

    constexpr auto test_18 = scan<"{}{}"_fs, "abc", std::string_view, std::string_view>();
    static_assert((std::get<0>(test_18.values).empty()) && (std::get<1>(test_18.values) == "abc"),
                  "adjacent_string_placeholders");

    constexpr auto test_19 = scan<"50% constant"_fs, "50% constant">();
    static_assert((std::tuple_size_v<decltype(test_19.values)> == 0), "literal_percent");

    return 0;
}
