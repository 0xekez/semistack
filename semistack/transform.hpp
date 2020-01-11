//
//  preprocessor.hpp
//  semistack
//
//  Created by Zeke Medley on 1/11/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#pragma once

#include "module.hpp"
#include <map>
#include <vector>

namespace vm {
namespace transform {

bool assembleModule(Module& m);
bool linkModules(std::vector<Module>& modules,
                 std::map<std::string, std::vector<Module>::size_type> table);

}

}
