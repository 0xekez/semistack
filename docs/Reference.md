## Module

A module is a container for a list of instructions. Modules are added to the virtual machine and the virtual machine handles their linking and assembly. Typically Modules are used to represent units of code. For example, a new Module might be created for every function in the source program.

## VM

An instance of a VM class takes a collection of Modules and runs them. When a module is added to a VM, the VM resolves all local jumps into absolute jumps and removes all label instructions from the Module. Before running a Module, the VM replaces all inter-module jumps inside of it with special absolute inter-module jump instructions.

## Memory

The VM provides both local and global memory. Local memory lasts for the duration of a Module's execution and global memory lasts for the duration of the VM's lifetime.

## Types

For the moment, the VM supports strings and floating point numbers.

## Immediate

Some instructions take an immediate value. This value can be either a float or a string. It is an error to not provide an immediate to an instruction that requires one and an error to provide an immediate to one that does not.

## Instructions

### PI - Push Immediate.

Pushes the value in its immediate to the top of the stack.

### SL - Store Local.

Pops a value off the top of the stack and stores it in local memory at the index specified by its immediate.

### LL - Load local.

Pushes the value in local memory at the index provided by its immediate to the top of the stack.

### SG - Store global.

Same as store local except stores the value on top of the stack in global memory.

### LG - Load global.

Same as load local except loads a value from global memory.

### PUTS

Pops a value off the top of the stack and prints it.

### COPY

Duplicates the value on top of the stack and pushes it to the top.

### EXIT

Stops execution of all Modules on the virtual machine.

### RET - Return.

Stops execution of the current module and resumes execution at the calling one.

### ADD

Adds the top two values on the stack, popping them, and then pushes the result. The value on top of the stack is the right operand and the next one is the left.

### SUB

Subtracts the top two values on the stack, popping them, and then pushes the result. The value on top of the stack is the right operand and the next one is the left.

### MUL

Multiplies the top two values on the stack, popping them, and then pushes the result. The value on top of the stack is the right operand and the next one is the left.

### DIV

Divides the top two values on the stack, popping them, and then pushes the result. The value on top of the stack is the right operand and the next one is the left.

### JUMP

Jumps to the label specified by the instructions immediate.

### CALL

Calls the module specified by the instructions immediate. A return instruction in the called module will cause execution to resume immediately after the call instruction.

### JEQ

Jumps if the values on top of the stack are equal. Pops the compared values off of the stack.

### JNEQ

Jumps if the values on top of the stack are not equal. Pops the compared values off of the stack.

### JLT

Pops two values from the stack. Jumps of the second value popped from the stack is less than the first one.

### JGT

Pops two values from the stack. Jumps of the second value popped from the stack is greater than the first one.

### LABEL

Takes a string as an argument. Jump instructions can jump to the label. Labels are removed from Modules by the VM before it is run.