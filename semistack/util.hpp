//
//  util.hpp
//  semistack
//
//  Created by Zeke Medley on 1/9/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#pragma once

#include <variant>

namespace vm {
namespace util {

// From: http://en.cppreference.com/w/cpp/utility/variant/visit
// For using lambdas with std::visit. For example:
// std::visit(overloaded {
//              [](std::string f) { ... },
//              [](float f) { ... }
//              }, v);
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

}
}
