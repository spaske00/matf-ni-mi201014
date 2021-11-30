import os
import argparse
import subprocess
import time
if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument("--num_of_iterations", type=int, default=32)
    arg_parser.add_argument("--num_of_clusters", type=int, default=8)
    arg_parser.add_argument("--num_of_threads", type=int, default=8)
    args = arg_parser.parse_args()
    num_of_iterations = args.num_of_iterations
    num_of_clusters = args.num_of_clusters
    num_of_threads = args.num_of_threads

    print("num_of_iterations: ", num_of_iterations)
    print("num_of_clusters: ", num_of_clusters)
    print("num_of_threads: ", num_of_threads)

    os.chdir(os.path.dirname(__file__))
    print('Generating cmake...')
    subprocess.run(['cmake',
                    '-DCMAKE_BUILD_TYPE=Release',
                    '-DCMAKE_DEPENDS_USE_COMPILER=FALSE',
                    '-GUnix Makefiles',
                    '-S .',
                    '-B cmake-build-release/'])
    os.chdir('cmake-build-release/')

    print('Building binaries...')
    subprocess.run(['make', 'k_means_cpp_cpu_parallel'])
    subprocess.run(['make', 'k_means_cpp_cpu_sequential'])
    os.chdir('..')

    data_path = 'data/clusters/'
    data_files = os.listdir(data_path)
    data_files.sort()
    os.chdir('bin/')
    print('Benchmarking cpu parallel...')
    start = time.time()
    for csv_file in data_files:
        if not csv_file.startswith('2'):
            continue
        path_to_input_points_from_exe_working_dir = str(os.path.join('..', data_path, csv_file))
        print(path_to_input_points_from_exe_working_dir)
        subprocess.run(['./k_means_cpp_cpu_parallel',
                        '--input_points_path', path_to_input_points_from_exe_working_dir,
                        '--num_of_iterations', str(num_of_iterations),
                        '--num_of_clusters', str(num_of_clusters),
                        '--num_of_threads', str(num_of_threads),
                        '--output_dir', '../data/output/cpp_parallel/',
                        '--benchmark_dir', '../data/benchmarks/cpp_parallel/'])

    print('Total elapsed time for cpu parallel: ', time.time() - start)

    print('Benchmarking cpu sequential...')
    start = time.time()
    for csv_file in data_files:
        if not csv_file.startswith('3'):
            continue
        path_to_input_points_from_exe_working_dir = str(os.path.join('..', data_path, csv_file))
        print(path_to_input_points_from_exe_working_dir)
        subprocess.run(['./k_means_cpp_cpu_sequential',
                        '--input_points_path', path_to_input_points_from_exe_working_dir,
                        '--num_of_iterations', str(num_of_iterations),
                        '--num_of_clusters', str(num_of_clusters),
                        '--output_dir', '../data/output/cpp_sequential/',
                        '--benchmark_dir', '../data/benchmarks/cpp_sequential/'])

    print('Total elapsed time for cpu sequential: ', time.time() - start)

    print('Done')
