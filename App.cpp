// NOTE : Even tho I am enforcing to treat warnings as errors in cmake
// I am overriding some here just to be able to iterate fast to do this challenge
#pragma region CompilerSpecific
#if defined _WIN32
#pragma warning(disable: 4996 4267 4100 4189)
#endif
#pragma endregion

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>

#define CONSOLE_LOG(x, ...) printf(x, __VA_ARGS__)
#define RESOURCES_DIR std::string("resources/shaders/")
#define APP_NAME std::string("Heat Map Vis App")
#define APP_WINDOW_INITIAL_WIDTH 800
#define APP_WINDOW_INITIAL_HEIGHT 800

namespace System
{
    struct Memory
    {
        char * data = nullptr;
        size_t size = 0;
    };

    Memory readFileToMemory(const char * fileName)
    {
        Memory memory;
        FILE * file = fopen(fileName, "rb");
        if (file != nullptr)
        {
            fseek(file, 0, SEEK_END);
            long fileSize = ftell(file);
            fseek(file, 0, SEEK_SET);
            memory.size = fileSize;
            memory.data = new char[fileSize + 1];
            fread(memory.data, 1, fileSize, file);
            memory.data[fileSize] = '\0';
            fclose(file);
        }        
        return memory;
    }

    inline float convertToNormalizedDeviceCoordinates(float value)
    {
        return (2.0f * value - 1.0f);
    }

    namespace Windowing
    {
        void setWindowIcon(GLFWwindow * window, unsigned char * iconData = nullptr, int iconWidth = -1, int iconHeight = -1)
        {
            GLFWimage icon = { iconWidth, iconHeight, iconData };
            if (!iconData || iconWidth == -1 || iconHeight == -1)
            {
                icon.width = 1;
                icon.height = 1;
                icon.pixels = new unsigned char[4] { 0, 0, 0, 255 };
            }
            glfwSetWindowIcon(window, 1, &icon);
            if (!iconData || iconWidth == -1 || iconHeight == -1)
            {
                delete[] icon.pixels;
            }
        }        
    }
}

namespace Geometry
{
    struct Data4D
    {
        std::vector<glm::vec4> vertices;
        std::vector<unsigned int> indices;
    };

    struct Data2D
    {
        std::vector<glm::vec2> vertices;
        std::vector<unsigned int> indices;
    };

    Data2D createRectangle(float width, float height, glm::vec2 center)
    {
        Data2D result;
        result.vertices.reserve(4);
        result.indices.reserve(6);
        result.vertices.push_back(glm::vec2(-width * 0.5f, +height * 0.5f) + center);
        result.vertices.push_back(glm::vec2(+width * 0.5f, +height * 0.5f) + center);
        result.vertices.push_back(glm::vec2(+width * 0.5f, -height * 0.5f) + center);
        result.vertices.push_back(glm::vec2(-width * 0.5f, -height * 0.5f) + center);
        result.indices.push_back(0);
        result.indices.push_back(1);
        result.indices.push_back(2);
        result.indices.push_back(2);
        result.indices.push_back(3);
        result.indices.push_back(0);
        return result;
    }

    Data4D createRectangleWithTexture(float width, float height, glm::vec2 center)
    {
        Data4D result;
        result.vertices.reserve(4);
        result.indices.reserve(6);
        result.vertices.push_back(glm::vec4(glm::vec2(-width * 0.5f, +height * 0.5f) + center, glm::vec2(0.0f, 1.0f)));
        result.vertices.push_back(glm::vec4(glm::vec2(+width * 0.5f, +height * 0.5f) + center, glm::vec2(1.0f, 1.0f)));
        result.vertices.push_back(glm::vec4(glm::vec2(+width * 0.5f, -height * 0.5f) + center, glm::vec2(1.0f, 0.0f)));
        result.vertices.push_back(glm::vec4(glm::vec2(-width * 0.5f, -height * 0.5f) + center, glm::vec2(0.0f, 0.0f)));
        result.indices.push_back(0);
        result.indices.push_back(1);
        result.indices.push_back(2);
        result.indices.push_back(2);
        result.indices.push_back(3);
        result.indices.push_back(0);
        return result;
    }
}

namespace Math
{
    struct ScalarField2D
    {
        ScalarField2D(int width, int height) : width(width), height(height)
        {
            data.resize(width * height);
        }

