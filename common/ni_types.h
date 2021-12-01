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

        template<typename T, u64 N>
        class StaticSizeArray {
            static_assert(std::is_trivial_v<T>);
        public:
            StaticSizeArray() = default;
            StaticSizeArray(const T& value) : m_data(value) {}
            void push_back(const T& value) {
                assert(m_length < m_capacity);
                m_data[m_length++] = value;
            }
            void clear() { m_length = 0; }
            T& operator[](u64 i) { assert(i < m_length); return m_data[i]; }
            const T& operator[](u64 i) const { assert(i < m_length); return m_data[i]; }
            u64 length() const { return m_length; }
            u64 capacity() const {return m_capacity; }
            auto begin() const { return &m_data[0]; }
            auto end() const { return &m_data[0] + m_length; }
            auto begin() { return &m_data[0]; }
            auto end() { return &m_data[0] + m_length; }
            bool contains(const T& value) const {
                return std::find(begin(), end(), value) != end();
            }
        private:
            u64 m_length{0};
            u64 m_capacity{N};
            std::array<T, N> m_data;
        };
    }

}
#endif //MARKO_SPASIC_NI2020_NI_TYPES_H
