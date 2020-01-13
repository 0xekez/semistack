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

#include "semistack/util.hpp"
#include "semistack/instruction.hpp"
#include "semistack/module.hpp"
#include "semistack/logger.hpp"
#include "semistack/vm.hpp"

int main(int argc, const char * argv[]) {
    std::string input = "(+ 2 (+ 4 5))";
    
    std::cout << "Compiling: " << input << "\n";
    
    auto res = lust::parse_expression(lust::tokenize(input));
    
    if ( ! res.has_value() )
    {
        return 1;
    }
    
    lust::Expression& expr = res.value().first;
    
    vm::Module m("main");
    
    bool success = lust::codegen(expr, m);
    if ( ! success ) return 1;
    
    m.addInstruction(vm::InstType::puts);
    m.addInstruction(vm::InstType::exit);
    
    vm::VM v;
    v.addModule(std::move(m));
    v.run("main");
    
    return 0;
}
