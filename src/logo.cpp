#include "logo.hpp"
#include "debug.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace glm;

const char* LogoException::what(void) const throw() {
	return message.c_str();
}


string Logo::getLogoV(void) {
    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<mt19937::result_type> distribution(1, 100);
    int x = distribution(rng);

    if (x <= 5)
        return "2";
    else if (x >= 95)
        return "3";
    else
        return "1";
}


void Logo::load(const char* path) {
    float vertices[] = {
        // positions         // colors           // texture coords
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
       -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
       -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
    };

    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    glGenVertexArrays(1, &(this->VAO));
    glGenBuffers(1, &(this->VBO));
    glGenBuffers(1, &(this->EBO));

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glGenTextures(1, &(this->texture));
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char* data = stbi_load(path, &(this->data.width), &(this->data.height), &(this->data.nrChannels), 0);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->data.width, this->data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); // TODO: FIND BUG
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        stbi_image_free(data);
        throw LogoException("Failed to load logo");
    }

    stbi_image_free(data);
}

bool Logo::wasSuccessful(void) {
    return success;
}

bool Logo::isAvailable(void) {
    return success;
}

void Logo::render(unsigned int width, unsigned int height) {
    float size = static_cast<float>((width < height) ? width : height);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture);
    
    this->shader.use();
    this->shader.setInt("ourTexture", 0);
    this->shader.setMat4("projection", glm::ortho(-1.0f, 1.0f, static_cast<float>(height) / static_cast<float>(width), -static_cast<float>(height) / static_cast<float>(width)));
    
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glUseProgram(0);
}

Logo::Logo(const char* path, Shader s) {
    debugs("\n%s\n", path);
    this->success = false;
    this->data.path = string(path);
    this->shader = s;
    try {
        load(path);
    } catch (const LogoException& e) {
        debugs("%s\n", e.what());
        return;
    }

    this->success = true;
}
