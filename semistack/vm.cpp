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

using namespace vm;

bool vm::VM::addModule(vm::Module m)
{
    auto [where, success] = _moduleLookup.insert({m._name, _modules.size()});
    if ( ! transform::assembleModule(m) )
    {
        logger()->error("Failed to assemble module: " + m._name);
        return false;
    }
    _modules.emplace_back(std::move(m));
    return success;
}

ExitStatus vm::VM::run(std::string module_name)
{
    logger()->maintain(_callStack.size() == 0,
                       "Non-empty call stack for top level run instruction.");
    
    if ( ! transform::linkModules(_modules, _moduleLookup) )
    {
        logger()->error("Failed to link modules");
        return ExitStatus::error;
    }
    
    auto where = _moduleLookup.find(module_name);
    if (where == _moduleLookup.end())
    {
        logger()->error("Failed to lookup module: " + module_name);
        return ExitStatus::error;
    }
    
    // Push a CallFrame for the module.
    _callStack.emplace(where->second);
    
    auto res = runModule(_modules.at(where->second));
    
    if (_valueStack.size())
    {
        logger()->debug("Non empty call stack at end of execution.");
    }
    return res;
}

ExitStatus vm::VM::runModule(const Module& m)
{
    auto nextInstruction = [this]()-> Instruction
    {
        auto& m =  _modules.at(_callStack.top()._moduleIndex);
        return m._instructions.at(_callStack.top()._pc++);
    };
    
    ExitStatus res = ExitStatus::cont;
    while (res == ExitStatus::cont)
    {
        auto instruction = nextInstruction();
        res = runInstruction(instruction);
    }
    return res;
}

