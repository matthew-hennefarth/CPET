# Classical Protein Electric Field Topology
Tool used to compute the classical electric field from a protein structure (or structures) as well as sample topological data.

## System Requirements
- CMake Version >= 3.11
- Clang Version >= 12.0.0 or GCC Version >= 9.3.0
- C++ Standard >= 17

## Installation
### Download Binaries
Binaries (version 0.0.0) are available for [Linux](https://github.com/matthew-hennefarth/CPET/releases/download/v0.0.0/cpet_Linux-x86_64) (x86_64 with gcc >= 9.3) and [MacOSX](https://github.com/matthew-hennefarth/CPET/releases/download/v0.0.0/cpet_MacOSX-ARM64) (ARM 64, 'Apple Silicon'). If you receive an error such as 

    /usr/lib64/libstdc++.so.6: version `GLIBCXX_3.4.26' not found

you will have to compile from source (see below).

### Compile from source
Download source code from latest [release](https://github.com/matthew-hennefarth/CPET/releases/download/v0.0.0/Full-CPET-Source.tar). Or, for latest (possible unstable), clone this repository and the submodules using

    git clone https://github.com/matthew-hennefarth/CPET.git
             
Then, go into the project and make a build directory, `mkdir build` and go into this directory, `cd build`. We will then run cmake to build the binary.

    cmake ../
    make

This should create the executable, `cpet`, in `CPET/bin` to be used.

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

## Protein Trajectories
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

## Choosing an Origin and Basis Set
The electric field is a vector quantity with an x,y,z component and which is dependent on the basis that we are using. Often times, we are interested in a primary direction of the electric field (along some bond) and as such want to measure the field in this alternative basis set ({x<sub>1</sub>, x<sub>2</sub>, x<sub>3</sub>}) along the trajectory. We can specify the translation and rotation of the standard basis to an alternative basis using the keyword `align arg1 arg2 arg3` in the options file. 1 or 3 parameters then follows. The first will be the new center. The second and third option are the new directions for which we should align our basis in, relative to the center just specified. That is, x<sub>1</sub> = arg2 - arg1. We then guess x<sub>2</sub> to arg3-arg1. Since x<sub>1</sub> and x<sub>2</sub> are not always orthogonal, we then use the cross-product to determine x<sub>3</sub> = x<sub>1</sub> x x<sub>2</sub>. Now, x<sub>1</sub> and x<sub>3</sub> are orthonal, and we finally complete the orthogonal basis by taking x<sub>2</sub> = x<sub>3</sub> x x<sub>1</sub>. All vectors are normalized to createthe orthonormal basis. Some examples:

    align 1:5:4
   
In the above example, we only change the origin to (1,5,4).
    
    align 1:5:4 1:6:4 1:5:5   

In this example, will change the origin to (1,5,4) and set the x<sub>1</sub> to (1,6,4) - (1,5,4) = (0,1,0). The x<sub>3</sub>-direction will be specified by the normal vector to the plane created by (0,1,0) and (1,5,5) - (1,5,4), or (0,0,1). That means, x<sub>3</sub> = (1,0,0). To find x<sub>2</sub>, we take the cross product between (1,0,0) and (0,1,0), which is (0,0,1). 

When we specify a new center and basis with the `align` keyword, all of the commands in the options file will then take place in this new frame. That is, 
    
    align 1:5:4
    field 0:0:0
    
will first translate the system to 1:5:4 (the new origin), and then compute the field at 0:0:0 (which is really 1:5:4 in the original pdb coordinates). If we did
    
    align 1:5:4
    field 1:0:0
    
we would be calculating the field at (2,5,4). While this is not too interesting for simple field calculations, it becomes useful for protein trajectories, where we want the field at some atom (which could be moving) and along some bond (which the direction could also be moving). Furthermore, for the more complex options of `plot` and `topology`, we use the align section to orient our 3D volumes.

## Further Command-line Options
The additional options (beyond `-o` and `-p` one can specify are
- `-d`
- `-c` 
- `-t` 
- `-O` 
- `-h` 
- `-v`

`-d`, `-h`, and `-v` are boolean flags which signal 'debug', 'help message', and 'verbose' respectively. `-c` specifies an alternative file from which to reference the partial atomic charges for each atom. Again, the charges should be in the occupancy column. `-t` tells the program the number of threads to use when computing topological quantities. Note that there is ONLY parallizability for the `topology` keyword in the options file. `-O` specifies an alternative output file prefix. Note that if this is not specified, then the default file prefix will be the pdb file from the `-p` option.

## Uses
This code uses the following C++ libraries:
- Eigen
- cs_libguarded
- cxxopts
- spdlog

