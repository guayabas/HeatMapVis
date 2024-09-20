cmake_minimum_required(VERSION 3.0..3.30)
project(
    NavierAIChallengeProblem
    VERSION 0.1.0
    LANGUAGES CXX C
)
add_subdirectory(thirdparty)
set(AppName HeatMapVis)
add_executable(${AppName} App.cpp thirdparty/glad/src/gl.c)
set_target_properties(${AppName} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    # COMPILE_WARNING_AS_ERROR ON
)
target_include_directories(${AppName} PRIVATE thirdparty/glad/include)
target_include_directories(${AppName} PRIVATE thirdparty/glfw/include/)
target_include_directories(${AppName} PRIVATE thirdparty/glm/)
target_link_directories(${AppName} PRIVATE build/thirdparty/glfw/src/Debug/)
target_link_libraries(${AppName} PRIVATE glfw3)

# NOTE : The glm target is not really a static link since glm is header only
# but it is still required to have the glm.hpp header available to the project
# Another way would be to use explicitly target_include_directories but for
# consistency I am leaving the glm directly in the target_link_libraries
# target_link_libraries(${AppName} PRIVATE glfw glm)

if (WIN32)
    target_link_libraries(${AppName} PRIVATE opengl32)
endif()

# NOTE : Uncomment if you want to be rigorous about the compiler
if (MSVC)
    # target_compile_options(${AppName} PRIVATE /W4)
else()
    # target_compile_options(${AppName} PRIVATE -Wall -Wextra -pedantic)
endif()

file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR}/Debug)
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR}/source)