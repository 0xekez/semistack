

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

See the unit tests in `main.cpp` for an example of how this looks when using the C++ api.