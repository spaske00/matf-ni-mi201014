import os
import argparse
import subprocess
import time


class Benchmark:
    def __init__(self, name, program,
                 input_points_path,
                 num_of_iterations,
                 num_of_clusters,
                 num_of_threads,
                 output_dir,
                 benchmark_csv_path):
        self.name = name
        self.program = program
        self.input_points_path = input_points_path
        self.num_of_iterations = num_of_iterations
        self.num_of_clusters = num_of_clusters
        self.num_of_threads = num_of_threads
        self.output_dir = output_dir
        self.benchmark_csv_path = benchmark_csv_path

    def run(self):
        print(self.input_points_path)
        subprocess.run([self.program,
                        '--input_points_path', self.input_points_path,
                        '--num_of_iterations', str(self.num_of_iterations),
                        '--num_of_clusters', str(self.num_of_clusters),
                        '--num_of_threads', str(self.num_of_threads),
                        '--output_dir', self.output_dir,
                        '--benchmark_output_csv', self.benchmark_csv_path])


if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument("--num_of_iterations", type=int, default=32)
    arg_parser.add_argument("--num_of_clusters", type=int, default=12)
    arg_parser.add_argument("--num_of_threads", type=int, default=16)
    arg_parser.add_argument("--generate_data", type=bool, default=False)
    arg_parser.add_argument("--benchmark_name", type=str, default='benchmark')
    arg_parser.add_argument("--output_path", type=str, default='.')
    arg_parser.add_argument("--append", type=bool, default=False)
    args = arg_parser.parse_args()

    num_of_iterations = args.num_of_iterations
    num_of_clusters = args.num_of_clusters
    num_of_threads = args.num_of_threads
    generate_data = args.generate_data
    benchmark_name = args.benchmark_name
    output_path = args.output_path
    append_to_previous_benchmark = args.append

    dirs_to_make = ['data',
                    'data/clusters',
                    'data/benchmarks',
                    'data/labels',
                    'data/output',
                    'data/output/cpp_parallel',
                    'data/output/cpp_sequential',
                    'data/output/python_sklearn']
    print('Creating directories')
    for dir_to_make in dirs_to_make:
        try:
            os.makedirs(os.path.join(output_path, dir_to_make))
        except FileExistsError as e:
            print(dir_to_make, " already exists")

    for output_dir in os.listdir('data/benchmarks'):
        benchmark_output_csv = os.path.join('data/benchmarks', benchmark_name + ".csv")
        mode = 'a' if append_to_previous_benchmark else 'w'
        with open(benchmark_output_csv, mode) as f:
            if not append_to_previous_benchmark:
                f.write("file,type,num_of_samples,num_of_features,num_of_iterations,num_of_clusters,num_of_threads,"
                        "load_csv_ms,fit_ms,save_to_csv_ms\n")
            f.close()

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
        path_to_input_points_from_exe_working_dir = str(os.path.join('..', data_path, csv_file))
        print(path_to_input_points_from_exe_working_dir)
        subprocess.run(['./k_means_cpp_cpu_parallel',
                        '--input_points_path', path_to_input_points_from_exe_working_dir,
                        '--num_of_iterations', str(num_of_iterations),
                        '--num_of_clusters', str(num_of_clusters),
                        '--num_of_threads', str(num_of_threads),
                        '--output_dir', '../data/output/cpp_parallel/',
                        '--benchmark_output_csv', '../data/benchmarks/' + benchmark_name + ".csv"])

    print('Total elapsed time for cpu parallel: ', time.time() - start)

    print('Benchmarking cpu sequential...')
    start = time.time()
    for csv_file in data_files:
        if csv_file.startswith('3') or csv_file.startswith('4'):
            continue
        path_to_input_points_from_exe_working_dir = str(os.path.join('..', data_path, csv_file))
        print(path_to_input_points_from_exe_working_dir)
        subprocess.run(['./k_means_cpp_cpu_sequential',
                        '--input_points_path', path_to_input_points_from_exe_working_dir,
                        '--num_of_iterations', str(num_of_iterations),
                        '--num_of_clusters', str(num_of_clusters),
                        '--output_dir', '../data/output/cpp_sequential/',
                        '--benchmark_output_csv', str(os.path.join('../data/benchmarks/', benchmark_name + ".csv"))])
    print('Total elapsed time for cpu sequential: ', time.time() - start)

    print('Benchmarking python sklearn...')
    os.chdir('../k_means_python_sklearn')
    start = time.time()
    for csv_file in data_files:
        path_to_input_points_from_exe_working_dir = str(os.path.join('..', data_path, csv_file))
        print(path_to_input_points_from_exe_working_dir)
        subprocess.run(['python3.9', "k_means_sklearn.py",
                        '--input_points_path', path_to_input_points_from_exe_working_dir,
                        '--num_of_iterations', str(num_of_iterations),
                        '--num_of_clusters', str(num_of_clusters),
                        '--output_dir', '../data/output/python_sklearn/',
                        '--benchmark_output_csv', str(os.path.join('../data/benchmarks/', benchmark_name + ".csv"))])
    print('Total elapsed time for python sklearn: ', time.time() - start)

    print('Done')