        std::pair<float, float> computeInverseOfDimensions()
        {
            if (width < 1 || height < 1)
            {
                std::cerr << "Warning, width and height in ScalarField2D are negative\n";
                return std::pair(1.0f, 1.0f);
            }
            else
            {
                return std::make_pair(1.0f / float(width - 1), 1.0f / float(height - 1));
            }
        }

        std::vector<float> data;
        int width = -1;
        int height = -1;
        float min = 0.0f;
        float max = 1.0f;
    };

    ScalarField2D generateConstant2DScalarField(int width = 10, int height = 10, float value = 0.0f)
    {
        assert(width > 0 && height > 0);
        glm::clamp(value, 0.0f, 1.0f);
        ScalarField2D result(width, height);
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                result.data[i + (j * width)] = value + 0.5f;
            }
        }
        return result;
    }

    ScalarField2D generateLinearX2DScalarField(int width = 10, int height = 10)
    {
        assert(width > 0 && height > 0);
        ScalarField2D result(width, height);
        auto inverseDimensions = result.computeInverseOfDimensions();
        for (int i = 0; i < width; i++)
        {
            float x = i * inverseDimensions.first;
            for (int j = 0; j < height; j++)
            {
                result.data[i + (j * width)] = x;
            }
        }
        return result;
    }

    ScalarField2D generateLinearXY2DScalarField(int width = 10, int height = 10)
    {
        assert(width > 0 && height > 0);
        ScalarField2D result(width, height);
        auto inverseDimensions = result.computeInverseOfDimensions();
        for (int i = 0; i < width; i++)
        {
            float x = i * inverseDimensions.first;
            x = System::convertToNormalizedDeviceCoordinates(x);
            for (int j = 0; j < height; j++)
            {
                float y = j * inverseDimensions.second;
                y = System::convertToNormalizedDeviceCoordinates(y);
                result.data[i + (j * width)] = x + y;
            }
        }
        return result;
    }

    ScalarField2D generateQuadratic2DScalarField(int width = 10, int height = 10)
    {
        assert(width > 0 && height > 0);
        ScalarField2D result(width, height);
        auto inverseDimensions = result.computeInverseOfDimensions();
        for (int i = 0; i < width; i++)
        {
            float x = i * inverseDimensions.first;
            x = System::convertToNormalizedDeviceCoordinates(x);
            for (int j = 0; j < height; j++)
            {
                float y = j * inverseDimensions.second;
                y = System::convertToNormalizedDeviceCoordinates(y);
                result.data[i + (j * width)] = (x * x + y * y);
            }
        }
        return result;
    }


    ScalarField2D generatePythonReference2DScalarField(int width = 10, int height = 10)
    {
        float scalarFieldMaxValue = FLT_MIN;
        float scalarFieldMinValue = FLT_MAX;
        assert(width > 0 && height > 0);
        ScalarField2D result(width, height);
        auto inverseDimensions = result.computeInverseOfDimensions();
        for (int i = 0; i < width; i++)
        {
            float x = i * inverseDimensions.first;
            x = 3.0f * System::convertToNormalizedDeviceCoordinates(x);
            for (int j = 0; j < height; j++)
            {
                float y = j * inverseDimensions.second;
                y = 3.0f * System::convertToNormalizedDeviceCoordinates(y);
                float z = std::sinf(x * x + y * y);
                if (z > scalarFieldMaxValue)
                {
                    scalarFieldMaxValue = z;
                }
                if (z < scalarFieldMinValue)
                {
                    scalarFieldMinValue = z;
                }
                result.data[i + (j * width)] = z;
            }
        }
        CONSOLE_LOG("Min: %f\n", scalarFieldMinValue);
        CONSOLE_LOG("Max: %f\n", scalarFieldMaxValue);
        result.min = scalarFieldMinValue;
        result.max = scalarFieldMaxValue;
        return result;
    }
}

namespace Graphics
{
    struct BuffersHandles
    {
        unsigned int VAO;
        unsigned int VBO;
        unsigned int EBO;
    };

    BuffersHandles createBuffers(const Geometry::Data2D & data)
    {
        BuffersHandles result;
        glGenVertexArrays(1, &result.VAO);
        glGenBuffers(1, &result.VBO);
        glGenBuffers(1, &result.EBO);
        glBindVertexArray(result.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, result.VBO);
        glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(glm::vec2), data.vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
        glEnableVertexAttribArray(0);
        return result;
    }

