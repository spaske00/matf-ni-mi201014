//
// Created by spaske on 13.10.21..
//

#ifndef UNTITLED3_KMEANS_H
#define UNTITLED3_KMEANS_H

#include <vector>
#include <cassert>
#include <cstdint>
#include <memory>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <vector>
#include "ni_common.h"
namespace ni {
    namespace cpu_parallel {
        using namespace ni::types;
        struct KMeansParams {
            u32 num_of_clusters = 7;
            u32 num_of_iterations = 300;
            u32 num_of_threads = std::thread::hardware_concurrency() - 1;
            static KMeansParams get_default() {
                KMeansParams params;
                return params;
            }
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
            std::atomic<bool> m_fitted = false;

            std::array<std::thread, 64> m_thread_pool;
            u32 m_num_of_threads;

            std::atomic<u64> m_current_iteration;
            std::atomic<u32> m_num_of_threads_that_finished_cluster_update_for_this_iteration;

            std::mutex m_lk_worker_threads_start;
            std::condition_variable m_cv_worker_threads_start;

            std::mutex m_lk_worker_threads_done;
            std::condition_variable m_cv_worker_threads_done;

            void update_rows(u64 start, u64 end);
            void worker_thread(u64 thread_index);
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

            // initialize m_thread_pool with m_num_of_threads
            m_num_of_threads = std::min(std::thread::hardware_concurrency() - 1, m_params.num_of_threads);
            assert(m_num_of_threads < m_thread_pool.size());
            m_current_iteration.store(m_params.num_of_iterations + 1);
            for (u64 i = 0; i < m_num_of_threads; ++i) {
                m_thread_pool[i] = std::thread([this](auto index){worker_thread(index);}, i);
            }
            for (u64 iteration = 0; iteration < M; ++iteration) {
                sum_of_all_points_in_cluster.fill(T{});
                std::fill(range(num_of_points_in_cluster), 0);
                // start parallel cluster update
                // TODO: give this thread clusters to update also
                m_current_iteration.store(iteration, std::memory_order_release);
                // TODO: this doesn't need to be an atomic store/read because of cv mutex lk
                m_num_of_threads_that_finished_cluster_update_for_this_iteration.store(0, std::memory_order_release);
                m_cv_worker_threads_start.notify_all();
                { // NOTE: stmt block because of std::unique_lock lk
                    std::unique_lock lk(m_lk_worker_threads_done);
                    m_cv_worker_threads_done.wait(lk, [this] {
                        return m_num_of_threads_that_finished_cluster_update_for_this_iteration.load(
                                std::memory_order_acquire) == m_num_of_threads;
                    });
                }

                // update centroids
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
            for (u64 i = 0; i < m_num_of_threads; ++i) {
                m_thread_pool[i].join();
            }
        }

        template<typename T>
        void KMeans<T>::update_rows(u64 start, u64 end) {
            const auto K = m_params.num_of_clusters;
            auto& input_points = *m_input_points;
            for (u64 i = start; i < end; ++i) {
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
        }

        template<typename T>
        void KMeans<T>::worker_thread(u64 thread_index) {
            const u64 N = m_input_points->num_of_points();
            const u64 num_of_threads = m_num_of_threads;
            const u64 minimum_chunk_size = 16;
            const u64 chunk_size = std::max(minimum_chunk_size, N / num_of_threads);
            const u64 start = std::min(thread_index * chunk_size, m_input_points->num_of_points());
            const u64 end = std::min(start + chunk_size, m_input_points->num_of_points());
            for (u64 iteration = 0; iteration < m_params.num_of_iterations; ++iteration) {
                std::unique_lock lk(m_lk_worker_threads_start);
                m_cv_worker_threads_start.wait(lk, [this, iteration] {
                    return m_current_iteration.load(std::memory_order_acquire) == iteration;
                });
                update_rows(start, end);
                m_num_of_threads_that_finished_cluster_update_for_this_iteration.fetch_add(1,
                                                                                           std::memory_order_release);
                m_cv_worker_threads_done.notify_one();
            }
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