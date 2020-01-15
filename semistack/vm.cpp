//
//  vm.cpp
//  semistack
//
//  Created by Zeke Medley on 1/9/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#include "vm.hpp"
#include "logger.hpp"
#include "util.hpp"
#include "transform.hpp"
#include "instruction.hpp"

using namespace vm;

bool vm::VM::addFunction(vm::Function fn, std::string name)
{
    auto [where, success] = _fnLookup.insert({name, _functions.size()});
    if ( ! transform::assembleFunction(fn) )
    {
        logger()->error("Failed to assemble function: " + name);
        return false;
    }
    _functions.emplace_back(std::move(fn));
    return success;
}

ExitStatus vm::VM::run(std::string fn_name)
{
    logger()->maintain(_callStack.size() == 0,
                       "Non-empty call stack for top level run insstruction.");
    
    if ( ! transform::linkFunctions(_functions, _fnLookup) )
    {
        logger()->error("Failed to link functions");
        return ExitStatus::error;
    }
    
    auto where = _fnLookup.find(fn_name);
    if (where == _fnLookup.end())
    {
        logger()->error("Failed to lookup function: " + fn_name);
        return ExitStatus::error;
    }
    
    // Push a CallFrame for the function.
    _callStack.emplace(where->second);
    
    auto res = runFunction(_functions.at(where->second));
    
    if (_valueStack.size())
    {
        logger()->debug("Non empty value stack at end of execution.");
    }
    return res;
}

ExitStatus vm::VM::runFunction(const Function& m)
{
    auto nextInstruction = [this]()-> Instruction&
    {
        auto& m =  _functions.at(_callStack.top()._fnIndex);
        return m._instructions.at(_callStack.top()._pc++);
    };
    
    ExitStatus res = ExitStatus::cont;
    while (res == ExitStatus::cont)
    {
        auto& instruction = nextInstruction();
        res = runInstruction(instruction);
    }
    return res;
}

