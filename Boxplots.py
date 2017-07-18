import json
import os
import glob
import re
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import Polygon
from tqdm import tqdm


def get_merged_benchmarks(merged_dict, f_dict):
    for benchmark in f_dict['benchmarks']:
        merged_dict[benchmark['name']].append(benchmark['real_time'])
    return merged_dict


def plot_benchmarks(title, x_label, sorted_keys, file_name=None):
    scan_type = sorted_keys[0].split('_')[1] # ColumnScan or RowScan
    json_files = glob.glob('benchmark_*.json')
    merged_dict = dict()
    for json_file in json_files:
        f = open(json_file, "r").read()
        f = f.replace(u'/u0000', '')
        # f = re.sub(r'0{5,}', '', f) # Sometimes needed for random 0s in JSON
        f_dict = json.loads(f)
        if not bool(merged_dict):
        # Dict is empty
            for benchmark in f_dict['benchmarks']:
                merged_dict[benchmark['name']] = []
        merged_dict = get_merged_benchmarks(merged_dict, f_dict)

    # Remove broken keys and keys not present in sorted_keys
    merged_dict_copy = merged_dict.copy()
    for key in merged_dict_copy.keys():
        if '/0' in key or key not in sorted_keys:
            del merged_dict[key]

    unit_number = len(sorted_keys)//2
    unit_labels = [label.split('/')[1] for label in sorted_keys]
    data = [merged_dict[k] for k in sorted_keys]

    fig, ax1 = plt.subplots(figsize=(10, 6))
    plt.subplots_adjust(left=0.075, right=0.95, top=0.9, bottom=0.25)

    # Scale y axis
    if max([max(x) for x in data]) > 1000*1000:
        ax1.set_ylabel('Time [s]')
        data = [[s/float(1000*1000) for s in row] for row in data]
    elif max([max(x) for x in data]) > 1000:
        ax1.set_ylabel('Time [ms]')
        data = [[s/float(1000) for s in row] for row in data]
    else:
        ax1.set_ylabel('Time [µs]')


    bp = plt.boxplot(data, notch=0, sym='+', vert=1, whis=1.5)
    plt.setp(bp['boxes'], color='black')
    plt.setp(bp['whiskers'], color='black')
    plt.setp(bp['fliers'], color='red', marker='+')

    # Add a horizontal grid to the plot, but make it very light in color
    # so we can use it for reading data values but not be distracting
    ax1.yaxis.grid(True, linestyle='-', which='major', color='lightgrey', alpha=0.5)

    # Hide these grid behind plot objects
    ax1.set_axisbelow(True)
    ax1.set_title(title)
    ax1.set_xlabel(x_label)

    # Now fill the boxes with desired colors
    box_colors = ['darkkhaki', 'royalblue']
    num_boxes = unit_number*2
    medians = list(range(num_boxes))
    for i in range(num_boxes):
        box = bp['boxes'][i]
        box_x = []
        box_y = []
        for j in range(5):
            box_x.append(box.get_xdata()[j])
            box_y.append(box.get_ydata()[j])
        box_coords = list(zip(box_x, box_y))
        # Alternate between Dark Khaki and Royal Blue
        k = i % 2
        box_polygon = Polygon(box_coords, facecolor=box_colors[k])
        ax1.add_patch(box_polygon)
        # Now draw the median lines back over what we just filled in
        med = bp['medians'][i]
        median_x = []
        median_y = []
        for j in range(2):
            median_x.append(med.get_xdata()[j])
            median_y.append(med.get_ydata()[j])
            plt.plot(median_x, median_y, 'k')
            medians[i] = median_y[0]


    ax1.set_xlim(0.5, num_boxes + 0.5)
    top = max(map(max, data)) * 1.1
    bottom = 0
    ax1.set_ylim(bottom, top)
    xticks_names = plt.setp(ax1, xticklabels=unit_labels)
    plt.setp(xticks_names, fontsize=8)

    pos = np.arange(num_boxes) + 1
    upper_labels = [str(np.round(s, 2)) for s in medians]
    weights = ['bold', 'semibold']
    for tick, _ in zip(range(num_boxes), ax1.get_xticklabels()):
        k = tick % 2
        ax1.text(pos[tick], top - (top*0.05), upper_labels[tick],
                 horizontalalignment='center', size='medium', weight=weights[k],
                 color=box_colors[k])

    #plt.figtext(0.80, 0.08, 'Local', backgroundcolor=box_colors[0], color='black', weight='roman', size='small')
    #plt.figtext(0.80, 0.045, 'Remote', backgroundcolor=box_colors[1], color='white', weight='roman', size='small')

    plt.plot()
    if file_name is None:
        plt.savefig(scan_type + ".pdf", format="pdf")
        plt.savefig(scan_type + ".svg", format="svg")
        print("Saved plots in {}".format(scan_type + ".pdf"))
    else:
        plt.savefig(file_name + ".pdf", format="pdf")
        plt.savefig(file_name + ".svg", format="svg")
        print("Saved plots in {}".format(file_name + ".pdf"))
    #plt.show()