ExitStatus VM::runInstruction(const Instruction& instruction)
{
    auto getImmediateValue = [](const Immediate& im)-> Value
    {
        logger()->maintain(im.has_value(), "Expected value for immediate.");
        return im.value();
    };
    switch (instruction.first)
    {
        case InstType::pi:
        {
            Value v = getImmediateValue(instruction.second);
            _valueStack.push(std::move(v));
            break;
        }
        case InstType::sl:
        {
            Value v = getImmediateValue(instruction.second);
            return std::visit( util::overloaded {
                [&, this](std::string)
                {
                    logger()->error("String type in sl instruction.");
                    return ExitStatus::error;
                },
                [&, this](float index)
                {
                    _callStack.top()._locals.at(index) = Value(_valueStack.top());
                    _valueStack.pop();
                    return ExitStatus::cont;
                }
            }, v);
        }
        case InstType::ll:
        {
            Value v = getImmediateValue(instruction.second);
            return std::visit( util::overloaded {
                [&, this](std::string)
                {
                    logger()->error("String type in ll instruction.");
                    return ExitStatus::error;
                },
                [&, this](float index)
                {
                    _valueStack.emplace(_callStack.top()._locals.at(index));
                    return ExitStatus::cont;
                }
            }, v);
        }
        case InstType::sg:
        {
            Value v = getImmediateValue(instruction.second);
            return std::visit( util::overloaded {
                [&, this](std::string)
                {
                    logger()->error("String type in sg instruction.");
                    return ExitStatus::error;
                },
                [&, this](float index)
                {
                    _globals.at(index) = Value(_valueStack.top());
                    return ExitStatus::cont;
                }
            }, v);
        }
        case InstType::lg:
        {
            Value v = getImmediateValue(instruction.second);
            return std::visit( util::overloaded {
                [&, this](std::string)
                {
                    logger()->error("String type in lg instruction.");
                    return ExitStatus::error;
                },
                [&, this](float index)
                {
                    _valueStack.emplace(_globals.at(index));
                    return ExitStatus::cont;
                }
            }, v);
        }
        case InstType::puts:
        {
            Value v = _valueStack.top();
            _valueStack.pop();
            _outputFn(vm::to_string(v));
            break;
        }
        case InstType::copy:
            _valueStack.push(Value(_valueStack.top()));
            break;
        case InstType::exit:
        {
            return ExitStatus::exit;
        }
        case InstType::ret:
        {
            _callStack.pop();
            return ExitStatus::cont;
        }
        case InstType::add:
        {
            Value right = _valueStack.top();
            _valueStack.pop();
            Value left = _valueStack.top();
            
            if (left.index() != right.index())
            {
                logger()->error("Type missmatch in add instruction.");
                return ExitStatus::error;
            }
            
            return std::visit(util::overloaded {
                [&, this](std::string left)
                {
                    _valueStack.top() = left + std::get<std::string>(right);
                    return ExitStatus::cont;
                },
                [&, this](float left)
                {
                    _valueStack.top() = left + std::get<float>(right);
                    return ExitStatus::cont;
                }
            }, left);
            break;
        }
        case InstType::sub:
        {
            Value right = _valueStack.top();
            _valueStack.pop();
            Value left = _valueStack.top();
            
            if (left.index() != right.index())
            {
                logger()->error("Type missmatch in sub instruction.");
                return ExitStatus::error;
            }
            
            return std::visit(util::overloaded {
                [&, this](std::string left)
                {
                    logger()->error("Subtraction not allowed for string types.");
                    return ExitStatus::error;
                },
                [&, this](float left)
                {
                    _valueStack.top() = left - std::get<float>(right);
                    return ExitStatus::cont;
                }
            }, left);
            break;
        }
        case InstType::mul:
        {
            Value right = _valueStack.top();
            _valueStack.pop();
            Value left = _valueStack.top();
            
            if (left.index() != right.index())
            {
                logger()->error("Type missmatch in mul instruction.");
                return ExitStatus::error;
            }
            
            return std::visit(util::overloaded {
                [&, this](std::string left)
                {
                    logger()->error(
                            "Multiplication not allowed for string types.");
                    return ExitStatus::error;
                },
                [&, this](float left)
                {
                    _valueStack.top() = left * std::get<float>(right);
                    return ExitStatus::cont;
                }
            }, left);
            break;
        }
        case InstType::div:
        {
            Value right = _valueStack.top();
            _valueStack.pop();
            Value left = _valueStack.top();
            
            if (left.index() != right.index())
            {
                logger()->error("Type missmatch in div instruction.");
                return ExitStatus::error;
            }
            
            return std::visit(util::overloaded {
                [&, this](std::string left)
                {
                    logger()->error("Division not allowed for string types.");
                    return ExitStatus::error;
                },
                [&, this](float left)
                {
                    _valueStack.top() = left / std::get<float>(right);
                    return ExitStatus::cont;
                }
            }, left);
            break;
        }
        case InstType::jump:
        {
            Value v = getImmediateValue(instruction.second);
            
            return std::visit(util::overloaded {
                [&, this](std::string s)
                {
                    logger()->maintain(false,
                                       "Inter module jumps not implemented");
                    return ExitStatus::error;
                },
                [&, this](float jump)
                {
                    _callStack.top()._pc += (jump - 1);
                    return ExitStatus::cont;
                }
            }, v);
            break;
        }
        case InstType::jeq:
        {
            Value right = _valueStack.top();
            _valueStack.pop();
            Value left = _valueStack.top();
            _valueStack.pop();
            
            if (left.index() != right.index())
            {
                logger()->error("Type missmatch in jeq instruction.");
                return ExitStatus::error;
            }
            
            return std::visit(util::overloaded {
                [&, this](std::string left)
                {
                    if (left == std::get<std::string>(right))
                    {
                        return runInstruction({vm::InstType::jump,
                                               instruction.second});
                    }
                    return ExitStatus::cont;
                },
                [&, this](float left)
                {
                    if (left == std::get<float>(right))
                    {
                        return runInstruction({vm::InstType::jump,
                                               instruction.second});
                    }
                    return ExitStatus::cont;
                }
            }, left);
            break;
        }
        case InstType::jneq:
        {
            Value right = _valueStack.top();
            _valueStack.pop();
            Value left = _valueStack.top();
            _valueStack.pop();
            
            if (left.index() != right.index())
            {
                logger()->error("Type missmatch in jeq instruction.");
                return ExitStatus::error;
            }
            
            return std::visit(util::overloaded {
                [&, this](std::string left)
                {
                    if (left != std::get<std::string>(right))
                    {
                        return runInstruction({vm::InstType::jump,
                                               instruction.second});
                    }
                    return ExitStatus::cont;
                },
                [&, this](float left)
                {
                    if (left != std::get<float>(right))
                    {
                        return runInstruction({vm::InstType::jump,
                                               instruction.second});
                    }
                    return ExitStatus::cont;
                }
            }, left);
            break;
        }
        case InstType::jlt:
        {
            Value right = _valueStack.top();
            _valueStack.pop();
            Value left = _valueStack.top();
            _valueStack.pop();
            
            if (left.index() != right.index())
            {
                logger()->error("Type missmatch in jeq instruction.");
                return ExitStatus::error;
            }
            
            return std::visit(util::overloaded {
                [&, this](std::string left)
                {
                    if (left < std::get<std::string>(right))
                    {
                        return runInstruction({vm::InstType::jump,
                                               instruction.second});
                    }
                    return ExitStatus::cont;
                },
                [&, this](float left)
                {
                    if (left < std::get<float>(right))
                    {
                        return runInstruction({vm::InstType::jump,
                                               instruction.second});
                    }
                    return ExitStatus::cont;
                }
            }, left);
            break;
        }
        case InstType::jgt:
        {
            Value right = _valueStack.top();
            _valueStack.pop();
            Value left = _valueStack.top();
            _valueStack.pop();
            
            if (left.index() != right.index())
            {
                logger()->error("Type missmatch in jeq instruction.");
                return ExitStatus::error;
            }
            
            return std::visit(util::overloaded {
                [&, this](std::string left)
                {
                    if (left > std::get<std::string>(right))
                    {
                        return runInstruction({vm::InstType::jump,
                                               instruction.second});
                    }
                    return ExitStatus::cont;
                },
                [&, this](float left)
                {
                    if (left > std::get<float>(right))
                    {
                        return runInstruction({vm::InstType::jump,
                                               instruction.second});
                    }
                    return ExitStatus::cont;
                }
            }, left);
            break;
        }
        case InstType::label:
            logger()->maintain(false,
                               "Label instructions should not be executed.");
            return ExitStatus::error;
        case InstType::call:
        {
            Value v = getImmediateValue(instruction.second);
            if ( ! std::holds_alternative<float>(v) )
            {
                logger()->error("String type in call instruction.");
                return ExitStatus::error;
            }
            float index = std::get<float>(v);
            _callStack.emplace(index);
            break;
        }
        default:
            logger()->maintain(false, "Unhandled instruction type.");
            break;
    }
    return ExitStatus::cont;
}

