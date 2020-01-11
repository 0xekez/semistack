//
//  preprocessor.cpp
//  semistack
//
//  Created by Zeke Medley on 1/11/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//
#include <algorithm>

#include "transform.hpp"
#include "module.hpp"
#include "instruction.hpp"
#include "logger.hpp"

using namespace vm;

bool transform::assembleModule(Module& m)
{
    // 1. Collect locations of and remove all labels from Module.
    auto& iList = m._instructions;
    std::vector<Instruction> unlabeled;
    std::map<std::string, size_t> labelLocs;
    
    for (Instruction& i : iList)
    {
        if (i.first == InstType::label)
        {
            if ( ! i.second.has_value() )
            {
                logger()->error("No immediate for label instruction.");
            }
            
            Value im = i.second.value();
            
            if (! std::holds_alternative<std::string>(im))
            {
                logger()->error("Wrong immediate type for label instruction.");
                return false;
            }
            
            std::string labelName = std::get<std::string>(im);
            auto [where, success] = labelLocs.insert({labelName,
                                                      unlabeled.size()});
            
            if ( ! success )
            {
                logger()->error("Failed to insert label into lookup table");
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
            assert(i.second.has_value() && "No immediate for jump.");
            Value im = i.second.value();
            
            // Possible to add absolute jumps. We ignore those.
            if (std::holds_alternative<std::string>(im))
            {
                std::string jumpTarget = std::get<std::string>(im);
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
    
    m._instructions = unlabeled;
    return true;
}

bool transform::linkModules(std::vector<Module> &modules,
                     std::map<std::string, std::vector<Module>::size_type> table)
{
    // 2. Find all unresolved jump instructions.
    // 3. Resolve them to locations in our vector.
    for (auto& module : modules)
    {
        for (auto& instruction : module._instructions)
        {
            if (instruction.first == InstType::call)
            {
                if ( ! instruction.second.has_value() )
                {
                    logger()->error("No jump location for call instruction.");
                    return false;
                }

                if (std::holds_alternative<std::string>(instruction.second.value()))
                {
                    auto index = table.at(
                             std::get<std::string>(instruction.second.value()));
                    instruction.second = index;
                }
            }
        }
    }
    
    return true;
}


#include "doctest.h"

TEST_CASE("assembler output")
{
    vm::Module pre("m");

    pre.addInstruction({vm::InstType::jump, "first"});
    pre.addInstruction({vm::InstType::label, "second"});
    pre.addInstruction({vm::InstType::pi, 2});
    pre.addInstruction({vm::InstType::puts, std::nullopt});
    pre.addInstruction({vm::InstType::exit, std::nullopt});
    pre.addInstruction({vm::InstType::label, "first"});
    pre.addInstruction({vm::InstType::pi, 1});
    pre.addInstruction({vm::InstType::puts, std::nullopt});
    pre.addInstruction({vm::InstType::jump, "second"});

    // What the module should look like after processing.
    vm::Module post("m");

    post.addInstruction({vm::InstType::jump, 4});
    post.addInstruction({vm::InstType::pi, 2});
    post.addInstruction({vm::InstType::puts, std::nullopt});
    post.addInstruction({vm::InstType::exit, std::nullopt});
    post.addInstruction({vm::InstType::pi, 1});
    post.addInstruction({vm::InstType::puts, std::nullopt});
    post.addInstruction({vm::InstType::jump, -5});

    vm::transform::assembleModule(pre);

    bool a = (pre == post);
    CHECK(a);
}
