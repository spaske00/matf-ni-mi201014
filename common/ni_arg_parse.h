//
// Created by spaske on 21.11.21..
//

#ifndef MARKO_SPASIC_NI2020_NI_ARG_PARSE_H
#define MARKO_SPASIC_NI2020_NI_ARG_PARSE_H

#include<cstdlib>
#include<type_traits>
#include "ni_types.h"

namespace ni {
    using namespace ni::types;

    class ArgParser {
    public:
        ArgParser(int argc, char **argv)
                : argc(argc), begin(argv), end(argv + argc) {}

        template<typename T>
        Optional<T> argument(std::string_view name) {
            char **itr = std::find(begin, end, name);
            Optional<T> result;
            if (itr != end) {
                auto value_ptr = *itr + name.length() + 1;
                char *ptr = nullptr;
                if constexpr(std::is_same_v<T, s32> || std::is_same_v<T, b8>) {
                    result.has_value = true;
                    result.value = strtol(value_ptr, &ptr, 10);
                } else if constexpr (std::is_same_v<T, u32>) {
                    result.has_value = true;
                    result.value = strtoul(value_ptr, &ptr, 10);
                } else if constexpr(std::is_same_v<T, float>) {
                    result.has_value = true;
                    result.value = strtof(value_ptr, &ptr);
                } else if constexpr(std::is_same_v<T, double>) {
                    result.has_value = true;
                    result.value = strtod(value_ptr, &ptr);
                } else if constexpr(std::is_same_v<T, std::string_view>) {
                    result.has_value = true;
                    result.value = value_ptr;
                } else {
                    []<bool flag = false>() {
                        static_assert(flag, "type T not supported");
                    }();
                }
            }
            return result;
        }

        template<typename T>
        T argument(std::string_view name, T default_value) {
            Optional<T> parsed_argument = argument<T>(name);
            return parsed_argument.value_or(default_value);
        }

    private:
        int argc;
        char **const begin;
        char **const end;
    };
}


#endif //MARKO_SPASIC_NI2020_NI_ARG_PARSE_H
