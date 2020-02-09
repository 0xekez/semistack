//
//  expression.cpp
//  lust
//
//  Created by Zeke Medley on 1/12/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#include "expression.hpp"
#include "logger.hpp"

#include <unordered_map>
#include <variant>

using namespace lust;

void lust::printExpression(const Expression& e)
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
                printExpression(expr);
                std::cout << " ";
            }
            std::cout << ")";
        },
        [&](const std::unique_ptr<lust::Symbol>& s)
        {
            std::cout << s->_sym;
        },
        [&](const std::unique_ptr<lust::Function>& f)
        {
            std::cout << "<fn>";
        },
    }, e);
    std::cout << "\n";
}

bool lust::codegen(const Expression& e, vm::Function& target)
{
    static const std::unordered_map<std::string, std::function<bool()>> dispatch
    {
        {"+", [&](){ target.addInstruction(vm::InstType::add); return true; }},
        {"-", [&](){ target.addInstruction(vm::InstType::sub); return true; }},
        {"*", [&](){ target.addInstruction(vm::InstType::mul); return true; }},
        {"/", [&](){ target.addInstruction(vm::InstType::div); return true; }},
        
        {"print", [&]()
            {
                target.addInstruction(vm::InstType::puts);
                return true;
            }},
    };
    return false;
//    return std::visit(vm::util::overloaded {
//        [&](const std::unique_ptr<lust::Number>& n)-> bool
//        {
//            target.addInstruction(vm::InstType::pi, n->_num);
//            return true;
//        },
//        [&](const std::unique_ptr<lust::List>& le)-> bool
//        {
//            auto& exprs = le->_exprs;
//            if (exprs.empty())
//            {
//                vm::logger()->error("Can't evaluate an empty list.");
//                return false;
//            }
//
//            Expression& op = exprs.front();
//            if ( ! std::holds_alternative<Symbol>(op) )
//            {
//                vm::logger()->error(
//                            "First expression in a list must be a function.");
//            }
//
//            Symbol& first = std::get<Symbol>(op);
//
//        },
//        [&](const std::unique_ptr<lust::Symbol>& s)-> bool
//        {
//            auto it = dispatch.find(s->_sym);
//            if (it != dispatch.end())
//            {
//                return it->second();
//            } else
//            {
//                vm::logger()->error("Unhandled symbol: " + s->_sym);
//                return false;
//            }
//        }
//    }, e);
}
