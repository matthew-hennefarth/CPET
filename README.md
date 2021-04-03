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

## Protein Trajectory
One the key strengths of this program is computing the electric field along a protein trajectory. A single pdb file should contain all the different structures, seperated by the `ENDMDL` line. That is to say, if we have a pdb file, `traj.pdb` which is as such:

    ATOM      1  N   MET A   1     105.084 111.090  87.799 -0.836  0.00           N
    ATOM      2  CA  MET A   1     104.846 109.857  87.085 -0.170  0.00           C
    ATOM      3  C   MET A   1     105.505 108.643  87.741  0.627  0.00           C
    ATOM      4  O   MET A   1     106.241 107.910  87.076 -0.847  0.00           O
    ...
    ENDMDL
    ATOM      1  N   MET A   1     105.384 111.190  87.769 -0.836  0.00           N
    ATOM      2  CA  MET A   1     104.346 109.957  87.082 -0.170  0.00           C
    ATOM      3  C   MET A   1     105.205 108.743  87.753  0.627  0.00           C
    ATOM      4  O   MET A   1     106.120 107.650  87.064 -0.847  0.00           O
    ...
    ENDMDL

and an options file as:

    field 0:0:0

calling `cpet -p traj.pdb -o options` will output something similar to:

    [center] ==>> 0 0 0
    [User Basis]
    1 0 0
    0 1 0
    0 0 1
    [center] ==>> 0 0 0
    [User Basis]
    1 0 0
    0 1 0
    0 0 1
    =~=~=~=~[Field at 0:0:0]=~=~=~=~
    Field: -0.108758  0.147238  0.122794 [0.22042156737787447]
    Field: -0.0369991   0.068987  0.0420764 [0.08887381298443407]

Notice how we have calculate the electric field at the origin for both structures in the pdb file! Though, for trajectory files, it becomes cumbersome to assign partial atomic charges for every atom in every structure. Instead, if we take the first frame and generate a .pdb with the partial atomic charges in the occupancy column, we can use this as a template for all of the structures in our trajectory. As such, we notify the program to use the charges from a different file using the `-c` option. For example, if the first frame contains the partial atomic charges for our trajectory (in the file `frame0.pdb`), then we could have called `cpet -p traj.pdb -c frame0.pdb -o options` instead. Note that this means that the charges are fixed throughout the trajectory, whereas they may be dynamic. Though, often in classical force fields, the partial atomic charges are fixed anyways, and it is not too poor of an approximation.

## Further Command-line Options
The additional options (beyond `-o` and `-p` one can specify are
- `-d`
- `-c` 
- `-t` 
- `-O` 
- `-h` 
- `-v`
`-d`, `-h`, and `-v` are boolean flags which signal 'debug', 'help message', and 'verbose' respectively. `-c` specifies an alternative file from which to reference the partial atomic charges for each atom. Again, the charges should be in the occupancy column. `-t` tells the program the number of threads to use when computing topological quantities. Note that there is ONLY parallizability for the `topology` keyword in the options file. `-O` specifies an alternative output file prefix. Note that if this is not specified, then the default file prefix will be the pdb file from the `-p` option.
