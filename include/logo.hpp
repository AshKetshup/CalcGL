#ifndef LOGO_H
#define LOGO_H

#include <string>
#include <exception>
#include "shader_m.h"

using namespace std;

class LogoException : public exception {
    public:
        LogoException(const string& msg) : message(msg) {}
        virtual const char* what() const throw ();
    private:
        const string message;
};

typedef struct {
    string path;
    int width, height;
    int nrChannels;
} ImgData;

class Logo {
    private:
        bool success;
        unsigned int VBO, VAO, EBO;
        ImgData data;
        unsigned int texture;
        Shader shader;
        void load(const char* path);
    public:
        Logo(void) {};
        Logo(const char* path, Shader s);
        string getLogoV(void);
        bool wasSuccessful(void);
        bool isAvailable(void);
        void render(unsigned int w, unsigned int h);
};

#endif