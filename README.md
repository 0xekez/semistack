

# Semistack

Semistack is a stack based virtual machine written in C++ designed to be reasonably performant while still remaining readable. I've made Semistack primarially as a learning exersise and it is still very much a work in progress.

## Getting Started

You can find the VM's implementation in `./semistack/` and a WIP version of [Lust](https://github.com/ZekeMedley/lust/) running on the VM in `./lust/`.

For the time being, Semistack is being developed in Xcode on my Mac and I haven't taken the time to put together an alternate build system. If you're using Xcode, you're in luck. If not, the build is pretty simple. There are no external dependencies and the project makes heavy use of `C++17` features. To compile the virtual machine just run:

```bash
clang++ -o vm -std=c++17 *.cpp
```

In order to run the compiled version of [Lust](https://github.com/ZekeMedley/lust/) that's being built here, you'll need to link it against the `.cpp` files in `./semistack/` .

## Your First Program

While the virtual machine doesn't have a parser wired up to it, the instruction set still lends itself to being typed out. Here is a simple hello world program.

```assembly
pi "Hello world!" ; push-immediate
puts
exit
```

To get that to run in the VM, you'll need to call it from somewhere and we can wire that up as follows:

```c++
#include "vm.hpp"
#include "module.hpp"

int main(int argc, const char * argv[])
{
    vm::Module main("main");
    
    main.addInstruction(InstType::pi, "Hello world!");
    main.addInstruction(InstType::puts);
    main.addInstruction(InstType::exit);
    
    VM v;
    v.addModule(std::move(main));
    v.run("main");
}
```



# VM

The end goal here is to write a simple virtual machine that I can later compile programming languages that I've written to. This is still a work in progress and this doccument holds my notes on the whole thing so far.

## Types of Virtual Machines

There are two primary types of VMs: stack machines, and register ones. 

Stack machines store information on a stack and operations pop things from said stack to get their arguments. The Web Assembly and Wren virtual machines are stack based. The advantage to stack machines is that they closely emulate the way that high level languages work. Their disadvantage is that they are slower than register based virtual machines usually (or at least this one is for sure).

Register machines are most widely used in hardware. They store the result of operations in high speed registers which allows their reuse across operations. RISC-V and LLVM are both register based. Register machines seem to be a little easier to optimize and the general consensus seems to be that they are faster.

## Stack Based VM

A pure stack based virtual machine is doomed to failure. I learned this the hard way. This is actually my second attempt at implementing a virtual machine. The reason that a purely stack based VM isn't realistic is that you can only use the value on the top of the stack at any given time. This is basically the equivalent of only using one variable. This is fine for very simple programs, but not for more realistic ones.

## Semistack VM

In an effort to keep the simplicity of a stack machine and maintain the ability to run normal programs, this virtual machine has call frames and global variables that allow for the usage of global and local variables. Using instructions like `sl - store local` and `ll - load local`, we can then write a program to compute the Fibonacci Sequence as follows. I've added the contents of the stack to the right of each line to making following along easier.

```assembly
    ; code        | stack
      copy        | n n
      pi 2        | n n 2
      jlt done    | n
      copy        | n n
      sl 1        | n
      pi 1        | n 1
      sub         | n-1
      call fib    | fib(n-1)
      ll 1        | fib(n-1) n
      pi 2        | fib(n-1) n 2
      sub         | fib(n-1) n-2
      call fib    | fib(n-1) fib(n-2)
      add         | fib(n)
    done:
      ret
```



# Notes

The following are my notes as I work on this project.

# Version 2

This version will be very similar to Version one and will likely share much of the same code. The two things that we're adding here are support for function calls and more types in the VM. As with before though, we'll start with just a floating point type.

## Limitations & Modifications

The trouble with a stack based VM is that you only ever have access to one value at a time. This basically boils down to needing to do all of your computations with a single variable - not possible. In order to get around this, you need some cleverness and not a pure stack based virtual machine. I'll do the following based on Crafting Interpreters and my own thoughts:

1. The stack will need to be able to hold a multipurpose Value type. Crafting Interpreters uses a tagged union, but we're in C++, so we can go ahead and use a variant type. I'm confident in the performance of `std::variant` now because it appears as if the f18 Fortran compiler uses it.
2. We'll need a call stack in addition to a regular stack. The call stack will hold Frames.
3. We'll need some concept of a Frame. This will need to hold local variables, and a program counter to tell where they left off when / if they call another function and get returned.

# Version 1

## Retrospective

I think in a lot of ways this was pretty well designed. Ultimately the reason that I'm killing it was just because of things that I didn't expect. For example, I didn't forsee the need to a call frame, nor did I add proper support for local variables.

### Things that I liked

1. Module dissassembly worked well.
2. Instruction representation was pretty spot on.
3. Linking / preprocessing also pretty spot on.
4. Cool output function method.
5. Good tests.

### Things I'd like to do differently

1. Need a notion of a call frame.
2. Be less aggressive about limiting the VM to just one type. I'm happy now to support strings and objects in the VM.
3. Rather than make use of a ton of `assert` statements, I should really have used a singleton logger type.

## Design

### Calls

When a function is called its arguments should be put on the top of the stack. On return, the returned value should be on the top of the stack. The return address should be stored in the `ra` register.

### Special Registers:

```assembly
; pc - the index of the instruction being executed
```

There is no need to have special ra value becasue we already handle these str

### RAM

The VM has some RAM which is sequential memory that we'll represent as an array. This can be used as a way to store local variables.

```assembly
; sl [index] [value] - stores value at index in RAM.
; ll [index] - stores value at index in RAM at the top of the stack.
```

### Code Representation

We'll represent code like LLVM does in Modules. A module will contain a dynamic array of instructions and each function will be contained in a module.

### Global State

For global state we'll hold all of our global values (functions, values) in a lookup table. To begin, I think that using a hash map will be fine, but if it turns out that is too slow, then we can consider assigning offsets to items and storing them in a dynamic array like Zeek.

### Types

For now, lets go ahead and only support the C++ `float` type. Going forward we ought to add support for integer types. Long term I'd like to support most of the simple types supported by [LLVM](https://llvm.org/docs/LangRef.html). Most importantly, we need to add support for lists. If we do that, we get most other things for free. Strings for example can be represented as list of ascii numbers.

### Representing Instructions

Instructions are represented in two phases. First, in a preprocessed form, and then in a processed one. In their preprocessed form they look like this:

```c++
using PreInst = std::pair<InstType, std::optional<std::variant<std::string, float>>;
```

This form, as one might imagine, isn't particularly quick. It is useful though because we want to be able to cleanly represent labels as strings for users of the VM. There are two passes that are run on instructions on this form:

1. The assembler. This resolves jumps that are inside of the Module. For example, might applies the following transformation:

   ```assembly
   label "foo"
   pi 3
   jump "foo"
   
   ; becomes:
   
   pi 3
   jump -1
   ```

   Labels that jump to a different Module are left in this phase and will be resolved once code is actually run.

2. The linker. The linker resolves these cross Module dependencies with a special instruction called `jumpm` -  *jump module*. In addition, the linker flattens all of the VM's modules into one sequential list. The `jumpm` instruction takes a Module's index into that list as an argument.

Instructions are represented as their opcode, and optionally, their immediate value.

```c++
using Inst = std::pair<InstType, std::optional<std::variant<std::string, float>>;
```

The inclusion of `std::string` here is bad at the moment and will need to go away. Its just there for now until we implement some sort of preprocessor which removes labels. We need strings until then because otherwise we can't represent jump instructions that go to labels.

In addition to adding a preprocessor, we'll also likely need to add some way for a jump instruction to specify that it goes to another module (and to store the name of that module). That way we don't need strings there either. My understanding is that in RISC-V that is handled by the linker because everything lives in some sort of address space. It seems possible that we'll have to implement something like that as well, or otherwise resolve jumps between modules to jumps to the modules index in the VM with some way to indicate that is the sort of jump we'd like to perform.

#### Linking

Once Modules have all been added to the preprocessor, we can perform a linking phase. At this phase there are two options that I've thought of so far:

1. We could return a list of assembled modules. Jumps between Modules could be represented as a `jumpm <index>` instruction that tells the VM to execute the module at that index.
2. We could assemble the Modules into one large Module, the way that LLVM's linker class does it.
3. We could resolve jumps at runtime with a lookup table and ignore linking alltogether.

Option 2 likely gives better performance as everything is just in on continuous list, option 1 is a middle groud, and option 3 favors convience over runtime performance.

### Jumps

There are two kinds of jumps:

1. Jumps that can be resolved inside of a Module.
2. Jumps that go to another Module.

For the first kind of jump, we can resolve those when a module is added to the VM. All other jumps will need to be resolved when they are evaluated. To implement this, I'll add a pass to the VM that runs when a module is added to it. First, this will collect the location of all labels in the Module. Then, it will transform jumps to those labels into jumps with a value associated with them. That value will add itself to the program counter.

In order to handle jumps between modules, we'll need to allow a string as an immediate for an instruction.

### Return vs. Exit

When we jump to a new module, that module can either indicate that it would like to return or that it would like to exit. In order to do this we will modify the return type of a call to run to indicate if the Module it was running returned or if it exited.

### VM Interface

I like how LLVM's LLJIT handles adding and running code. I'd like the VM to follow that:

```c++
int main()
{
  // Create the VM with 100 slots for ram.
  vm::VM v(100);
  
	// Create some Modules...
  
  // Add the modules.
  v.add(std::move(m1));
  v.add(std::move(m2));
  v.add(std::move(m3));
  
  // Tell it to run the main function.
  // Return rather or not an error occured.
	auto error = v.run("main");
}
```

### Output

When a puts instruction is called, we'd like to be able to control where the output is sent. To that end, we'll make it possible to connect a VM to an output stream. Users of the VM will then be able to call getter and setter methods to connect it up.

## Full Instruction List

```assembly
; Arguments in the format [arg] come from the stack from left to right.
; Arguments in the form <arg> should be supplied with the instruction.
; All functions consume their arguments and pop them from the stack.

; puts [value] - prints a value to stdio.
; exit - stops execution of the program.

; jump <label> - jumps to a label in the current module.
; call <label> - calls a function (a module).
; jump <value> - adds value to the program counter.
; ret - returns to the calling function, or exits if called from the top level module.

; add [value] [value] - adds the values on top of the stack and pushes the result
; sub [value] [value] - subtracts second value from the value on the top and pushes the result.
; mul [value] [value] - multiplies the value on top of the stack and pushes the result.
; div [value] [value] - divides the value on top of the stack by the next value and pushes the
;												result.

; pi <value> - pushes value to the top of the stack.
```



## Example Programs and Output

```python
def foo(x):
  x += 1
	return x
```

```assembly
foo:
	pi 1 ; push immediate
  add  ; adds the argument and 1
  ret	 ; jumps to the value stored in ra
```

```c++
i = 5
while i != 0:
	i -= 1
```

```assembly
pi 5 ; i is on top of stack
label "while"
pi -1
add
bnez "while"
```

