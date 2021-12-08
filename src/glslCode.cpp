#include "glslcode.hpp"

#include <filesystem>
#include <fstream>
 
using namespace std;
namespace fs = filesystem;

string nameOfShader(int code) {
    if (code & (SURFACE | VERTEX))    return SURF_VS;
    if (code & (SURFACE | FRAGMENT))  return SURF_FS;
    if (code & (FONT | VERTEX))       return FONT_VS;
    if (code & (FONT | FRAGMENT))     return FONT_FS;
    if (code & (LOGO | VERTEX))       return LOGO_VS;
    if (code & (LOGO | FRAGMENT))     return LOGO_FS;
}

bool shaderExists(string path) {
    return fs::exists(path);
}

bool createShader(string path, int code) {
    string fullpath = path + "/" + nameOfShader(code);
    if (!shaderExists(fullpath)) {
        ofstream shader(fullpath, ofstream::out);
        switch (code) {
        case SURFACE | VERTEX:
            shader << surface_vertex_shader;
            break;

        case SURFACE | FRAGMENT:
            shader << surface_fragment_shader;
            break;

        case FONT | VERTEX:
            shader << font_vertex_shader;
            break;

        case FONT | FRAGMENT:
            shader << font_fragment_shader;
            break;

        case LOGO | VERTEX:
            shader << logo_vertex_shader;
            break;

        case LOGO | FRAGMENT:
            shader << logo_fragment_shader;
            break;

        default:
            shader.close();
            return false;
        }
        shader.close();
    }
    return true;
}

int autoCorrectShaders(string path) {
    int c = 0;      // Number of corrected shaders
    c += (int) !createShader(path, SURFACE | VERTEX);
    c += (int) !createShader(path, SURFACE | FRAGMENT);
    c += (int) !createShader(path, FONT | VERTEX);
    c += (int) !createShader(path, FONT | FRAGMENT);
    c += (int) !createShader(path, LOGO | VERTEX);
    c += (int) !createShader(path, LOGO | FRAGMENT);
    return c;
}