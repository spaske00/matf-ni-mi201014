//
// Created by spaske on 11.11.21..
//

#ifndef MARKO_SPASIC_NI2020_TYPES_H
#define MARKO_SPASIC_NI2020_TYPES_H

#include "fast_io/fast_io.h"
#include "fast_io/fast_io_device.h"
#include "fast_float/fast_float.h"
#include<cstddef>
#include<memory>
#include<cassert>
#include<random>
#include<type_traits>
#include<cstring>
#include<chrono>
#include "ni_types.h"

namespace ni {
    using namespace ni::types;

    struct CSVInfo {
        std::string_view filepath;
        std::string_view filename;
        u64 num_of_rows;
        u64 num_of_cols;
        u64 num_of_clusters;
        u64 id;
    };

    void debug_dump(const CSVInfo &csv) {
        using fast_io::mnp::os_c_str;
        println("fileepath: ", csv.filepath);
        println("filename: ", csv.filename);
        println("id: ", csv.id);
        println("num_of_rows: ", csv.num_of_rows);
        println("num_of_cols: ", csv.num_of_cols);
        println("num_of_clusters: ", csv.num_of_clusters);
    }


    CSVInfo extract_csv_info_from_filename(std::string_view filepath) {
        using fast_io::mnp::os_c_str;
        CSVInfo result;
        result.filepath = filepath;
        assert(filepath.find_last_of('/') != std::string_view::npos);
        result.filename = filepath.substr(filepath.find_last_of('/') + 1);
        fast_io::ibuffer_view buffer(result.filename.begin(), result.filename.end());
        scan(buffer, result.id, "_nsamples_", result.num_of_rows, "_ncenters_", result.num_of_clusters,
             "_nfeatures_", result.num_of_cols);
        assert(result.num_of_rows && result.num_of_clusters && result.num_of_cols);
#if 0
        std::string_view filename(result.filename);
        {
            const char clusters_[] = "clusters_";
            auto clusters_position = filename.find(clusters_);
            assert(clusters_position != std::string::npos);
            result.id = atoll(&filename[clusters_position + array_len(clusters_) - 1]);
        }
        {
            const char nsamples_[]= "nsamples_";
            auto nsamples_position = filename.find(nsamples_);
            assert(nsamples_position != std::string_view::npos);
            result.num_of_rows = atoll(&filename[nsamples_position + array_len(nsamples_) - 1]);
        }
        {
            const char nfeatures_[] = "nfeatures_";
            auto nfeatures_position = filename.find(nfeatures_);
            assert(nfeatures_position != std::string_view::npos);
            result.num_of_cols = atoll(&filename[nfeatures_position + array_len(nfeatures_) - 1]);
        }
        {
            const char ncenters_[] = "ncenters_";
            auto ncenters_position = filename.find(ncenters_);
            assert(ncenters_position != std::string_view::npos);
            result.num_of_clusters = atoll(&filename[ncenters_position + array_len(ncenters_) - 1]);

        }
#endif
        return result;
    }

    template<typename T>
    class UniformDistributionGenerator {
        static_assert(std::is_arithmetic_v<T>);
    public:
        using distribution_type = std::conditional_t<
                std::is_floating_point_v<T>,
                std::uniform_real_distribution<T>,
                std::uniform_int_distribution<T>
        >;

        UniformDistributionGenerator(T a, T b)
                : m_gen(s_rd()), m_dis(a, b) {}

        T next() {
            return m_dis(m_gen);
        }

        void set_range(T a, T b) {
            m_dis = distribution_type(a, b);
        }

    private:
        std::random_device s_rd;
        std::mt19937_64 m_gen;
        distribution_type m_dis;
    };

    template<typename T>
    struct vec_ref {
        static_assert(std::is_arithmetic<T>::value);
        u64 len = 0;
        T *e = nullptr;

        vec_ref(u64 len, T *data) : len(len), e(data) {}

        u64 size() const { return len; }

        T &operator[](u64 i) {
            assert(i < len);
            return e[i];
        }

        const T &operator[](u64 i) const {
            assert(i < len);
            return e[i];
        }

        void copy_assign(const vec_ref &other) noexcept {
            assert(other.len == len);
            assert(&other != this);
            std::memcpy(e, other.e, sizeof(T) * len);
        }
    };

    template<typename T>
    void inplace_add(vec_ref<T> lhs, const vec_ref<T> rhs) {
        assert(lhs.len == rhs.len);
        auto *dest = lhs.e;
        auto *src = rhs.e;
        for (u64 i = 0; i < lhs.len; ++i) {
            dest[i] += src[i];
        }
    }

    template<typename T>
    vec_ref<T> operator+=(vec_ref<T> lhs, const vec_ref<T> &rhs) {
        inplace_add(lhs, rhs);
        return lhs;
    }

    template<typename T>
    void inplace_scalar_mul(vec_ref<T> lhs, T scalar) {
        auto *e = lhs.e;
        for (u64 i = 0; i < lhs.len; ++i) {
            e[i] *= scalar;
        }
    }

    template<typename T>
    vec_ref<T> operator*=(vec_ref<T> lhs, T scalar) {
        inplace_scalar_mul(lhs, scalar);
        return lhs;
    }

    template<typename T>
    void inplace_scalar_div(vec_ref<T> lhs, T scalar) {
        auto *e = lhs.e;
        for (u64 i = 0; i < lhs.len; ++i) {
            e[i] /= scalar;
        }
    }

    template<typename T>
    vec_ref<T> operator/=(vec_ref<T> lhs, T scalar) {
        inplace_scalar_div(lhs, scalar);
        return lhs;
    }

