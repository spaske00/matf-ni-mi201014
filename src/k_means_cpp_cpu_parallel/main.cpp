#include "kmeans.h"
#include "ni_common.h"
void print_usage() {
    println("--help");
    println(R"s(
        ./kmeans_cpu_sequential --input_points_path path/to/input_points.csv
                                [--num_of_iterations 32]
                                [--num_of_clusters 7]
                                [--output_dir dir/to/output/results]
                                [--benchmark_output_csv dir/to/output/benchmark.csv]
    )s");
}

int main(int argc, char **argv) {
    using namespace ni::types;
    if (argc == 1) {
        print_usage();
        std::exit(EXIT_SUCCESS);
    }
    ni::ArgParser arg_parser(argc, argv);
    auto help = arg_parser.argument<bool>("--help");
    if (help.has_value) {
        print_usage();
        std::exit(EXIT_SUCCESS);
    }
    auto input_points_path = arg_parser.argument<std::string_view>("--input_points_path");
    if (input_points_path.is_none()) {
        print_usage();
        std::exit(EXIT_SUCCESS);
    }
    auto kmeans_default_params = ni::cpp_parallel::KMeansParams::get_default();
    kmeans_default_params.num_of_iterations = arg_parser.argument<u32>("--num_of_iterations", kmeans_default_params.num_of_iterations);
    kmeans_default_params.num_of_clusters = arg_parser.argument<u32>("--num_of_clusters", kmeans_default_params.num_of_clusters);

    ni::CSVInfo info = ni::extract_csv_info_from_filename(input_points_path.value);

    ni::logging::Timer timer;
    timer.start(ni::logging::TimedEvent_load_from_csv);
    auto points = ni::Points<float>::load_from_csv(info);
    timer.stop_and_log();
    auto kmeans = ni::cpp_parallel::KMeans<float>(kmeans_default_params);
    timer.start(ni::logging::TimedEvent_kmeans_fit);
    kmeans.fit(points);
    timer.stop_and_log();

    auto output_dir = arg_parser.argument<std::string_view>("--output_dir");
    if (output_dir.has_value) {
        const auto& centroids = kmeans.get_output_closet_centroid_indices();
        timer.start(ni::logging::TimedEvent_save_to_csv);
        ni::save_to_csv<ni::VectorCsvFormatSave::AsCol>(output_dir.value, info.filename, centroids);
        timer.stop_and_log();
    }

    auto benchmark_output_csv = arg_parser.argument<std::string_view>("--benchmark_output_csv");
    if (benchmark_output_csv.has_value) {
        fast_io::obuf_file benchmark_results_file(benchmark_output_csv.value, fast_io::open_mode::app);
        println(benchmark_results_file,info.filename, ",cpp_parallel,", info.num_of_rows,",", info.num_of_cols, ",",
                kmeans_default_params.num_of_iterations, ",",
                kmeans_default_params.num_of_clusters, ",",
                kmeans_default_params.num_of_threads, ",",
                timer.elapsed(ni::logging::TimedEvent_load_from_csv), ",",
                timer.elapsed(ni::logging::TimedEvent_kmeans_fit), ",",
                timer.elapsed(ni::logging::TimedEvent_save_to_csv));
    }

    return 0;
}