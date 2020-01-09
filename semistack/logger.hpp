//
//  Singleton logger class.
//
//  Created by Zeke Medley on 1/9/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#pragma once

#include <string>
#include <iostream>

#include "module.hpp";

namespace {

class Logger
{
private:
    std::string _history;
    
    void log(std::string what)
    {
        std::string tmp = what + "\n";
        _history += tmp;
        std::cout << tmp;
    }
    
public:
    void debug(std::string what)
    {
        std::cout << "[debug] ";
        log(std::move(what));
    }
    
    void error(std::string what)
    {
        std::cout << "[error] ";
        log(std::move(what));
    }
    
    void fatalError(std::string what)
    {
        std::cout << "[FATAL ERROR] ";
        log(std::move(what));
        exit(1);
    }
    
    std::string getHistory()
    {
        return _history;
    }
};

}

namespace vm {

Logger* logger()
{
    static Logger l;
    return &l;
}

}
