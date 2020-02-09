//
//  main.cpp
//  lust
//
//  Created by Zeke Medley on 1/12/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#include <iostream>
#include <string>
#include <variant>

#include "parse.hpp"
#include "expression.hpp"
#include "compiler.hpp"

#include "semistack/util.hpp"
#include "semistack/instruction.hpp"
#include "semistack/function.hpp"
#include "semistack/logger.hpp"
#include "semistack/vm.hpp"

int main(int argc, const char * argv[]) {
//    std::string input = "(print (+ 2 (+ 4 5)))";
    const std::string input = "(def dog 10) (print dog)";
    
    std::cout << "Compiling: " << input << "\n";
    
    auto res = lust::parse_expression(lust::tokenize(input));
    
    if ( ! res.has_value() )
    {
        return 1;
    }
    
    vm::Function main;
    lust::Compiler C(main);
    
    auto tokens = lust::tokenize(input);
    while ( ! tokens.empty() )
    {
        auto resq = lust::parse_expression(tokens);
        
        if ( ! resq.has_value() )
        {
            vm::logger()->error("Parse error.");
            return 1;
        }
        
        auto res = std::move(resq.value());
        tokens = res.second;
        
        const auto expr = std::move(res.first);
        
        if ( ! C.compileExpression(expr) )
        {
            vm::logger()->error("Compilation error.");
            return 1;
        }
    }
    
    main.addInstruction(vm::InstType::exit);
    
//    vm::logger()->dumpFn(main);
    
    vm::VM v;
    v.addFunction(std::move(main), "main");
    v.run("main");
    
    return 0;
}
