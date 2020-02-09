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
    ctx.setOption("no-colors", true);

    int res = ctx.run();

    // Query flags (and --exit) rely on this.
    if (ctx.shouldExit())
        return res;
    return 0;
}

TEST_CASE("Processed call instruction.")
{
    vm::Function start;

    start.addInstruction({vm::InstType::pi, 10});
    start.addInstruction({vm::InstType::call, 1});

    vm::Function end;
    end.addInstruction({vm::InstType::puts, std::nullopt});
    end.addInstruction({vm::InstType::exit, std::nullopt});

    std::string output;
    vm::VM v([&](std::string s){ output += s; });
    v.addFunction(std::move(start), "start");
    v.addFunction(std::move(end), "end");

    v.run("start");

    CHECK(output == "10.000000");
}

TEST_CASE("Unprocessed call instruction.")
{
    vm::Function start;

    start.addInstruction(vm::InstType::pi, 10.0);
    start.addInstruction(vm::InstType::call, "end");

    vm::Function end;
    end.addInstruction(vm::InstType::puts);
    end.addInstruction(vm::InstType::exit);

    std::string output;
    vm::VM v([&](std::string s){ output += s; });
    v.addFunction(std::move(start), "start");
    v.addFunction(std::move(end), "end");

    v.run("start");

    CHECK(output == "10.000000");
}

TEST_CASE("Hello world.")
{
    vm::Function main;
    main.addInstruction(vm::InstType::pi, "hello world!");
    main.addInstruction(vm::InstType::pi, -5.0);
    main.addInstruction(vm::InstType::pi, -5.0);
    main.addInstruction(vm::InstType::jeq, 2.0);
    main.addInstruction(vm::InstType::div);
    main.addInstruction(vm::InstType::puts);
    main.addInstruction(vm::InstType::exit);

    std::string output;
    vm::VM v([&](std::string s){ output += s; });
    v.addFunction(std::move(main), "main");
    v.run("main");

    CHECK(output == "hello world!");
}

TEST_CASE("Basic loop")
{
    std::string baseline = "";
    float i = 10;
    while ( i > 0)
    {
        --i;
        baseline += std::to_string(i);
    }

    //  pi 10     | 10
    //  copy      | 10 10
    //  pi 0      | 10 10 0
    //  jlt done  | 10
    //loop:
    //  pi -1     | 10 -1
    //  add       | 9
    //  copy      | 9 9
    //  puts      | 9
    //  copy      | 9 9
    //  pi 0      | 9 9 0
    //  jgt loop  | 9
    //done:
    //  exit

    vm::Function loop;
    loop.addInstruction(vm::InstType::pi, 10);
    loop.addInstruction(vm::InstType::copy);
    loop.addInstruction(vm::InstType::pi, 0);
    loop.addInstruction(vm::InstType::jlt, "done");
    loop.addInstruction(vm::InstType::label, "loop");
    loop.addInstruction(vm::InstType::pi, -1);
    loop.addInstruction(vm::InstType::add);
    loop.addInstruction(vm::InstType::copy);
    loop.addInstruction(vm::InstType::puts);
    loop.addInstruction(vm::InstType::copy);
    loop.addInstruction(vm::InstType::pi, 0);
    loop.addInstruction(vm::InstType::jgt, "loop");
    loop.addInstruction(vm::InstType::label, "done");
    loop.addInstruction(vm::InstType::exit);

    std::string output;
    vm::VM v([&](std::string s){ output += s; });
    v.addFunction(std::move(loop), "loop");
    v.run("loop");

    CHECK(baseline == output);
}

TEST_CASE("fib!")
{
    vm::Function main;
    vm::Function fib;

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

    std::string output;
    vm::VM v([&](std::string s){ output += s; });

    v.addFunction(std::move(main), "main");
    v.addFunction(std::move(fib), "fib");
    v.run("main");

    CHECK(output == "21.000000");
}

TEST_CASE("assembler output")
{
    vm::Function pre;

    pre.addInstruction({vm::InstType::jump, "first"});
    pre.addInstruction({vm::InstType::label, "second"});
    pre.addInstruction({vm::InstType::pi, 2});
    pre.addInstruction({vm::InstType::puts, std::nullopt});
    pre.addInstruction({vm::InstType::exit, std::nullopt});
    pre.addInstruction({vm::InstType::label, "first"});
    pre.addInstruction({vm::InstType::pi, 1});
    pre.addInstruction({vm::InstType::puts, std::nullopt});
    pre.addInstruction({vm::InstType::jump, "second"});

    // What the Function should look like after processing.
    vm::Function post;

    post.addInstruction({vm::InstType::jump, 4});
    post.addInstruction({vm::InstType::pi, 2});
    post.addInstruction({vm::InstType::puts, std::nullopt});
    post.addInstruction({vm::InstType::exit, std::nullopt});
    post.addInstruction({vm::InstType::pi, 1});
    post.addInstruction({vm::InstType::puts, std::nullopt});
    post.addInstruction({vm::InstType::jump, -5});

    bool result = vm::transform::assembleFunction(pre);
    CHECK(result);

    bool a = (pre == post);
    CHECK(a);
}
