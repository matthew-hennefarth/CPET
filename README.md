# Classical Protein Electric Field Topology
Tool used to compute the classical electric field from a protein structure (or structures) as well as sample topological data.

## Requirements
- CMake Version >= 3.17
- Clang Version >= 12.0.0 or GCC Version >= 9.3.0
- Eigen
- cs_libguarded
- cxxopts
- spdlog

***

## Installation
Place the Eigen, cs_libguarded, cxxopts, and spdlog header file directories in the libs/include. The structure should be like the following:  

    CPET
     |- cmake  
     |- include  
     |- libs  
     |- include 
         |- Eigen 
             |- <Eigen .h files>
         |- cs_libguarded 
             |- <Eigen .h files>
         |- cxxopts
             |- <Eigen .h files>
         |- spdlog
             |- <Eigen .h files>
     |- src
             
Then, make a build directory, `mkdir build` and go into this directory, `cd build`. We will then run cmake to build the binary.

    cmake ../
    make

This should create the executable, `cpet`, in `CPET/bin/Release` to be used.
