
# Multithreaded Log Analysis System

## Project Description
A C program that analyzes log files using multiple threads to identify errors, warnings, and suspicious events.

## Technologies Used
- C
- POSIX Threads (Pthreads)
- GCC
- Ubuntu Linux

## Compilation
gcc -O2 -Wall -pthread log_analyzer.c -o log_analyzer

## Execution
./log_analyzer sample.log 3

## Features
- Counts log lines
- Detects errors and warnings
- Identifies predefined suspicious keywords
- Uses multiple threads
- Protects shared statistics using a mutex

## Limitations
Detection relies on predefined keywords, and the entire log file is loaded into memory.
