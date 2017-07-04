#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import glob
import numpy as np
from matplotlib.patches import Polygon
from tqdm import tqdm
import matplotlib

matplotlib.use('Agg')
import matplotlib.pyplot as plt


def get_merged_benchmarks(merged_dict, f_dict):
    for benchmark in f_dict['benchmarks']:
        merged_dict[benchmark['name']].append(benchmark['real_time'])
    return merged_dict


def plot_benchmarks(title, x_label, sorted_keys, file_name=None, legend_texts=[], unit_labels=None):
    scan_type = sorted_keys[0].split('_')[1]
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

    unit_number = len(sorted_keys) // 2
    if unit_labels is None:
        unit_labels = [label.split('/')[1] for label in sorted_keys]
    data = [merged_dict[k] for k in sorted_keys]

    fig, ax1 = plt.subplots(figsize=(10, 6))
    plt.subplots_adjust(left=0.075, right=0.95, top=0.9, bottom=0.25)

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
    ax1.set_ylabel('Time [microseconds]')

    # Now fill the boxes with desired colors
    box_colors = ['darkkhaki', 'royalblue']
    num_boxes = unit_number * 2
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
        if len(legend_texts) == 0:
            k = 0
        else:
            k = tick % len(legend_texts)
        ax1.text(pos[tick], top - (top * 0.05), upper_labels[tick],
                 horizontalalignment='center', size='medium', weight=weights[k],
                 color=box_colors[k])

    for i in range(len(legend_texts)):
        legend = legend_texts[i]
        plt.figtext(0.80, 0.15 - 0.03 * i, legend, backgroundcolor=box_colors[i], weight='roman', size='small')

    plt.plot()
    if file_name is None:
        plt.savefig(scan_type + ".pdf", format="pdf")
        plt.savefig(scan_type + ".png", format="png")
        print("Saved plots in {}".format(scan_type + ".pdf"))
    else:
        plt.savefig(file_name + ".pdf", format="pdf")
        plt.savefig(file_name + ".png", format="png")
        print("Saved plots in {}".format(file_name + ".pdf"))


def main():
    ITERATIONS = 300
    print("Running {} iterations".format(ITERATIONS))
    for i in tqdm(range(ITERATIONS)):
        os.system("./build/src/tuk_numa_benchmark --benchmark_format=json > benchmark_{}.json".format(i))

    for rows_number in ["20", "100"]:
        # Sequential Access on Columns
        title = "Comparison of number of columns for sequential access ({}M Rows)".format(rows_number)
        x_label = 'Number of Columns'
        legend_texts = ['Local', 'Remote']
        sorted_keys = []
        for column_number in [1, 2, 4, 8]:
            for location in ["LocalCols", "RemoteCols"]:
                sorted_keys.append("BM_ColumnScan_{}M_Rows__{}/{}".format(rows_number, location, column_number))
        plot_benchmarks(title, x_label, sorted_keys, file_name="ColumnScan_{}M".format(rows_number),
                        legend_texts=legend_texts)

        # Random Access on Rows
        title = "Comparison of number of rows for random access ({}M Rows)".format(rows_number)
        x_label = 'Number of Rows'
        sorted_keys = []
        for row_number in [1, 10, 100, 1000]:
            for location in ["LocalCols", "RemoteCols"]:
                sorted_keys.append("BM_RowScan_{}M_Rows__{}/{}".format(rows_number, location, row_number))
        plot_benchmarks(title, x_label, sorted_keys, file_name="RowScan_{}M".format(rows_number),
                        legend_texts=legend_texts)

        # Joins grouped by distribution
        x_label = 'Number of Join Partners'
        titles = ['Comparison of number of join partners for local access',
                  'Comparison of number of join partners when one table is local and one is remote',
                  'Comparison of number of join partners when both tables are on the same remote node',
                  'Comparison of number of join partners when both tables are on different remote nodes']
        locations = ['LocalTables', 'LocalTable_RemoteTable', 'SameRemoteTables', 'DifferentRemoteTables']
        file_names = ['Join_Local', 'Join_Local_Remote', 'Join_Same_Remote', 'Join_Different_Remote']

        for title, location, file_name in zip(titles, locations, file_names):
            sorted_keys = ["BM_Join_{}M_Rows__{}/{}".format(rows_number, location, partner_number) for partner_number in
                           [2000, 20000, 200000, 2000000]]
            plot_benchmarks(title + " ({}M Rows)".format(rows_number), x_label, sorted_keys,
                            file_name=file_name + "_{}M".format(rows_number))

        # Joins grouped by partner number
        legend_texts = ['Local-Local', 'Local-Remote', 'Remote-Remote (same)', 'Remote-Remote (different)']
        table_distributions = ['LocalTables', 'LocalTable_RemoteTable', 'SameRemoteTables', 'DifferentRemoteTables']
        join_partners = [2000, 20000, 200000, 2000000]
        x_label = ''
        for join_partner in join_partners:
            title = 'Comparison of join partner locations for ' + str(join_partner) + ' partners'
            sorted_keys = ["BM_Join_{}M_Rows__{}/{}".format(rows_number, distribution, join_partner) for distribution in
                           table_distributions]
            plot_benchmarks(title + " ({}M Rows)".format(rows_number), x_label, sorted_keys,
                            file_name="Join_{}M_{}".format(rows_number, join_partner), legend_texts=[],
                            unit_labels=legend_texts)


if __name__ == '__main__':
    main()