    template<typename T>
    T dot(const vec_ref<T> &lhs, const vec_ref<T> &rhs) {
        assert(lhs.len == rhs.len);
        T result{};
        const auto len = lhs.len;
        for (u64 i = 0; i < len; ++i) {
            result += lhs[i] * rhs[i];
        }
        return result;
    }

    template<typename T>
    T L2(const vec_ref<T> &lhs, const vec_ref<T> &rhs) {
        assert(lhs.len == rhs.len);
        T result{};
        const auto len = lhs.len;
        for (u64 i = 0; i < len; ++i) {
            const auto a = lhs[i];
            const auto b = rhs[i];
            result += (a - b) * (a - b);
        }
        return result;
    }


    template<typename T>
    class Points {
    public:
        Points() = default;

        Points(u64 num_of_points, u64 dim_of_point, T init = {})
                : m_num_of_points(num_of_points), m_dim_of_points(dim_of_point) {
            if (__builtin_umull_overflow(num_of_points, dim_of_point, &m_num_of_values)) {
                std::terminate();
            } else {
                m_data.reset(new T[num_of_points * dim_of_point]{init});
            }
        }

        const vec_ref<T> operator[](u64 r) const {
            return row(r);
        }

        vec_ref<T> operator[](u64 r) {
            return row(r);
        }

        vec_ref<T> row(u64 r) const {
            assert(r < m_num_of_points);
            vec_ref <T> result(m_dim_of_points, m_data.get() + (r * m_dim_of_points));
            return result;
        }

        const T &at(u64 r, u64 c) const {
            assert(r < m_num_of_points && c < m_dim_of_points);
            return m_data[r * m_dim_of_points + c];
        }

        T &at(u64 r, u64 c) {
            assert(r < m_num_of_points && c < m_dim_of_points);
            return m_data[r * m_dim_of_points + c];
        }

        void fill(T value) {
            for (u64 i = 0; i < m_num_of_values; ++i) {
                m_data[i] = value;
            }
        }

        static Points<T> load_from_csv(const CSVInfo &info);

        void save_to_csv(std::string_view filepath) const;

        void save_to_csv(std::string_view output_dir, std::string_view filepath) const;

        u64 num_of_points() const { return m_num_of_points; }

        u64 dim_of_points() const { return m_dim_of_points; }

    private:
        u64 m_num_of_points = 0;
        u64 m_dim_of_points = 0;
        u64 m_num_of_values = 0;
        std::unique_ptr<T[]> m_data = nullptr;
    };

    template<typename T>
    Points<T> Points<T>::load_from_csv(const CSVInfo &info) {
        assert(info.num_of_rows && info.num_of_cols);
        Points <T> result(info.num_of_rows, info.num_of_cols);
        fast_io::native_file_loader csv_file(info.filepath);
        auto file_it = csv_file.cbegin();
        for (u64 i = 0; i < result.m_num_of_values; ++i) {
            assert(file_it != csv_file.end());
            auto from_chars_result = fast_float::from_chars(file_it, csv_file.cend(), result.m_data[i]);
            file_it = std::next(from_chars_result.ptr);
        }
        return result;
    }

    template<typename T>
    void Points<T>::save_to_csv(std::string_view filepath) const {
        fast_io::obuf_file csv_file(filepath);
        const auto rows = m_num_of_points;
        const auto cols = m_dim_of_points;
        for (u64 i = 0; i < rows; ++i) {
            for (u64 j = 0; j < cols - 1; ++j) {
                print(csv_file, at(i, j), " ");
            }
            println(csv_file, at(i, cols - 1));
        }
    }

    template<typename T>
    void Points<T>::save_to_csv(std::string_view output_dir, std::string_view filepath) const {
        using fast_io::mnp::os_c_str;
        char buffer[1024] = {0};
        assert(output_dir.length() + filepath.length() < sizeof(buffer));
        output_dir.copy(buffer, output_dir.length());
        filepath.copy(buffer + output_dir.length(), filepath.length());
        println(os_c_str(buffer, filepath.length() + output_dir.length()));
        save_to_csv(buffer);
    }

    enum VectorCsvFormatSave {
        AsRow,
        AsCol
    };

    template<VectorCsvFormatSave format, typename T>
    void save_to_csv(std::string_view filepath, const std::vector<T> &vec) {
        static_assert(std::is_arithmetic_v<T>);
        fast_io::obuf_file csv_file(filepath);
        const auto n = vec.size();
        for (u64 i = 0; i < n; ++i) {
            if constexpr(format == AsRow)
                print(csv_file, vec[i], " ");
            else if constexpr(format == AsCol)
                println(csv_file, vec[i]);
        }
    }

    template<VectorCsvFormatSave format, typename T>
    void save_to_csv(std::string_view output_dir, std::string_view filename, const std::vector<T> &vec) {
        static_assert(std::is_arithmetic_v<T>);
        char buffer[1024] = {0};
        using fast_io::mnp::os_c_str;
        assert(output_dir.length() + filename.length() < sizeof(buffer));
        output_dir.copy(buffer, output_dir.length());
        filename.copy(buffer + output_dir.length(), filename.length());
        save_to_csv<format>(buffer, vec);
    }


    template<typename T>
    void debug_dump(const Points<T> &points) {
        char buffer[64] = {0};
        const auto rows = points.num_of_points();
        const auto cols = points.dim_of_points();
        for (u64 i = 0; i < rows; ++i) {
            for (u64 j = 0; j < cols; ++j) {
                print(fast_io::dbg(), points.at(i, j), " ");
            }
            println(fast_io::dbg());
        }
    }

}
#endif //MARKO_SPASIC_NI2020_TYPES_H

