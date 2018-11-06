# Virtual Turing Machine

This is an interpreter for turing machine programs defined in comma separated values charts, with the columns:
State Name, Read Char, Write Char, Move head Direction and Next State

## Building

### Build Requirements
* CMake
* C Compiler (MSVC, Clang, GCC)

### Building

#### Linux:
* clone the git repository
* create a `build` folder and change into it with `mkdir build; cd build`
* run `cmake ..`
* build by running `make`
* the executable will be in `/build/src/VTM`

#### Windows:
* Trust me its easiest to just open the folder in visual studio (with the native c++ stuff) and build it from there.

## Running

The program takes the following arguments:
`./VTM [-flags] program_file tape [cycle_limit] [right_fill_char] [left_fill_char] [tape-offset]`

* `program_file` the csv file with the program
* `tape` a string with the initial data read by the machine
* `fill_chars` if the machine reads over the limits of the defined `tape` these chars will be generated
* `tape-offset` normally the machine starts on the first element of the tape this defines an offset

### Flags
* `-a` auto stepping is currently always true since no stopping is implemented
* `-q` won't print any intermediate steps just the resulting tape
* `-f` might let you provide a path to a text file with the path in the future but not implemented right now