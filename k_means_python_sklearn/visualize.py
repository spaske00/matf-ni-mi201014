import pandas as pd
import matplotlib.pyplot as plt
import argparse
import numpy as np

def plot_data_2d(x, labels):
    df = pd.DataFrame(dict(x1=x[:, 0], x2=x[:, 1], label=labels))
    groups = df.groupby('label')
    fig, ax = plt.subplots()
    colors = ['blue', 'orange', 'green', 'red', 'purple', 'brown', 'pink', 'gray', 'olive', 'cyan']
    for idx, classification in groups:
        classification.plot(ax=ax, kind='scatter', x='x1', y='x2', label=int(idx), color=colors[int(idx)])
    plt.show()


def plot_data_3d(X, y):
    raise NotImplemented()


if __name__ == '__main__':
    argparser = argparse.ArgumentParser(
        description='''
            --clusters clusters_file_path.csv
            --labels labels_file_path.csv
            [--delimiter ","]
        '''
    )
    argparser.add_argument('--clusters', type=str, required=True)
    argparser.add_argument('--labels', type=str, required=True)
    argparser.add_argument('--delimiter', type=str, default=' ')

    args = argparser.parse_args()
    clusters_file_path_csv = args.clusters
    labels_file_path_csv = args.labels
    delimiter = args.delimiter

    clusters = np.genfromtxt(clusters_file_path_csv, delimiter=delimiter, dtype=np.double)
    print(clusters.shape)
    labels = np.genfromtxt(labels_file_path_csv, delimiter=delimiter, dtype=np.uint)
    print(labels.shape)
    plot_data_2d(clusters, labels)