    BuffersHandles createBuffers(const Geometry::Data4D & data)
    {
        BuffersHandles result;
        glGenVertexArrays(1, &result.VAO);
        glGenBuffers(1, &result.VBO);
        glGenBuffers(1, &result.EBO);
        glBindVertexArray(result.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, result.VBO);
        glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(glm::vec4), data.vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void *)0);
        glEnableVertexAttribArray(0);
        return result;
    }

    unsigned int createTexture(const Math::ScalarField2D & data)
    {
        unsigned int textureID = 0;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, data.width, data.height, 0, GL_RED, GL_FLOAT, &(data.data[0]));
        glBindTexture(GL_TEXTURE_2D, 0);
        return textureID;
    }

    int loadShader(const char * vertexShaderSource, const char * fragmentShaderSource)
    {
        int success = 0;
        char infoLog[512] = { 0 };
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            printf("Failed to compile vertex shader %s\n", infoLog);
            return -1;
        }
        CONSOLE_LOG("Vertex shader compiled\n");
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            printf("Failed to compile fragment shader %s\n", infoLog);
            return -1;
        }
        CONSOLE_LOG("Fragment shader compiled\n");
        unsigned int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            printf("Failed to link program %s\n", infoLog);
            return -1;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        CONSOLE_LOG("Shader program %d linked correctly\n", shaderProgram);
        return shaderProgram;
    }
}

int main(int, char **)
{
    std::cout << APP_NAME << "\n";
    if (!glfwInit())
    {
        std::cerr << "Error at glfwInit()\n";
        return -1;
    }

    // NOTE : For glad in the online generator https://gen.glad.sh/
    // I left OpenGL core 3.0 but on glfwWindowHints I have to do 3.3
    // If I remember correclty that is a mismatch from Khronos Group
    // when moving from fixed to programmable shaders, but just double
    // check all this is correct (or go straight to latest OpenGL - 4.6)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

    std::cout << "Initializing glfw window\n";
    GLFWwindow * window = glfwCreateWindow(
        APP_WINDOW_INITIAL_WIDTH, 
        APP_WINDOW_INITIAL_HEIGHT, 
        std::string(APP_NAME + " - Alejandro Guayaquil 2024").c_str(), 
        nullptr, 
        nullptr
    );
    if (!window)
    {
        std::cerr << "Error at glfwCreateWindow()\n";
        glfwTerminate();
        return -1;
    }
    System::Windowing::setWindowIcon(window);

    glfwMakeContextCurrent(window);

    std::cout << "Loading GL extensions\n";
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cerr << "Error at gladLoadGL()\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    std::cout << "Reading shaders\n";
    auto vertexShader = System::readFileToMemory((RESOURCES_DIR + "texture.vs").c_str());
    auto fragmentShader = System::readFileToMemory((RESOURCES_DIR + "texture.fs").c_str());
    auto shaderProgramID = Graphics::loadShader(vertexShader.data, fragmentShader.data);

    std::cout << "Creating quad geometry and loading it to the GPU\n";
    auto quadGeometry = Geometry::createRectangleWithTexture(2.0f, 2.0f, glm::vec2(0.0f));
    auto graphicsBuffers = Graphics::createBuffers(quadGeometry);

    std::cout << "Generating 2D scalar field and texture for it\n";
    auto scalarField2D = Math::generatePythonReference2DScalarField(100, 100);
    auto textureID = Graphics::createTexture(scalarField2D);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow * window, int width, int height)
    {
        CONSOLE_LOG("Resizing window %d %d\n", width, height);
        glViewport(0, 0, width, height);
    });

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUseProgram(shaderProgramID);
    glUniform1i(glGetUniformLocation(shaderProgramID, "uTextureSampler"), 0);
    glUniform1f(glGetUniformLocation(shaderProgramID, "uScalarFieldMax"), scalarField2D.max);
    glUniform1f(glGetUniformLocation(shaderProgramID, "uScalarFieldMin"), scalarField2D.min);
    glUseProgram(0);

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgramID);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glBindVertexArray(graphicsBuffers.VAO);
        glDrawElements(GL_TRIANGLES, quadGeometry.indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        glfwSwapBuffers(window);
    }

    std::cout << "Destroying glfw window and exiting program\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}