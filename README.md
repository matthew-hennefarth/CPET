# Classical Protein Electric Field Topology
Tool used to compute the classical electric field from a protein structure (or structures) as well as sample topological data.

## System Requirements
- CMake (Version 3.11 or higher)
- Clang Version >= 12.0.0 or GCC Version >= 9.3.0
- gnuplot (only for plotting)

## Installation
Make sure that
### Download Binaries
Binaries (version 0.1.0) are available for [Linux](https://github.com/matthew-hennefarth/CPET/releases/download/v0.1.0/cpet_Linux-x86_64) (x86_64 with gcc >= 9.3) and [MacOSX](https://github.com/matthew-hennefarth/CPET/releases/download/v0.1.0/cpet_MacOSX-ARM64) (ARM 64, 'Apple Silicon'). If you receive an error such as 

    /usr/lib64/libstdc++.so.6: version `GLIBCXX_3.4.26' not found

you will have to compile from source (see below).

### Compile from source
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
