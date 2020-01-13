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

#include "util.hpp"
#include "instruction.hpp"
#include "module.hpp"
#include "logger.hpp"
#include "vm.hpp"

int main(int argc, const char * argv[]) {
    // insert code here...
    std::string input = "(+ 2 (+ 4 5))";
    auto res = lust::parse_expression(lust::tokenize(input));
    
    if ( ! res.has_value() )
    {
        return 1;
    }
    
    std::function<void(lust::Expression& e)> valuePrinter;
    valuePrinter = [&](lust::Expression& e)
    {
        std::visit(vm::util::overloaded {
            [](const std::unique_ptr<lust::Number>& n)
            {
                std::cout << n->_num;
            },
            [&](const std::unique_ptr<lust::List>& le)
            {
                std::cout << "( ";
                for( auto& expr : le->_exprs )
                {
                    valuePrinter(expr);
                    std::cout << " ";
                }
                std::cout << ")";
            },
            [&](const std::unique_ptr<lust::Symbol>& s)
            {
                std::cout << s->_sym;
            }
        }, e);
    };
    
    lust::Expression& expr = res.value().first;
    
    valuePrinter(expr);
    std::cout << "\n";
    
    lust::CodegenRes c = lust::codegen(expr);
    if ( ! c.has_value() ) return 1;
    
    std::vector<vm::Instruction> vec = c.value();
    
    vm::Module m("main");
    m._instructions = vec;
    
    vm::logger()->dumpModule(m);
    
    return 0;
}
