# Performace award programing
Course by Casey Muratori's at https://www.computerenhance.com

The course involes learning how modern CPUs work and run programs, how to estimate and predict the expected speed of software code, 
and the optimization techniques.

 
## Status -> currently working through  
This repo will be updated as I progress through the course and will contain my follow along exercises and homework from the course.


## 8086 Simulator

#### 8086 Decoder and Assembler 

Built an accurate decoder that can take in a stream of bytes and output the correct assembly instructions. Being able to then re-assemble the instructions back into the original byte stream to test whether there is a difference. 

#### 8086 Simulation    

The 8086 simulator is a software implementation of the Intel 8086 microprocessor. It has the same instruction set and architecture of the original 8086, whilst seeing the changes in registers and memory as the instructions are executed. 

#### To build

To build the project, you can use the provided Makefile. Simply run the following command  int he 8086_cpu directory once you have downloaded the repository:

```bash
gcc -O0 8086_sim.c -o 8086_sim
```
To run the assemblfy file and print to stdout the decoded instructions
```bash
8086_sim <assembly_file> 
```
Run all the tests in the 'tests' folder and automatically check if the ouput it correct.
```bash
./script_test_decoder.sh 
```

Passing the '-exec' flag will run the instructions through the simulator and print the changes in registers and memory as the instructions are executed.
```bash
8086_sim -exec <assembly_file> 
```
