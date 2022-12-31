#include<iostream>
#include<string>
#include<fstream>
#include<sstream>

#include<glad/glad.h>
#include<GLFW/glfw3.h>

struct ShaderProgramSource {
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath) {
    std::fstream stream(filepath);

    enum class ShaderType {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;

    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = ShaderType::VERTEX;
            } else if (line.find("fragment") != std::string::npos) {
                type = ShaderType::FRAGMENT;
            }
        } else {
            ss[(int)type] << line << '\n';
        }
    }

    return {
        ss[0].str(),
        ss[1].str()
    };
}

static unsigned int CompileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = &source[0]; //source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*) alloca(length * sizeof(char)); // Allocates dynamically on the stack instead on the heap.

        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile shader" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;

        glDeleteShader(id);
        return 0;
    }

    return id;
}

static int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();

    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glValidateProgram(program);
    
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}


int main()
{
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(800, 800, "Graphics Programming", nullptr, nullptr); if (window == nullptr) return -1;

    glfwMakeContextCurrent(window); // Alle OpenGL-kommandoer som kalles i denne threaden går til window

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // Denne definerer forløpig bare koordinater noder, offset 0 for første attributt, offset 8 for rgb for eksempel
    float positions[] = { 
        0.0f, 0.0f,
        0.5f, 0.0f,
        0.5f, 0.5f,
        0.0f, 0.5f
    };

    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };
     
    // Vertex buffer
    unsigned int buffer; //Hvis første parameter er større enn 1, så må unsigned int buffer være et array. 
    glGenBuffers(1, &buffer); // buffer hvis buffer er et array
    glBindBuffer(GL_ARRAY_BUFFER, buffer); // buffer[i] hvis buffer er et array
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Parameteren her referer til den første parameteren over
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0); // Spesifiserer hvordan attributtene

    // Index-buffer
    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);


    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    std::cout << "VERTEX" << std::endl;
    std::cout << source.VertexSource << std::endl;
    std::cout << "FRAGMENT" << std::endl;
    std::cout << source.FragmentSource << std::endl;

    unsigned int shaderProgram = CreateShader(source.VertexSource, source.FragmentSource);
    glUseProgram(shaderProgram);


    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        //glDrawArrays(GL_TRIANGLES, 0, 3); // draws the back buffer that is prepared to be swapped with the front buffer
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window); // swap front and back buffers, changing frames.
        glfwPollEvents();  // Poll for and process events
    }

    glDeleteProgram(shaderProgram);
    //glfwDestroyWindow(window); // Frees the resources associated with the windows
    glfwTerminate(); // Frees up the resources allocated by glfw

    return 0;
}