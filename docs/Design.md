# Design

This file documents provides some background on types of virtual machines and gives a high level overview of some design decisions made in Semistack.

## Types of Virtual Machines

There are two primary types of VMs: stack machines, and register based ones.

Stack machines store information on a stack and operations pop things from said stack to get their arguments. The Web Assembly and Wren virtual machines are stack based. The advantage to stack machines is that they closely emulate the way that high level languages work. Local variables have a lifetime that is easily modeled by a stack. Their disadvantage is that they are often slower than their register machine counterparts.

Register machines are most widely used in hardware. They store the result of operations in high speed registers which allows their reuse across operations. RISC-V and LLVM are both register based. Register machines seem to be a little easier to optimize and the general consensus seems to be that they are faster.

## Stack Based VM

A pure stack based virtual machine is doomed to failure. I learned this the hard way. This is actually my second attempt at implementing a virtual machine. The reason that a purely stack based VM isn't realistic is that you can only use the value on the top of the stack at any given time. This is basically the equivalent of only using one variable for all of your code. It is fine for very simple programs, but not for more realistic ones.

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

