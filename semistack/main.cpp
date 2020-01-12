//
//  main.cpp
//  semistack
//
//  Created by Zeke Medley on 1/7/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#include <iostream>

#include "logger.hpp"
#include "instruction.hpp"
#include "vm.hpp"
#include "transform.hpp"

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

using namespace vm;

int main(int argc, const char * argv[]) {
    doctest::Context ctx;

    // --exit to just run tests, --no-run to skip running them.
    ctx.applyCommandLine(argc, argv);

    // Don't break in the debugger.
    ctx.setOption("no-breaks", true);

    int res = ctx.run();

    // Query flags (and --exit) rely on this.
    if (ctx.shouldExit())
        return res;

    vm::Module main("main");
    vm::Module fib("fib");
    
    main.addInstruction(InstType::pi, 8);
    main.addInstruction(InstType::call, "fib");
    main.addInstruction(InstType::puts);
    main.addInstruction(InstType::exit);
    
    //  return n < 2 ? n : fib(n - 1) + fib(n - 2)
    //
    //  copy        | n n
    //  pi 2        | n n 2
    //  jlt done    | n
    //  copy        | n n
    //  sl 1        | n
    //  pi 1        | n 1
    //  sub         | n-1
    //  call fib    | fib(n-1)
    //  ll 1        | fib(n-1) n
    //  pi 2        | fib(n-1) n 2
    //  sub         | fib(n-1) n-2
    //  call fib    | fib(n-1) fib(n-2)
    //  add         | fib(n)
    //done:
    //  ret
    
    fib.addInstruction(InstType::copy);
    fib.addInstruction(InstType::pi, 2);
    fib.addInstruction(InstType::jlt, "done");
    fib.addInstruction(InstType::copy);
    fib.addInstruction(InstType::sl, 1);
    fib.addInstruction(InstType::pi, 1);
    fib.addInstruction(InstType::sub);
    fib.addInstruction(InstType::call, "fib");
    fib.addInstruction(InstType::ll, 1);
    fib.addInstruction(InstType::pi, 2);
    fib.addInstruction(InstType::sub);
    fib.addInstruction(InstType::call, "fib");
    fib.addInstruction(InstType::add);
    fib.addInstruction(InstType::label, "done");
    fib.addInstruction(InstType::ret);

    vm::VM v;
    v.addModule(std::move(main));
    v.addModule(std::move(fib));
    v.run("main");
    
    return 0;
}
