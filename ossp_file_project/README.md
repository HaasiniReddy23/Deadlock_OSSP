# Linux File Management System

## Overview
A Linux Operating Systems project developed using C to demonstrate file management through system calls and file descriptors.

## Objectives
- Understand Linux file descriptors.
- Implement file opening and closing.
- Read and write file contents.
- Retrieve file metadata.
- Demonstrate file management and resource handling.

## Technologies Used
- C Programming
- Ubuntu Linux
- GCC Compiler
- Linux System Calls

## Modules
1. **Open and Close:** Opens an existing file and closes it.
2. **Read File:** Reads file contents and demonstrates file-position handling.
3. **Write and Edit:** Appends or replaces file contents.
4. **File Information:** Retrieves file metadata such as size, permissions, and ownership.
5. **Close File:** Demonstrates closing and releasing file descriptors.

## Compilation
Compile an individual module using GCC:

```bash
gcc module1_open_close/module1_open_close.c -o module1
```

Run it using:

```bash
./module1
```

Repeat the process for the other modules, adjusting the source-file path and executable name.

## Purpose
This project demonstrates fundamental Linux file I/O operations and operating system concepts through practical C programs.
