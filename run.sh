# Run 300 benchmarks and visualize the column and row scans in ColumnScan.pdf and RowScan.pdf

# Compile the C++ code
mkdir build && cd build
cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release ..
make -j8
cd ..

# The dependencies from requirements.txt should be installed
# Furthermore, you should already have python3-tk
python3 Boxplots.py
