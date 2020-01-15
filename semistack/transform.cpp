//
//  preprocessor.cpp
//  semistack
//
//  Created by Zeke Medley on 1/11/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//
#include <algorithm>

#include "transform.hpp"
#include "function.hpp"
#include "instruction.hpp"
#include "logger.hpp"
#include "util.hpp"

using namespace vm;

bool transform::assembleFunction(Function& m)
{
    // 1. Collect locations of and remove all labels from function.
    std::vector<Instruction> unlabeled;
    std::map<std::string, size_t> labelLocs;
    
    for (Instruction& i : m._instructions)
    {
        if (i.first == InstType::label)
        {
            if ( ! i.second.has_value() )
            {
                logger()->error("No immediate for label instruction.");
            }
            
            auto sq = util::get<std::string>(i.second.value());
            if ( ! sq )
            {
                logger()->error("Non-string immediate for label instruction.");
                return false;
            }
            
            const std::string& s = sq.value();
            auto [where, success] = labelLocs.insert({s, unlabeled.size()});
            if ( ! success )
            {
                logger()->error("Failed to insert label into lookup table.");
                return false;
            }
        } else
        {
            unlabeled.push_back(std::move(i));
        }
    }
    
    auto isJump = [](InstType t) -> bool
    {
        return t == InstType::jump || t == InstType::jeq || t == InstType::jneq
        || t == InstType::jlt || t == InstType::jgt;
    };
    
    // 2. Make all jump instructions into relative jumps
    size_t loc = 0;
    for (Instruction& i : unlabeled)
    {
        // NOTE: need to support all jump instructions here.
        if (isJump(i.first))
        {
            if ( ! i.second.has_value() )
            {
                logger()->error("No immediate for jump");
                return false;
            }
            
            Value& im = i.second.value();
            
            auto stringq = util::get<std::string>(im);
            if (stringq)
            {
                const std::string& jumpTarget = stringq.value();
                auto where = labelLocs.find(jumpTarget);
                if (where != labelLocs.end() )
                {
                    long long int diff = where->second - loc;
                    i.second.value() = diff;
                }
            }
        }
        ++loc;
    }
    
    m._instructions = std::move(unlabeled);
    return true;
}

bool transform::linkFunctions(std::vector<Function>& functions,
                              const std::map<std::string,
                              std::vector<Function>::size_type>& table)
{
    // 2. Find all unresolved jump instructions.
    // 3. Resolve them to locations in our vector.
    for (auto& fn : functions)
    {
        for (auto& instruction : fn._instructions)
        {
            if (instruction.first == InstType::call)
            {
                if ( ! instruction.second.has_value() )
                {
                    logger()->error("No jump location for call instruction.");
                    return false;
                }

                auto sq = util::get<std::string>(instruction.second.value());
                if (sq)
                {
                    const std::string& s = sq.value();
                    instruction.second = table.at(s);
                }
            }
        }
    }
    
    return true;
}
