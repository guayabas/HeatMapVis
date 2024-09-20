## Document Scope

The following are the steps I used to solve the *Graphics Challenge Problem* where its instructions are [here](https://colab.research.google.com/drive/18xE6icoAdhLrMLZggKaYMHB3tBMqt1Y_?usp=sharing#scrollTo=MADwnenA2nwV) (The hyperlink might not work anymore so the TLDR of the problem is to <u>create a C++ program using OpenGL that visualizes a 2D scalar field as a heatmap</u>).

### Development Environment
* Windows 11 OS : What I have as a personal computer
* Visual Studio 2022 Community : Provides a C++ compiler on Windows.
* Visual Studio Code : For developing (code editor, file system, terminal, etc.)
* CMake 3.0+ : Common way of doing OS-independent project.
* GLFW : Handles OS windowing system.
* GLAD : Enables OpenGL extensions (required for things like shaders).
* GLM : To have performant graphics math that is common on C++ and GLSL.

### How To Run The App
```
git clone https://github.com/guayabas/HeatMapVis.git heatmapvis
cd heatmapvis
mkdir build

# Window - PowerShell
cmake -S . -B .\build\ -Wno-dev ; cmake --build .\build\ ; .\build\Debug\HeatMapVis.exe

# Unix - Shell
cmake -S . -B .\build\ -Wno-dev && cmake --build .\build\ && .\build\Debug\HeatMapVis
```

### What Is A 2D Scalar Field?
<details><summary>Show Content</summary>

If you are not familiar with calculus or physics, then you can think of a 2D scalar field as the values that the basic game Tic-tac-toe (The one that one puts a circle or a cross), i.e. a 2D scalar field is simply the values that take a cell in a grid.

Not relevant for this project but scalar fields are important because represent relevant information for physics like temperature or pressure. You can read more about it in the Wikipedia https://en.wikipedia.org/wiki/Scalar_field or ask some AI like ChatGPT.

What is important for the taks at hand is to do a program that is able to display (and to some extent generate) images like this one
<p align="center"><img src="./resources/images/scalarfield2d_python_reference.png"></p>

</details>

### 1. Generate a 2D Scalar Field
<details><summary>Show Content</summary>

This can be done easily by just creating a 2D grid and then assigning a value at each grid point.
```Cpp
namespace Math
{
    struct ScalarField2D
    {
        ScalarField2D(int width, int height) : width(width), height(height)
        {
            data.resize(width * height);
        }
        std::vector<float> data;
        int width = -1;
        int height = -1;
    };

    ScalarField2D generate_XXX_2DScalarField(int width = 10, int height = 10)
    {
        assert(width > 0 && height > 0);
        ScalarField2D result(width, height);
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                result.data[i + (j * width)] = f(i, j);
            }
        }
        return result;
    }    
}
```

The important aspects about above code is the line 
```Cpp
result.data[i + (j * width)] = f(i, j)
```

because it does something named *linear indexing* which converts a 2D index into a 1D index. Nothing stops for doing something like `result.data[i][j]` (which will require a `std::vector<std::vector<float>>` as data container) but part of doing efficient programming is to be aware that having compact data in memory is _mostly_ always better than having it sparse. Notice that yes, we could have done `j + (i * height)` for the linear indexing (and yes, will have some consequences in memory alignment) but that is left as possible exercise for you the reader.

The other relevant aspect is the function `f(i, j)` which will represent the <u>2D scalar field</u> that we are interested to display. In the Python reference this line of code is equivalent to
```Python
z = np.sin(x**2 + y**2)
```

Obviously, `f(i, j)` is not actual code, so let me show you how it will look for a <u>constant scalar field</u>
```Cpp
ScalarField2D generateConstant2DScalarField(int width = 10, int height = 10, float value = 0.0f)
{
    assert(width > 0 && height > 0);
    ScalarField2D result(width, height);
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            result.data[i + (j * width)] = value;
        }
    }
    return result;
}
```
</details>

### 2. Color Map The Scalar Field
<details><summary>Show Content</summary>

To do this we simply need to do 

* Define a color scheme (for example, as suggested use two colors)
* Assign color to grid points based on the color scheme and *interpolate* values in between grid points.


Why is the interpolation needed? Well, the monitor (or any other device to display some graphics) will be composed of pixels and nothing tells you that one pixel is map exactly to one grid node. We can do all this in the CPU side but is better to use the GPU capabilities for it.

For this, we have to introduce two things
1. Passing data from the CPU to GPU
2. Shaders (specifically the fragment shader)

#### Passing Data From CPU To GPU
What we want is basically to move the `std::vector<float> data` from the `ScalarField2D` data structure into the GPU. To do so one can use the concept of *textures* in the graphics world. These are not more than buffers of GPU memory that can have different dimensions (commonly 1D, 2D, or 3D) and data types (such as int, float, or even 3|4 components -commonly used as RGB|A channels-).

In OpeGL this can be done like

```Cpp
namespace Graphics
{
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
}
```

If you are not familiar with OpenGL, take a look [here](https://learnopengl.com/). And the important aspect of above code is the line 
```Cpp
glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, data.width, data.height, 0, GL_RED, GL_FLOAT, &(data.data[0]));
```
since that would be the one that defines the mapping from CPU data into GPU data. A note is that the actual transfer of the bytes does not necessarily (depends on how OpenGL is setup as well the implementation of the graphics driver) happens at that line of the code.

#### Shaders
Now the data is at the GPU but then how can we manipulate it? This is where the *shaders* come in handy. As a simple explanation, shaders are *small* pieces of code (written in a shading language, for example GLSL for OpenGL) that do things on the GPU in parallel. We don't need to go into details how a graphics pipeline works (in you want to go into that rabbit hole take a look [here](https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview)) but only to mention that the *two important* shaders are the <u>vertex</u> and <u>fragment</u> shaders. In few words, the vertex one processes vertices (geometry) while the fragment one processes pixels (colors).

For the purpose of the task, here is the fragment shader that does the *magic* of coloring the 2D scalar field
```GLSL
#version 330 core
in vec2 vTextureCoordinates;
uniform sampler2D uTextureSampler;
out vec4 fragColor;
vec3 getColorForValue(float value) 
{
    vec3 coldColor = vec3(0.5, 0.5, 0.5);
    vec3 hotColor = vec3(1.0, 1.0, 1.0);
    return mix(coldColor, hotColor, value);
}
void main()
{
    float colorFromTexture = texture(uTextureSampler, vTextureCoordinates).r;
    fragColor = vec4(getColorForValue(colorFromTexture), 1.0);
}
```
Notice that the color scheme is at the line
```GLSL
mix(coldColor, hotColor, value)
```
which does a linear interpolation between those two colors provided a value.

That is great you think, but what are the `vTextureCoordinates`? Those are simply a *coordinate space* that ranges between 0 and 1 (commonly known as UV space). You are familiar with a coordinate space since elementary school which is the real coordinate space that say things like *a point is at the position (10, 25)*. To understand a little more about UV space just imagine a square, you can define it via its geometrical points (a = [-1, -1], b = [-1, 1],  c = [1, 1], d = [1, -1]) but also via its texture points (uv_a = [0, 0], uv_b = [1, 0], uv_c = [1, 1], uv_d = [0, 1])
</details>

### 3.Render The Heatmap Using OpenGL.

### 4. Optional Tasks

#### Loading 2D Scalar Field From External File

For the following task does not have code (yet, limited time to finish this challenge as tech interview) but it has ideas on how to implement them given this framework

#### Adding Camera

### Solution

Here are some images comparison between the ones created with the Python code provided as reference (right image) and the ones created with the solution of this repository (left image)

<details><summary>Constant : f(x, y) = 0</summary>
Grid Size : (10, 10)
<p align="center">
    <img src="./resources/images/scalarfield2d_solution_constant.png" style="width: 45%;">
    <img src="./resources/images/scalarfield2d_python_reference_constant.png" style="width: 45%;">
</p>
</details>

<details><summary>Linear X : f(x, y) = x</summary>
Grid Size : (10, 10)
<p align="center">
    <img src="./resources/images/scalarfield2d_solution_linearx_small.png" style="width: 45%;">
    <img src="./resources/images/scalarfield2d_python_reference_linearx_small.png" style="width: 45%;">
</p>
Grid Size : (100, 100)
<p align="center">
    <img src="./resources/images/scalarfield2d_solution_linearx_large.png" style="width: 45%;">
    <img src="./resources/images/scalarfield2d_python_reference_linearx_large.png" style="width: 45%;">
</p>
</details>

<details><summary>Linear XY : f(x, y) = (x + y)</summary>
Grid Size : (10, 10)
<p align="center">
    <img src="./resources/images/scalarfield2d_solution_linearxy_small.png" style="width: 45%;">
    <img src="./resources/images/scalarfield2d_python_reference_linearxy_small.png" style="width: 45%;">
</p>
Grid Size : (100, 100)
<p align="center">
    <img src="./resources/images/scalarfield2d_solution_linearxy_large.png" style="width: 45%;">
    <img src="./resources/images/scalarfield2d_python_reference_linearxy_large.png" style="width: 45%;">
</p>
</details>

<details><summary>Quadratic : f(x, y) = (x * x + y * y)</summary>
Grid Size : (10, 10)
<p align="center">
    <img src="./resources/images/scalarfield2d_solution_quadratic_small.png" style="width: 45%;">
    <img src="./resources/images/scalarfield2d_python_reference_quadratic_small.png" style="width: 45%;">
</p>
Grid Size : (100, 100)
<p align="center">
    <img src="./resources/images/scalarfield2d_solution_quadratic_large.png" style="width: 45%;">
    <img src="./resources/images/scalarfield2d_python_reference_quadratic_large.png" style="width: 45%;">
</p>
</details>

<details open><summary>Provided Example : f(x, y) = sin(x * x + y * y)</summary>
Grid Size : (10, 10)
<p align="center">
    <img src="./resources/images/scalarfield2d_solution_reference_small.png" style="width: 45%;">
    <img src="./resources/images/scalarfield2d_python_reference_small.png" style="width: 45%;">
</p>
Grid Size : (100, 100)
<p align="center">
    <img src="./resources/images/scalarfield2d_solution_reference_large.png" style="width: 45%;">
    <img src="./resources/images/scalarfield2d_python_reference_large.png" style="width: 45%;">
</p>
</details>

<p align="center">
    <video width="800" height="600" controls>
        <source src="./resources/videos/solution.mp4" type="video/mp4">
        Your browser does not support the video tag.
    </video>
</p>