#include "doctest.h"

TEST_CASE("Processed call instruction.")
{
    vm::Module start("start");
    
    start.addInstruction({vm::InstType::pi, 10});
    start.addInstruction({vm::InstType::call, 1});
    
    vm::Module end("end");
    end.addInstruction({vm::InstType::puts, std::nullopt});
    end.addInstruction({vm::InstType::exit, std::nullopt});
    
    std::string output;
    vm::VM v([&](std::string s){ output += s; });
    v.addModule(std::move(start));
    v.addModule(std::move(end));
    
    v.run("start");
    
    CHECK(output == "10.000000");
}

TEST_CASE("Unprocessed call instruction.")
{
    vm::Module start("start");
    
    start.addInstruction({vm::InstType::pi, 10});
    start.addInstruction({vm::InstType::call, "end"});
    
    vm::Module end("end");
    end.addInstruction({vm::InstType::puts, std::nullopt});
    end.addInstruction({vm::InstType::exit, std::nullopt});
    
    std::string output;
    vm::VM v([&](std::string s){ output += s; });
    v.addModule(std::move(start));
    v.addModule(std::move(end));
    
    v.run("start");
    
    CHECK(output == "10.000000");
}

TEST_CASE("Hello world.")
{
    vm::Module m("main");
    m.addInstruction({vm::InstType::pi, "hello world!"});
    m.addInstruction({vm::InstType::pi, -5});
    m.addInstruction({vm::InstType::pi, -5});
    m.addInstruction({vm::InstType::jeq, 2});
    m.addInstruction({vm::InstType::div, std::nullopt});
    m.addInstruction({vm::InstType::puts, std::nullopt});
    m.addInstruction({vm::InstType::exit, std::nullopt});

    std::string output;
    vm::VM v([&](std::string s){ output += s; });
    v.addModule(std::move(m));
    v.run("main");
    
    CHECK(output == "hello world!");
}

