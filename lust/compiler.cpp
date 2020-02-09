//
//  compiler.cpp
//  lust
//
//  Created by Zeke Medley on 1/16/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#include "compiler.hpp"

using namespace lust;

bool lust::Compiler::compileDef(const std::list<Expression> &args)
{
    if (args.size() != 2)
    {
        vm::logger()->error("def takes two arguments, got: "
                            + std::to_string(args.size()));
        return false;
    }
    
    const Expression& symq = args.front();
    
    if ( ! std::holds_alternative<std::unique_ptr<Symbol>>(symq) )
    {
        vm::logger()->error("first argument to def is not a symbol");
        return false;
    }
    
    const auto& sym = std::get<std::unique_ptr<Symbol>>(symq);
    if ( _locals.find(sym->_sym) != _locals.end() )
    {
        vm::logger()->error("symbol redefinition: " + sym->_sym);
        return false;
    }
    
    // oof
    const Expression& val = *std::next(args.begin());
    compileExpression(val);
    
    _target.addInstruction(vm::InstType::sl, _locals.size());
    _locals.emplace(sym->_sym, _locals.size());
    
    return true;
}

bool lust::Compiler::compilePrint(const std::list<Expression> &args)
{
    if (args.size() != 1)
    {
        vm::logger()->error("print takes one argument, got: "
                            + std::to_string(args.size()));
        return false;
    }
    
    compileExpression(args.front());
    // FIXME: VM needs to handle case where args doesn't push to stack.
    _target.addInstruction(vm::InstType::puts);
    return true;
}

bool lust::Compiler::compileExpression(const Expression &e)
{
    return std::visit(vm::util::overloaded {
        [this](const std::unique_ptr<List>& l)
        {
            if (l->_exprs.empty())
            {
                vm::logger()->error("Can't evaluate empty list.");
                return false;
            }
            
            const auto& first_form = l->_exprs.front();
            
            if (!std::holds_alternative<std::unique_ptr<Symbol>>(first_form))
            {
                vm::logger()->error("Invalid first form in list.");
                return false;
            }
            
            const auto& sym = std::get<std::unique_ptr<Symbol>>(first_form);
            const std::string name = sym->_sym;
            l->_exprs.pop_front();
            
            if (name == "def")
            {
                return compileDef(l->_exprs);
            } else if (name == "print")
            {
                return compilePrint(l->_exprs);
            }
            
            vm::logger()->debug("not def or print: " + name);
            return false;
        },
        [this](const std::unique_ptr<Symbol>& s)
        {
            auto where = _locals.find(s->_sym);
            if (where == _locals.end())
            {
                vm::logger()->error("undefined symbol: " + s->_sym);
                return false;
            }
            _target.addInstruction(vm::InstType::ll, where->second);
            return true;
        },
        [this](const std::unique_ptr<Number>& n)
        {
            _target.addInstruction(vm::InstType::pi, n->_num);
            return true;
        },
        [this](const std::unique_ptr<Function>& f)
        {
            vm::logger()->error("Unexpected top level function.");
            return false;
        }
    }, e);
}
