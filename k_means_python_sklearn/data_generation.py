import sklearn
from sklearn.datasets import make_blobs
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import argparse
import os
from progress.bar import Bar

def make_clusters(n_samples, centers, n_features):
    X, y = make_blobs(n_samples=n_samples, centers=centers, n_features=n_features)
    return X, y.T

def plot_data(data):
    df = pd.DataFrame(dict(x1=data[:, 0], x2=data[:, 1], label=data[:,2]))
    groups = df.groupby('label')
    fig, ax = plt.subplots()
    colors = ['blue', 'orange', 'green', 'red', 'purple', 'brown', 'pink', 'gray', 'olive', 'cyan']
    for idx, classification in groups:
        classification.plot(ax=ax, kind='scatter', x='x1', y='x2', label=int(idx), color=colors[int(idx)])
    plt.show()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter,
        description=
        """ 
        This script uses python3. 
        Please install the following python libraries:
        pip3 install progress sklearn numpy pandas matplotlib
        
        Generates blobs of clustered data-points with specified parameters
        A dataset is described by n_samples, n_centers, n_features which represent: 
        number of samples in a dataset, number of centers that points form, and number of features 
        that each point has.
        
        The i-th specified value for any parameter determines that parameters value for the i-th dataset.
        If the number of values for any parameter is less than for any other parameter, then the provided value
        is extended to the rest of the datasets.
        
        Example:
        --n_samples 10 100 --n_centers 43 10 24 --n_features 4
        Generates 3 data sets with following characteristics:
        dataset[0] = {n_samples:10, n_centers:43, n_features:4}
        dataset[1] = {n_samples:100, n_centers: 10, n_features:4}
        dataset[2] = {n_samples:100, n_centers:24, --n_features 4}
        """
    )
    parser.add_argument('--n_samples', type=int, nargs='+',
                        help=
                        '''
                        Number of total samples in a dataset.
                        Ex --n_samples 2 generates a dataset with 2 sample points.
                        Ex --n_samples 2 5 7 10 generates datasets with 2,5,7, and 10 sample points
                        ''')
    parser.add_argument('--n_centers', type=int, nargs='+',
                        help=
                        '''
                        Number of centers that sample points form within a dataset. 
                        Ex --n_centers 4 2 5 10 generates datasets with 4,2,5, and 10 centers
                        ''')
    parser.add_argument('--n_features', type=int, nargs='+', help='''Number of features that each dataset will have''')
    parser.add_argument('--dataset_name', type=str, nargs='?', default='')
    parser.add_argument('--output_dir', help='output directory for .csv files', type=str, nargs=1, default='../data/')


    args = parser.parse_args()

    n_samples = args.n_samples
    n_centers = args.n_centers
    n_features = args.n_features
    dataset_name = args.dataset_name
    output_directory = args.output_dir

    dirs_to_create = [
        'clusters',
        'labels',
        'output/cpp_sequential',
        'output/cpp_parallel',
        'output/python_sklearn',
        'benchmarks/cpp_sequential',
        'benchmarks/cpp_parallel',
        'benchmarks/python_sklearn'
    ]

    print('Creating output directories')
    for dir_to_create in dirs_to_create:
        path = os.path.join(output_directory, dir_to_create)
        try:
            os.makedirs(path)
            print(f'Creating {path}')
        except FileExistsError as e:
            print(f'Already exists: {path}')

    # extend each parameter values list with it's last element so that they are all equal length
    max_len= max(len(n_samples), len(n_features), len(n_centers))
    n_samples = n_samples + [n_samples[-1]]*(max_len-len(n_samples))
    n_centers = n_centers + [n_centers[-1]]*(max_len-len(n_centers))
    n_features = n_features + [n_features[-1]]*(max_len-len(n_features))

    dataset_parameters = list(zip(n_samples, n_centers, n_features))
    print('Generating following datasets: ')
    for i, params in enumerate(dataset_parameters):
        print(f'dataset[{i}] = {{n_samples: {params[0]}, n_centers: {params[1]}, n_features: {params[2]}}}')

    bar = Bar("Generating data files", max=max_len)
    for i, params in enumerate(dataset_parameters):
        X, y = make_clusters(params[0], params[1], params[2])
        csv_name_clusters = f'{i}_nsamples_{params[0]}_ncenters_{params[1]}_nfeatures_{params[2]}.csv'
        full_path = os.path.join(output_directory, "clusters", csv_name_clusters)
        np.savetxt(full_path, X, delimiter=' ', newline='\n', fmt=['%e']*params[2])

        csv_name_labels = f'{i}_nsamples_{params[0]}_ncenters_{params[1]}_nfeatures_{params[2]}.csv'
        full_path = os.path.join(output_directory, "labels", csv_name_labels)
        np.savetxt(full_path, y, delimiter=' ', newline='\n', fmt=['%d'])
        bar.next()
    bar.finish()
    print('Done')