def main():
    #ITERATIONS = 300
    #print("Running {} iterations".format(ITERATIONS))
    #for i in tqdm(range(ITERATIONS)):
#        os.system("./build/src/tuk_numa_benchmark --benchmark_format=json > benchmark_{}.json".format(i))

    title = 'Comparison of number of columns for sequential access'
    x_label = 'Number of Columns'
    sorted_keys = ['BM_ColumnScan_1M_Rows__LocalCols/1', 'BM_ColumnScan_1M_Rows__RemoteCols/1',
                  'BM_ColumnScan_1M_Rows__LocalCols/2', 'BM_ColumnScan_1M_Rows__RemoteCols/2',
                  'BM_ColumnScan_1M_Rows__LocalCols/4', 'BM_ColumnScan_1M_Rows__RemoteCols/4',
                  'BM_ColumnScan_1M_Rows__LocalCols/8', 'BM_ColumnScan_1M_Rows__RemoteCols/8']
    plot_benchmarks(title, x_label, sorted_keys)
    title = 'Comparison of number of rows for random access'
    x_label = 'Number of Rows'
    sorted_keys = ['BM_RowScan_1M_Rows__LocalCols/10', 'BM_RowScan_1M_Rows__RemoteCols/10',
                  'BM_RowScan_1M_Rows__LocalCols/100', 'BM_RowScan_1M_Rows__RemoteCols/100',
                  'BM_RowScan_1M_Rows__LocalCols/1000', 'BM_RowScan_1M_Rows__RemoteCols/1000']
                  #'BM_RowScan_1M_Rows__LocalCols/10000', 'BM_RowScan_1M_Rows__RemoteCols/10000']
    plot_benchmarks(title, x_label, sorted_keys)


    title = 'Comparison of number of join partners for local access'
    x_label = 'Number of Join Partners'
    sorted_keys = ["BM_Join__LocalTables/2000", "BM_Join__LocalTables/10000",
                   "BM_Join__LocalTables/100000", "BM_Join__LocalTables/1000000",
                   "BM_Join__LocalTables/2000000"]
    plot_benchmarks(title, x_label, sorted_keys, file_name="Join_Local")

    title = 'Comparison of number of join partners when one table is local and one is remote'
    x_label = 'Number of Join Partners'
    sorted_keys = ["BM_Join__LocalTable_RemoteTable/2000", "BM_Join__LocalTable_RemoteTable/10000",
                   "BM_Join__LocalTable_RemoteTable/100000", "BM_Join__LocalTable_RemoteTable/1000000",
                   "BM_Join__LocalTable_RemoteTable/2000000"]
    plot_benchmarks(title, x_label, sorted_keys, file_name="Join_Local_Remote")

    title = 'Comparison of number of join partners when both tables are on the same remote node'
    x_label = 'Number of Join Partners'
    sorted_keys = ["BM_Join__SameRemoteTables/2000", "BM_Join__SameRemoteTables/10000",
                   "BM_Join__SameRemoteTables/100000", "BM_Join__SameRemoteTables/1000000",
                   "BM_Join__SameRemoteTables/2000000"]
    plot_benchmarks(title, x_label, sorted_keys, file_name="Join_Same_Remote")

    title = 'Comparison of number of join partners when both tables are on different remote nodes'
    x_label = 'Number of Join Partners'
    sorted_keys = ["BM_Join__DifferentRemoteTables/2000", "BM_Join__DifferentRemoteTables/10000",
                   "BM_Join__DifferentRemoteTables/100000", "BM_Join__DifferentRemoteTables/1000000",
                   "BM_Join__DifferentRemoteTables/2000000"]
    plot_benchmarks(title, x_label, sorted_keys, file_name="Join_Different_Remote")

if __name__ == '__main__':
    main()
