#ifndef FUNCREAD_H
#define FUNCREAD_H

// CParse
//#include "shunting-yard.h"
//#include "builtin-features.inc"

// Others
#include "camera.hpp"
#include <glm/glm.hpp>
#include <iostream>

using namespace std;
#include <string>

namespace reader {
	bool isFunctionFile(const string fileName);
	vector<string> readFunctionFile(const string fileName);
}

#endif