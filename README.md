# Assignment 3

## Usage

1. Build from source
```
make clean

make
```
    - Creates binary in build directory
    - build/etf

2. Can also build using
```
gcc -o build/etf -Iinclude src/main.c
```

3. Run using
```
build/etf inputfile.txt no_workers
```
    - no_workers is an integer to indicate number of workers
    - input file can be in any directory relative to running directory

## Troubleshooting

1. include/main.h contains the macro NUM_ACCS used to define the max number of accounts that can be initialized. Update this number to be atleast the number of accounts in the input file

2. If facing syscall error please run the build/etf_debug build instead which is garunteed to run on any system unlike the build/etf binary.

