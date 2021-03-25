# Multi-pass Multiway Merge Sort
*Solution to an assignment for Database Management Systems*

An efficient way to sort data that is too large to fit into system memory. In this implementation there are 22 blocks of memory.

The program takes a CSV named "Emp.csv", containing the following columns: ID, name, age, and salary.

The output will be in EmpSorted.csv, with the records sorted in increasing order according to ID. 

This programs uses C++ file I/O to create multiple "run-files" which are used to output memory from the memory buffer, to free it from constraint.

The file can be compiled:

`g++ sort.cpp -o sort`

The program can be run with an Emp.csv defined in the same directory.