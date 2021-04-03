# Classical Protein Electric Field Topology
Tool used to compute the classical electric field from a protein structure (or structures) as well as sample topological data.

## Requirements
- CMake Version >= 3.17
- Clang Version >= 12.0.0 or GCC Version >= 9.3.0
- Eigen
- cs_libguarded
- cxxopts
- spdlog

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

## Usage
Calling `cpet -h` will output the various options available. What is always needed is a pdb file and an options file. The pdb file should contain the partial atomic charges in the occupancy column (columns 55-60) for each atom. I recommend using the [Atomic Charge Calculate II](https://acc2.ncbr.muni.cz/) for generating partial atomic charges, and it will place the charges in the occupancy column automatically. The options file will tell the program what to compute and how.

### A First Calculation
Given a pdb that is properly formatted with the charges in the occupancy column, we can calculate the electric field at the origin (0,0,0) using the following options file:

    field 0:0:0
    
Calling the program as 

    cpet -p my_pdb_file.pdb -o options
   
 will produce output similar to
 
     [center] ==>> 0 0 0
     [User Basis]
     1 0 0
     0 1 0
     0 0 1
     =~=~=~=~[Field at 0:0:0]=~=~=~=~
     Field: -0.0728784   0.108112  0.0824354 [0.15425672813686317]
     
Furthermore, you should see a new file `my_pdb_file.pdb.field` which contains

    #my_pdb_file.pdb
    #0:0:0
    -0.0728784   0.108112  0.0824354

To understand the output, lets consider the text from the standard out. Note that we print out the center that we are using, which is at (0,0,0). Next we printout the user basis that we are using. Since we have not specified any alternative basis, we use the standard basis. Finally, we printout the field at the location 0:0:0. Note that the value in brackets `[0.15425672813686317]` is the magnitude of the field at that position. Note that the field output is in units of V/Å. Note that the output in the `my_pdb_file.pdb.field` file is a condensed version of this data.

## Further Command-line Options
The additional options (beyond `-o` and `-p` one can specify are
- -d
- -c 
- -t 
- -O 
- -h 
- -v
`-d`, `-h`, and `-v` are boolean flags which signal 'debug', 'help message', and 'verbose' respectively. `-c` specifies an alternative file from which to reference the partial atomic charges for each atom. Again, the charges should be in the occupancy column. `-t` tells the program the number of threads to use when computing topological quantities. Note that there is ONLY parallizability for the `topology` keyword in the options file. `-O` specifies an alternative output file prefix. Note that if this is not specified, then the default file prefix will be the pdb file from the `-p` option.
