from sklearn.cluster import KMeans
import numpy as np
import argparse
import os
import re
import logging

logger = logging.Logger()


def extract_number_of_clusters_from_file_name(filename):
    matcher = re.compile("ncenters_(\d+)")
    result = matcher.search(filename)
    return result.group(1)


if __name__ == "__main__":

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument("--n_init", type=int, default='10', help='''
        Number of time the k-means algorithm will be run with different centroid seeds. 
        The final results will be the best output of n_init consecutive runs in terms of inertia.''')
    arg_parser.add_argument("--max_iter", type=int, default=300, help='''
        Maximum number of iterations of the k-means algorithm for a single run.
    ''')
    arg_parser.add_argument("--tol", type=float, default=1e-4, help='''
        Relative tolerance with regards to Frobenius norm of the difference in the 
        cluster centers of two consecutive iterations to declare convergence.
    ''')
    arg_parser.add_argument("--data_files", type=str, nargs='*', default='')
    arg_parser.add_argument("--data_dir", type=str, default='')
    args = arg_parser.parse_args()

    n_init = args.n_init
    max_iter = args.max_iter
    tol = args.tol
    data_files = args.data_files
    data_dir_path = args.data_dir

    logger.setLevel()

    if len(data_files) == 0:
        data_files = os.listdir(data_dir_path)

    print(data_dir_path)
    print(data_files)

    for cluster_file_path_csv in data_files:
        path = os.path.join(data_dir_path, cluster_file_path_csv)
        X = np.loadtxt(path, dtype=np.single, delimiter=',')
        ncenters = extract_number_of_clusters_from_file_name(path)
        kmeans = KMeans(n_clusters=ncenters, n_init=n_init, max_iter=max_iter, tol=tol)
