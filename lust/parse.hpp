//
//  parser.hpp
//  lust
//
//  Created by Zeke Medley on 1/12/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#pragma once

#include <string>
#include <list>
#include <variant>
#include <utility>
#include <optional>

#include "expression.hpp"

namespace lust {

using ParseResult = std::optional<
                        std::pair<Expression, std::list<std::string>>>;

// Tokenizes input code. This is the equivalent of splitting on whitespace.
std::list<std::string> tokenize(std::string in);

// Parses a single expression from a list of tokens. Returns the expression and
// the remaining tokens, or std::nullopt on error.
ParseResult parse_expression(std::list<std::string> tokens);
ParseResult parse_seq(std::list<std::string> tokens);
std::optional<Expression> parse_atom(std::string token);

}
