# Classical Protein Electric Field Topology
Software used to compute the classical electric field from a protein structure (or structures) as well as sample the electric field topological data.

## System Requirements
- gnuplot (only for plotting)

You can easily install gnuplot on Mac using homebrew

    brew install gnuplot

On Linux, gnuplot should be in your package manager repo's already. For example on Ubuntu

    sudo apt-get update
    sudo apt-get install gnuplot

## Installation
### Download Binaries
Binaries (version 0.4.1) are available for [Linux](https://github.com/matthew-hennefarth/CPET/releases/download/v0.4.1/cpet_Linux-x86_64) (x86_64 architecture) and [MacOSX](https://github.com/matthew-hennefarth/CPET/releases/download/v0.4.1/cpet_MacOSX-ARM64) (ARM 64, 'Apple Silicon'). Linux binaries are fully statically linked; however, MacOS binaries are still dynamically linked to system libraries. If you receive an error such as 

    /usr/lib64/libstdc++.so.6: version `GLIBCXX_3.4.26' not found

or something similar, you will have to compile from source (see below).

### Compile from source
#### Requirements
- CMake (Version 3.14 or higher)
- A compiler that implements C++17 features or newer

Note, this may not compile or run on Windows.

#### Compilation Steps
Download or clone this repository using

    git clone https://github.com/matthew-hennefarth/CPET.git
             
Then, go into the project and make a build directory, `mkdir build` and go into this directory, `cd build`. We will then run cmake to build the binary.

    cmake ../
    make

This should create the executable, `cpet`, in `CPET/bin` to be used.

## Usage
Calling `cpet -h` will output the various options available. What is always needed is a pdb file and an options file. The pdb file should contain the partial atomic charges in the occupancy column (columns 55-60) for each atom. I recommend using the [Atomic Charge Calculate II](https://acc2.ncbr.muni.cz/) for generating partial atomic charges, and it will place the charges in the occupancy column automatically. The options file will tell the program what to compute and how.

## Acknowledgements
This code uses the following C++ libraries:
- [Eigen](https://gitlab.com/libeigen/eigen)
- [cs_libguarded](https://github.com/copperspice/cs_libguarded)
- [cxxopts](https://github.com/jarro2783/cxxopts)
- [spdlog](https://github.com/gabime/spdlog)
- [matplotplusplus](https://github.com/alandefreitas/matplotplusplus)
