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
#include <thread>
#include "sism.hpp"
#include <thread>

const unsigned int SCR_WIDTH   = 600;
const unsigned int SCR_HEIGHT  = 600;
const unsigned int THREAD_AMNT = thread::hardware_concurrency() / 2;
const char*        RMODE       = "CPU";

const char* HELP =
"CalcGL - An implicit function render with Ray Marching\n"
"\n"
"-W, --width   [int]\t\t\tsetting width pixels\n"
"-H  --height  [int]\t\t\tsetting height pixels\n"
"-r  --render  [CPU|GPU|SPHERE]\t\tsetting render mode pixels\n"
"-t  --threads [int]\t\t\tsetting height pixels\n"
"\n"
"-h, --help\t\t\t\tdisplays this help and exit\n";

using namespace calcgl;


/*
 * CalcGL.exe -h / --help  
 * CalcGL.exe -W [width] / --width [width]
 * CalcGL.exe -H [height] / --height [height]
 *  
 * CalcGL.exe -r [CPU | GPU | SPHERE] / --render [CPU | GPU | SPHERE]
 * CalcGL.exe -t [thread ammount] / --threads [thread ammount]
 *  
 */
int main(int argc, char const* argv[]) {
	unsigned int width = SCR_WIDTH;
	unsigned int height = SCR_HEIGHT;
	unsigned int threads = THREAD_AMNT;
	char const* rMode = RMODE;

	if (argc == 2)
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
			printf(HELP);

			return 0;
		}

	if (((argc - 1) % 2) == 0) {
		int amount = (argc - 1) / 2;
		
		for (size_t i = 0; i < amount; i++) {
			char const* tag = argv[1 + (i * 2)];
			char const* arg = argv[2 + (i * 2)];
			
			if (strcmp(tag, "-W") == 0 || strcmp(tag, "--width") == 0) {
				try {
					width = stoi(arg);
				} catch (...) {    }
			}
			
			if (strcmp(tag, "-H") == 0 || strcmp(tag, "--height") == 0) {
				try {
					height = stoi(arg);
				} catch (...) {    }
			}

			if (strcmp(tag, "-r") == 0 || strcmp(tag, "--render") == 0) {
				if (strcmp(arg, "CPU") != 0 && strcmp(arg, "GPU") != 0 && strcmp(arg, "SPHERE") != 0) {
					return -1;
				}

				rMode = arg;
			}

			if (strcmp(tag, "-t") == 0 || strcmp(tag, "--threads") == 0) {
				try {
					threads = stoi(arg);
					if (threads > THREAD_AMNT)
						return -1;
				} catch (...) {    }
			}
		}
	}

	CalcGL calcgl(width, height, rMode, threads);
	if (!calcgl.launchSuccessful())
		return -1;

	calcgl.main();
	
	return 0;
}
