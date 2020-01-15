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
#include "value.hpp"

#include <variant>

using namespace vm;

std::string vm::to_string(const InstType& i)
{
    switch (i)
    {
        case InstType::pi:
            return "pi";
        case InstType::puts:
            return "puts";
        case InstType::add:
            return "add";
        case InstType::sub:
            return "sub";
        case InstType::mul:
            return "mul";
        case InstType::div:
            return "div";
        case InstType::jump:
            return "jump";
        case InstType::jeq:
            return "jeq";
        case InstType::jneq:
            return "jneq";
        case InstType::jgt:
            return "jgt";
        case InstType::jlt:
            return "jlt";
        case InstType::call:
            return "call";
        case InstType::copy:
            return "copy";
        case InstType::sl:
            return "sl";
        case InstType::ll:
            return "ll";
        case InstType::sg:
            return "sg";
        case InstType::lg:
            return "lg";
        case InstType::ret:
            return "ret";
        case InstType::label:
            return "label";
        case InstType::exit:
            return "exit";
        default:
            logger()->error("Unhandled instruction type in to_string.");
            return "";
    }
}

std::string vm::to_string(const Value& v)
{
    return std::visit(util::overloaded {
        [](float f)-> std::string
        {
            return std::to_string(f);
        },
        [](const Object& obj)-> std::string
        {
            return std::visit(util::overloaded {
                [](const std::string& s)-> std::string
                {
                    return s;
                },
                [](const std::shared_ptr<Function>&)-> std::string
                {
                    return "<function>";
                }
            }, obj);
        }
    }, v);
}

std::string vm::to_string(const Immediate& i)
{
    if (i.has_value())
    {
        return ::to_string(i.value());
    }
    return "";
}

std::string vm::to_string(const Instruction& i)
{
    auto r = ::to_string(i.first);
    if (i.second.has_value())
    {
        r += " " + ::to_string(i.second);
    }
    return r;
}

bool vm::operator==(const Instruction& l, const Instruction& r)
{
    bool same = (l.first == r.first);
    if (l.second.has_value())
    {
        same &= r.second.has_value();
        same &= std::visit(util::overloaded {
            [&](float f)
            {
                return f == std::get<float>(r.second.value());
            },
            [&](const Object& lobj)
            {
                const auto& robj = std::get<Object>(r.second.value());
                if (robj.index() != lobj.index()) return false;
                return std::visit(util::overloaded {
                    [&](const std::string& s)
                    {
                        return s == std::get<std::string>(lobj);
                    },
                    [&](const std::shared_ptr<Function>& fn)
                    {
                        return *fn == *std::get<std::shared_ptr<Function>>(robj);
                    }
                }, lobj);
            }
        }, l.second.value());
    } else
    {
        same &= (not r.second.has_value());
    }
    return same;
}
