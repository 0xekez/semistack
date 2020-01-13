# Notes

The following are my notes as I work on this project.



# Version 2

This version will be very similar to Version one and will likely share much of the same code. The two things that we're adding here are support for function calls and more types in the VM. As with before though, we'll start with just a floating point type.

## Limitations & Modifications

The trouble with a stack based VM is that you only ever have access to one value at a time. This basically boils down to needing to do all of your computations with a single variable - not possible. In order to get around this, you need some cleverness and not a pure stack based virtual machine. I'll do the following based on Crafting Interpreters and my own thoughts:

1. The stack will need to be able to hold a multipurpose Value type. Crafting Interpreters uses a tagged union, but we're in C++, so we can go ahead and use a variant type. I'm confident in the performance of `std::variant` now because it appears as if the f18 Fortran compiler uses it.
2. We'll need a call stack in addition to a regular stack. The call stack will hold Frames.
3. We'll need some concept of a Frame. This will need to hold local variables, and a program counter to tell where they left off when / if they call another function and get returned.

## Closures

These are going to be hard. [Here](https://craftinginterpreters.com/closures.html) is Crafting Interpreters implementation. Basically, when resolving a variable, we need to first search the current scope, then the closures of that scope, and the global variables. Sounds good. The big question though is how to indicate to the VM that we'd like to search the closures?

One way to do this would be to modify how we get local and global variables. We could let those instructions have string arguments and then at link time we could attempt to resolve them.

A problem that I foresee though is how we can even go about declaring functions that are closed over by other ones. At the moment, we pretty much assume that all functions compiled into separate modules. If we wanted to declare a function inside of another one, I'm not sure we even have the means to do that at the moment.

C gets around this by allowing for function pointers and the likes but even then it's not particularly elegant. It seems entirely possible that we're not going to get closures out of this without some serious work. 

Another problem that I foresee, while we're on the topic, is how we would even return a function from another function. We have no notion of pointers and don't have function objects in the VM, so it seems unlikely that we could even do that as it stands.

**Moral of the story:** our current implementation does not, and will not support first class functions and closures. At this point I'm in favor of stepping away from this for a little bit, doing some more reading, and then coming back to tear it up and put closures in this bad boy.

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

