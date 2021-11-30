//
// Created by spaske on 22.11.21..
//

#ifndef MARKO_SPASIC_NI2020_NI_TYPES_H
#define MARKO_SPASIC_NI2020_NI_TYPES_H

#include<cstdint>

#define range(c) std::begin(c), std::end(c)
namespace ni {
    namespace types {
        using u8 = uint8_t;
        using u16 = uint16_t;
        using u32 = uint32_t;
        using u64 = uint64_t;

        using s8 = int8_t;
        using s16 = int16_t;
        using s32 = int32_t;
        using s64 = int64_t;

        using f32 = float;
        using f64 = double;
        using f128 = long double;

        using b8 = bool;
        using b32 = s32;

        struct None {
        };

        template<typename T>
        struct Just {
            T value;

            Just(T value) : value(value) {}
        };

        template<typename T>
        struct Optional {
            bool has_value = false;
            T value;

            Optional() = default;

            Optional(None) : has_value(false) {}

            Optional(Just<T> just) : has_value(true), value(just.value) {}

            T value_or(T default_value) const {
                return has_value ? value : default_value;
            }

            bool is_none() const { return !has_value; }
        };

        template<typename T, u64 N>
        u64 array_len(const T(&)[N]) {
            return N;
        }


    }

}
#endif //MARKO_SPASIC_NI2020_NI_TYPES_H
