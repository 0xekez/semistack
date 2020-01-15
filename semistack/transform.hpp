//
//  preprocessor.hpp
//  semistack
//
//  Created by Zeke Medley on 1/11/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#pragma once

#include "function.hpp"
#include <map>
#include <vector>

namespace vm {
namespace transform {

bool assembleFunction(Function& fn);
bool linkFunctions(std::vector<Function>& modules,
         const std::map<std::string, std::vector<Function>::size_type>& table);

}

}
