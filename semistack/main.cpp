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

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

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
    
    vm::logger()->debug("Hello!");
    
    vm::Instruction i{vm::InstType::pi, 10};
    
    vm::logger()->debug(vm::to_string(i));
    return 0;
}
