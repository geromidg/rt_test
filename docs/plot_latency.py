from sys import argv

import numpy as np
from matplotlib import pyplot as plt

def parse_logfile(filename, cycle_time):
    """
    Read the logfile and get x,y value arrays

    Args:
        filename (str): the path of the logfile

    Returns:
        (x, y): the x and y arrays
    """

    with open(filename) as f:
        data = f.read()

    timestamps = data.split("\n", 6)[6].splitlines()
    latency = []

    for i in xrange(1, len(timestamps)):
    	actual_timestamp = float(timestamps[i])
    	expected_timestamp = float(timestamps[0]) + float(cycle_time) * i
    	latency.append(abs(actual_timestamp - expected_timestamp))

    timestamps.pop(0)

    x = np.asarray(timestamps, dtype=float)
    y = np.asarray(latency, dtype=float)

    x = x - x[0]  # x: start from 0
    y = y * 1000000  # y: convert in usec

    return x, y

def generate_plot(x, y):
    """
    Given x and y arrays, generate a plot

    Args:
        x (double array): the time values in sec
        y (double array): the latency values in usec
    """
    
    figure = plt.figure(figsize=(10, 6))
    # figure.suptitle('Timestamp - Latency', fontsize=24)
    axes = figure.add_subplot(1, 1, 1)
    axes.set_xlabel('Samples', fontsize=18)
    axes.set_ylabel('Latency [usec]', fontsize=18)
    axes.bar(x, y, width=0.005, color="grey")
    axes.set_xticks([])
    axes.margins(0.04)

    # figure.autofmt_xdate()
    figure.tight_layout()
    figure.subplots_adjust(top=0.88)
    figure.savefig('latency.png', dpi=60, bbox_inches='tight')

if __name__ == '__main__':
    x, y = parse_logfile('../src/timestamps.txt', argv[1])
    generate_plot(x, y)
