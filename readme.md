# Cache Simulator

A cache simulator written in C++. It simulates its own main memory and maintains a cache for it.  

On program startup, the cache undergoes a 'cold-start', ie. it is completely empty.  

The main memory is populated with randomly generated values on program startup. 

## Dependencies and Running Locally 

1. C++ 17 >=
2. Standard Library (stdc++.h)

To run, simply clone the `.cpp` file and build it using a compiler. The `.exe` compiled should run without any issues. 

## Features
* The user can specify the following constraints for the simulator: 

    1. Main memory size
    2. Address width
    3. Cache size
    4. Block size 
    5. Associativity

* The program also holds functionality to print the main and cache memory at each step of the program. However, there are currently not in use, but are present in the code file.

## Limitations
* Since the program simulates an actual main memory, I've coded it so that a total of 4096 locations (0xfff) are valid. This can be tuned to the user's preferance by modifying the value of MEM_SIZE constant to another power of 2. 