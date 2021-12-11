//------------------------------------------------------------------------------
//
// CalcGL - Implicit Function Renderer
//
// DESCRIPTION:
// -----------
//		Implicit Function Renderer
// 
// AUTHORS:
// -------
//      Diogo Simões (https://github.com/AshKetshup)
// 
// LICENSE:
// -------
//      GNU GPL V3.0
//------------------------------------------------------------------------------

#include <iostream>
#include "sism.hpp"

const unsigned int SCR_WIDTH  = 600;
const unsigned int SCR_HEIGHT = 600;

using namespace calcgl;

int main(int argc, char const* argv[]) {
	unsigned int width;
	unsigned int height;

	//try {
	//	width = stoi(argv[1]);
	//	height = stoi(argv[2]);
	//} catch (...) {
	width = SCR_WIDTH;
	height = SCR_HEIGHT;
	//}

	CalcGL calcgl(width, height);
	if (!calcgl.launchSuccessful())
		return -1;

	calcgl.main();
	
	return 0;
}
