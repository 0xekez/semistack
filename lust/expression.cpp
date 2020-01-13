//
//  expression.cpp
//  lust
//
//  Created by Zeke Medley on 1/12/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#include "expression.hpp"

using namespace lust;

CodegenRes lust::codegen(const Expression& e)
{
    return std::visit(vm::util::overloaded {
        [](const std::unique_ptr<lust::Number>& n)-> CodegenRes
        {
            std::vector<vm::Instruction> res;
            res.push_back({vm::InstType::pi, n->_num});
            return res;
        },
        [](const std::unique_ptr<lust::List>& le)-> CodegenRes
        {
            if (le->_exprs.size() != 3)
            {
                vm::logger()->error("Only two arguments supported.");
                return std::nullopt;
            }
            
            std::vector<vm::Instruction> result;
            
            auto it = le->_exprs.rbegin();
            while (it != le->_exprs.rend())
            {
                auto res = codegen(*it);
                if ( ! res.has_value() )
                    return std::nullopt;
                
                auto& resv = res.value();
                
                result.insert(result.end(), resv.begin(), resv.end());
                
                ++it;
            }
            
            return result;
        },
        [](const std::unique_ptr<lust::Symbol>& s)-> CodegenRes
        {
            if (s->_sym == "+")
            {
                std::vector<vm::Instruction> res;
                res.push_back({vm::InstType::add, std::nullopt});
                return res;
            } else
            {
                vm::logger()->error("Unhandled symbol: " + s->_sym);
                return std::nullopt;
            }
        }
    }, e);
}
