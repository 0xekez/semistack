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

//TEST_CASE("Processed call instruction.")
//{
//    vm::Module start("start");
//
//    start.addInstruction({vm::InstType::pi, 10});
//    start.addInstruction({vm::InstType::call, 1});
//
//    vm::Module end("end");
//    end.addInstruction({vm::InstType::puts, std::nullopt});
//    end.addInstruction({vm::InstType::exit, std::nullopt});
//
//    std::string output;
//    vm::VM v([&](std::string s){ output += s; });
//    v.addModule(std::move(start));
//    v.addModule(std::move(end));
//
//    v.run("start");
//
//    CHECK(output == "10.000000");
//}
//
//TEST_CASE("Unprocessed call instruction.")
//{
//    vm::Module start("start");
//
//    start.addInstruction(vm::InstType::pi, 10.0);
//    start.addInstruction(vm::InstType::call, "end");
//
//    vm::Module end("end");
//    end.addInstruction(vm::InstType::puts);
//    end.addInstruction(vm::InstType::exit);
//
//    std::string output;
//    vm::VM v([&](std::string s){ output += s; });
//    v.addModule(std::move(start));
//    v.addModule(std::move(end));
//
//    v.run("start");
//
//    CHECK(output == "10.000000");
//}
//
//TEST_CASE("Hello world.")
//{
//    vm::Module m("main");
//    m.addInstruction(vm::InstType::pi, "hello world!");
//    m.addInstruction(vm::InstType::pi, -5.0);
//    m.addInstruction(vm::InstType::pi, -5.0);
//    m.addInstruction(vm::InstType::jeq, 2.0);
//    m.addInstruction(vm::InstType::div);
//    m.addInstruction(vm::InstType::puts);
//    m.addInstruction(vm::InstType::exit);
//
//    std::string output;
//    vm::VM v([&](std::string s){ output += s; });
//    v.addModule(std::move(m));
//    v.run("main");
//
//    CHECK(output == "hello world!");
//}
//
//TEST_CASE("Basic loop")
//{
//    std::string baseline = "";
//    float i = 10;
//    while ( i > 0)
//    {
//        --i;
//        baseline += std::to_string(i);
//    }
//
//    //  pi 10     | 10
//    //  copy      | 10 10
//    //  pi 0      | 10 10 0
//    //  jlt done  | 10
//    //loop:
//    //  pi -1     | 10 -1
//    //  add       | 9
//    //  copy      | 9 9
//    //  puts      | 9
//    //  copy      | 9 9
//    //  pi 0      | 9 9 0
//    //  jgt loop  | 9
//    //done:
//    //  exit
//
//    vm::Module m("loop");
//    m.addInstruction(vm::InstType::pi, 10);
//    m.addInstruction(vm::InstType::copy);
//    m.addInstruction(vm::InstType::pi, 0);
//    m.addInstruction(vm::InstType::jlt, "done");
//    m.addInstruction(vm::InstType::label, "loop");
//    m.addInstruction(vm::InstType::pi, -1);
//    m.addInstruction(vm::InstType::add);
//    m.addInstruction(vm::InstType::copy);
//    m.addInstruction(vm::InstType::puts);
//    m.addInstruction(vm::InstType::copy);
//    m.addInstruction(vm::InstType::pi, 0);
//    m.addInstruction(vm::InstType::jgt, "loop");
//    m.addInstruction(vm::InstType::label, "done");
//    m.addInstruction(vm::InstType::exit);
//
//    std::string output;
//    vm::VM v([&](std::string s){ output += s; });
//    v.addModule(std::move(m));
//    v.run("loop");
//
//    CHECK(baseline == output);
//}
//
//TEST_CASE("fib!")
//{
//    vm::Module main("main");
//    vm::Module fib("fib");
//
//    main.addInstruction(InstType::pi, 8);
//    main.addInstruction(InstType::call, "fib");
//    main.addInstruction(InstType::puts);
//    main.addInstruction(InstType::exit);
//
//    //  return n < 2 ? n : fib(n - 1) + fib(n - 2)
//    //
//    //  copy        | n n
//    //  pi 2        | n n 2
//    //  jlt done    | n
//    //  copy        | n n
//    //  sl 1        | n
//    //  pi 1        | n 1
//    //  sub         | n-1
//    //  call fib    | fib(n-1)
//    //  ll 1        | fib(n-1) n
//    //  pi 2        | fib(n-1) n 2
//    //  sub         | fib(n-1) n-2
//    //  call fib    | fib(n-1) fib(n-2)
//    //  add         | fib(n)
//    //done:
//    //  ret
//
//    fib.addInstruction(InstType::copy);
//    fib.addInstruction(InstType::pi, 2);
//    fib.addInstruction(InstType::jlt, "done");
//    fib.addInstruction(InstType::copy);
//    fib.addInstruction(InstType::sl, 1);
//    fib.addInstruction(InstType::pi, 1);
//    fib.addInstruction(InstType::sub);
//    fib.addInstruction(InstType::call, "fib");
//    fib.addInstruction(InstType::ll, 1);
//    fib.addInstruction(InstType::pi, 2);
//    fib.addInstruction(InstType::sub);
//    fib.addInstruction(InstType::call, "fib");
//    fib.addInstruction(InstType::add);
//    fib.addInstruction(InstType::label, "done");
//    fib.addInstruction(InstType::ret);
//
//    std::string output;
//    vm::VM v([&](std::string s){ output += s; });
//
//    v.addModule(std::move(main));
//    v.addModule(std::move(fib));
//    v.run("main");
//
//    CHECK(output == "21.000000");
//}

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

    // What the module should look like after processing.
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
