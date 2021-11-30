#include "kmeans.h"
#include "ni_common.h"
void print_usage() {
    println("--help");
    println(R"s(
        ./kmeans_cpu_sequential --input_points_path path/to/input_points.csv
                                [--num_of_iterations 32]
                                [--num_of_clusters 7]
                                [--output_dir dir/to/output/results]
                                [--benchmark_dir dir/to/output/benchmark]
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
    auto kmeans_default_params = ni::cpu_sequential::KMeansParams::get_default();
    kmeans_default_params.num_of_iterations = arg_parser.argument<u32>("--num_of_iterations", kmeans_default_params.num_of_iterations);
    kmeans_default_params.num_of_clusters = arg_parser.argument<u32>("--num_of_clusters", kmeans_default_params.num_of_clusters);

    ni::CSVInfo info = ni::extract_csv_info_from_filename(input_points_path.value);

    ni::logging::Timer timer;
    timer.start("load_from_csv");
    auto points = ni::Points<float>::load_from_csv(info);
    timer.stop_and_log();
    auto kmeans = ni::cpu_sequential::KMeans<float>(kmeans_default_params);
    timer.start("kmeans_fit");
    kmeans.fit(points);
    timer.stop_and_log();

    auto output_dir = arg_parser.argument<std::string_view>("--output_dir");
    if (output_dir.has_value) {
        const auto& centroids = kmeans.get_output_closet_centroid_indices();
        timer.start("save_to_csv");
        ni::save_to_csv<ni::VectorCsvFormatSave::AsCol>(output_dir.value, info.filename, centroids);
        timer.stop_and_log();
    }

    auto benchmark_dir = arg_parser.argument<std::string_view>("--benchmark_dir");
    if (benchmark_dir.has_value) {
        char buffer[1024];
        auto end = benchmark_dir.value.copy(buffer, benchmark_dir.value.length());
        end += info.filename.copy(buffer + end, info.filename.length());
        fast_io::obuf_file benchmark_results_file(std::string_view(buffer, end));
        println(timer.get_log());
        println(benchmark_results_file, timer.get_log());
        println(benchmark_results_file, "num_of_iterations: ", kmeans_default_params.num_of_iterations);
        println(benchmark_results_file, "num_of_clusters: ", kmeans_default_params.num_of_clusters);
        println(benchmark_results_file, "num_of_threads: ", kmeans_default_params.num_of_threads);
    }

    return 0;
}
