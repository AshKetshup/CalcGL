#ifndef TEXT_H
#define TEXT_H

#pragma once
#include <shader_m.h>
// #include <glad/glad.h>
#include <glm/glm.hpp>
#include <map>

using namespace std;
using namespace glm;

struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    ivec2 Size;             // size of glyph
    ivec2 Bearing;          // offset from baseline to left/top of glyph
    unsigned int Advance;   // horizontal offset to advance to next glyph
};

class TextRenderer {
public:
    // Holds a list of pre-compiled Characters
    map<char, Character> Characters;

    // shader used for text rendering
    // Shader TextShader = Shader("shaders/font_vs.glsl", "shaders/font_fs.glsl");
    // Constructor
    TextRenderer();
    TextRenderer(unsigned int width, unsigned int height, Shader shader);
    
    // Pre-compiles a list of characters from the given font
    void Load(string font, unsigned int fontSize);
   
    // Renders a string of text using the precompiled list of characters
    void RenderText(string text, float x, float y, float scale, vec3 color = vec3(1.0f));

    void WriteText(ostringstream* text, float x, float y, float scale, vec3 color = vec3(1.0f));

    unsigned int getFontSize();
    Shader getShader();
    void setShader(Shader);
    string getFont();
    void setFont(string);

private:
    // render state
    unsigned int fontSize;
    unsigned int VAO, VBO;
    Shader TextShader;
    string TextFont;
};



#endif


