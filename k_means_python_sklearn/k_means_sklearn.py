from sklearn.cluster import KMeans
import numpy as np
import argparse
import os
import re
import logging
import time

if __name__ == "__main__":
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument("--input_points_path", type=str, required=True)
    arg_parser.add_argument("--num_of_iterations", type=int, default=300)
    arg_parser.add_argument("--num_of_clusters", type=int, default=8)
    arg_parser.add_argument("--num_of_threads", type=int, default=15)
    arg_parser.add_argument("--output_dir", type=str, required=True)
    arg_parser.add_argument("--benchmark_output_csv", type=str, required=True)
    args = arg_parser.parse_args()

    num_of_iterations = args.num_of_iterations
    num_of_clusters = args.num_of_clusters
    num_of_threads = args.num_of_threads
    output_dir = args.output_dir
    benchmark_output_csv = args.benchmark_output_csv
    input_points_path = args.input_points_path

    start = time.time()
    X = np.loadtxt(input_points_path, dtype=np.single, delimiter=' ')
    load_csv_ms = (time.time() - start) * 1000.0

    kmeans = KMeans(init='random', n_clusters=num_of_clusters, n_init=1, max_iter=num_of_iterations, tol=0.0,
                    algorithm='full')

    start = time.time()
    kmeans.fit(X)
    kmeans_fit_ms = (time.time() - start) * 1000.0

    input_points_filename = os.path.basename(input_points_path)
    start = time.time()
    np.savetxt(os.path.join(output_dir, input_points_filename), kmeans.cluster_centers_)
    save_to_csv_ms = (time.time() - start) * 1000.0

    with open(benchmark_output_csv, 'a') as f:
        f.write(
            f'{input_points_filename},python_sklearn, {X.shape[0]}, {X.shape[1]}, {num_of_iterations},{num_of_clusters},{kmeans._n_threads},{load_csv_ms},{kmeans_fit_ms},{save_to_csv_ms}\n')
        f.close()