// TODO: How can we remove all these damn type checks.
// One idea is to run some sort of "verifyFunction" function before we attempt
// to run everything that makes sure things are typed correctly. This still
// does totally remove things for when there are multiple supported types,
// but would be a nice touch. Such a function could also hold some state and
// give better error messages!
//
// My thought on that is that we'd be able to reuse all of the code in here
// anyway, so we might as well write this for now and then move it out.
ExitStatus VM::runInstruction(const Instruction& instruction)
{
    switch (instruction.first) {
        case InstType::pi:
        {
            const auto& imm = instruction.second;
            if( ! imm.has_value() )
            {
                logger()->error("Expected immediate in pi instruction.");
                return ExitStatus::error;
            }
            // Need to make a copy here.
            Value v = imm.value();
            _valueStack.emplace(std::move(v));
            return ExitStatus::cont;
        }
        case InstType::sl:
        {
            const auto& imm = instruction.second;
            if( ! imm.has_value() )
            {
                logger()->error("Expected immediate in sl instruction.");
                return ExitStatus::error;
            }
            auto fq = util::get<float>(imm.value());
            if ( ! fq )
            {
                logger()->error("Got non-float type in sl immediate.");
                return ExitStatus::error;
            }
            float index = fq.value();
            _callStack.top()._locals.at(index) = std::move(_valueStack.top());
            _valueStack.pop();
            return ExitStatus::cont;
        }
        case InstType::ll:
        {
            const auto& imm = instruction.second;
            if( ! imm.has_value() )
            {
                logger()->error("Expected immediate in ll instruction.");
                return ExitStatus::error;
            }
            auto fq = util::get<float>(imm.value());
            if ( ! fq )
            {
                logger()->error("Got non-float type in ll immediate.");
                return ExitStatus::error;
            }
            float index = fq.value();
            _valueStack.push(Value(_callStack.top()._locals.at(index)));
            return ExitStatus::cont;
        }
        case InstType::sg:
        {
            const auto& imm = instruction.second;
            if( ! imm.has_value() )
            {
                logger()->error("Expected immediate in sg instruction.");
                return ExitStatus::error;
            }
            auto fq = util::get<float>(imm.value());
            if ( ! fq )
            {
                logger()->error("Got non-float type in sg immediate.");
                return ExitStatus::error;
            }
            float index = fq.value();
            _globals.at(index) = std::move(_valueStack.top());
            _valueStack.pop();
            return ExitStatus::cont;
        }
        case InstType::lg:
        {
            const auto& imm = instruction.second;
            if( ! imm.has_value() )
            {
                logger()->error("Expected immediate in lg instruction.");
                return ExitStatus::error;
            }
            auto fq = util::get<float>(imm.value());
            if ( ! fq )
            {
                logger()->error("Got non-float type in lg immediate.");
                return ExitStatus::error;
            }
            float index = fq.value();
            _valueStack.push(Value(_globals.at(index)));
            return ExitStatus::cont;
        }
        case InstType::puts:
            _outputFn(vm::to_string(_valueStack.top()));
            _valueStack.pop();
            return ExitStatus::cont;
        case InstType::copy:
            _valueStack.push(Value(_valueStack.top()));
            return ExitStatus::cont;
        case InstType::exit:
            return ExitStatus::exit;
        case InstType::ret:
            _callStack.pop();
            return ExitStatus::cont;
        case InstType::add:
        {
            Value right(std::move(_valueStack.top()));
            _valueStack.pop();
            Value& left(_valueStack.top());
            
            if (!util::holds_same(right, left))
            {
                logger()->error("Different types in add instruction");
                return ExitStatus::error;
            }
            
            if (util::holds<float>(right))
            {
                auto lq = util::get<float>(left);
                auto rq = util::get<float>(right);
                _valueStack.top() = lq.value() + rq.value();
                return ExitStatus::cont;
            }
            
            if (util::holds<std::string>(right))
            {
                auto lq = util::get<std::string>(left);
                auto rq = util::get<std::string>(right);
                _valueStack.top() = lq.value().get() + rq.value().get();
                return ExitStatus::cont;
            }
            
            logger()->error("Unsupported types in add instruction.");
            return ExitStatus::error;
        }
        case InstType::sub:
        {
            Value right(std::move(_valueStack.top()));
            _valueStack.pop();
            Value& left(_valueStack.top());
            
            if (!util::holds_same(right, left))
            {
                logger()->error("Different types in sub instruction");
                return ExitStatus::error;
            }
            
            if (util::holds<float>(right))
            {
                auto lq = util::get<float>(left);
                auto rq = util::get<float>(right);
                _valueStack.top() = lq.value() - rq.value();
                return ExitStatus::cont;
            }
            
            logger()->error("Unsupported types in sub instruction.");
            return ExitStatus::error;
        }
        case InstType::mul:
        {
            Value right(std::move(_valueStack.top()));
            _valueStack.pop();
            Value& left(_valueStack.top());
            
            if (!util::holds_same(right, left))
            {
                logger()->error("Different types in mul instruction");
                return ExitStatus::error;
            }
            
            if (util::holds<float>(right))
            {
                auto lq = util::get<float>(left);
                auto rq = util::get<float>(right);
                _valueStack.top() = lq.value() * rq.value();
                return ExitStatus::cont;
            }
            
            logger()->error("Unsupported types in mul instruction.");
            return ExitStatus::error;
        }
        case InstType::div:
        {
            Value right(std::move(_valueStack.top()));
            _valueStack.pop();
            Value& left(_valueStack.top());
            
            if (!util::holds_same(right, left))
            {
                logger()->error("Different types in div instruction");
                return ExitStatus::error;
            }
            
            if (util::holds<float>(right))
            {
                auto lq = util::get<float>(left);
                auto rq = util::get<float>(right);
                _valueStack.top() = lq.value() / rq.value();
                return ExitStatus::cont;
            }
            
            logger()->error("Unsupported types in div instruction.");
            return ExitStatus::error;
        }
        case InstType::jump:
        {
            if ( ! instruction.second.has_value() )
            {
                logger()->error("No immediate for jump instruction.");
                return ExitStatus::error;
            }
            auto fq = util::get<float>(instruction.second.value());
            if ( ! fq )
            {
                logger()->error("Unexpected non-float type in jump instruction.");
                return ExitStatus::error;
            }
            float distance = fq.value();
            // The program counter has already been moved to the next
            // instruction, hence the -1.
            _callStack.top()._pc += (distance - 1);
            return ExitStatus::cont;
        }
        case InstType::jeq:
        {
            Value right(std::move(_valueStack.top()));
            _valueStack.pop();
            Value left(std::move(_valueStack.top()));
            _valueStack.pop();
            
            if (left == right)
            {
                return runInstruction({vm::InstType::jump, instruction.second});
            }
            return ExitStatus::cont;
        }
        case InstType::jneq:
        {
            Value right(std::move(_valueStack.top()));
            _valueStack.pop();
            Value left(std::move(_valueStack.top()));
            _valueStack.pop();
            
            if (left != right)
            {
                return runInstruction({vm::InstType::jump, instruction.second});
            }
            return ExitStatus::cont;
        }
        case InstType::jgt:
        {
            Value right(std::move(_valueStack.top()));
            _valueStack.pop();
            Value left(std::move(_valueStack.top()));
            _valueStack.pop();
            
            // Relative comparasons aren't quite as clean here as function's
            // aren't comparable leading to UB if we're not careful. See:
            // https://en.cppreference.com/w/cpp/utility/variant/operator_cmp
            
            if (! util::holds_same(right, left))
            {
                logger()->error("Type missmatch in jgt instruction.");
                return ExitStatus::error;
            }
            
            if (util::holds<std::shared_ptr<Function>>(right))
            {
                logger()->error("Function type in jgt instruction.");
                return ExitStatus::error;
            }
            
            // See above link for why this works.
            if (left > right)
            {
                return runInstruction({vm::InstType::jump, instruction.second});
            }
            
            return ExitStatus::cont;
        }
        case InstType::jlt:
        {
            Value right(std::move(_valueStack.top()));
            _valueStack.pop();
            Value left(std::move(_valueStack.top()));
            _valueStack.pop();
            
            // Relative comparasons aren't quite as clean here as function's
            // aren't comparable leading to UB if we're not careful. See:
            // https://en.cppreference.com/w/cpp/utility/variant/operator_cmp
            
            if (! util::holds_same(right, left))
            {
                logger()->error("Type missmatch in jlt instruction.");
                return ExitStatus::error;
            }
            
            if (util::holds<std::shared_ptr<Function>>(right))
            {
                logger()->error("Function type in jlt instruction.");
                return ExitStatus::error;
            }
            
            // See above link for why this works.
            if (left < right)
            {
                return runInstruction({vm::InstType::jump, instruction.second});
            }
            
            return ExitStatus::cont;
        }
        case InstType::call:
        {
            const auto& imm = instruction.second;
            if( ! imm.has_value() )
            {
                logger()->error("Expected immediate in sg instruction.");
                return ExitStatus::error;
            }
            
            if ( ! util::holds<float>(imm.value()) )
            {
                logger()->error("Wrong type in call instruction.");
                return ExitStatus::error;
            }
            float index = util::get<float>(imm.value()).value();
            _callStack.emplace(index);
            return ExitStatus::cont;
        }
        case InstType::label:
            logger()->maintain(false,
                               "Label instructions should not be executed.");
            return ExitStatus::error;
    }
    // We should never reach this if all of our instructions are properly
    // implemented.
    return ExitStatus::error;
}
