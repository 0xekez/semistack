//
//  instruction.cpp
//  semistack
//
//  Created by Zeke Medley on 1/9/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#include "instruction.hpp"
#include "logger.hpp"
#include "util.hpp"

#include <variant>

using namespace vm;

std::string vm::to_string(InstType i)
{
    switch (i)
    {
        case InstType::pi:
            return "pi";
        case InstType::puts:
            return "puts";
        default:
            logger()->error("Unhandled instruction type in to_string.");
    }
}

std::string vm::to_string(Immediate i)
{
    if (i.has_value())
    {
        return std::visit(util::overloaded {
            [](float f)
            {
                return std::to_string(f);
            },
            [](std::string s)
            {
                return s;
            },
            [](auto a)
            {
                logger()->error("Unhandled type in immediate to_string.");
                return "";
            }
        }, i.value());
    }
    return "";
}

std::string vm::to_string(Instruction i)
{
    auto r = ::to_string(i.first);
    if (i.second.has_value())
    {
        r += " " + ::to_string(i.second);
    }
    return r;
}
