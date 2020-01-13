//
//  expression.cpp
//  lust
//
//  Created by Zeke Medley on 1/12/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#include "expression.hpp"

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
        }
    }, e);
    std::cout << "\n";
}

bool lust::codegen(const Expression& e, vm::Module& target)
{
    return std::visit(vm::util::overloaded {
        [&](const std::unique_ptr<lust::Number>& n)-> bool
        {
            target.addInstruction(vm::InstType::pi, n->_num);
            return true;
        },
        [&](const std::unique_ptr<lust::List>& le)-> bool
        {
            auto& exprs = le->_exprs;
            for (auto it = exprs.rbegin(); it != exprs.rend(); ++it)
            {
                bool res = codegen(*it, target);
                if ( ! res )
                    return false;
            }
            return true;
        },
        [&](const std::unique_ptr<lust::Symbol>& s)-> bool
        {
            if (s->_sym == "+")
            {
                target.addInstruction(vm::InstType::add);
                return true;
            } else
            {
                vm::logger()->error("Unhandled symbol: " + s->_sym);
                return false;
            }
        }
    }, e);
}
