# TuK NUMA Benchmarks

## Setup
Requires libnuma, so it will only compile on Linux; for macOS you can use a Docker container (see below).

1. Requirements: gcc, cmake, libnuma-dev
2. Clone this repository and `cd` into it
3. `mkdir build && cd build`
4. `cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release ..` (or set build type to Debug if needed)
5. `make -j8`
6. Run `./src/tuk_numa_benchmark`

## Develop with a Docker container (e.g. on macOS)
1. Assuming docker is installed [https://store.docker.com/editions/community/docker-ce-desktop-mac](https://store.docker.com/editions/community/docker-ce-desktop-mac)
2. Build a docker image using the included Dockerfile: `cd <repo> && docker build -t tuk-numa ./docker`
3. After creating the build directory, you can run CMake and make inside a container, e.g.:
```
docker run --rm \
    -v `pwd`:`pwd` \
    -w `pwd`/build \
    tuk-numa /bin/bash -c "cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Debug .."
```
4. Because all this seems pretty tedious to type, I've set up a task and debug configuration for Visual Studio Code:
   1. Once you've opened the repo folder in VS Code
   2. Press Cmd-Shift-P and select "Tasks: Run Task" to build the docker image and initialize the CMake build directory
   3. Compile the project with Cmd-Shift-B
   4. Set a breakpoint and start debugging in VS Code, or select "Run Test Task" from the Cmd-Shift-P menu to run without a debugger

## Results
Based on 7 computations (took 9 hours)
### Task 1
![columnscan_20m](https://user-images.githubusercontent.com/6676439/28309789-ca1863f6-6baa-11e7-87b0-49ba8d6de57c.png)
![columnscan_100m](https://user-images.githubusercontent.com/6676439/28309792-ca1cba1e-6baa-11e7-8949-856fc0fd9f47.png)
![rowscan_20m](https://user-images.githubusercontent.com/6676439/28309807-ca5e230a-6baa-11e7-8deb-76b35553113c.png)
![rowscan_100m](https://user-images.githubusercontent.com/6676439/28309808-ca611862-6baa-11e7-83f4-480f617045e2.png)
### Task 2
![join_20m_2000](https://user-images.githubusercontent.com/6676439/28309793-ca1d4754-6baa-11e7-9015-679821597766.png)
![join_20m_20000](https://user-images.githubusercontent.com/6676439/28309790-ca1bebde-6baa-11e7-91d1-e3ecba5eee59.png)
![join_20m_200000](https://user-images.githubusercontent.com/6676439/28309791-ca1c8c06-6baa-11e7-9a4c-2e9a47fad93a.png)
![join_20m_2000000](https://user-images.githubusercontent.com/6676439/28309794-ca1e264c-6baa-11e7-8e33-8569c21e6403.png)
![join_100m_2000](https://user-images.githubusercontent.com/6676439/28309795-ca2f064c-6baa-11e7-9128-070fcc189bab.png)
![join_100m_20000](https://user-images.githubusercontent.com/6676439/28309796-ca3308f0-6baa-11e7-843f-3b0a97d4e51e.png)
![join_100m_200000](https://user-images.githubusercontent.com/6676439/28309800-ca431182-6baa-11e7-90f7-5acfcd0527e0.png)
![join_100m_2000000](https://user-images.githubusercontent.com/6676439/28309797-ca35ee80-6baa-11e7-9391-cc9cbc8156f3.png)
![join_local_20m](https://user-images.githubusercontent.com/6676439/28309801-ca45227e-6baa-11e7-905a-4dca53a137b4.png)
![join_local_100m](https://user-images.githubusercontent.com/6676439/28309802-ca48b40c-6baa-11e7-9341-fb82318f60f2.png)
![join_local_remote_20m](https://user-images.githubusercontent.com/6676439/28309803-ca4c20a6-6baa-11e7-99fd-151eff361473.png)
![join_local_remote_100m](https://user-images.githubusercontent.com/6676439/28309804-ca4c455e-6baa-11e7-89fd-3ae63df6b8aa.png)
![join_same_remote_20m](https://user-images.githubusercontent.com/6676439/28309809-ca7d2552-6baa-11e7-8303-31aa6dd5033e.png)
![join_same_remote_100m](https://user-images.githubusercontent.com/6676439/28309805-ca59cf30-6baa-11e7-9ffc-23fc82079e2f.png)
![join_different_remote_20m](https://user-images.githubusercontent.com/6676439/28309798-ca361b44-6baa-11e7-9e11-4fa6863bbcca.png)
![join_different_remote_100m](https://user-images.githubusercontent.com/6676439/28309799-ca364970-6baa-11e7-8714-992b1814610a.png)
### Task 3
![query_plots](https://user-images.githubusercontent.com/6676439/28309806-ca5d5fba-6baa-11e7-92f4-12815408091f.png)