TEST_CASE("Basic loop")
{
    std::string baseline = "";
    float i = 10;
    while ( i > 0)
    {
        --i;
        baseline += std::to_string(i);
    }

    //  pi 10     | 10
    //  copy      | 10 10
    //  pi 0      | 10 10 0
    //  jlt done  | 10
    //loop:
    //  pi -1     | 10 -1
    //  add       | 9
    //  copy      | 9 9
    //  puts      | 9
    //  copy      | 9 9
    //  pi 0      | 9 9 0
    //  jgt loop  | 9
    //done:
    //  exit

    vm::Module m("loop");
    m.addInstruction({vm::InstType::pi, 10});
    m.addInstruction({vm::InstType::copy, std::nullopt});
    m.addInstruction({vm::InstType::pi, 0});
    m.addInstruction({vm::InstType::jlt, "done"});
    m.addInstruction({vm::InstType::label, "loop"});
    m.addInstruction({vm::InstType::pi, -1});
    m.addInstruction({vm::InstType::add, std::nullopt});
    m.addInstruction({vm::InstType::copy, std::nullopt});
    m.addInstruction({vm::InstType::puts, std::nullopt});
    m.addInstruction({vm::InstType::copy, std::nullopt});
    m.addInstruction({vm::InstType::pi, 0});
    m.addInstruction({vm::InstType::jgt, "loop"});
    m.addInstruction({vm::InstType::label, "done"});
    m.addInstruction({vm::InstType::exit, std::nullopt});
    
    std::string output;
    vm::VM v([&](std::string s){ output += s; });
    v.addModule(std::move(m));
    v.run("loop");
    
    CHECK(baseline == output);
}

TEST_CASE("fib!")
{
    vm::Module main("main");
    vm::Module fib("fib");
    
    main.addInstruction(InstType::pi, 8);
    main.addInstruction(InstType::call, "fib");
    main.addInstruction(InstType::puts);
    main.addInstruction(InstType::exit);
    
    //  return n < 2 ? n : fib(n - 1) + fib(n - 2)
    //
    //  copy        | n n
    //  pi 2        | n n 2
    //  jlt done    | n
    //  copy        | n n
    //  sl 1        | n
    //  pi 1        | n 1
    //  sub         | n-1
    //  call fib    | fib(n-1)
    //  ll 1        | fib(n-1) n
    //  pi 2        | fib(n-1) n 2
    //  sub         | fib(n-1) n-2
    //  call fib    | fib(n-1) fib(n-2)
    //  add         | fib(n)
    //done:
    //  ret
    
    fib.addInstruction(InstType::copy);
    fib.addInstruction(InstType::pi, 2);
    fib.addInstruction(InstType::jlt, "done");
    fib.addInstruction(InstType::copy);
    fib.addInstruction(InstType::sl, 1);
    fib.addInstruction(InstType::pi, 1);
    fib.addInstruction(InstType::sub);
    fib.addInstruction(InstType::call, "fib");
    fib.addInstruction(InstType::ll, 1);
    fib.addInstruction(InstType::pi, 2);
    fib.addInstruction(InstType::sub);
    fib.addInstruction(InstType::call, "fib");
    fib.addInstruction(InstType::add);
    fib.addInstruction(InstType::label, "done");
    fib.addInstruction(InstType::ret);
    
    std::string output;
    vm::VM v([&](std::string s){ output += s; });
    
    v.addModule(std::move(main));
    v.addModule(std::move(fib));
    v.run("main");
    
    CHECK(output == "21.000000");
}
