//
// Created by spaske on 13.10.21..
//

#ifndef UNTITLED3_KMEANS_H
#define UNTITLED3_KMEANS_H

#include <vector>
#include <cassert>
#include <cstdint>
#include <memory>

#include "ni_common.h"

namespace ni {
    namespace cpu_sequential {
        using namespace ni::types;

        struct KMeansParams {
            u32 num_of_clusters = 7;
            u32 num_of_iterations = 300;
            u32 num_of_threads = 1;
            static KMeansParams get_default() {
                KMeansParams result;
                return result;
            };
        };

        void debug_dump(const KMeansParams& params) {
            println(fast_io::dbg(), "num_of_clusters: ", params.num_of_clusters);
            println(fast_io::dbg(), "num_of_iterations: ", params.num_of_iterations);
            println(fast_io::dbg(), "num_of_threads: ", params.num_of_threads);
        }

        template<typename T>
        class KMeans {
        public:

            void setParams(const KMeansParams &params) { m_params = params; }

            KMeans(KMeansParams params = KMeansParams::get_default())
                    : m_params(params) {}

            void fit(const Points<T> &input_points);

            bool fitted() const { return m_fitted; }

            const Points<T> &get_output_centroid_coors() const { return m_output_centroid_coords; }

            const std::vector<u64> &
            get_output_closet_centroid_indices() const { return m_output_closets_centroid_indices; }

            void predict(const Points<T> &input_points, std::vector<u64> &output_cluster_indices);

            KMeansParams m_params;
        private:
            Points<T> m_output_centroid_coords;
            std::vector<u64> m_output_closets_centroid_indices;
            const Points<T> *m_input_points = nullptr;
            bool m_fitted = false;

        };

        template<typename T>
        void KMeans<T>::fit(const Points<T> &input_points) {
            assert(m_params.num_of_clusters);
            assert(m_params.num_of_iterations);
            const auto N = input_points.num_of_points();
            const auto K = m_params.num_of_clusters;
            const auto M = m_params.num_of_iterations;
            assert(K <= N);
            m_input_points = &input_points;

            m_output_closets_centroid_indices.resize(N);
            m_output_centroid_coords = Points<T>(m_params.num_of_clusters, input_points.dim_of_points());

            {
                std::vector<u64> indices(input_points.num_of_points());
                std::iota(range(indices), u64{});
                UniformDistributionGenerator<u64> random_indices(0, input_points.num_of_points() - 1);
                for (u64 i = 0; i < input_points.num_of_points() - 1; ++i) {
                    random_indices.set_range(i, input_points.num_of_points() - 1);
                    auto j = random_indices.next();
                    std::swap(indices[i], indices[j]);
                }
                for (u64 k = 0; k < K; ++k) {
                    m_output_centroid_coords[k].copy_assign(input_points[indices[k]]);
                }
            }

            // helper arrays
            Points<T> sum_of_all_points_in_cluster(m_params.num_of_clusters, input_points.dim_of_points(), T{});
            std::vector<u64> num_of_points_in_cluster(m_params.num_of_clusters, 0);

            for (u64 iteration = 0; iteration < M; ++iteration) {
                sum_of_all_points_in_cluster.fill(T{});
                std::fill(range(num_of_points_in_cluster), 0);

                // update cluster
                for (u64 i = 0; i < N; ++i) {
                    const vec_ref<T> ith_point = input_points[i];
                    T distance_from_closest_centroid = std::numeric_limits<T>::max();
                    u64 index_of_closest_centroid = 0;
                    for (u64 k = 0; k < K; ++k) {
                        assert(k < m_output_centroid_coords.num_of_points());
                        const T distance_from_kth_centroid = L2(ith_point, m_output_centroid_coords[k]);
                        if (distance_from_kth_centroid < distance_from_closest_centroid) {
                            distance_from_closest_centroid = distance_from_kth_centroid;
                            index_of_closest_centroid = k;
                        }
                    }
                    m_output_closets_centroid_indices[i] = index_of_closest_centroid;
                }

                for (u64 i = 0; i < N; ++i) {
                    const auto cluster_index = m_output_closets_centroid_indices[i];
                    sum_of_all_points_in_cluster[cluster_index] += input_points[i];
                    num_of_points_in_cluster[cluster_index] += 1;
                }

                for (u64 k = 0; k < K; ++k) {
                    const T count = static_cast<T>(std::max(1ul, num_of_points_in_cluster[k]));
                    m_output_centroid_coords[k].copy_assign(sum_of_all_points_in_cluster[k]);
                    m_output_centroid_coords[k] /= count;
                }
            }
            m_fitted = true;
        }

        template<typename T>
        void KMeans<T>::predict(const Points<T> &input_points, std::vector<u64> &output_cluster_indices) {
            assert(input_points.dim_of_points() == m_output_centroid_coords.dim_of_points());
            output_cluster_indices.resize(input_points.num_of_points());
            for (u64 point_index = 0; point_index < input_points.num_of_points(); ++point_index) {
                T min_distance = std::numeric_limits<T>::max();
                u64 closest_centroid_index = 0;
                for (u64 centroid_index = 0;
                     centroid_index < m_output_centroid_coords.num_of_points(); ++centroid_index) {
                    const T distance_to_centroid = L2(input_points[point_index],
                                                      m_output_centroid_coords[centroid_index]);
                    if (distance_to_centroid < min_distance) {
                        closest_centroid_index = centroid_index;
                        min_distance = distance_to_centroid;
                    }
                }
                output_cluster_indices[point_index] = closest_centroid_index;
            }
        }
    }
}

#endif //UNTITLED3_KMEANS_H
// stage N of programming development wrt architecture
// -Individual element thinking
// -Constructors/Destructors RAII
// -Thousands/Millions of malloc/free new/delete for individual objects
// -smart pointers
// -lifetime constant concern and mental overhead

// stage N + 1 of programming development wrt architecture
// -Group/System level thinking
// -ZII
// -Very few allocations. Grouped together. Heavy use of arenas and scratch space
// -No need for smart pointers
// -lifetime is trivial and obvious in 99% of the cases
// -Heavy use of Caches, Hashing and Result reuse