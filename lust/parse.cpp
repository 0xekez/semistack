//
//  parser.cpp
//  lust
//
//  Created by Zeke Medley on 1/12/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#include <sstream>

#include "parse.hpp"
#include "logger.hpp"

using namespace lust;

std::list<std::string> lust::tokenize(std::string source)
{
    auto replaceAll = [](std::string& target,
                         std::string search, std::string replace)
    {
       // Get the first occurrence
       size_t pos = target.find(search);
    
       // Repeat till end is reached
       while( pos != std::string::npos)
       {
           // Replace this occurrence of Sub String
           target.replace(pos, search.size(), replace);
           // Get the next occurrence from the current position
           pos = target.find(search, pos + replace.size());
       }
    };
    
    replaceAll(source, "(", "( ");
    replaceAll(source, ")", " )");
    
    std::istringstream iss(source);
    std::list<std::string> result {
        std::istream_iterator<std::string>(iss), {}
    };
    return result;
}

ParseResult lust::parse_expression(std::list<std::string> tokens)
{
    vm::logger()->debug("parse expression");
    std::string front = tokens.front();
    tokens.pop_front();
    
    if (front == "(")
    {
        return parse_seq(std::move(tokens));
    } else if (front == ")")
    {
        vm::logger()->error("Unexpected closing ')'");
        return std::nullopt;
    } else
    {
        auto atomq = parse_atom(std::move(front));
        if ( atomq.has_value() )
            return std::make_pair(std::move(atomq.value()), std::move(tokens));
        return std::nullopt;
    }
}

ParseResult lust::parse_seq(std::list<std::string> tokens)
{
    vm::logger()->debug("parse seq");
    std::list<lust::Expression> sequence;
    
    while (true)
    {
        if ( ! tokens.size() )
        {
            vm::logger()->error("Unexpected end of tokens.");
            return std::nullopt;
        }
        
        std::string front = tokens.front();
        
        if (front == ")")
        {
            tokens.pop_front();
            Expression result =
                std::make_unique<List>(std::move(sequence));
            return std::make_pair(std::move(result), std::move(tokens));
        }
        
        ParseResult pr = parse_expression(std::move(tokens));
        if ( ! pr.has_value() ) return std::nullopt;
        
        Expression expr(std::move(pr.value().first));
        tokens = pr.value().second;
        sequence.push_back(std::move(expr));
    }
}

std::optional<Expression> lust::parse_atom(std::string token)
{
    vm::logger()->debug("parse atom.");
    try {
        float value = std::stof(token.c_str());
        return std::make_unique<Number>(value);
    } catch (const std::invalid_argument& ia) {
        return std::make_unique<Symbol>(std::move(token));
    }
}
